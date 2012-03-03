#include "optiondialog.h"

OptionDialog::OptionDialog(CSettings *pSetting, AnimeGLWidget *pGlWidget, QWidget *parent) :
    QDialog(parent)
{
	FileTab *pFileTab = new FileTab(pSetting) ;
	AnimeWindowTab *pAnimeTab = new AnimeWindowTab(pSetting, pGlWidget) ;
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
	: QWidget(parent),
	ui(new Ui::OptionFileTab)
{
	ui->setupUi(this);

	m_pSetting = pSetting ;

	ui->checkBox_saveImage->setChecked(pSetting->getSaveImage()) ;
	ui->checkBox_flat->setChecked(pSetting->getFlat());
	ui->checkBox_hierarchyLayer->setChecked(pSetting->getLayerHierarchy());
	ui->checkBox_backup->setChecked(pSetting->getBackup());
	ui->spinBox_backupNum->setValue(pSetting->getBackupNum()) ;
	ui->spinBox_backupNum->setEnabled(pSetting->getBackup());

	connect(ui->checkBox_saveImage, SIGNAL(toggled(bool)), this, SLOT(slot_clickedSaveImage(bool))) ;
	connect(ui->checkBox_flat, SIGNAL(toggled(bool)), this, SLOT(slot_clickedFlat(bool))) ;
	connect(ui->checkBox_hierarchyLayer, SIGNAL(toggled(bool)), this, SLOT(slot_clickedHierarchy(bool))) ;
	connect(ui->checkBox_backup, SIGNAL(toggled(bool)), this, SLOT(slot_clickedBackup(bool))) ;
	connect(ui->spinBox_backupNum, SIGNAL(valueChanged(int)), this, SLOT(slot_changeBackupNum(int))) ;
}

FileTab::~FileTab()
{
	delete ui ;
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

void FileTab::slot_clickedBackup(bool flag)
{
	m_pSetting->setBackup(flag) ;
	ui->spinBox_backupNum->setEnabled(flag) ;
}

void FileTab::slot_changeBackupNum(int num)
{
	m_pSetting->setBackupNum(num) ;
}

// アニメーションウィンドウ タブ
AnimeWindowTab::AnimeWindowTab(CSettings *pSetting, AnimeGLWidget *pGlWidget, QWidget *parent)
	: QWidget(parent),
	ui(new Ui::OptionAnimationTab),
	m_pGlWidget(pGlWidget)
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

	ui->spinBox_scrw->setValue(pSetting->getAnmWindowScreenW()) ;
	ui->spinBox_scrh->setValue(pSetting->getAnmWindowScreenH()) ;
	ui->spinBox_winw->setValue(pSetting->getAnmWindowW()) ;
	ui->spinBox_winh->setValue(pSetting->getAnmWindowH()) ;
	ui->checkBox_useDepthTest->setChecked(pSetting->getUseDepthTest()) ;
	ui->checkBox_useZSort->setChecked(pSetting->getUseZSort()) ;

	connect(pComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_changeBGColor(QString))) ;
	connect(pCheckBox, SIGNAL(toggled(bool)), this, SLOT(slot_changeUseBackImage(bool))) ;
	connect(ui->pushButton_imagePath, SIGNAL(clicked()), this, SLOT(slot_openFileDialog())) ;
	connect(ui->spinBox_scrw, SIGNAL(valueChanged(int)), this, SLOT(slot_changeScreenW(int))) ;
	connect(ui->spinBox_scrh, SIGNAL(valueChanged(int)), this, SLOT(slot_changeScreenH(int))) ;
	connect(ui->spinBox_winw, SIGNAL(valueChanged(int)), this, SLOT(slot_changeWindowW(int))) ;
	connect(ui->spinBox_winh, SIGNAL(valueChanged(int)), this, SLOT(slot_changeWindowH(int))) ;
	connect(ui->checkBox_useDepthTest, SIGNAL(toggled(bool)), this, SLOT(slot_changeUseDepthTest(bool))) ;
	connect(ui->checkBox_useZSort, SIGNAL(toggled(bool)), this, SLOT(slot_changeUseZSort(bool))) ;
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

void AnimeWindowTab::slot_changeWindowW(int val)
{
	m_pSetting->setAnmWindowW(val) ;
}

void AnimeWindowTab::slot_changeWindowH(int val)
{
	m_pSetting->setAnmWindowH(val) ;
}

void AnimeWindowTab::slot_changeUseDepthTest(bool flag)
{
	m_pSetting->setUseDepthTest(flag) ;
	if ( m_pGlWidget ) {
		m_pGlWidget->update();
	}
}

void AnimeWindowTab::slot_changeUseZSort(bool flag)
{
	m_pSetting->setUseZSort(flag) ;
	if ( m_pGlWidget ) {
		m_pGlWidget->update();
	}
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
	list << trUtf8("PATH 選択") << m_pSetting->getShortcutPathSelect().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ コピー") << m_pSetting->getShortcutCopyFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ ペースト") << m_pSetting->getShortcutPasteFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アニメーション開始/一時停止") << m_pSetting->getShortcutPlayAnime().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アニメーション停止") << m_pSetting->getShortcutStopAnime().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("開始フレームに移動") << m_pSetting->getShortcutJumpStartFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("最終フレームに移動") << m_pSetting->getShortcutJumpEndFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ追加") << m_pSetting->getShortcutAddFrameData().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("フレームデータ削除") << m_pSetting->getShortcutDelFrameData().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アイテム削除") << m_pSetting->getShortcutDelItem().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アイテム表示") << m_pSetting->getShortcutDispItem().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アイテムロック") << m_pSetting->getShortcutLockItem().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("アニメーションウィンドウ移動") << m_pSetting->getShortcutMoveAnimeWindow().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("ルーペウィンドウ ロック") << m_pSetting->getShortcutLockLoupe().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("全レイヤのフレームデータ コピー") << m_pSetting->getShortcutCopyAllFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("全レイヤのフレームデータ ペースト") << m_pSetting->getShortcutPasteAllFrame().toString() ;
	datas << list ;
	list.clear() ;
	list << trUtf8("全レイヤのフレームデータ 削除") << m_pSetting->getShortcutDeleteAllFrame().toString() ;
	datas << list ;
	list.clear() ;

	return datas ;
}

