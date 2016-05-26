#include "imagewindow.h"
#include "animationform.h"
#include "defines.h"
#include "gridlabel.h"
#include "mainwindow.h"
#include "util.h"
#include <QGraphicsView>
#include <QPixmap>

ImageWindow::ImageWindow(CSettings *p, EditData *pEditImage, AnimationForm *pAnimForm, MainWindow *pMainWindow, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImageWindow)
{
    ui->setupUi(this);

    m_pSetting = p;
    m_pEditData = pEditImage;
    setAnimationForm(pAnimForm);
    m_pMainWindow = pMainWindow;
    m_oldWinSize = QSize(-1, -1);

    setAcceptDrops(true);

    ui->checkBox->setChecked(true);

    m_pActDelImage = new QAction(trUtf8("Delete"), this);

    connect(m_pActDelImage, SIGNAL(triggered()), this, SLOT(slot_delImage()));
    connect(this, SIGNAL(sig_addImage(int)), m_pAnimationForm, SLOT(slot_addImage(int)));
    connect(this, SIGNAL(sig_delImage(int)), m_pAnimationForm, SLOT(slot_delImage(int)));
    connect(ui->doubleSpinBox_uv_bottom, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUVBottom(double)));
    connect(ui->doubleSpinBox_uv_top, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUVTop(double)));
    connect(ui->doubleSpinBox_uv_left, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUVLeft(double)));
    connect(ui->doubleSpinBox_uv_right, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUVRight(double)));
    //	connect(ui->pushButton_scale, SIGNAL(clicked()), this, SLOT(slot_clickedScaleButton())) ;

    ui->tabWidget->clear();
    for (int i = 0; i < m_pEditData->getImageDataListSize(); i++)
    {
        EditData::ImageData *p = m_pEditData->getImageData(i);
        if (!p)
        {
            continue;
        }
        addTab(p->nNo);
    }

    setWindowTitle(tr("Image Window"));
}

ImageWindow::~ImageWindow()
{
    delete ui;
}

// ドラッグ進入イベント
void ImageWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

// ドロップイベント
void ImageWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    int index = 0;

    for (int i = 0; i < urls.size(); i++)
    {
        QString fileName = urls[i].toLocalFile();

        index = getFreeTabIndex();

        EditData::ImageData data;
        QImage image;
        if (!image.load(fileName))
        {
            QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName));
            continue;
        }
        data.origImageW = image.width();
        data.origImageH = image.height();
        util::resizeImage(image);

        data.fileName = fileName;
        data.Image = image;
        data.lastModified = QDateTime::currentDateTimeUtc();
        data.nTexObj = 0;
        data.nNo = index;
        m_pEditData->addImageData(data);
        addTab(index);

        emit sig_addImage(index);

        index++;
    }
}

// タブ追加
void ImageWindow::addTab(int imageIndex)
{
    EditData::ImageData *p = m_pEditData->getImageDataFromNo(imageIndex);
    if (!p)
    {
        qDebug("not found ImageData[%d]", imageIndex);
        return;
    }

    QLabel *pLabel = new QLabel(ui->tabWidget);
    pLabel->setPixmap(QPixmap::fromImage(p->Image));
    pLabel->setObjectName("ImageLabel");
    pLabel->setScaledContents(true);
    pLabel->setAutoFillBackground(true);
    QPalette palette = pLabel->palette();
    palette.setColor(QPalette::Background, m_pSetting->getImageBGColor());
    pLabel->setPalette(palette);

    GridLabel *pGridLabel = new GridLabel(m_pEditData, imageIndex, pLabel);
    pGridLabel->setObjectName("CGridLabel");
    pGridLabel->show();
    pGridLabel->setDrawCenter(m_pSetting->getDrawCenter());

    QScrollArea *pScrollArea = new QScrollArea(ui->tabWidget);
    pScrollArea->setWidget(pLabel);

    //	ui->tabWidget->addTab(pScrollArea, tr("%1").arg(imageIndex)) ;
    ui->tabWidget->insertTab(imageIndex, pScrollArea, QIcon(), tr("%1").arg(imageIndex));

    connect(ui->checkBox, SIGNAL(clicked(bool)), pGridLabel, SLOT(slot_gridOnOff(bool)));
    connect(pGridLabel, SIGNAL(sig_changeSelectLayerUV(RectF)), m_pAnimationForm, SLOT(slot_changeSelectLayerUV(RectF)));
    connect(pGridLabel, SIGNAL(sig_changeCatchRect(RectF)), this, SLOT(slot_setUI(RectF)));
    connect(m_pAnimationForm, SIGNAL(sig_imageRepaint()), pGridLabel, SLOT(update()));
    connect(m_pAnimationForm, SIGNAL(sig_imageChangeRect(RectF)), this, SLOT(slot_setUI(RectF)));
    connect(m_pAnimationForm, SIGNAL(sig_imageChangeTab(int)), this, SLOT(slot_changeTab(int)));
}

// メニュー
void ImageWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(m_pActDelImage);
    menu.exec(event->globalPos());
}

// グリッドアップデート
void ImageWindow::updateGridLabel(void)
{
    QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(ui->tabWidget->currentIndex());

    QLabel *label = pScrollArea->findChild<QLabel *>("ImageLabel");
    if (label)
    {
        label->update();
    }
}

