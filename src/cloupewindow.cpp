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

//	m_pLabelCheck = new QLabel(this) ;
//	m_pLabelCheck->setText(trUtf8("カーソルの移動にあわせる"));

	m_pCheckBox_Cursor = new QCheckBox(this) ;
	m_pCheckBox_Cursor->setChecked(true);
	m_pCheckBox_Cursor->setText(trUtf8("カーソルの移動にあわせる"));

	m_pCheckBox_Center = new QCheckBox(this) ;
	m_pCheckBox_Center->setChecked(true);
	m_pCheckBox_Center->setText(trUtf8("センター表示"));

	QGridLayout *layout = new QGridLayout(this) ;
	layout->addWidget(m_pCheckBox_Cursor, 0, 0);
	layout->addWidget(m_pCheckBox_Center, 0, 1);
	layout->addWidget(pLabelScale, 1, 0);
	layout->addWidget(pComboBox, 1, 1);
	layout->addWidget(m_pLabel, 2, 0, 2, 2) ;
	setLayout(layout);

	m_pTimer = new QTimer(this) ;
	m_pTimer->setInterval(100/3);
	m_pTimer->start();

	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_cursorScreenShort())) ;
	connect(pComboBox, SIGNAL(activated(QString)), this, SLOT(slot_changeScale(QString))) ;

	m_Scale = 2 ;

	setWindowTitle(trUtf8("ルーペ"));

	m_CenterPos = QApplication::desktop()->cursor().pos() ;
}

void CLoupeWindow::toggleLock( void )
{
	m_pCheckBox_Cursor->setChecked( !m_pCheckBox_Cursor->isChecked() );
}

void CLoupeWindow::slot_cursorScreenShort()
{
	if ( m_pEditData->getDraggingImage() ) {
		return ;
	}

	if ( m_pCheckBox_Cursor->isChecked() ) {
		m_CenterPos = QApplication::desktop()->cursor().pos() ;
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

void CLoupeWindow::resizeEvent( QResizeEvent */*event*/ )
{
	QSize size = m_pLabel->size() ;
	fixImage(size) ;
}

void CLoupeWindow::fixImage( QSize &size )
{
	int imgWidth = size.width() ;
	int imgHeight = size.height() ;
	QPoint pos = m_CenterPos ;
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
	QPoint cursorPos = QPoint(0, 0) ;
	if ( !m_pCheckBox_Cursor->isChecked() ) {
		cursorPos = pos - QApplication::desktop()->cursor().pos() ;
	}

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
	if ( m_pCheckBox_Center->isChecked() ) {
		for ( i = width/2-3-cursorPos.x() ; i <= width/2+3-cursorPos.x() ; i ++ ) {
			j = height/2 - cursorPos.y() ;
			if ( i >= 0 && i < width && j >= 0 && j < height ) {
				image.setPixel(i, j, QColor(255, 0, 0).rgba()) ;
			}
		}
		for ( i = height/2-3-cursorPos.y() ; i <= height/2+3-cursorPos.y() ; i ++ ) {
			j = width/2 - cursorPos.x() ;
			if ( i >= 0 && i < height && j >= 0 && j < width ) {
				image.setPixel(j, i, QColor(255, 0, 0).rgba()) ;
			}
		}
	}

	image = image.scaled(imgWidth, imgHeight) ;
	pix = QPixmap::fromImage(image) ;
	m_pLabel->setPixmap(pix);
}