void KeyboardTab::setShortcut(int type, QKeySequence ks)
{
	switch ( type ) {
		case  0:	m_pSetting->setShortcutPosSelect(ks) ;		break ;	// POS
		case  1:	m_pSetting->setShortcutRotSelect(ks) ;		break ;	// ROT
		case  2:	m_pSetting->setShortcutCenterSelect(ks) ;	break ;	// CENTER
		case  3:	m_pSetting->setShortcutScaleSelect(ks) ;	break ;	// SCALE
		case  4:	m_pSetting->setShortcutPathSelect(ks) ;		break ;	// PATH
		case  5:	m_pSetting->setShortcutCopyFrame(ks) ;		break ;	// framedata copy
		case  6:	m_pSetting->setShortcutPasteFrame(ks) ;		break ;	// framedata paste
		case  7:	m_pSetting->setShortcutPlayAnime(ks) ;		break ;	// play animation
		case  8:	m_pSetting->setShortcutStopAnime(ks) ;		break ;	// stop animation
		case  9:	m_pSetting->setShortcutJumpStartFrame(ks) ;	break ;	// jump start frame
		case 10:	m_pSetting->setShortcutJumpEndFrame(ks) ;	break ;	// jump end frame
		case 11:	m_pSetting->setShortcutAddFrameData(ks) ;	break ;	// add framedata
		case 12:	m_pSetting->setShortcutDelFrameData(ks) ;	break ;	// del framedata
		case 13:	m_pSetting->setShortcutDelItem(ks) ;		break ;	// del item
		case 14:	m_pSetting->setShortcutDispItem(ks) ;		break ;	// disp item
		case 15:	m_pSetting->setShortcutLockItem(ks) ;		break ;	// lock item
		case 16:	m_pSetting->setShortcutMoveAnimeWindow(ks) ;break ;	// move animation window
		case 17:	m_pSetting->setShortcutLockLoupe(ks) ;		break ;	// lock loupe window
		case 18:	m_pSetting->setShortcutCopyAllFrame(ks) ;	break ;
		case 19:	m_pSetting->setShortcutPasteAllFrame(ks) ;	break ;
		case 20:	m_pSetting->setShortcutDeleteAllFrame(ks) ;	break ;
	}

	ui->lineEdit->setText(ks.toString()) ;
	m_pKeyModel->setData(m_selIndex, ks.toString()) ;
	ui->treeView->update(m_selIndex) ;
}