// リサイズイベント
void ImageWindow::resizeEvent(QResizeEvent *event)
{
    if (m_oldWinSize.width() < 0 || m_oldWinSize.height() < 0)
    {
        m_oldWinSize = event->size();
        return;
    }

    QSize add = event->size() - m_oldWinSize;
    //	QSize add_h = QSize(0, add.height()) ;
    //	QSize add_w = QSize(add.width(), 0) ;

    m_oldWinSize = event->size();

    ui->tabWidget->resize(ui->tabWidget->size() + add);

    QWidget *pTmp[] = {
        //		ui->groupBox_scale,
        ui->groupBox_uv};
    for (int i = 0; i < ARRAY_NUM(pTmp); i++)
    {
        pTmp[i]->move(pTmp[i]->pos() + QPoint(add.width(), 0));
    }
}

// 未使用タブ取得
int ImageWindow::getFreeTabIndex(void)
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        int idx = ui->tabWidget->tabText(i).toInt();
        if (idx != i)
        {
            return i;
        }
    }
    return ui->tabWidget->count();
}

// 画像削除
void ImageWindow::slot_delImage(void)
{
    int index = ui->tabWidget->currentIndex();
    int no = ui->tabWidget->tabText(index).toInt();

    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (pModel->isUseImage(no))
    {
        int ret = QMessageBox::question(this, trUtf8("警告"), trUtf8("この画像は使用されています。削除しますか？"), trUtf8("削除"), trUtf8("キャンセル"));
        qDebug() << "ret" << ret;
        if (ret == 1)
        {
            return;
        }
    }

    ui->tabWidget->removeTab(index);

    emit sig_delImage(no);
}

// 画像更新
void ImageWindow::slot_modifiedImage(int index)
{
    QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(index);

    QLabel *label = pScrollArea->findChild<QLabel *>("ImageLabel");
    if (!label)
    {
        qDebug() << "ERROR:ImageLabel not found!!!!!";
        return;
    }
    EditData::ImageData *p = m_pEditData->getImageData(index);
    if (!p)
    {
        qDebug() << "ERROR:ImageData not found!!!!!";
        return;
    }

    label->setPixmap(QPixmap::fromImage(p->Image));
    QPalette palette = label->palette();
    palette.setColor(QPalette::Background, m_pSetting->getImageBGColor());
    label->setPalette(palette);
    label->update();
}

// UV 下 変更
void ImageWindow::slot_changeUVBottom(double val)
{
    RectF r = m_pEditData->getCatchRect();
    r.setBottom(val);
    m_pEditData->setCatchRect(r);

    updateGridLabel();
}

// UV 上 変更
void ImageWindow::slot_changeUVTop(double val)
{
    RectF r = m_pEditData->getCatchRect();
    r.setTop(val);
    m_pEditData->setCatchRect(r);

    updateGridLabel();
}

// UV 左 変更
void ImageWindow::slot_changeUVLeft(double val)
{
    RectF r = m_pEditData->getCatchRect();
    r.setLeft(val);
    m_pEditData->setCatchRect(r);

    updateGridLabel();
}

// UV 右 変更
void ImageWindow::slot_changeUVRight(double val)
{
    RectF r = m_pEditData->getCatchRect();
    r.setRight(val);
    m_pEditData->setCatchRect(r);

    updateGridLabel();
}

// UI セット
void ImageWindow::slot_setUI(RectF rect)
{
    if (rect.bottom() != ui->doubleSpinBox_uv_bottom->value())
    {
        ui->doubleSpinBox_uv_bottom->setValue(rect.bottom());
    }
    if (rect.top() != ui->doubleSpinBox_uv_top->value())
    {
        ui->doubleSpinBox_uv_top->setValue(rect.top());
    }
    if (rect.left() != ui->doubleSpinBox_uv_left->value())
    {
        ui->doubleSpinBox_uv_left->setValue(rect.left());
    }
    if (rect.right() != ui->doubleSpinBox_uv_right->value())
    {
        ui->doubleSpinBox_uv_right->setValue(rect.right());
    }
}

// オプション終了時
void ImageWindow::slot_endedOption(void)
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        slot_modifiedImage(i);
    }
}

// センター表示変更
void ImageWindow::slot_changeDrawCenter(bool flag)
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(i);

        GridLabel *pGrid = pScrollArea->findChild<GridLabel *>("CGridLabel");
        if (!pGrid)
        {
            qDebug() << "pGrid not found";
            continue;
        }
        qDebug("slot_changeDrawCenter[%d]:%d", i, flag);
        pGrid->setDrawCenter(flag);
    }
}

// フレームデータドラッグ中
void ImageWindow::slot_dragedImage(FrameData /*data*/)
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(i);

        GridLabel *pGrid = pScrollArea->findChild<GridLabel *>("CGridLabel");
        if (!pGrid)
        {
            qDebug() << "pGrid not found";
            continue;
        }
        pGrid->update();
    }
}

// タブ変更
void ImageWindow::slot_changeTab(int nImage)
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        if (nImage == ui->tabWidget->tabText(i).toInt())
        {
            ui->tabWidget->setCurrentIndex(i);
            break;
        }
    }
}

// スケールボタン押した
void ImageWindow::slot_clickedScaleButton()
{
#if 0
	int no = ui->tabWidget->tabText(ui->tabWidget->currentIndex()).toInt() ;
	double scale = ui->doubleSpinBox_scale->value() ;
	int ret = QMessageBox::question(this, trUtf8("確認"), trUtf8("%1番画像を使用しているフレームデータのUVを%2倍しますか？").arg(no).arg(scale), QMessageBox::Ok, QMessageBox::Cancel) ;
	if ( ret == QMessageBox::Ok ) {
		m_pEditData->cmd_changeUvScale(no, scale) ;
	}
#endif
}
