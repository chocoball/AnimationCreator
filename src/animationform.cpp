#include "animationform.h"
#include "defines.h"
#include "mainwindow.h"
#include "ui_animationform.h"
#include <QStandardItemModel>

AnimationForm::AnimationForm(CEditData *pImageData, CSettings *pSetting, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AnimationForm)
{
    m_pEditData = pImageData;
    m_pSetting = pSetting;
    m_bDontSetData = false;
    m_frameStart = pSetting->getFrameStart();
    m_frameEnd = pSetting->getFrameEnd();

    m_oldWinSize = QSize(-1, -1);

    ui->setupUi(this);

    m_pEditData->setTreeView(ui->treeView);

    ui->label_frame->setEditData(m_pEditData);
    ui->label_frame->setHorizontalBar(ui->horizontalScrollBar_frame);
    m_pDataMarker = ui->label_frame;

    //	setFocusPolicy(Qt::StrongFocus);

    m_pGlWidget = new AnimeGLWidget(pImageData, pSetting, this);
    ui->scrollArea_anime->setWidget(m_pGlWidget);
    m_pGlWidget->resize(m_pSetting->getAnmWindowW(), m_pSetting->getAnmWindowH());
    m_pGlWidget->setDrawArea(m_pSetting->getAnmWindowW(), m_pSetting->getAnmWindowH());
    m_pGlWidget->show();

    ui->radioButton_pos->setChecked(true);
    ui->checkBox_grid->setChecked(m_pSetting->getCheckGrid());
    ui->spinBox_fps->setValue(60);
    ui->checkBox_frame->setChecked(pSetting->getDrawFrame());
    ui->checkBox_center->setChecked(pSetting->getDrawCenter());
    ui->checkBox_linear_filter->setChecked(pSetting->getCheckLinearFilter());
    ui->spinBox_frame_start->setValue(m_frameStart);
    ui->spinBox_frame_end->setValue(m_frameEnd);
    ui->label_frame->slot_setFrameStart(m_frameStart);
    ui->label_frame->slot_setFrameEnd(m_frameEnd);

    for (int i = 0; i < m_pEditData->getImageDataListSize(); i++)
    {
        CEditData::ImageData *p = m_pEditData->getImageData(i);
        if (!p)
        {
            continue;
        }
        ui->comboBox_image_no->addItem(tr("%1").arg(p->nNo));
    }

    m_pSplitter = new AnimationWindowSplitter(this);
    m_pSplitter->addWidget(ui->treeView);
    m_pSplitter->addWidget(ui->scrollArea_anime);
    m_pSplitter->setGeometry(ui->treeView->pos().x(),
                             ui->treeView->pos().y(),
                             ui->scrollArea_anime->width() + ui->scrollArea_anime->pos().x() - ui->treeView->pos().x(),
                             ui->treeView->height());

    {
        CObjectModel *pModel = m_pEditData->getObjectModel();

        ui->treeView->setModel(pModel);
        ui->treeView->header()->setHidden(true);
        ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

        ui->treeView->setDragEnabled(true);
        ui->treeView->setAcceptDrops(true);
        ui->treeView->setDropIndicatorShown(true);
        ui->treeView->setDragDropMode(QAbstractItemView::DragDrop);
        ui->treeView->setFocusPolicy(Qt::NoFocus);

        ObjectItem *root = pModel->getItemFromIndex(QModelIndex());
        if (!root->childCount())
        {
            addNewObject(trUtf8("New Object"));
        }
        else
        {
            if (root->child(0))
            {
                QModelIndex index = pModel->index(0);
                ui->treeView->setCurrentIndex(index);
            }
        }
    }

    m_pActTreeViewAdd = new QAction(QString("Add Object"), this);
    m_pActTreeViewCopy = new QAction(QString("Copy Object"), this);
    m_pActTreeViewDel = new QAction(QString("Delete"), this);
    m_pActTreeViewLayerDisp = new QAction(QString("Disp"), this);
    m_pActTreeViewLayerLock = new QAction(QString("Lock"), this);
    m_pActTreeViewCopyLayer = new QAction(QString("CopyLayer"), this);
    m_pActTreeViewPasteLayer = new QAction(QString("PasteLayer"), this);

    m_pTimer = new QTimer(this);
    m_pTimer->setInterval((int)(100.0f / 6.0f));

    connect(ui->label_frame, SIGNAL(sig_changeValue(int)), this, SLOT(slot_frameChanged(int)));
    connect(ui->radioButton_pos, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioPos(bool)));
    connect(ui->radioButton_rot, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioRot(bool)));
    connect(ui->radioButton_center, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioCenter(bool)));
    connect(ui->radioButton_scale, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioScale(bool)));
    connect(ui->radioButton_path, SIGNAL(clicked(bool)), this, SLOT(slot_clickedRadioPath(bool)));

    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_treeViewMenuReq(QPoint)));
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_changeSelectObject(QModelIndex)));

    connect(m_pGlWidget, SIGNAL(sig_dropedImage(RectF, QPoint, int)), this, SLOT(slot_dropedImage(RectF, QPoint, int)));
    connect(m_pGlWidget, SIGNAL(sig_selectLayerChanged(QModelIndex)), this, SLOT(slot_selectLayerChanged(QModelIndex)));
    connect(m_pGlWidget, SIGNAL(sig_dragedImage(FrameData)), this, SLOT(slot_setUI(FrameData)));
    connect(m_pGlWidget, SIGNAL(sig_deleteFrameData()), this, SLOT(slot_deleteFrameData()));
    connect(m_pGlWidget, SIGNAL(sig_selectPrevLayer(QModelIndex, int, FrameData)), this, SLOT(slot_addNewFrameData(QModelIndex, int, FrameData)));
    connect(m_pGlWidget, SIGNAL(sig_frameDataMoveEnd(FrameData)), this, SLOT(slot_frameDataMoveEnd(FrameData)));
    connect(m_pGlWidget, SIGNAL(sig_dragedImage(FrameData)), this, SLOT(slot_portDragedImage(FrameData)));

    connect(ui->doubleSpinBox_pos_x, SIGNAL(valueChanged(double)), this, SLOT(slot_changePosX(double)));
    connect(ui->doubleSpinBox_pos_y, SIGNAL(valueChanged(double)), this, SLOT(slot_changePosY(double)));
    connect(ui->doubleSpinBox_pos_z, SIGNAL(valueChanged(double)), this, SLOT(slot_changePosZ(double)));
    connect(ui->doubleSpinBox_rot_x, SIGNAL(valueChanged(double)), this, SLOT(slot_changeRotX(double)));
    connect(ui->doubleSpinBox_rot_y, SIGNAL(valueChanged(double)), this, SLOT(slot_changeRotY(double)));
    connect(ui->doubleSpinBox_rot_z, SIGNAL(valueChanged(double)), this, SLOT(slot_changeRotZ(double)));
    connect(ui->doubleSpinBox_scale_x, SIGNAL(valueChanged(double)), this, SLOT(slot_changeScaleX(double)));
    connect(ui->doubleSpinBox_scale_y, SIGNAL(valueChanged(double)), this, SLOT(slot_changeScaleY(double)));
    connect(ui->doubleSpinBox_uv_left, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUvLeft(double)));
    connect(ui->doubleSpinBox_uv_right, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUvRight(double)));
    connect(ui->doubleSpinBox_uv_top, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUvTop(double)));
    connect(ui->doubleSpinBox_uv_bottom, SIGNAL(valueChanged(double)), this, SLOT(slot_changeUvBottom(double)));
    connect(ui->doubleSpinBox_center_x, SIGNAL(valueChanged(double)), this, SLOT(slot_changeCenterX(double)));
    connect(ui->doubleSpinBox_center_y, SIGNAL(valueChanged(double)), this, SLOT(slot_changeCenterY(double)));
    connect(ui->spinBox_frame_start, SIGNAL(valueChanged(int)), this, SLOT(slot_changeFrameStart(int)));
    connect(ui->spinBox_frame_end, SIGNAL(valueChanged(int)), this, SLOT(slot_changeFrameEnd(int)));

    connect(m_pActTreeViewAdd, SIGNAL(triggered()), this, SLOT(slot_createNewObject()));
    connect(m_pActTreeViewCopy, SIGNAL(triggered()), this, SLOT(slot_copyObject()));
    connect(m_pActTreeViewDel, SIGNAL(triggered()), this, SLOT(slot_deleteObject()));
    connect(m_pActTreeViewLayerDisp, SIGNAL(triggered()), this, SLOT(slot_changeLayerDisp()));
    connect(m_pActTreeViewLayerLock, SIGNAL(triggered()), this, SLOT(slot_changeLayerLock()));
    connect(m_pActTreeViewCopyLayer, SIGNAL(triggered()), this, SLOT(slot_copyLayer()));
    connect(m_pActTreeViewPasteLayer, SIGNAL(triggered()), this, SLOT(slot_pasteLayer()));

    connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation()));
    connect(ui->pushButton_stop, SIGNAL(clicked()), this, SLOT(slot_stopAnimation()));
    connect(ui->pushButton_backward, SIGNAL(clicked()), this, SLOT(slot_backwardFrameData()));
    connect(ui->pushButton_forward, SIGNAL(clicked()), this, SLOT(slot_forwardFrameData()));
    connect(ui->checkBox_grid, SIGNAL(clicked(bool)), m_pGlWidget, SLOT(slot_setDrawGrid(bool)));
    connect(ui->checkBox_uv_anime, SIGNAL(clicked(bool)), this, SLOT(slot_changeUVAnime(bool)));
    connect(ui->spinBox_fps, SIGNAL(valueChanged(int)), this, SLOT(slot_changeAnimeSpeed(int)));
    connect(ui->comboBox_image_no, SIGNAL(activated(QString)), this, SLOT(slot_changeImageIndex(QString)));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_timerEvent()));
    connect(ui->spinBox_loop, SIGNAL(valueChanged(int)), this, SLOT(slot_changeLoop(int)));
    connect(ui->spinBox_r, SIGNAL(valueChanged(int)), this, SLOT(slot_changeColorR(int)));
    connect(ui->spinBox_g, SIGNAL(valueChanged(int)), this, SLOT(slot_changeColorG(int)));
    connect(ui->spinBox_b, SIGNAL(valueChanged(int)), this, SLOT(slot_changeColorB(int)));
    connect(ui->spinBox_a, SIGNAL(valueChanged(int)), this, SLOT(slot_changeColorA(int)));
    connect(ui->checkBox_frame, SIGNAL(clicked(bool)), this, SLOT(slot_changeDrawFrame(bool)));
    connect(ui->checkBox_center, SIGNAL(clicked(bool)), this, SLOT(slot_changeDrawCenter(bool)));
    connect(ui->checkBox_linear_filter, SIGNAL(clicked(bool)), this, SLOT(slot_changeLinearFilter(bool)));
    connect(ui->toolButton_picker, SIGNAL(clicked()), this, SLOT(slot_clickPicker()));
    connect(m_pSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(slot_splitterMoved(int, int)));
    connect(m_pEditData->getObjectModel(), SIGNAL(sig_copyIndex(int, ObjectItem *, QModelIndex, Qt::DropAction)), this, SLOT(slot_copyIndex(int, ObjectItem *, QModelIndex, Qt::DropAction)));

    connect(ui->pushButton_del_path, SIGNAL(clicked()), this, SLOT(slot_delPath()));
    connect(this, SIGNAL(sig_changeFrameStart(int)), ui->label_frame, SLOT(slot_setFrameStart(int)));
    connect(this, SIGNAL(sig_changeFrameEnd(int)), ui->label_frame, SLOT(slot_setFrameEnd(int)));

    connect(ui->label_frame, SIGNAL(sig_changeValue(int)), ui->spinBox_nowSequence, SLOT(setValue(int)));
    connect(ui->spinBox_nowSequence, SIGNAL(valueChanged(int)), this, SLOT(slot_frameChanged(int)));
    connect(ui->horizontalScrollBar_frame, SIGNAL(valueChanged(int)), ui->label_frame, SLOT(slot_moveScrollBar(int)));
    connect(ui->label_frame, SIGNAL(sig_moveFrameData(int, int)), this, SLOT(slot_moveFrameData(int, int)));
    connect(ui->label_frame, SIGNAL(sig_moveAllFrameData(int, int)), this, SLOT(slot_moveAllFrameData(int, int)));

    connect(m_pGlWidget, SIGNAL(sig_scrollWindow(QPoint)), this, SLOT(slot_scrollWindow(QPoint)));

    connect(ui->pushButton_item_up, SIGNAL(clicked()), this, SLOT(slot_itemMoveUp()));
    connect(ui->pushButton_item_down, SIGNAL(clicked()), this, SLOT(slot_itemMoveDown()));
}

