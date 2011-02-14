#include <QGridLayout>
#include <QComboBox>
#include "cloupewindow.h"
#include "mainwindow.h"

CLoupeWindow::CLoupeWindow(CEditData *pEditData, MainWindow *pMainWindow, QWidget *parent) :
    QWidget(parent)
{
	m_pEditData = pEditData ;
	m_pMainWindow = pMainWindow ;

	QLabel *pLabelScale = new QLabel(trUtf8("倍率"), this) ;

	QComboBox *pComboBox = new QComboBox(this) ;
	pComboBox->addItem(tr("2"));
	pComboBox->addItem(tr("4"));
	pComboBox->addItem(tr("8"));
	pComboBox->addItem(tr("16"));

	m_pLabel = new QLabel(this) ;
	m_pLabel->setGeometry(QRect(0, 0, 200, 200));
	m_pLabel->setScaledContents(true);
	m_pLabel->setMinimumSize(200, 200);

	QGridLayout *layout = new QGridLayout(this) ;
	layout->addWidget(pLabelScale, 0, 0);
	layout->addWidget(pComboBox, 0, 1);
	layout->addWidget(m_pLabel, 1, 0, 2, 2) ;
	setLayout(layout);

	m_pTimer = new QTimer(this) ;
	m_pTimer->setInterval(100/3);
	m_pTimer->start();

	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_cursorScreenShort())) ;
	connect(pComboBox, SIGNAL(activated(QString)), this, SLOT(slot_changeScale(QString))) ;

	m_Scale = 2 ;

	setWindowTitle(trUtf8("ルーペ"));
}

void CLoupeWindow::slot_cursorScreenShort()
{
	if ( m_pEditData->isDraggingImage() ) {
		return ;
	}

	QSize size = m_pLabel->size() ;
	fixImage(size) ;
}

void CLoupeWindow::slot_changeScale(QString str)
{
	bool bOk ;
	int val = str.toInt(&bOk) ;
	if ( !bOk ) { return ; }

	m_Scale = val ;
}

void CLoupeWindow::resizeEvent( QResizeEvent *event )
{
	QSize size = m_pLabel->size() ;
	fixImage(size) ;
}

void CLoupeWindow::fixImage( QSize &size )
{
	int imgWidth = size.width() ;
	int imgHeight = size.height() ;
	QPoint pos = QApplication::desktop()->cursor().pos() ;
	int width = imgWidth / m_Scale ;
	int height = imgHeight / m_Scale ;
	int x = pos.x() - width/2 ;
	int y = pos.y() - height/2 ;
	QPixmap pix = QPixmap::grabWindow(QApplication::desktop()->winId(),
									  x,
									  y,
									  width,
									  height) ;
	QImage image = pix.toImage() ;
	int i, j ;
	// デスクトップ範囲外を黒に。
	for ( i = x ; i < 0 ; i ++ ) {
		for ( j = 0 ; j < height ; j ++ ) {
			image.setPixel(i-x, j, 0);
		}
	}
	for ( i = y ; i < 0 ; i ++ ) {
		for ( j = 0 ; j < width ; j ++ ) {
			image.setPixel(j, i-y, 0);
		}
	}
	for ( i = x+width ; i > QApplication::desktop()->width() ; i -- ) {
		for ( j = 0 ; j < height ; j ++ ) {
			image.setPixel(width-(i-QApplication::desktop()->width()), j, 0);
		}
	}
	for ( i = y+height ; i > QApplication::desktop()->height() ; i -- ) {
		for ( j = 0 ; j < width ; j ++ ) {
			image.setPixel(j, height-(i-QApplication::desktop()->height()), 0);
		}
	}

	// 中心
	for ( i = width/2-3 ; i <= width/2+3 ; i ++ ) {
		image.setPixel(i, height/2, QColor(255, 0, 0).rgba()) ;
	}
	for ( i = height/2-3 ; i <= height/2+3 ; i ++ ) {
		image.setPixel(width/2, i, QColor(255, 0, 0).rgba()) ;
	}

	image = image.scaled(imgWidth, imgHeight) ;
	pix = QPixmap::fromImage(image) ;
	m_pLabel->setPixmap(pix);
}

