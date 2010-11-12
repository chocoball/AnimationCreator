#include <QPixmap>
#include <QGraphicsView>
#include "defines.h"
#include "imagewindow.h"
#include "gridlabel.h"
#include "animationform.h"
#include "mainwindow.h"

ImageWindow::ImageWindow(CSettings *p, CEditImageData *pEditImage, AnimationForm *pAnimForm, MainWindow *pMainWindow, QWidget *parent)
	: QWidget(parent),
	ui(new Ui::ImageWindow)
{
	ui->setupUi(this) ;

	m_pSetting = p ;
	m_pEditImageData = pEditImage ;
	setAnimationForm(pAnimForm);
	m_pMainWindow = pMainWindow ;

	setAcceptDrops(true) ;

	ui->checkBox->setChecked(true);
	ui->spinBox_uv_bottom->setMaximum(1024);
	ui->spinBox_uv_top->setMaximum(1024);
	ui->spinBox_uv_left->setMaximum(1024);
	ui->spinBox_uv_right->setMaximum(1024);

	m_pActDelImage = new QAction(trUtf8("Delete"), this) ;

	connect(m_pActDelImage, SIGNAL(triggered()), this, SLOT(slot_delImage())) ;
	connect(this, SIGNAL(sig_addImage(int)), m_pAnimationForm, SLOT(slot_addImage(int))) ;
	connect(this, SIGNAL(sig_delImage(int)), m_pAnimationForm, SLOT(slot_delImage(int))) ;
	connect(ui->spinBox_uv_bottom,	SIGNAL(valueChanged(int)), this, SLOT(slot_changeUVBottom(int))) ;
	connect(ui->spinBox_uv_top,		SIGNAL(valueChanged(int)), this, SLOT(slot_changeUVTop(int))) ;
	connect(ui->spinBox_uv_left,	SIGNAL(valueChanged(int)), this, SLOT(slot_changeUVLeft(int))) ;
	connect(ui->spinBox_uv_right,	SIGNAL(valueChanged(int)), this, SLOT(slot_changeUVRight(int))) ;

	ui->tabWidget->clear();
	for ( int i = 0 ; i < m_pEditImageData->getImageDataSize() ; i ++ ) {
		addTab(i);
	}

	setWindowTitle(tr("Image Window")) ;
}

ImageWindow::~ImageWindow()
{
	delete ui ;
}

void ImageWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}

void ImageWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls() ;
	int index = m_pEditImageData->getImageDataSize() ;

	for ( int i = 0 ; i < urls.size() ; i ++ ) {
		QString fileName = urls[i].toLocalFile() ;

		CEditImageData::ImageData data ;
		QImage image ;
		if ( !image.load(fileName) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			continue ;
		}
		data.fileName		= fileName ;
		data.Image			= image ;
		data.lastModified	= QDateTime::currentDateTimeUtc() ;
		data.nTexObj		= 0 ;
		m_pEditImageData->addImageData(data);
		addTab(index) ;

		emit sig_addImage(index) ;

		index ++ ;
	}
}

void ImageWindow::addTab(int imageIndex)
{
	QLabel *pLabel = new QLabel(ui->tabWidget) ;
	pLabel->setPixmap(QPixmap::fromImage(m_pEditImageData->getImage(imageIndex))) ;
	pLabel->setObjectName("ImageLabel");
	pLabel->setScaledContents(true) ;
	pLabel->setAutoFillBackground(true);
	QPalette palette = pLabel->palette() ;
	palette.setColor(QPalette::Background, m_pSetting->getImageBGColor()) ;
	pLabel->setPalette(palette);

	CGridLabel *pGridLabel = new CGridLabel(m_pEditImageData, imageIndex, pLabel) ;
	pGridLabel->show() ;

	QScrollArea *pScrollArea = new QScrollArea(ui->tabWidget) ;
	pScrollArea->setWidget(pLabel) ;

	ui->tabWidget->addTab(pScrollArea, tr("%1").arg(imageIndex)) ;

	connect(ui->checkBox, SIGNAL(clicked(bool)), pGridLabel, SLOT(slot_gridOnOff(bool))) ;
	connect(pGridLabel, SIGNAL(sig_changeSelectLayerUV(QRect)), m_pAnimationForm, SLOT(slot_changeSelectLayerUV(QRect))) ;
	connect(pGridLabel, SIGNAL(sig_changeCatchRect(QRect)), this, SLOT(slot_setUI(QRect))) ;
	connect(m_pAnimationForm, SIGNAL(sig_imageRepaint()), pGridLabel, SLOT(update())) ;
}