AnimationForm::~AnimationForm()
{
    delete ui;
}

void AnimationForm::Init()
{
    slot_endedOption();
    setBarCenter();
    setFrame();
}

// サイズ変更イベント
void AnimationForm::resizeEvent(QResizeEvent *event)
{
    //	QSize add_w = QSize(add.width(), 0) ;

    if (m_oldWinSize.width() < 0 || m_oldWinSize.height() < 0)
    {
        m_oldWinSize = event->size();
        return;
    }

    QSize add = event->size() - m_oldWinSize;
    QSize add_h = QSize(0, add.height());
    QSize add_w = QSize(add.width(), 0);
    QPoint add_w_p = QPoint(add.width(), 0);
    QPoint add_h_p = QPoint(0, add.height());

    m_oldWinSize = event->size();

    ui->treeView->resize(ui->treeView->size() + add_h);
    ui->scrollArea_anime->resize(ui->scrollArea_anime->size() + add);
    m_pSplitter->resize(m_pSplitter->size() + add);
    ui->label_frame->resize(ui->label_frame->size() + add_w);
    ui->horizontalScrollBar_frame->resize(ui->horizontalScrollBar_frame->size() + add_w);

    setSplitterPos(m_pSetting->getAnmWindowTreeWidth(), m_pSetting->getAnmWindowTreeWidthIndex());

    ui->comboBox_image_no->move(ui->comboBox_image_no->pos() + add_w_p);
    ui->groupBox->move(ui->groupBox->pos() + add_w_p);
    ui->pushButton_item_up->move(ui->pushButton_item_up->pos() + add_h_p);
    ui->pushButton_item_down->move(ui->pushButton_item_down->pos() + add_h_p);

    ui->pushButton_backward->move(ui->pushButton_backward->pos() + add_w_p);
    ui->pushButton_forward->move(ui->pushButton_forward->pos() + add_w_p);
    ui->spinBox_nowSequence->move(ui->spinBox_nowSequence->pos() + add_w_p);
    ui->label_fps->move(ui->label_fps->pos() + add_w_p);
    ui->spinBox_fps->move(ui->spinBox_fps->pos() + add_w_p);

    ui->label_frame->setScrollBarSize();

    QWidget *tmp[] = {
        ui->label_x,
        ui->label_y,
        ui->label_z,
        ui->label_uv,
        ui->label_uv_left,
        ui->label_uv_right,
        ui->label_uv_top,
        ui->label_uv_bottom,
        ui->label_image,
        ui->label_loop,
        ui->label_color,
        ui->label_r,
        ui->label_g,
        ui->label_b,
        ui->label_a,
        ui->doubleSpinBox_pos_x,
        ui->doubleSpinBox_pos_y,
        ui->doubleSpinBox_pos_z,
        ui->doubleSpinBox_rot_x,
        ui->doubleSpinBox_rot_y,
        ui->doubleSpinBox_rot_z,
        ui->doubleSpinBox_uv_left,
        ui->doubleSpinBox_uv_right,
        ui->doubleSpinBox_uv_top,
        ui->doubleSpinBox_uv_bottom,
        ui->doubleSpinBox_center_x,
        ui->doubleSpinBox_center_y,
        ui->spinBox_loop,
        ui->spinBox_r,
        ui->spinBox_g,
        ui->spinBox_b,
        ui->spinBox_a,
        ui->doubleSpinBox_scale_x,
        ui->doubleSpinBox_scale_y,
        ui->checkBox_uv_anime,
        ui->line_0,
        ui->line_1,
        ui->checkBox_center,
        ui->checkBox_frame,
        ui->checkBox_grid,
        ui->toolButton_picker,
        ui->checkBox_linear_filter,
        ui->spinBox_frame_end,
        ui->pushButton_del_path};
    for (int i = 0; i < ARRAY_NUM(tmp); i++)
    {
        tmp[i]->move(tmp[i]->pos() + QPoint(add.width(), 0));
    }
}

