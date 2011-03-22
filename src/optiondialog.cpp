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
	: QWidget(parent),
	ui(new Ui::OptionAnimationTab)
{
	ui->setupUi(this);

	m_pSetting = pSetting ;

	QComboBox *pComboBox = ui->comboBox_bgColor ;
	QStringList colors = QColor::colorNames() ;
	for ( int i = 0 ; i < colors.size() ; i ++ ) {
		QColor color(colors[i]) ;

		pComboBox->insertItem(i, colors[i]);
		pComboBox->setItemData(i, color, Qt::DecorationRole);
		if ( color == pSetting->getAnimeBGColor() ) {
			pComboBox->setCurrentIndex(i);
		}
	}

	QCheckBox *pCheckBox = ui->checkBox_useImage ;
	pCheckBox->setChecked(pSetting->getUseBackImage());
	QLineEdit *pTextEdit = ui->lineEdit_imagePath ;
	pTextEdit->setEnabled(pSetting->getUseBackImage());
	pTextEdit->setText(pSetting->getBackImagePath());

	connect(pComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_changeBGColor(QString))) ;
	connect(pCheckBox, SIGNAL(clicked(bool)), this, SLOT(slot_changeUseBackImage(bool))) ;
	connect(ui->pushButton_imagePath, SIGNAL(clicked()), this, SLOT(slot_openFileDialog())) ;
}

AnimeWindowTab::~AnimeWindowTab()
{
	delete ui ;
}

void AnimeWindowTab::slot_changeBGColor(QString colorName)
{
	QColor color(colorName) ;
	m_pSetting->setAnimeBGColor(color) ;
}

void AnimeWindowTab::slot_changeUseBackImage( bool flag )
{
	m_pSetting->setUseBackImage(flag) ;
	ui->lineEdit_imagePath->setEnabled(flag);
}

void AnimeWindowTab::slot_openFileDialog( void )
{
	QString dir = m_pSetting->getBackImagePath() ;
	QString fileName = QFileDialog::getOpenFileName(
											this,
											tr("Open File"),
											dir,
											tr("Image Files (*.png *.bmp *.jpg);;")) ;
	if ( fileName.isEmpty() ) {
		return ;
	}
	m_pSetting->setBackImagePath(fileName) ;
	ui->lineEdit_imagePath->setText(fileName);
}

// イメージウィンドウ タブ
ImageWindowTab::ImageWindowTab(CSettings *pSetting, QWidget *parent)
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





