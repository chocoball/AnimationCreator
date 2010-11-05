#include <QPixmap>
#include <QGraphicsView>
#include "imagewindow.h"
#include "gridlabel.h"
#include "animationform.h"

ImageWindow::ImageWindow(CSettings *p, CEditImageData *pEditImage, AnimationForm *pAnimForm, QWidget *parent)
	: QWidget(parent)
{
	m_pSetting = p ;
	m_pEditImageData = pEditImage ;
	setAnimationForm(pAnimForm);

	setAcceptDrops(true) ;

	m_pCheckBox = new QCheckBox(trUtf8("グリッド"), this) ;
	m_pCheckBox->setChecked(true);

	m_pActDelImage = new QAction(trUtf8("Delete"), this) ;

	connect(m_pActDelImage, SIGNAL(triggered()), this, SLOT(slot_delImage())) ;
	connect(this, SIGNAL(sig_addImage(int)), m_pAnimationForm, SLOT(slot_addImage(int))) ;
	connect(this, SIGNAL(sig_delImage(int)), m_pAnimationForm, SLOT(slot_delImage(int))) ;

	m_pTabWidget = new QTabWidget(this) ;
	for ( int i = 0 ; i < m_pEditImageData->getImageDataSize() ; i ++ ) {
		addTab(i);
	}

	QGridLayout *layout = new QGridLayout ;
	layout->addWidget(m_pCheckBox, 0, 0);
	layout->addWidget(m_pTabWidget, 1, 0, 1, 3) ;
	setLayout(layout) ;

	setWindowTitle(tr("Image Window")) ;
}

ImageWindow::~ImageWindow()
{
	QPoint Pos = pos() ;
	QSize Size = size() ;
	m_pSetting->setImgWinPos(Pos) ;
	m_pSetting->setImgWinSize(Size) ;
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

		QImage image ;
		if ( !image.load(fileName) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			continue ;
		}
		m_pEditImageData->addImage(image);
		addTab(index) ;

		emit sig_addImage(index) ;

		index ++ ;
	}
}

void ImageWindow::addTab(int imageIndex)
{
	QLabel *pLabel = new QLabel(m_pTabWidget) ;
	pLabel->setPixmap(QPixmap::fromImage(m_pEditImageData->getImage(imageIndex))) ;
	pLabel->setScaledContents(true) ;

	CGridLabel *pGridLabel = new CGridLabel(m_pEditImageData, imageIndex, pLabel) ;
	pGridLabel->show() ;

	QScrollArea *pScrollArea = new QScrollArea(m_pTabWidget) ;
	pScrollArea->setWidget(pLabel) ;

	m_pTabWidget->addTab(pScrollArea, tr("%1").arg(imageIndex)) ;

	connect(m_pCheckBox, SIGNAL(clicked(bool)), pGridLabel, SLOT(slot_gridOnOff(bool))) ;
	connect(pGridLabel, SIGNAL(sig_changeSelectLayerUV(QRect)), m_pAnimationForm, SLOT(slot_changeSelectLayerUV(QRect))) ;
	connect(m_pAnimationForm, SIGNAL(sig_imageRepaint()), pGridLabel, SLOT(update())) ;
}

void ImageWindow::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this) ;
	menu.addAction(m_pActDelImage) ;
	menu.exec(event->globalPos()) ;
}

void ImageWindow::slot_delImage( void )
{
	int index = m_pTabWidget->currentIndex() ;
	m_pTabWidget->removeTab(index);

	for ( int i = 0 ; i < m_pTabWidget->count() ; i ++ ) {
		m_pTabWidget->setTabText(i, tr("%1").arg(i));
	}

	emit sig_delImage(index) ;
}