void AnimationForm::closeEvent(QCloseEvent * /*event*/)
{
    m_pEditData->setTreeView(NULL);

    for (int i = 0; i < m_pEditData->getImageDataListSize(); i++)
    {
        CEditData::ImageData *p = m_pEditData->getImageData(i);
        if (p->nTexObj)
        {
            m_pGlWidget->deleteTexture(p->nTexObj);
            p->nTexObj = 0;
        }
    }
}

void AnimationForm::dumpObjects(ObjectItem *p, int tab)
{
    QString space, str;
    for (int i = 0; i < tab; i++)
    {
        space += "  ";
    }
    str = space + QString("name:%1").arg(p->getName());
    qDebug() << str;
    const QList<FrameData> list = p->getFrameData();
    for (int i = 0; i < list.size(); i++)
    {
        const FrameData &d = list.at(i);
        str = space + QString(" frame:%1 pos x:%2 y:%3 z:%4").arg(d.frame).arg(d.pos_x).arg(d.pos_y).arg(d.pos_z);
        str += QString("  rot x:%1 y:%2 z:%3").arg(d.rot_x).arg(d.rot_y).arg(d.rot_z);
        qDebug() << str;
    }
    for (int i = 0; i < p->childCount(); i++)
    {
        dumpObjects(p->child(i), tab + 1);
    }
}

// スクロールバーをセンターに。
void AnimationForm::setBarCenter(void)
{
    QScrollBar *pBar;
    pBar = ui->scrollArea_anime->horizontalScrollBar();
    pBar->setValue(pBar->maximum() / 2);
    pBar = ui->scrollArea_anime->verticalScrollBar();
    pBar->setValue(pBar->maximum() / 2);
}

void AnimationForm::setFrame()
{
    ui->label_frame->setScrollBarSize();
}

// デバッグ用。オブジェクト情報ダンプ
void AnimationForm::dbgDumpObject(void)
{
    if (!m_pEditData->getObjectModel())
    {
        return;
    }

    qDebug("Deump Object ------------------------");
    ObjectItem *root = m_pEditData->getObjectModel()->getItemFromIndex(QModelIndex());
    dumpObjects(root, 0);
    qDebug("end ---------------------------------");
}

void AnimationForm::setSplitterPos(int pos, int index)
{
    if (!m_pSplitter)
    {
        return;
    }
    m_pSplitter->MoveSplitter(pos, index);
}

// オブジェクト新規作成
void AnimationForm::slot_createNewObject(void)
{
    bool bOk = false;
    QString str = QInputDialog::getText(this,
                                        trUtf8("New Object"),
                                        trUtf8("オブジェクト名を入力してください"),
                                        QLineEdit::Normal,
                                        trUtf8("New Object"),
                                        &bOk);
    if (!bOk)
    {
        return;
    }
    if (str.isEmpty())
    {
        return;
    }

    QModelIndex index = m_pEditData->cmd_addItem(str);
    ui->treeView->setCurrentIndex(index);
    //	m_pEditData->setSelIndex(index) ;

    ui->spinBox_loop->setValue(0);
}

// オブジェクト削除
void AnimationForm::slot_deleteObject(void)
{
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }

    m_pEditData->cmd_delItem(index);
    slot_changeSelectObject(ui->treeView->currentIndex());
}

// フレームデータ削除
void AnimationForm::slot_deleteFrameData(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }
    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    int frame = m_pEditData->getSelectFrame();
    if (!pItem->getFrameDataPtr(frame))
    {
        return;
    }

    m_pEditData->cmd_delFrameData(index, frame);
}

