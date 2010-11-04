#include <QPixmap>
#include <QGraphicsView>
#include "imagewindow.h"
#include "gridlabel.h"

ImageWindow::ImageWindow(CSettings *p, CEditImageData *pEditImage, QWidget *parent)
	: QWidget(parent)
{
	m_pSetting = p ;
	m_pEditImageData = pEditImage ;

	QCheckBox *pCheckBox = new QCheckBox(trUtf8("グリッド"), this) ;
	pCheckBox->setChecked(true);

	m_pImageLabel = new QLabel(this) ;
	m_pImageLabel->setPixmap(QPixmap::fromImage(m_pEditImageData->getImage()));
	m_pImageLabel->setScaledContents(true);

	m_pGridLabel = new CGridLabel(m_pEditImageData, m_pImageLabel) ;
	m_pGridLabel->show();

	m_pScrollArea = new QScrollArea(this) ;
	m_pScrollArea->setWidget(m_pImageLabel);

	QGridLayout *layout = new QGridLayout ;
	layout->addWidget(pCheckBox, 0, 0);
	layout->addWidget(m_pScrollArea, 1, 0, 1, 3) ;
	setLayout(layout) ;

	m_Scale = 1 ;

	connect(pCheckBox, SIGNAL(clicked(bool)), m_pGridLabel, SLOT(slot_gridOnOff(bool))) ;

	setWindowTitle(tr("Image Window")) ;
}

ImageWindow::~ImageWindow()
{
	QPoint Pos = pos() ;
	QSize Size = size() ;
	m_pSetting->setImgWinPos(Pos) ;
	m_pSetting->setImgWinSize(Size) ;
}