void ImageWindow::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this) ;
	menu.addAction(m_pActDelImage) ;
	menu.exec(event->globalPos()) ;
}

void ImageWindow::updateGridLabel( void )
{
	QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(ui->tabWidget->currentIndex()) ;

	QLabel *label = pScrollArea->findChild<QLabel *>("ImageLabel") ;
	if ( label ) {
		label->update();
	}
}

void ImageWindow::resizeEvent(QResizeEvent *event)
{
	QSize add = event->size() - event->oldSize() ;
	QSize add_h = QSize(0, add.height()) ;
	QSize add_w = QSize(add.width(), 0) ;

	if ( event->oldSize().width() < 0 || event->oldSize().height() < 0 ) {
		return ;
	}

	ui->tabWidget->resize(ui->tabWidget->size()+add);

	QLabel *pTmpLabel[] = {
		ui->label_uv,
		ui->label_uv_bottom,
		ui->label_uv_left,
		ui->label_uv_right,
		ui->label_uv_top,
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(pTmpLabel) ; i ++ ) {
		pTmpLabel[i]->move(pTmpLabel[i]->pos() + QPoint(add.width(), 0));
	}

	QSpinBox *pTmpBox[] = {
		ui->spinBox_uv_bottom,
		ui->spinBox_uv_left,
		ui->spinBox_uv_right,
		ui->spinBox_uv_top,
	} ;
	for ( int i = 0 ; i < ARRAY_NUM(pTmpBox) ; i ++ ) {
		pTmpBox[i]->move(pTmpBox[i]->pos() + QPoint(add.width(), 0));
	}
}

void ImageWindow::slot_delImage( void )
{
	int index = ui->tabWidget->currentIndex() ;
	ui->tabWidget->removeTab(index);

	for ( int i = 0 ; i < ui->tabWidget->count() ; i ++ ) {
		ui->tabWidget->setTabText(i, tr("%1").arg(i));
	}

	emit sig_delImage(index) ;
}

void ImageWindow::slot_modifiedImage( int index )
{
	QScrollArea *pScrollArea = (QScrollArea *)ui->tabWidget->widget(index) ;

	QLabel *label = pScrollArea->findChild<QLabel *>("ImageLabel") ;
	if ( !label ) {
		qDebug() << "ERROR:ImageLabel not found!!!!!" ;
		return ;
	}
	label->setPixmap(QPixmap::fromImage(m_pEditImageData->getImage(index))) ;
	QPalette palette = label->palette() ;
	palette.setColor(QPalette::Background, m_pSetting->getImageBGColor()) ;
	label->setPalette(palette);
	label->update();
}

void ImageWindow::slot_changeUVBottom( int val )
{
	QRect r = m_pEditImageData->getCatchRect() ;
	r.setBottom(val);
	m_pEditImageData->setCatchRect(r);

	updateGridLabel() ;
}

void ImageWindow::slot_changeUVTop( int val )
{
	QRect r = m_pEditImageData->getCatchRect() ;
	r.setTop(val);
	m_pEditImageData->setCatchRect(r);

	updateGridLabel() ;
}

void ImageWindow::slot_changeUVLeft( int val )
{
	QRect r = m_pEditImageData->getCatchRect() ;
	r.setLeft(val);
	m_pEditImageData->setCatchRect(r);

	updateGridLabel() ;
}

void ImageWindow::slot_changeUVRight( int val )
{
	QRect r = m_pEditImageData->getCatchRect() ;
	r.setRight(val);
	m_pEditImageData->setCatchRect(r);

	updateGridLabel() ;
}

void ImageWindow::slot_setUI( QRect rect )
{
	ui->spinBox_uv_bottom->setValue(rect.bottom());
	ui->spinBox_uv_top->setValue(rect.top());
	ui->spinBox_uv_left->setValue(rect.left());
	ui->spinBox_uv_right->setValue(rect.right());
}

void ImageWindow::slot_endedOption( void )
{
	for ( int i = 0 ; i < ui->tabWidget->count() ; i ++ ) {
		slot_modifiedImage(i);
	}
}