// ドロップ時のスロット
// レイヤ追加
void AnimationForm::slot_dropedImage(RectF rect, QPoint pos, int imageIndex)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    int frameNum = ui->label_frame->value();
    QModelIndex index = ui->treeView->currentIndex();

    if (!index.isValid())
    {
        qWarning() << "slot_dropedImage current index invalid 0";
        return;
    }

    ObjectItem *pObjItem = pModel->getObject(index);
    if (!pObjItem)
    {
        qWarning() << "slot_dropedImage current obj 0";
        return;
    }

    if (!m_pSetting->getLayerHierarchy())
    {
        index = pObjItem->getIndex();
    }

    pos -= QPoint((m_pSetting->getAnmWindowW() / 2), (m_pSetting->getAnmWindowH() / 2)); // GLWidgetのローカルポスに変換

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    bool valid;
    QMatrix4x4 mat = pItem->getDisplayMatrix(frameNum, &valid);
    if (valid)
    {
        QMatrix4x4 inv = mat.inverted(&valid);
        if (valid)
        {
            pos = inv.map(pos);
        }
    }

    index = m_pEditData->cmd_addItem(QString("Layer %1").arg(pObjItem->childCount()), index);
    ui->treeView->setCurrentIndex(index);
    //	m_pEditData->setSelIndex(index) ;

    // ツリービューに追加
    FrameData frameData;
    frameData.pos_x = pos.x();
    frameData.pos_y = pos.y();
    frameData.pos_z = 0;
    frameData.rot_x = 0;
    frameData.rot_y = 0;
    frameData.rot_z = 0;
    frameData.center_x = (int)((rect.width()) / 2);
    frameData.center_y = (int)((rect.height()) / 2);
    frameData.frame = frameNum;
    frameData.fScaleX = frameData.fScaleY = 1.0f;
    frameData.setRect(rect);
    frameData.nImage = imageIndex;
    frameData.bUVAnime = false;
    frameData.rgba[0] = 255;
    frameData.rgba[1] = 255;
    frameData.rgba[2] = 255;
    frameData.rgba[3] = 255;

    m_pEditData->cmd_addFrameData(index, frameData);
}

// 現在フレーム変更
void AnimationForm::slot_frameChanged(int frame)
{
    bool bChange = (m_pEditData->getSelectFrame() != frame);
    ui->label_frame->setValue(frame);
    m_pEditData->setSelectFrame(frame);
    if (bChange)
    {
        if (m_pEditData->getSelIndex().isValid())
        {
            ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(m_pEditData->getSelIndex());
            if (pItem)
            {
                bool valid;
                FrameData d = pItem->getDisplayFrameData(frame, &valid);
                if (valid)
                {
                    slot_setUI(d);
                }
            }
        }

        m_pGlWidget->update();
    }
}

// 選択レイヤ変更
void AnimationForm::slot_selectLayerChanged(QModelIndex indexLayer)
{
    if (indexLayer.isValid())
    {
        ui->treeView->setCurrentIndex(indexLayer);
        //		m_pEditData->setSelIndex(indexLayer) ;
    }

    ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(indexLayer);
    FrameData *pData = p->getFrameDataPtr(m_pEditData->getSelectFrame());
    if (pData)
    {
        slot_setUI(*pData);
    }

    m_pGlWidget->update();

    emit sig_changeSelectLayer(indexLayer);
}

// UI数値セット
void AnimationForm::slot_setUI(FrameData data)
{
    m_bDontSetData = true;
    if (data.pos_x != ui->doubleSpinBox_pos_x->value())
    {
        ui->doubleSpinBox_pos_x->setValue(data.pos_x);
    }
    if (data.pos_y != ui->doubleSpinBox_pos_y->value())
    {
        ui->doubleSpinBox_pos_y->setValue(data.pos_y);
    }
    if (data.pos_z != ui->doubleSpinBox_pos_z->value())
    {
        ui->doubleSpinBox_pos_z->setValue(data.pos_z);
    }
    if (data.rot_x != ui->doubleSpinBox_rot_x->value())
    {
        ui->doubleSpinBox_rot_x->setValue(data.rot_x);
    }
    if (data.rot_y != ui->doubleSpinBox_rot_y->value())
    {
        ui->doubleSpinBox_rot_y->setValue(data.rot_y);
    }
    if (data.rot_z != ui->doubleSpinBox_rot_z->value())
    {
        ui->doubleSpinBox_rot_z->setValue(data.rot_z);
    }
    if (data.fScaleX != (float)ui->doubleSpinBox_scale_x->value())
    {
        ui->doubleSpinBox_scale_x->setValue(data.fScaleX);
    }
    if (data.fScaleY != (float)ui->doubleSpinBox_scale_y->value())
    {
        ui->doubleSpinBox_scale_y->setValue(data.fScaleY);
    }

    if (data.left != ui->doubleSpinBox_uv_left->value())
    {
        ui->doubleSpinBox_uv_left->setValue(data.left);
    }
    if (data.right != ui->doubleSpinBox_uv_right->value())
    {
        ui->doubleSpinBox_uv_right->setValue(data.right);
    }
    if (data.top != ui->doubleSpinBox_uv_top->value())
    {
        ui->doubleSpinBox_uv_top->setValue(data.top);
    }
    if (data.bottom != ui->doubleSpinBox_uv_bottom->value())
    {
        ui->doubleSpinBox_uv_bottom->setValue(data.bottom);
    }

    if (data.center_x != ui->doubleSpinBox_center_x->value())
    {
        ui->doubleSpinBox_center_x->setValue(data.center_x);
    }
    if (data.center_y != ui->doubleSpinBox_center_y->value())
    {
        ui->doubleSpinBox_center_y->setValue(data.center_y);
    }
    if (data.nImage != ui->comboBox_image_no->currentText().toInt())
    {
        for (int i = 0; i < ui->comboBox_image_no->count(); i++)
        {
            if (ui->comboBox_image_no->itemText(i).toInt() == data.nImage)
            {
                ui->comboBox_image_no->setCurrentIndex(i);
                break;
            }
        }
        emit sig_imageChangeTab(data.nImage);
    }
    if (data.bUVAnime != ui->checkBox_uv_anime->isChecked())
    {
        ui->checkBox_uv_anime->setChecked(data.bUVAnime);
    }
    if (data.rgba[0] != ui->spinBox_r->value())
    {
        ui->spinBox_r->setValue(data.rgba[0]);
    }
    if (data.rgba[1] != ui->spinBox_g->value())
    {
        ui->spinBox_g->setValue(data.rgba[1]);
    }
    if (data.rgba[2] != ui->spinBox_b->value())
    {
        ui->spinBox_b->setValue(data.rgba[2]);
    }
    if (data.rgba[3] != ui->spinBox_a->value())
    {
        ui->spinBox_a->setValue(data.rgba[3]);
    }
    m_bDontSetData = false;

    if (data.getRect() != m_pEditData->getCatchRect())
    {
        RectF rect = data.getRect();
        m_pEditData->setCatchRect(rect);
        emit sig_imageChangeRect(rect);
        emit sig_imageRepaint();
    }
}

// pos x 変更
void AnimationForm::slot_changePosX(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->pos_x = val;
    addCommandEdit(*p);
}

// pos y 変更
void AnimationForm::slot_changePosY(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->pos_y = val;
    addCommandEdit(*p);
}

// pos z 変更
void AnimationForm::slot_changePosZ(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->pos_z = val;
    addCommandEdit(*p);
}

// rot x 変更
void AnimationForm::slot_changeRotX(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rot_x = val;
    addCommandEdit(*p);
}

// rot y 変更
void AnimationForm::slot_changeRotY(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rot_y = val;
    addCommandEdit(*p);
}

// rot z 変更
void AnimationForm::slot_changeRotZ(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rot_z = val;
    addCommandEdit(*p);
}

// scale x 変更
void AnimationForm::slot_changeScaleX(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->fScaleX = val;
    addCommandEdit(*p);
}

// scale y 変更
void AnimationForm::slot_changeScaleY(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->fScaleY = val;
    addCommandEdit(*p);
}

