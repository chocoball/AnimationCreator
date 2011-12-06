#include "optiondialog.h"

OptionDialog::OptionDialog(CSettings *pSetting, QWidget *parent) :
    QDialog(parent)
{
	FileTab *pFileTab = new FileTab(pSetting) ;
	AnimeWindowTab *pAnimeTab = new AnimeWindowTab(pSetting) ;
	ImageWindowTab *pImageTab = new ImageWindowTab(pSetting) ;
	KeyboardTab *pKeyboardTab = new KeyboardTab(pSetting) ;

	QTabWidget *pTabWidget = new QTabWidget(this) ;
	pTabWidget->addTab(pFileTab, trUtf8("ファイル")) ;
	pTabWidget->addTab(pAnimeTab, trUtf8("Animation Window")) ;
	pTabWidget->addTab(pImageTab, trUtf8("Image Window")) ;
	pTabWidget->addTab(pImageTab, trUtf8("Image Window")) ;
	pTabWidget->addTab(pKeyboardTab, trUtf8("キーボード")) ;

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
	connect(pCheckBox, SIGNAL(clicked(bool)), this, SLOT(slot_clickedSaveImage(bool))) ;

	QCheckBox *pCheckBox2 = new QCheckBox(trUtf8("JSON形式で保存時、レイヤの親子関係をなくす"), this) ;
	pCheckBox2->setChecked(pSetting->getFlat());
	connect(pCheckBox2, SIGNAL(clicked(bool)), this, SLOT(slot_clickedFlat(bool))) ;

	QCheckBox *pCheckBox3 = new QCheckBox(trUtf8("レイヤに階層を持たせる"), this) ;
	pCheckBox3->setChecked(pSetting->getLayerHierarchy());
	connect(pCheckBox3, SIGNAL(clicked(bool)), this, SLOT(slot_clickedHierarchy(bool))) ;

	QVBoxLayout *pLayout = new QVBoxLayout(this) ;
	pLayout->addWidget(pCheckBox);
	pLayout->addWidget(pCheckBox2);
	pLayout->addWidget(pCheckBox3);
}

void FileTab::slot_clickedSaveImage(bool flag)
{
	m_pSetting->setSaveImage(flag) ;
}

void FileTab::slot_clickedFlat(bool flag)
{
	m_pSetting->setFlat(flag) ;
}

void FileTab::slot_clickedHierarchy(bool flag)
{
	m_pSetting->setLayerHierarchy(flag) ;
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
	connect(ui->spinBox_scrw, SIGNAL(valueChanged(int)), this, SLOT(slot_changeScreenW(int))) ;
	connect(ui->spinBox_scrh, SIGNAL(valueChanged(int)), this, SLOT(slot_changeScreenH(int))) ;
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

void AnimeWindowTab::slot_changeScreenW(int val)
{
	m_pSetting->setAnmWindowScreenW(val) ;
}

void AnimeWindowTab::slot_changeScreenH(int val)
{
	m_pSetting->setAnmWindowScreenH(val) ;
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

// キーボードタブ
KeyboardTab::KeyboardTab(CSettings *pSetting, QWidget *parent)
	: QWidget(parent),
	ui(new Ui::KeyboardTab)
{
	ui->setupUi(this) ;
	m_pSetting = pSetting ;
	m_bShift = m_bCtrl = m_bAlt = false ;

	QList<QStringList> datas = getData() ;
	m_pKeyModel = new KeyboardModel(datas, this) ;
	ui->treeView->setModel(m_pKeyModel) ;

	ui->lineEdit->setEnabled(false) ;
	ui->lineEdit->installEventFilter(this) ;

	connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_treeClicked(QModelIndex))) ;
	connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(slot_pushDelButton())) ;
}

KeyboardTab::~KeyboardTab()
{
	delete ui ;
}

void KeyboardTab::slot_treeClicked(QModelIndex index)
{
	m_selIndex = m_pKeyModel->index(index.row(), 1) ;

	ui->lineEdit->setEnabled(true) ;
	ui->lineEdit->setText(m_pKeyModel->data(m_selIndex, Qt::DisplayRole).toString()) ;
}

void KeyboardTab::slot_pushDelButton()
{
	if ( !m_selIndex.isValid() ) { return ; }
	setShortcut(m_selIndex.row(), QKeySequence(0)) ;
}

void KeyboardTab::handleKeyEvent(QKeyEvent *event)
{
	if ( !m_selIndex.isValid() ) { return ; }
	int key = event->key() ;
	if ( key == Qt::Key_Control
		 || key == Qt::Key_Shift
		 || key == Qt::Key_Meta
		 || key == Qt::Key_Alt ) {
		return ;
	}
	if ( event->modifiers() & Qt::ShiftModifier ) { key |= Qt::SHIFT ; }
	if ( event->modifiers() & Qt::ControlModifier ) { key |= Qt::CTRL ; }
	if ( event->modifiers() & Qt::MetaModifier ) { key |= Qt::META ; }
	if ( event->modifiers() & Qt::AltModifier ) { key |= Qt::ALT ; }

	setShortcut(m_selIndex.row(), QKeySequence(key)) ;
}

bool KeyboardTab::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *k = static_cast<QKeyEvent*>(e);
		handleKeyEvent(k);
		return true;
	}

	if ( e->type() == QEvent::Shortcut ||
		 e->type() == QEvent::KeyRelease ) {
		return true;
	}

	if (e->type() == QEvent::ShortcutOverride) {
		// for shortcut overrides, we need to accept as well
		e->accept();
		return true;
	}

	return false;

}

QList<QStringList> KeyboardTab::getData()
{
	QList<QStringList> datas ;
	QStringList list ;

	list << trUtf8("POS 選択") << m_pSetting->getShortcutPosSelect().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("ROT 選択") << m_pSetting->getShortcutRotSelect().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("CENTER 選択") << m_pSetting->getShortcutCenterSelect().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("SCALE 選択") << m_pSetting->getShortcutScaleSelect().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ コピー") << m_pSetting->getShortcutCopyFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ ペースト") << m_pSetting->getShortcutPasteFrame().toString() ;
	datas << list ;
	list.clear() ;

	return datas ;
}

void KeyboardTab::setShortcut(int type, QKeySequence ks)
{
	switch ( type ) {
		case 0:	// POS
			m_pSetting->setShortcutPosSelect(ks) ;
			break ;
		case 1:	// ROT
			m_pSetting->setShortcutRotSelect(ks) ;
			break ;
		case 2:	// CENTER
			m_pSetting->setShortcutCenterSelect(ks) ;
			break ;
		case 3:	// SCALE
			m_pSetting->setShortcutScaleSelect(ks) ;
			break ;
		case 4:	// framedata copy
			m_pSetting->setShortcutCopyFrame(ks) ;
			break ;
		case 5:	// framedata paste
			m_pSetting->setShortcutPasteFrame(ks) ;
			break ;
	}

	ui->lineEdit->setText(ks.toString()) ;
	m_pKeyModel->setData(m_selIndex, ks.toString()) ;
	ui->treeView->update(m_selIndex) ;
}


