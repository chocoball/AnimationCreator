#include <QGridLayout>
#include <QComboBox>
#include "cloupewindow.h"
#include "mainwindow.h"

CLoupeWindow::CLoupeWindow(CEditImageData *pEditImageData, MainWindow *pMainWindow, QWidget *parent) :
    QWidget(parent)
{
	m_pEditImageData = pEditImageData ;
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
//	setMinimumSize(200, 200);
	setFixedSize(200, 200);
	setMaximumSize(200, 200);
}

void CLoupeWindow::slot_cursorScreenShort()
{
	if ( m_pEditImageData->isDraggingImage() ) {
		return ;
	}

	int rowSize = 200/m_Scale ;
	QPoint pos = QApplication::desktop()->cursor().pos() ;
	QPixmap pix = QPixmap::grabWindow(QApplication::desktop()->winId(), pos.x()-rowSize/2-(rowSize%2), pos.y()-rowSize/2-(rowSize%2), rowSize+(rowSize%2), rowSize+(rowSize%2)) ;
	int i, j ;
	QImage image = pix.scaled(QSize(200, 200)).toImage() ;
	for ( i = 0 ; i < -(pos.x()-50)*2 ; i ++ ) {
		for ( j = 0 ; j < 200 ; j ++ ) {
			image.setPixel(i, j, 0);
		}
	}
	for ( i = 0 ; i < -(pos.y()-50)*2 ; i ++ ) {
		for ( j = 0 ; j < 200 ; j ++ ) {
			image.setPixel(j, i, 0);
		}
	}

	for ( i = 199 ; i >= (200-(pos.x()+50-QApplication::desktop()->width())*2) ; i -- ) {
		for ( j = 0 ; j < 200 ; j ++ ) {
			image.setPixel(i, j, 0);
		}
	}
	for ( i = 199 ; i >= (200-(pos.y()+50-QApplication::desktop()->height())*2) ; i -- ) {
		for ( j = 0 ; j < 200 ; j ++ ) {
			image.setPixel(j, i, 0);
		}
	}

	for ( i = 100-m_Scale*3 ; i <= 100+m_Scale*3 ; i ++ ) {
		for ( j = 0 ; j < m_Scale ; j ++ ) {
			image.setPixel(i, 100+j, QColor(255, 0, 0).rgba()) ;
			image.setPixel(100+j, i, QColor(255, 0, 0).rgba()) ;
		}
	}

	pix = QPixmap::fromImage(image) ;
	m_pLabel->setPixmap(pix);
}

void CLoupeWindow::slot_changeScale(QString str)
{
	bool bOk ;
	int val = str.toInt(&bOk) ;
	if ( !bOk ) { return ; }

	m_Scale = val ;
}