// uv left 変更
void AnimationForm::slot_changeUvLeft(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->left = val;
    addCommandEdit(*p);

    RectF rect = p->getRect();
    m_pEditData->setCatchRect(rect);
    emit sig_imageRepaint();
}

// uv right 変更
void AnimationForm::slot_changeUvRight(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->right = val;
    addCommandEdit(*p);

    RectF rect = p->getRect();
    m_pEditData->setCatchRect(rect);
    emit sig_imageRepaint();
}

// uv top 変更
void AnimationForm::slot_changeUvTop(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->top = val;
    addCommandEdit(*p);

    RectF rect = p->getRect();
    m_pEditData->setCatchRect(rect);
    emit sig_imageRepaint();
}

// uv bottom 変更
void AnimationForm::slot_changeUvBottom(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->bottom = val;
    addCommandEdit(*p);

    RectF rect = p->getRect();
    m_pEditData->setCatchRect(rect);
    emit sig_imageRepaint();
}

// center x 変更
void AnimationForm::slot_changeCenterX(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->center_x = val;
    addCommandEdit(*p);
    emit sig_imageRepaint();
}

// center y 変更
void AnimationForm::slot_changeCenterY(double val)
{
    if (m_pGlWidget->getDragMode() != AnimeGLWidget::kDragMode_None)
    {
        return;
    }
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->center_y = val;
    addCommandEdit(*p);
    emit sig_imageRepaint();
}

void AnimationForm::slot_changeFrameStart(int val)
{
    m_frameStart = val;
    m_pSetting->setFrameStart(val);
    emit sig_changeFrameStart(val);
}

void AnimationForm::slot_changeFrameEnd(int val)
{
    m_frameEnd = val;
    m_pSetting->setFrameEnd(val);
    emit sig_changeFrameEnd(val);
}

// ツリービュー メニューリクエスト
void AnimationForm::slot_treeViewMenuReq(QPoint treeViewLocalPos)
{
    QMenu menu(this);
    menu.addAction(m_pActTreeViewAdd);
    menu.addAction(m_pActTreeViewDel);

    CObjectModel *pModel = m_pEditData->getObjectModel();

    qDebug() << "isObject:" << pModel->isObject(m_pEditData->getSelIndex());
    qDebug() << "isLayer:" << pModel->isLayer(m_pEditData->getSelIndex());

    if (pModel->isObject(m_pEditData->getSelIndex()))
    {
        // オブジェクト選択中だったら
        menu.addAction(m_pActTreeViewCopy);
    }
    else if (pModel->isLayer(m_pEditData->getSelIndex()))
    {
        // レイヤ選択中だったら
        menu.addAction(m_pActTreeViewLayerDisp);
        menu.addAction(m_pActTreeViewLayerLock);
        menu.addAction(m_pActTreeViewCopyLayer);
    }
    if (m_pEditData->isCopyLayer())
    {
        menu.addAction(m_pActTreeViewPasteLayer);
    }

    menu.exec(ui->treeView->mapToGlobal(treeViewLocalPos) + QPoint(0, ui->treeView->header()->height()));
}

// ツリービュー ダブルクリック
void AnimationForm::slot_treeViewDoubleClicked(QModelIndex index)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(index))
    {
        return;
    }

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    QVariant flag = pItem->data(Qt::CheckStateRole);
    int f = flag.toInt();
    if (f & ObjectItem::kState_Disp)
    {
        f &= ~ObjectItem::kState_Disp;
    }
    else
    {
        f |= ObjectItem::kState_Disp;
    }
    pItem->setData(f, Qt::CheckStateRole);
    m_pGlWidget->update();
    ui->treeView->update(index);
}

// 選択オブジェクト変更
void AnimationForm::slot_changeSelectObject(QModelIndex index)
{
    //	m_pEditData->setSelIndex(index) ;

    if (!index.isValid())
    {
        return;
    }

    CObjectModel *pModel = m_pEditData->getObjectModel();

    if (pModel->isLayer(index))
    {
        bool valid;
        ObjectItem *pItem = pModel->getItemFromIndex(index);
        FrameData data = pItem->getDisplayFrameData(m_pEditData->getSelectFrame(), &valid);
        if (valid)
        {
            slot_setUI(data);
        }
        emit sig_changeSelectLayer(index);
    }

    ObjectItem *pObj = pModel->getObject(index);
    if (pObj)
    {
        ui->spinBox_loop->setValue(pObj->getLoop());
        ui->spinBox_fps->setValue(pObj->getFps());
    }

    m_pDataMarker->repaint();
    m_pGlWidget->update();
}

// アニメ再生
void AnimationForm::slot_playAnimation(void)
{
    if (!m_pEditData->getPauseAnime())
    { // ポーズ中じゃなかったら０フレームから再生
        ui->label_frame->setValue(0);
        m_pEditData->setCurrLoopNum(0);
    }

    m_pEditData->setPlayAnime(true);

    // 最大フレーム設定
    ObjectItem *p = m_pEditData->getObjectModel()->getObject(m_pEditData->getSelIndex());
    if (p)
    {
        m_nMaxFrameNum = p->getMaxFrameNum();
    }

    m_pEditData->setPauseAnime(false);
    disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation()));
    connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation()));

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Pause_32.png"), QSize(), QIcon::Normal, QIcon::Off);
    ui->pushButton_play->setIcon(icon);

    m_pTimer->start(); // タイマースタート
    m_pGlWidget->update();
}

// アニメ一時停止
void AnimationForm::slot_pauseAnimation(void)
{
    m_pEditData->setPlayAnime(false);
    m_pEditData->setPauseAnime(true);
    disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation()));
    connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation()));

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Play_32.png"), QSize(), QIcon::Normal, QIcon::Off);
    ui->pushButton_play->setIcon(icon);

    m_pTimer->stop(); // タイマー停止
    m_pGlWidget->update();
}

// アニメ停止
void AnimationForm::slot_stopAnimation(void)
{
    ui->label_frame->setValue(0);
    stopAnimation();
}

// 前フレームのフレームデータに変更
void AnimationForm::slot_backwardFrameData(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(m_pEditData->getSelIndex()))
    {
        return;
    }
    for (int i = m_pEditData->getSelectFrame() - 1; i >= 0; i--)
    {
        if (setSelectFrameDataFromFrame(i, m_pEditData->getSelIndex()))
        {
            break;
        }
    }
}

// 次フレームのフレームデータに変更
void AnimationForm::slot_forwardFrameData(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(m_pEditData->getSelIndex()))
    {
        return;
    }
    for (int i = m_pEditData->getSelectFrame() + 1; i <= CEditData::kMaxFrame; i++)
    {
        if (setSelectFrameDataFromFrame(i, m_pEditData->getSelIndex()))
        {
            break;
        }
    }
}

// タイマイベント。フレームを進める
void AnimationForm::slot_timerEvent(void)
{
    int frame = ui->label_frame->value();

    // PNG吐き出し中
    if (m_pEditData->isExportPNG())
    {
        if (m_pEditData->getExportEndFrame() < frame)
        {
            return;
        }
    }

    frame++;

    if (frame > m_nMaxFrameNum)
    {
        if (m_pEditData->isExportPNG() || m_pEditData->addCurrLoopNum(1))
        {
            // ループ終了
            m_pEditData->endExportPNG();
            stopAnimation();
            ui->label_frame->setValue(frame - 1);
            return;
        }
        frame = 0;
    }

    ui->label_frame->setValue(frame);
}

