#include "optiondialog.h"

OptionDialog::OptionDialog(CSettings *pSetting, QWidget *parent) :
    QDialog(parent)
{
	FileTab *pFileTab = new FileTab(pSetting) ;
	AnimeWindowTab *pAnimeTab = new AnimeWindowTab(pSetting) ;
	ImageWindowTab *pImageTab = new ImageWindowTab(pSetting) ;

	QTabWidget *pTabWidget = new QTabWidget(this) ;
	pTabWidget->addTab(pFileTab, trUtf8("ファイル")) ;
	pTabWidget->addTab(pAnimeTab, trUtf8("Animation Window")) ;
	pTabWidget->addTab(pImageTab, trUtf8("Image Window")) ;

	QBoxLayout *p = new QBoxLayout(QBoxLayout::TopToBottom, this) ;
	p->addWidget(pTabWidget);

	setWindowTitle(trUtf8("オプション"));
}

// ファイル タブ
FileTab::FileTab(CSettings *pSetting, QWidget *parent)
	: QWidget(parent)
{
	m_pSetting = pSetting ;

	QCheckBox *pCheckBox = new QCheckBox(trUtf8("XML形式で保存時、画像データを保存する"), this) ;
	pCheckBox->setChecked(pSetting->getSaveImage());
	connect(pCheckBox, SIGNAL(clicked(bool)), this, SLOT(slot_clickedCheckBox(bool))) ;

	QVBoxLayout *pLayout = new QVBoxLayout(this) ;
	pLayout->addWidget(pCheckBox);
}

void FileTab::slot_clickedCheckBox(bool flag)
{
	m_pSetting->setSaveImage(flag) ;
}

// アニメーションウィンドウ タブ
AnimeWindowTab::AnimeWindowTab(CSettings *pSetting, QWidget *parent)
	: QWidget(parent)
{
	m_pSetting = pSetting ;

	QLabel *pLabel = new QLabel(trUtf8("BG Color")) ;

	QComboBox *pComboBox = new QComboBox(this) ;
	QStringList colors = QColor::colorNames() ;
	for ( int i = 0 ; i < colors.size() ; i ++ ) {
		QColor color(colors[i]) ;

		pComboBox->insertItem(i, colors[i]);
		pComboBox->setItemData(i, color, Qt::DecorationRole);
		if ( color == pSetting->getAnimeBGColor() ) {
			pComboBox->setCurrentIndex(i);
		}
	}
	connect(pComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_changeBGColor(QString))) ;

	QGridLayout *pLayout = new QGridLayout(this) ;
	pLayout->addWidget(pLabel, 0, 0);
	pLayout->addWidget(pComboBox, 0, 1);
}

void AnimeWindowTab::slot_changeBGColor(QString colorName)
{
	QColor color(colorName) ;
	m_pSetting->setAnimeBGColor(color) ;
}

// イメージウィンドウ タブ
ImageWindowTab::ImageWindowTab(CSettings *pSetting, QWidget *parent)
{
	m_pSetting = pSetting ;

	QLabel *pLabel = new QLabel(trUtf8("BG Color")) ;

	QComboBox *pComboBox = new QComboBox(this) ;
	QStringList colors = QColor::colorNames() ;
	for ( int i = 0 ; i < colors.size() ; i ++ ) {
		QColor color(colors[i]) ;

		pComboBox->insertItem(i, colors[i]);
		pComboBox->setItemData(i, color, Qt::DecorationRole);
		if ( color == pSetting->getImageBGColor() ) {
			pComboBox->setCurrentIndex(i);
		}
	}
	connect(pComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_changeBGColor(QString))) ;

	QGridLayout *pLayout = new QGridLayout(this) ;
	pLayout->addWidget(pLabel, 0, 0);
	pLayout->addWidget(pComboBox, 0, 1);
}

void ImageWindowTab::slot_changeBGColor(QString colorName)
{
	QColor color(colorName) ;
	m_pSetting->setImageBGColor(color) ;
}