// フレームデータ追加
void AnimationForm::slot_addNewFrameData(QModelIndex indexLayer, int frame, FrameData data)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(indexLayer))
    {
        return;
    }

    qDebug() << "slot_addNewFrameData frame:" << frame;
    data.frame = frame;
    data.path[0] = data.path[1] = PathData(); // パス リセット
    m_pEditData->cmd_addFrameData(indexLayer, data);
}

// レイヤ表示ON/OFF
void AnimationForm::slot_changeLayerDisp(void)
{
    slot_treeViewDoubleClicked(m_pEditData->getSelIndex());
}

// レイヤロック ON/OFF
void AnimationForm::slot_changeLayerLock(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();

    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    QVariant flag = pItem->data(Qt::CheckStateRole);
    int f = flag.toInt();
    if (f & ObjectItem::kState_Lock)
    {
        f &= ~ObjectItem::kState_Lock;
    }
    else
    {
        f |= ObjectItem::kState_Lock;
    }
    pItem->setData(f, Qt::CheckStateRole);
    m_pGlWidget->update();
    ui->treeView->update(index);
}

// 選択中レイヤのUV変更
void AnimationForm::slot_changeSelectLayerUV(RectF rect)
{
    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }
    p->setRect(rect);
    addCommandEdit(*p);

    slot_setUI(*p);
}

// FPS変更
void AnimationForm::slot_changeAnimeSpeed(int val)
{
    if (val <= 0)
    {
        return;
    }

    bool bPlay = m_pTimer->isActive();
    m_pTimer->stop();
    m_pTimer->setInterval((int)(1000.0f / val));
    if (bPlay)
    {
        m_pTimer->start();
    }

    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    ObjectItem *pItem = pModel->getObject(index);
    if (!pItem)
    {
        return;
    }
    pItem->setFps(val);
}

// イメージ追加
void AnimationForm::slot_addImage(int imageNo)
{
    qDebug() << "AnimationForm::slot_addImage:" << imageNo;

    //	ui->comboBox_image_no->addItem(tr("%1").arg(imageNo));
    ui->comboBox_image_no->insertItem(imageNo, tr("%1").arg(imageNo));

    for (int i = 0; i < m_pEditData->getImageDataListSize(); i++)
    {
        CEditData::ImageData *p = m_pEditData->getImageData(i);
        if (!p)
        {
            continue;
        }
        if (p->nNo != imageNo)
        {
            continue;
        }
        if (p->nTexObj)
        {
            continue;
        }
        p->nTexObj = m_pGlWidget->bindTexture(p->Image);
    }
}

// イメージ削除
void AnimationForm::slot_delImage(int imageNo)
{
    ui->comboBox_image_no->removeItem(imageNo);
    m_pEditData->removeImageDataByNo(imageNo);
    m_pGlWidget->update();
}

// イメージ番号変更
void AnimationForm::slot_changeImageIndex(QString index)
{
    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }
    qDebug() << "changeImageIndex:" << index;

    p->nImage = index.toInt();
    m_pGlWidget->update();
    addCommandEdit(*p);
}

// UVアニメON/OFF
void AnimationForm::slot_changeUVAnime(bool flag)
{
    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }
    p->bUVAnime = flag;
    addCommandEdit(*p);
}

// イメージ更新
void AnimationForm::slot_modifiedImage(int index)
{
    CEditData::ImageData *p = m_pEditData->getImageData(index);
    if (!p)
    {
        return;
    }
    if (p->nTexObj)
    {
        m_pGlWidget->deleteTexture(p->nTexObj);
    }
    p->nTexObj = m_pGlWidget->bindTexture(p->Image);

    m_pGlWidget->update();
}

// オプションダイアログ終了時
void AnimationForm::slot_endedOption(void)
{
    QString path = QString();
    if (m_pSetting->getUseBackImage())
    {
        path = m_pSetting->getBackImagePath();
    }
    m_pGlWidget->setBackImage(path);
    m_pGlWidget->resize(m_pSetting->getAnmWindowW(), m_pSetting->getAnmWindowH());
    m_pGlWidget->setDrawArea(m_pSetting->getAnmWindowW(), m_pSetting->getAnmWindowH());
    m_pGlWidget->update();
}

// マウスでのデータ編集終了時
void AnimationForm::slot_frameDataMoveEnd(FrameData dataOld)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    FrameData *p = pItem->getFrameDataPtr(m_pEditData->getSelectFrame());
    if (!p)
    {
        return;
    }
    if (*p != dataOld)
    {
        addCommandEdit(*p, &dataOld);
    }
}

// ラジオボタン POS クリック
void AnimationForm::slot_clickedRadioPos(bool flag)
{
    if (flag)
    {
        m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Pos);
    }
    m_pGlWidget->update();
}

// ラジオボタン ROT クリック
void AnimationForm::slot_clickedRadioRot(bool flag)
{
    if (flag)
    {
        m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Rot);
    }
    m_pGlWidget->update();
}

// ラジオボタン CENTER クリック
void AnimationForm::slot_clickedRadioCenter(bool flag)
{
    if (flag)
    {
        m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Center);
    }
    m_pGlWidget->update();
}

// ラジオボタン SCALE クリック
void AnimationForm::slot_clickedRadioScale(bool flag)
{
    if (flag)
    {
        m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Scale);
    }
    m_pGlWidget->update();
}

// ラジオボタン PATH クリック
void AnimationForm::slot_clickedRadioPath(bool flag)
{
    if (flag)
    {
        m_pGlWidget->setEditMode(AnimeGLWidget::kEditMode_Path);
    }
    m_pGlWidget->update();
}

// ループ回数変更
void AnimationForm::slot_changeLoop(int val)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    ObjectItem *pItem = pModel->getObject(index);
    if (!pItem)
    {
        return;
    }
    pItem->setLoop(val);
}

// 色R変更
void AnimationForm::slot_changeColorR(int val)
{
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rgba[0] = val;
    addCommandEdit(*p);
}

// 色G変更
void AnimationForm::slot_changeColorG(int val)
{
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rgba[1] = val;
    addCommandEdit(*p);
}

// 色B変更
void AnimationForm::slot_changeColorB(int val)
{
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rgba[2] = val;
    addCommandEdit(*p);
}

// 色A変更
void AnimationForm::slot_changeColorA(int val)
{
    if (m_bDontSetData)
    {
        return;
    }

    addNowSelectLayerAndFrame();
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->rgba[3] = val;
    addCommandEdit(*p);
}

// 現在選択しているフレームデータ取得
FrameData *AnimationForm::getNowSelectFrameData(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(m_pEditData->getSelIndex()))
    {
        return NULL;
    }
    int frame = m_pEditData->getSelectFrame();
    return pModel->getItemFromIndex(m_pEditData->getSelIndex())->getFrameDataPtr(frame);
}

// 選択オブジェクトをコピー
void AnimationForm::slot_copyObject(void)
{
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }

    m_pEditData->cmd_copyObject(index);
}

void AnimationForm::slot_changeDrawFrame(bool flag)
{
    m_pSetting->setDrawFrame(flag);
    m_pGlWidget->update();
}

void AnimationForm::slot_changeDrawCenter(bool flag)
{
    m_pSetting->setDrawCenter(flag);
    m_pGlWidget->update();

    emit sig_portCheckDrawCenter(flag);
}

void AnimationForm::slot_changeLinearFilter(bool flag)
{
    m_pSetting->setCheckLinearFilter(flag);

    for (int i = 0; i < m_pEditData->getImageDataListSize(); i++)
    {
        CEditData::ImageData *p = m_pEditData->getImageData(i);
        if (!p)
        {
            continue;
        }
        if (p->nTexObj)
        {
            m_pGlWidget->deleteTexture(p->nTexObj);
        }
        p->nTexObj = m_pGlWidget->bindTexture(p->Image);
    }

    m_pGlWidget->update();
}

void AnimationForm::slot_portDragedImage(FrameData data)
{
    emit sig_portDragedImage(data);
}

void AnimationForm::slot_clickPicker(void)
{
    emit sig_pushColorToolButton();
}

void AnimationForm::slot_setColorFromPicker(QRgb rgba)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }

    int frame = m_pEditData->getSelectFrame();
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    FrameData *pData = pItem->getFrameDataPtr(frame);
    if (!pData)
    {
        pData = pModel->getFrameDataFromPrevFrame(index, frame, true);
        if (!pData)
        {
            return;
        }
        FrameData *pNext = pModel->getFrameDataFromNextFrame(index, frame);
        FrameData data = pData->getInterpolation(pNext, frame);
        data.rgba[0] = qRed(rgba);
        data.rgba[1] = qGreen(rgba);
        data.rgba[2] = qBlue(rgba);
        slot_addNewFrameData(index, frame, data);
    }
    else
    {
        pData->rgba[0] = qRed(rgba);
        pData->rgba[1] = qGreen(rgba);
        pData->rgba[2] = qBlue(rgba);

        addCommandEdit(*pData);
    }
}

void AnimationForm::slot_splitterMoved(int pos, int index)
{
    qDebug("splitterMoved pos:%d index:%d", pos, index);
    m_pSetting->setAnmWindowTreeWidth(pos);
    m_pSetting->setAnmWindowTreeWidthIndex(index);
}

void AnimationForm::slot_copyIndex(int row, ObjectItem *pItem, QModelIndex index, Qt::DropAction /*action*/)
{
    m_pEditData->cmd_copyIndex(row, pItem, index);
}

// フレームデータ 移動
void AnimationForm::slot_moveFrameData(int prevFrame, int nextFrame)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();

    if (!pModel->isLayer(index))
    {
        return;
    }
    if (prevFrame < 0 || nextFrame < 0)
    {
        return;
    }

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }
    if (!pItem->getFrameDataPtr(prevFrame))
    {
        return;
    }

    m_pEditData->cmd_moveFrameData(index, prevFrame, nextFrame);
}

// 全フレームデータ移動
void AnimationForm::slot_moveAllFrameData(int prevFrame, int nextFrame)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();

    if (!pModel->isObject(index))
    {
        return;
    }
    if (prevFrame < 0 || nextFrame < 0)
    {
        return;
    }

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    m_pEditData->cmd_moveAllFrameData(index, prevFrame, nextFrame);
}

void AnimationForm::slot_scrollWindow(QPoint move)
{
    QScrollBar *p;

    setCursor(QCursor(Qt::ClosedHandCursor));

    p = ui->scrollArea_anime->horizontalScrollBar();
    if (p->isEnabled() && !p->isHidden())
    {
        p->setValue(p->value() - move.x());
    }
    p = ui->scrollArea_anime->verticalScrollBar();
    if (p->isEnabled() && !p->isHidden())
    {
        p->setValue(p->value() - move.y());
    }
}

void AnimationForm::slot_delPath()
{
    FrameData *p = getNowSelectFrameData();
    if (!p)
    {
        return;
    }

    p->path[0].bValid = false;
    p->path[0].v.setX(0);
    p->path[0].v.setY(0);
    p->path[1].bValid = false;
    p->path[1].v.setX(0);
    p->path[1].v.setY(0);
    addCommandEdit(*p);
}

// ツリーアイテムを上に移動
void AnimationForm::slot_itemMoveUp()
{
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }
    if (index.row() <= 0)
    {
        return;
    }
    m_pEditData->cmd_moveItemUp(index);
}

// ツリーアイテムを下に移動
void AnimationForm::slot_itemMoveDown()
{
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }
    ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index.parent());
    if (!pItem)
    {
        return;
    }
    if (index.row() == pItem->childCount() - 1)
    {
        return;
    }
    m_pEditData->cmd_moveItemDown(index);
}

void AnimationForm::slot_copyLayer()
{
    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }
    if (!m_pEditData->getObjectModel()->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    m_pEditData->setCopyLayer(pItem);
}

void AnimationForm::slot_pasteLayer()
{
    if (!m_pEditData->isCopyLayer())
    {
        return;
    }

    QModelIndex index = m_pEditData->getSelIndex();
    if (!index.isValid())
    {
        return;
    }

    m_pEditData->cmd_pasteLayer(index, m_pEditData->getCopyLayer());
}

// オブジェクト追加
void AnimationForm::addNewObject(QString str)
{
    if (str.isEmpty())
    {
        return;
    }

    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = pModel->addItem(str, QModelIndex());
    ui->treeView->setCurrentIndex(index);
    //	m_pEditData->setSelIndex(index) ;
}

// フレームデータ編集コマンド
void AnimationForm::addCommandEdit(FrameData data, FrameData *pOld)
{
    int frame = m_pEditData->getSelectFrame();
    QList<QWidget *> update;

    m_pEditData->cmd_editFrameData(m_pEditData->getSelIndex(), frame, data, pOld, this);
    emit sig_changeSelectLayer(m_pEditData->getSelIndex());
}

// 指定フレームのフレームデータを選択する
bool AnimationForm::setSelectFrameDataFromFrame(int frame, QModelIndex indexLayer)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(indexLayer))
    {
        return false;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(indexLayer);
    FrameData *pData = pItem->getFrameDataPtr(frame);
    if (!pData)
    {
        return false;
    }

    ui->spinBox_nowSequence->setValue(frame);
    slot_setUI(*pData);
    m_pGlWidget->update();
    return true;
}

// 選択レイヤの選択フレームにフレームデータを追加
void AnimationForm::addNowSelectLayerAndFrame(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();

    if (!pModel->isLayer(index))
    {
        return;
    }

    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    int frame = m_pEditData->getSelectFrame();
    if (pItem->getFrameDataPtr(frame))
    {
        return;
    }

    FrameData *pPrev = pModel->getFrameDataFromPrevFrame(index, frame, true);
    if (!pPrev)
    {
        return;
    }
    FrameData *pNext = pModel->getFrameDataFromNextFrame(index, frame);
    FrameData data = pPrev->getInterpolation(pNext, frame);

    slot_addNewFrameData(index, frame, data); // フレームデータ追加
}

// キー押しイベント
bool AnimationForm::keyPress(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_pGlWidget->setPressCtrl(true);
        m_pGlWidget->update();
    }

    int key = event->key();
    if (event->modifiers() & Qt::ShiftModifier)
    {
        key |= Qt::SHIFT;
    }
    if (event->modifiers() & Qt::ControlModifier)
    {
        key |= Qt::CTRL;
    }
    if (event->modifiers() & Qt::MetaModifier)
    {
        key |= Qt::META;
    }
    if (event->modifiers() & Qt::AltModifier)
    {
        key |= Qt::ALT;
    }
    QKeySequence ks(key);

    if (ks == m_pSetting->getShortcutCopyFrame())
    {
        copyFrameData();
        return true;
    }
    else if (ks == m_pSetting->getShortcutPasteFrame())
    {
        pasteFrameData();
        return true;
    }
    else if (ks == m_pSetting->getShortcutPosSelect())
    {
        ui->radioButton_pos->setChecked(true);
        slot_clickedRadioPos(true);
        return true;
    }
    else if (ks == m_pSetting->getShortcutRotSelect())
    {
        ui->radioButton_rot->setChecked(true);
        slot_clickedRadioRot(true);
        return true;
    }
    else if (ks == m_pSetting->getShortcutCenterSelect())
    {
        ui->radioButton_center->setChecked(true);
        slot_clickedRadioCenter(true);
        return true;
    }
    else if (ks == m_pSetting->getShortcutScaleSelect())
    {
        ui->radioButton_scale->setChecked(true);
        slot_clickedRadioScale(true);
        return true;
    }
    else if (ks == m_pSetting->getShortcutPathSelect())
    {
        ui->radioButton_path->setChecked(true);
        slot_clickedRadioPath(true);
        return true;
    }
    else if (ks == m_pSetting->getShortcutPlayAnime())
    {
        if (m_pEditData->getPlayAnime())
        {
            slot_pauseAnimation();
        }
        else
        {
            slot_playAnimation();
        }
        return true;
    }
    else if (ks == m_pSetting->getShortcutStopAnime())
    {
        slot_stopAnimation();
        return true;
    }
    else if (ks == m_pSetting->getShortcutJumpStartFrame())
    {
        jumpStartFrame();
        return true;
    }
    else if (ks == m_pSetting->getShortcutJumpEndFrame())
    {
        jumpEndFrame();
        return true;
    }
    else if (ks == m_pSetting->getShortcutAddFrameData())
    {
        CObjectModel *pModel = m_pEditData->getObjectModel();
        QModelIndex index = m_pEditData->getSelIndex();
        int frame = m_pEditData->getSelectFrame();
        if (!pModel->isLayer(index))
        {
            return false;
        }
        ObjectItem *pItem = pModel->getItemFromIndex(index);

        FrameData data = pItem->getDisplayFrameData(frame);
        data.frame = frame;
        if (pItem->getFrameDataPtr(frame))
        { // 既にフレームデータある
            FrameData *p = pItem->getFrameDataPtr(frame);
            if (*p != data)
            { // データが違う場合編集状態に。
                addCommandEdit(data, p);
            }
        }
        else
        {
            slot_addNewFrameData(index, frame, data);
        }
        return true;
    }
    else if (ks == m_pSetting->getShortcutDelFrameData())
    {
        slot_deleteFrameData();
        return true;
    }
    else if (ks == m_pSetting->getShortcutDelItem())
    {
        slot_deleteObject();
        return true;
    }
    else if (ks == m_pSetting->getShortcutDispItem())
    {
        slot_changeLayerDisp();
        return true;
    }
    else if (ks == m_pSetting->getShortcutLockItem())
    {
        slot_changeLayerLock();
        return true;
    }
    else if (ks == m_pSetting->getShortcutMoveAnimeWindow())
    {
        m_pGlWidget->setPressWindowMove(true);
        setCursor(QCursor(Qt::OpenHandCursor));
        return true;
    }
    return false;
}

// キー離しイベント
void AnimationForm::keyRelease(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_pGlWidget->setPressCtrl(false);
        m_pGlWidget->update();
    }
    m_pGlWidget->setPressWindowMove(false);
    setCursor(QCursor());
}

// フレームデータ コピー
void AnimationForm::copyFrameData(void)
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);

    FrameData data, *p;
    int frame = m_pEditData->getSelectFrame();
    p = pItem->getFrameDataPtr(frame);
    if (p)
    {
        data = *p;
    }
    else
    {
        bool valid;
        data = pItem->getDisplayFrameData(frame, &valid);
        if (!valid)
        {
            return;
        }
    }
    qDebug() << "Copy Framedata";
    m_pEditData->setCopyFrameData(data);
}

// フレームデータ ペースト
void AnimationForm::pasteFrameData(void)
{
    if (!m_pEditData->isCopyData())
    {
        return;
    }

    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    int frame = m_pEditData->getSelectFrame();
    FrameData *pData = pItem->getFrameDataPtr(frame);
    if (pData)
    {
        FrameData data = m_pEditData->getCopyFrameData();
        data.frame = frame;
        addCommandEdit(data, pData);
    }
    else
    {
        slot_addNewFrameData(index, frame, m_pEditData->getCopyFrameData()); // フレームデータ追加
    }
    qDebug() << "Paste Framedata";
}

void AnimationForm::jumpStartFrame()
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    int minFrame = 0x7fffffff;
    const QList<FrameData> &datas = pItem->getFrameData();
    for (int i = 0; i < datas.size(); i++)
    {
        if (minFrame > datas.at(i).frame)
        {
            minFrame = datas.at(i).frame;
        }
    }
    if (!pItem->getFrameDataPtr(minFrame))
    {
        return;
    }

    slot_frameChanged(minFrame);
}

void AnimationForm::jumpEndFrame()
{
    CObjectModel *pModel = m_pEditData->getObjectModel();
    QModelIndex index = m_pEditData->getSelIndex();
    if (!pModel->isLayer(index))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    if (!pItem)
    {
        return;
    }

    int maxFrame = -1;
    const QList<FrameData> &datas = pItem->getFrameData();
    for (int i = 0; i < datas.size(); i++)
    {
        if (maxFrame < datas.at(i).frame)
        {
            maxFrame = datas.at(i).frame;
        }
    }
    if (!pItem->getFrameDataPtr(maxFrame))
    {
        return;
    }

    slot_frameChanged(maxFrame);
}

void AnimationForm::stopAnimation()
{
    disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation()));
    disconnect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_pauseAnimation()));
    connect(ui->pushButton_play, SIGNAL(clicked()), this, SLOT(slot_playAnimation()));

    m_pEditData->setPlayAnime(false);
    m_pEditData->setPauseAnime(false);

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/root/Resources/images/Button Play_32.png"), QSize(), QIcon::Normal, QIcon::Off);
    ui->pushButton_play->setIcon(icon);

    m_pTimer->stop(); // タイマー停止
    m_pGlWidget->update();
}
