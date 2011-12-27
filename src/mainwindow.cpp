#include <qglobal.h>
#include <QtGui>
//#include <QtNetwork/QNetworkAccessManager>
#include "mainwindow.h"
#include "imagewindow.h"
#include "canm2d.h"
#include "optiondialog.h"
#include "util.h"
#include "colorpickerform.h"

#define kExecName	"Animation Creator2"

#define FILE_EXT_ANM2D_XML	".xml"
#define FILE_EXT_ANM2D_BIN	".anm2"
#define FILE_EXT_JSON		".json"

#define kVersion	"2.0.0"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	m_pSubWindow_Anm = NULL ;
	m_pSubWindow_Img = NULL ;
	m_pSubWindow_Loupe = NULL ;
	m_pSubWindow_Curve = NULL ;

	m_pMdiArea = new CDropableMdiArea ;
	m_pMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(m_pMdiArea);

	createActions() ;
	createMenus() ;

	readRootSetting() ;

	setWindowTitle(tr(kExecName));
	setUnifiedTitleAndToolBarOnMac(true);

	connect(m_pMdiArea, SIGNAL(dropFiles(QString)), this, SLOT(slot_dropFiles(QString))) ;
	QUndoStack *pUndoStack = m_EditData.getUndoStack() ;
	connect(pUndoStack, SIGNAL(indexChanged(int)), this, SLOT(slot_checkDataModified(int))) ;

	m_UndoIndex = 0 ;

	m_pImageWindow = NULL ;
	m_pLoupeWindow = NULL ;
	m_pAnimationForm = NULL ;
	m_pExportPNGForm = NULL ;
	m_pCurveEditorForm = NULL ;

	setObjectName("AnimationCreator MainWindow");
/*
	QNetworkAccessManager *pManager = new QNetworkAccessManager(this) ;
	connect(pManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_reqFinished(QNetworkReply *))) ;
	pManager->get(QNetworkRequest(QUrl("http://chocobowl.biz:8800/AnimationCreator/version.txt"))) ;
*/

}

MainWindow::~MainWindow()
{
	delete m_pMdiArea ;
}

// ウィンドウ閉じイベント
void MainWindow::closeEvent(QCloseEvent *event)
{
	printf( "closeEvent\n" ) ;
	if ( !checkChangedFileSave() ) {
		event->ignore();
		return ;
	}

	m_pMdiArea->closeAllSubWindows();
	if ( m_pMdiArea->currentSubWindow() ) {
		event->ignore() ;
		return ;
	}

	writeRootSetting() ;
	event->accept() ;
}

// キー押しイベント
void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if ( event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_L ) {
		m_pLoupeWindow->toggleLock() ;
	}
}

void MainWindow::enterEvent(QEvent *)
{
	checkFileModified() ;
}

// ファイルオープン
void MainWindow::slot_open( void )
{
	if ( !checkChangedFileSave() ) {
		// キャンセルした
		return ;
	}

	QString fileName = QFileDialog::getOpenFileName(
											this,
											tr("Open File"),
											setting.getOpenDir(),
											tr("All Files (*);;Image Files (*.png *.bmp *.jpg);;Text (*"FILE_EXT_ANM2D_XML");;Bin (*"FILE_EXT_ANM2D_BIN")")) ;
	if ( fileName.isEmpty() ) {
		return ;
	}

	fileOpen(fileName) ;
}

// 上書き保存
void MainWindow::slot_save( void )
{
	if ( m_StrSaveFileName.isEmpty() ) {
		slot_saveAs();
	}
	else {
		saveFile(m_StrSaveFileName) ;
	}
}

// 名前を付けて保存
void MainWindow::slot_saveAs( void )
{
	QString str = QFileDialog::getSaveFileName(this,
											   trUtf8("名前を付けて保存"),
											   setting.getSaveDir(),
											   tr("Text (*"FILE_EXT_ANM2D_XML");;Bin (*"FILE_EXT_ANM2D_BIN")")) ;
	if ( str.isEmpty() ) { return ; }

	if ( saveFile(str) ) {
		m_StrSaveFileName = str ;

		QString tmp(str) ;
		int index = tmp.lastIndexOf( '/' ) ;
		if ( index < 0 ) { return ; }

		tmp.remove( index + 1, tmp.size() ) ;
		setting.setSaveDir(tmp);
	}
}

// ファイルドロップされたら
void MainWindow::slot_dropFiles(QString fileName)
{
	if ( !checkChangedFileSave() ) {
		// キャンセルした
		return ;
	}

	fileOpen(fileName) ;
}

// データを編集したか
void MainWindow::slot_checkDataModified(int index)
{
	if ( m_UndoIndex == index ) {
		setWindowTitle(tr(kExecName"[%1]").arg(m_StrSaveFileName));
	}
	else {	// 編集してる
		setWindowTitle(tr(kExecName"[%1]*").arg(m_StrSaveFileName));
	}
}

// ヘルプ選択時
void MainWindow::slot_help( void )
{
	HelpWindow *p = new HelpWindow ;
	if ( !p->isLoaded() ) {
		delete p ;
	}
}

// イメージウィンドウOn/Off
void MainWindow::slot_triggeredImageWindow( bool flag )
{
	if ( m_pImageWindow ) {
		m_pImageWindow->setVisible(flag);
	}
}

// ルーペウィンドウOn/Off
void MainWindow::slot_triggeredLoupeWindow( bool flag )
{
	if ( m_pLoupeWindow ) {
		m_pLoupeWindow->setVisible(flag);
	}
}

// アニメーションウィンドウOn/Off
void MainWindow::slot_triggeredAnimeWindow( bool flag )
{
	if ( m_pAnimationForm ) {
		m_pAnimationForm->setVisible(flag);
	}
}

// オプション選択時
void MainWindow::slot_option( void )
{
	OptionDialog dialog(&setting, this) ;
	dialog.exec() ;

	emit sig_endedOption() ;
}

// 連番PNG保存
void MainWindow::slot_exportPNG( void )
{
	if ( !m_pMdiArea->findChild<ExportPNGForm *>(trUtf8("ExportPNGForm")) ) {
		if ( !m_pMdiArea->findChild<AnimationForm *>(trUtf8("AnimationForm")) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("アニメーションウィンドウがありません")) ;
			return ;
		}
		m_pExportPNGForm = new ExportPNGForm(&m_EditData, &setting, this) ;
		m_pSubWindow_Expng = m_pMdiArea->addSubWindow( m_pExportPNGForm ) ;
		m_pExportPNGForm->show() ;
		connect(m_pAnimationForm->getGLWidget(), SIGNAL(sig_exportPNGRectChange()), m_pExportPNGForm, SLOT(slot_changeRect())) ;
		connect(m_pExportPNGForm, SIGNAL(sig_changeRect()), m_pAnimationForm->getGLWidget(), SLOT(update())) ;
		connect(m_pExportPNGForm, SIGNAL(sig_startExport()), m_pAnimationForm, SLOT(slot_playAnimation())) ;
		connect(m_pExportPNGForm, SIGNAL(sig_cancel()), this, SLOT(slot_closeExportPNGForm())) ;

		m_pAnimationForm->getGLWidget()->update();
	}
}

// 連番PNG 閉じる
void MainWindow::slot_closeExportPNGForm( void )
{
	m_pMdiArea->removeSubWindow(m_pSubWindow_Expng);
	delete m_pExportPNGForm ;
	m_pExportPNGForm = NULL ;
	m_pSubWindow_Expng = NULL ;
}

void MainWindow::slot_portCheckDrawCenter(bool flag)
{
	emit sig_portCheckDrawCenter(flag) ;
}

void MainWindow::slot_portDragedImage(FrameData data)
{
	emit sig_portDragedImage(data) ;
}

void MainWindow::slot_pushColorToolButton( void )
{
	if ( !m_pMdiArea->findChild<ColorPickerForm *>(trUtf8("ColorPickerForm")) ) {
		ColorPickerForm *p = new ColorPickerForm(&m_EditData, m_pMdiArea) ;
		m_pMdiArea->addSubWindow( p ) ;
		p->show();
		connect(p, SIGNAL(sig_setColorToFrameData(QRgb)), m_pAnimationForm, SLOT(slot_setColorFromPicker(QRgb))) ;
	}
}

void MainWindow::slot_destroyAnmWindow( void )
{
	if ( m_pSubWindow_Anm ) {
		qDebug("destroyAnmWindow save setting");
		setting.setAnmWindowGeometry(m_pSubWindow_Anm->saveGeometry()) ;
	}
	m_pSubWindow_Anm = NULL ;
}

void MainWindow::slot_destroyImgWindow( void )
{
	if ( m_pSubWindow_Img ) {
		qDebug("destroyImgWindow save setting");
		setting.setImgWindowGeometry(m_pSubWindow_Img->saveGeometry()) ;
	}
	m_pSubWindow_Img = NULL ;
}

void MainWindow::slot_destroyLoupeWindow( void )
{
	if ( m_pSubWindow_Loupe ) {
		qDebug("destroyLoupeWindow save setting");
		setting.setLoupeWindowGeometry(m_pSubWindow_Loupe->saveGeometry()) ;
	}
	m_pSubWindow_Loupe = NULL ;
}

void MainWindow::slot_destroyCurveWindow(void)
{
	if ( m_pSubWindow_Curve ) {
		qDebug("destroyCurveWindow save setting");
		setting.setCurveWindowGeometry(m_pSubWindow_Curve->saveGeometry()) ;
	}
	m_pSubWindow_Curve = NULL ;
}

void MainWindow::slot_reqFinished(QNetworkReply *reply)
{
	if ( reply->error() ) {
		qDebug() << "request error:" << reply->errorString() ;
		return ;
	}

	QString str = reply->readAll() ;
	qDebug() << "version:" << str ;
	if ( str == kVersion ) {
		return ;
	}
}

// JSON 吐き出し
void MainWindow::slot_exportJSON()
{
	QString fileName = QFileDialog::getSaveFileName(this,
													trUtf8("名前を付けて保存"),
													setting.getSaveJsonDir(),
													tr("JSON (*.json)")) ;

	if ( fileName.isEmpty() ) { return ; }

	setting.setSaveJsonDir(fileName) ;

	CAnm2DJson data(setting.getFlat()) ;
	if ( !data.makeFromEditData(m_EditData) ) {
		if ( data.getErrorNo() != CAnm2DBase::kErrorNo_Cancel ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
		}
		return ;
	}

	QFile file(fileName) ;
	if ( !file.open(QFile::WriteOnly) ) {
		QMessageBox::warning(this, trUtf8("エラー"), trUtf8("保存失敗 %1:\n%2").arg(fileName).arg(file.errorString())) ;
		return ;
	}
	file.write(data.getData().toAscii()) ;

	QMessageBox msgBox ;
	msgBox.setText(trUtf8("JSON吐き出し終了:")+fileName) ;
	msgBox.exec() ;
}

// asm 吐き出し
void MainWindow::slot_exportASM()
{
	QString	fileName = QFileDialog::getSaveFileName(this, trUtf8("名前を付けて保存"), setting.getSaveAsmDir(), tr("asm (*.asm)"));
	
	if ( fileName.isEmpty() ) { return ; }
	
	setting.setSaveAsmDir(fileName) ;
	
	CAnm2DAsm	data(setting.getFlat());
	if ( !data.makeFromEditData(m_EditData) ) {
		if ( data.getErrorNo() != CAnm2DBase::kErrorNo_Cancel ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
		}
		return ;
	}
	
	QFile file(fileName) ;
	if ( !file.open(QFile::WriteOnly) ) {
		QMessageBox::warning(this, trUtf8("エラー"), trUtf8("保存失敗 %1:\n%2").arg(fileName).arg(file.errorString())) ;
		return ;
	}
	file.write(data.getData().toAscii()) ;
	
	QMessageBox msgBox ;
	msgBox.setText(trUtf8("asm吐き出し終了:")+fileName) ;
	msgBox.exec() ;
}

void MainWindow::slot_changeSelectLayer(QModelIndex index)
{
	emit sig_changeSelectLayer(index) ;
}

#ifndef QT_NO_DEBUG
void MainWindow::slot_dbgObjectDump( void )
{
	if ( !m_pAnimationForm ) { return ; }

	m_pAnimationForm->dbgDumpObject() ;
}

void MainWindow::slot_dbgObjectFlat( void )
{
	m_EditData.getObjectModel()->flat() ;
}
#endif

// 設定を復元
void MainWindow::readRootSetting( void )
{
	setting.read();

	restoreGeometry(setting.getMainWindowGeometry()) ;
	restoreState(setting.getMainWindowState()) ;
}

// 設定を保存
void MainWindow::writeRootSetting( void )
{
	setting.setMainWindowGeometry(saveGeometry()) ;
	setting.setMainWindowState(saveState()) ;

	if ( m_pSubWindow_Anm ) {
		setting.setAnmWindowGeometry(m_pSubWindow_Anm->saveGeometry()) ;
	}
	if ( m_pSubWindow_Img ) {
		setting.setImgWindowGeometry(m_pSubWindow_Img->saveGeometry()) ;
	}
	if ( m_pSubWindow_Loupe ) {
		setting.setLoupeWindowGeometry(m_pSubWindow_Loupe->saveGeometry()) ;
	}
	if ( m_pSubWindow_Curve ) {
		setting.setCurveWindowGeometry(m_pSubWindow_Curve->saveGeometry()) ;
	}

	setting.write();
}

// アクションを作成
void MainWindow::createActions( void )
{
	// 開く
	m_pActOpen = new QAction(trUtf8("&Open..."), this) ;
	m_pActOpen->setShortcuts(QKeySequence::Open) ;
	m_pActOpen->setStatusTip(trUtf8("ファイルを開きます")) ;
	connect(m_pActOpen, SIGNAL(triggered()), this, SLOT(slot_open())) ;

	// 保存
	m_pActSave = new QAction(trUtf8("&Save"), this) ;
	m_pActSave->setShortcuts(QKeySequence::Save) ;
	m_pActSave->setStatusTip(trUtf8("ファイルを保存します")) ;
	connect(m_pActSave, SIGNAL(triggered()), this, SLOT(slot_save())) ;

	// 名前を付けて保存
	m_pActSaveAs = new QAction(trUtf8("Save &As..."), this) ;
	m_pActSaveAs->setShortcuts(QKeySequence::SaveAs) ;
	m_pActSaveAs->setStatusTip(trUtf8("ファイルを保存します")) ;
	connect(m_pActSaveAs, SIGNAL(triggered()), this, SLOT(slot_saveAs())) ;

	// 連番PNG保存
	m_pActExportPNG = new QAction(trUtf8("連番PNG"), this) ;
	connect(m_pActExportPNG, SIGNAL(triggered()), this, SLOT(slot_exportPNG())) ;

	// JSON吐き出し
	m_pActExportJson = new QAction(trUtf8("JSON"), this) ;
	connect(m_pActExportJson, SIGNAL(triggered()), this, SLOT(slot_exportJSON())) ;

	// asm吐き出し
	m_pActExportAsm = new QAction(trUtf8("asm"), this) ;
	connect(m_pActExportAsm, SIGNAL(triggered()), this, SLOT(slot_exportASM())) ;

	// 終了
	m_pActExit = new QAction(trUtf8("E&xit"), this) ;
	m_pActExit->setShortcuts(QKeySequence::Quit);
	m_pActExit->setStatusTip(trUtf8("アプリケーションを終了します"));
	connect(m_pActExit, SIGNAL(triggered()), this, SLOT(close())) ;

	// 戻す
	QUndoStack *pStack = m_EditData.getUndoStack() ;
	m_pActUndo = pStack->createUndoAction(this, trUtf8("&Undo")) ;
	m_pActUndo->setShortcuts(QKeySequence::Undo);

	// やり直す
	m_pActRedo = pStack->createRedoAction(this, trUtf8("&Redo")) ;
	m_pActRedo->setShortcuts(QKeySequence::Redo);

	// イメージウィンドウon/off
	m_pActImageWindow = new QAction(trUtf8("Image Window"), this) ;
	m_pActImageWindow->setEnabled(false);
	m_pActImageWindow->setCheckable(true);
	connect(m_pActImageWindow, SIGNAL(triggered(bool)), this, SLOT(slot_triggeredImageWindow(bool))) ;

	// ルーペウィンドウon/off
	m_pActLoupeWindow = new QAction(trUtf8("Loupe Window"), this) ;
	m_pActLoupeWindow->setEnabled(false);
	m_pActLoupeWindow->setCheckable(true);
	connect(m_pActLoupeWindow, SIGNAL(triggered(bool)), this, SLOT(slot_triggeredLoupeWindow(bool))) ;

	// アニメーションウィンドウon/off
	m_pActAnimeWindow = new QAction(trUtf8("Animeation Window"), this) ;
	m_pActAnimeWindow->setEnabled(false);
	m_pActAnimeWindow->setCheckable(true);
	connect(m_pActAnimeWindow, SIGNAL(triggered(bool)), this, SLOT(slot_triggeredAnimeWindow(bool))) ;

	// オプション
	m_pActOption = new QAction(trUtf8("&Option"), this) ;
	connect(m_pActOption, SIGNAL(triggered()), this, SLOT(slot_option())) ;

	// ヘルプ
	m_pActHelp = new QAction(trUtf8("&Help"), this) ;
	m_pActHelp->setShortcuts(QKeySequence::HelpContents) ;
	connect(m_pActHelp, SIGNAL(triggered()), this, SLOT(slot_help())) ;

	// Qtについて
	m_pActAboutQt = new QAction(trUtf8("About &Qt"), this) ;
	connect(m_pActAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt())) ;

#ifndef QT_NO_DEBUG
	m_pActDbgDump = new QAction(tr("Dump"), this) ;
	connect(m_pActDbgDump, SIGNAL(triggered()), this, SLOT(slot_dbgObjectDump())) ;
	m_pActDbgFlat = new QAction(tr("Flat"), this) ;
	connect(m_pActDbgFlat, SIGNAL(triggered()), this, SLOT(slot_dbgObjectFlat())) ;
#endif
}

// メニューを作成
void MainWindow::createMenus( void )
{
	QMenu *pMenu ;

	pMenu = menuBar()->addMenu(trUtf8("&File")) ;
	pMenu->addAction(m_pActOpen) ;
	pMenu->addAction(m_pActSave) ;
	pMenu->addAction(m_pActSaveAs) ;
	pMenu->addSeparator() ;
	{
		QMenu *p = pMenu->addMenu(trUtf8("Export")) ;
		p->addAction(m_pActExportPNG) ;
		p->addAction(m_pActExportJson) ;
		p->addAction(m_pActExportAsm);
	}
	pMenu->addSeparator() ;
	pMenu->addAction(m_pActExit) ;

	pMenu = menuBar()->addMenu(trUtf8("&Edit")) ;
	pMenu->addAction(m_pActUndo) ;
	pMenu->addAction(m_pActRedo) ;
#if 0
	pMenu = menuBar()->addMenu(trUtf8("&Window")) ;
	pMenu->addAction(m_pActImageWindow) ;
	pMenu->addAction(m_pActLoupeWindow) ;
	pMenu->addAction(m_pActAnimeWindow) ;
#endif
	pMenu = menuBar()->addMenu(trUtf8("Too&ls")) ;
	pMenu->addAction(m_pActOption) ;

	pMenu = menuBar()->addMenu(trUtf8("&Help")) ;
	pMenu->addAction(m_pActHelp) ;
	pMenu->addAction(m_pActAboutQt) ;

#ifndef QT_NO_DEBUG
	pMenu = menuBar()->addMenu(tr("Debug")) ;
	pMenu->addAction(m_pActDbgDump) ;
	pMenu->addAction(m_pActDbgFlat) ;
#endif
}

// 「開く」での現在のディレクトリを設定
void MainWindow::setCurrentDir( QString &fileName )
{
	QString tmp(fileName) ;
	int index = tmp.lastIndexOf( '/' ) ;
	if ( index < 0 ) { return ; }

	tmp.remove( index + 1, tmp.size() ) ;
	setting.setOpenDir(tmp) ;
}

// ウィンドウ達を作成
void MainWindow::createWindows( void )
{
	makeAnimeWindow() ;
	makeImageWindow() ;
	makeLoupeWindow() ;
	makeCurveWindow() ;
}

// imageDataのサイズを2の累乗に修正
void MainWindow::resizeImage( QImage &imageData )
{
	util::resizeImage(imageData) ;
}

// ファイル開く
bool MainWindow::fileOpen( QString fileName )
{
	setCurrentDir( fileName ) ;		// カレントディレクトリを設定

	if ( fileName.toLower().indexOf(".png")		<= 0
	  && fileName.toLower().indexOf(".bmp")		<= 0
	  && fileName.toLower().indexOf(".jpg")		<= 0
	  && fileName.indexOf(FILE_EXT_ANM2D_XML)	<= 0
	  && fileName.indexOf(FILE_EXT_ANM2D_BIN)	<= 0 ) {
		QMessageBox::warning(this, tr("warning"), trUtf8("対応していないファイルです") ) ;
		return false ;
	}

	if ( m_pMdiArea->currentSubWindow() ) {
		slot_destroyAnmWindow();
		slot_destroyImgWindow();
		slot_destroyLoupeWindow();
		slot_destroyCurveWindow() ;
		m_pMdiArea->closeAllSubWindows() ;	// 全部閉じる
	}
	m_EditData.resetData();
	m_UndoIndex = 0 ;

	// XML アニメファイル
	if ( fileName.indexOf(FILE_EXT_ANM2D_XML) > 0 ) {
		CAnm2DXml data(setting.getSaveImage()) ;

		QFile file(fileName) ;
		if ( !file.open(QFile::ReadOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー 0"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			return false ;
		}
		QDomDocument xml ;
		xml.setContent(&file) ;
		data.setFilePath(fileName);
		if ( !data.makeFromFile(xml, m_EditData) ) {
			QMessageBox::warning(this, trUtf8("エラー 1"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorString()) )  ;
			return false ;
		}
		m_StrSaveFileName = fileName ;
	}
	// バイナリ アニメファイル
	else if ( fileName.indexOf(FILE_EXT_ANM2D_BIN) > 0 ) {
		CAnm2DBin data ;
		QByteArray dataArray ;

		QFile file(fileName) ;
		if ( !file.open(QFile::ReadOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー 0"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			return false ;
		}
		QDataStream in(&file) ;
		dataArray.resize(file.size());
		in.readRawData(dataArray.data(), file.size()) ;
		data.setFilePath(fileName);
		if ( !data.makeFromFile(dataArray, m_EditData) ) {
			QMessageBox::warning(this, trUtf8("エラー 1"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorString()) )  ;
			return false ;
		}
		m_StrSaveFileName = fileName ;
	}
	// 画像ファイル
	else {
		CEditData::ImageData data ;
		QImage imageData ;
		if ( !imageData.load(fileName) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			return false ;
		}

		data.Image			= imageData ;
		data.fileName		= fileName ;
		data.nTexObj		= 0 ;
		data.lastModified	= QDateTime::currentDateTimeUtc() ;
		data.nNo			= m_EditData.getImageDataListSize() ;
		m_EditData.addImageData(data);

		m_StrSaveFileName = QString() ;
	}

	for ( int i = 0 ; i < m_EditData.getImageDataListSize() ; i ++ ) {
		CEditData::ImageData *p = m_EditData.getImageData(i) ;
		if ( !p ) { continue ; }
		p->origImageW = p->Image.width() ;
		p->origImageH = p->Image.height() ;
		resizeImage(p->Image);
	}

	setWindowTitle(tr(kExecName"[%1]").arg(m_StrSaveFileName));

	createWindows() ;

	return true ;
}

// ファイル保存
bool MainWindow::saveFile( QString fileName )
{
	qDebug() << "SaveFile:" << fileName ;

	m_EditData.sortFrameDatas() ;

	// XML
	if ( fileName.indexOf(FILE_EXT_ANM2D_XML) > 0 ) {
		CAnm2DXml data(setting.getSaveImage()) ;
		QProgressDialog prog(trUtf8("保存しています"), trUtf8("&Cancel"), 0, 100, this) ;
		prog.setWindowModality(Qt::WindowModal);
		prog.setAutoClose(true);
		data.setProgress(&prog);
		data.setFilePath(fileName);

		QApplication::setOverrideCursor(Qt::WaitCursor);
		if ( !data.makeFromEditData(m_EditData) ) {
			if ( data.getErrorNo() != CAnm2DBase::kErrorNo_Cancel ) {
				QMessageBox::warning(this, trUtf8("エラー"), trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
			}
			QApplication::restoreOverrideCursor();
			return false ;
		}
		prog.setValue(prog.maximum());

		QFile file(fileName) ;
		if ( !file.open(QFile::WriteOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("保存失敗 %1:\n%2").arg(fileName).arg(file.errorString())) ;
			QApplication::restoreOverrideCursor();
			return false ;
		}
		file.write(data.getData().toString(4).toAscii()) ;
		QApplication::restoreOverrideCursor();

		m_UndoIndex = m_EditData.getUndoStack()->index() ;
		setWindowTitle(tr(kExecName"[%1]").arg(fileName));
		return true ;
	}
	// バイナリ
	else if ( fileName.indexOf(FILE_EXT_ANM2D_BIN) > 0 ) {
		qDebug() << "save binary" ;

		CAnm2DBin data ;
		data.setFilePath(fileName);
		QApplication::setOverrideCursor(Qt::WaitCursor) ;
		if ( !data.makeFromEditData(m_EditData) ) {
			if ( data.getErrorNo() != CAnm2DBase::kErrorNo_Cancel ) {
				QMessageBox::warning(this, trUtf8("エラー"),
									 trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
			}
			QApplication::restoreOverrideCursor();
			return false ;
		}
		QFile file(fileName) ;
		if ( !file.open(QFile::WriteOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"),
								 trUtf8("保存失敗 %1:\n%2").arg(fileName).arg(file.errorString())) ;
			QApplication::restoreOverrideCursor();
			return false ;
		}
		QDataStream out(&file) ;
		out.writeRawData(data.getData().data(), data.getData().size()) ;

		QApplication::restoreOverrideCursor();

		m_UndoIndex = m_EditData.getUndoStack()->index() ;
		setWindowTitle(tr(kExecName"[%1]").arg(fileName));
		qDebug() << "save successed" ;
		return true ;
	}

	return false ;
}

// 変更したファイルを保存するか
// キャンセルならfalse
bool MainWindow::checkChangedFileSave( void )
{
	QUndoStack *p = m_EditData.getUndoStack() ;
	if ( p->index() != m_UndoIndex ) {
		QMessageBox::StandardButton reply = QMessageBox::question(this, trUtf8("確認"), trUtf8("現在のファイルを保存しますか？"),
																  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) ;
		if ( reply == QMessageBox::Yes ) {		// 保存する
			slot_save();
			m_UndoIndex = p->index() ;
		}
		else if ( reply == QMessageBox::Cancel ) {	// キャンセル
			return false ;
		}
	}
	return true ;
}

// イメージウィンドウ作成
void MainWindow::makeImageWindow( void )
{
	m_pImageWindow = new ImageWindow( &setting, &m_EditData, m_pAnimationForm, this, m_pMdiArea ) ;
	m_pSubWindow_Img = m_pMdiArea->addSubWindow(m_pImageWindow) ;
	m_pImageWindow->show();

	m_pSubWindow_Img->restoreGeometry(setting.getImgWindowGeometry()) ;

	m_pActImageWindow->setEnabled(true);
	m_pActImageWindow->setChecked(true);

	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pImageWindow, SLOT(slot_modifiedImage(int))) ;
	connect(this, SIGNAL(sig_endedOption()), m_pImageWindow, SLOT(slot_endedOption())) ;
	connect(this, SIGNAL(sig_portCheckDrawCenter(bool)), m_pImageWindow, SLOT(slot_changeDrawCenter(bool))) ;
	connect(this, SIGNAL(sig_portDragedImage(FrameData)), m_pImageWindow, SLOT(slot_dragedImage(FrameData))) ;

	connect(m_pSubWindow_Img, SIGNAL(destroyed()), this, SLOT(slot_destroyImgWindow())) ;
}

// ルーペウィンドウ作成
void MainWindow::makeLoupeWindow( void )
{
	m_pLoupeWindow = new CLoupeWindow(&m_EditData, this, m_pMdiArea) ;
	m_pSubWindow_Loupe = m_pMdiArea->addSubWindow( m_pLoupeWindow ) ;
	m_pLoupeWindow->show();

	m_pSubWindow_Loupe->restoreGeometry(setting.getLoupeWindowGeometry()) ;

	m_pActLoupeWindow->setEnabled(true);
	m_pActLoupeWindow->setChecked(true);

	connect(m_pSubWindow_Loupe, SIGNAL(destroyed()), this, SLOT(slot_destroyLoupeWindow())) ;
}

// アニメーションフォーム作成
void MainWindow::makeAnimeWindow( void )
{
	m_pAnimationForm = new AnimationForm( &m_EditData, &setting, m_pMdiArea ) ;
	m_pSubWindow_Anm = m_pMdiArea->addSubWindow( m_pAnimationForm ) ;
	m_pAnimationForm->show();
	m_pSubWindow_Anm->restoreGeometry(setting.getAnmWindowGeometry()) ;
	m_pAnimationForm->Init() ;

	m_pActAnimeWindow->setEnabled(true);
	m_pActAnimeWindow->setChecked(true);

	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pAnimationForm, SLOT(slot_modifiedImage(int))) ;
	connect(this, SIGNAL(sig_endedOption()), m_pAnimationForm, SLOT(slot_endedOption())) ;
	connect(m_pAnimationForm, SIGNAL(sig_portCheckDrawCenter(bool)), this, SLOT(slot_portCheckDrawCenter(bool))) ;
	connect(m_pAnimationForm, SIGNAL(sig_portDragedImage(FrameData)), this, SLOT(slot_portDragedImage(FrameData))) ;
	connect(m_pAnimationForm, SIGNAL(sig_pushColorToolButton()), this, SLOT(slot_pushColorToolButton())) ;
	connect(m_pAnimationForm, SIGNAL(sig_changeSelectLayer(QModelIndex)), this, SLOT(slot_changeSelectLayer(QModelIndex))) ;

	connect(m_pSubWindow_Anm, SIGNAL(destroyed()), this, SLOT(slot_destroyAnmWindow())) ;
}

// カーブエディタフォーム作成
void MainWindow::makeCurveWindow( void )
{
	m_pCurveEditorForm = new CurveEditorForm(&m_EditData, &setting, m_pMdiArea) ;
	m_pSubWindow_Curve = m_pMdiArea->addSubWindow(m_pCurveEditorForm) ;
	m_pCurveEditorForm->show() ;
	m_pSubWindow_Curve->restoreGeometry(setting.getCurveWindowGeometry()) ;
	m_pCurveEditorForm->setSplitterPos() ;

	connect(this, SIGNAL(sig_changeSelectLayer(QModelIndex)), m_pCurveEditorForm, SLOT(slot_changeSelLayer(QModelIndex))) ;
	connect(m_pSubWindow_Curve, SIGNAL(destroyed()), this, SLOT(slot_destroyCurveWindow())) ;
}

// 読み込んでるイメージデータの最終更新日時をチェック
void MainWindow::checkFileModified( void )
{
	for ( int i = 0 ; i < m_EditData.getImageDataListSize() ; i ++ ) {
		CEditData::ImageData *p = m_EditData.getImageData(i) ;

		QString fullPath = p->fileName ;
		QFileInfo info(fullPath) ;
		if ( !info.isFile() ) { continue ; }
		if ( !info.lastModified().isValid() ) { continue ; }
		if ( info.lastModified().toUTC() <= p->lastModified ) { continue ; }

		QDateTime time = info.lastModified().toUTC() ;
		p->lastModified = time ;

		QMessageBox::StandardButton reply = QMessageBox::question(this,
																  trUtf8("質問"),
																  trUtf8("%1が更新されています。読み込みますか？").arg(info.fileName()),
																  QMessageBox::Yes | QMessageBox::No) ;
		if ( reply != QMessageBox::Yes ) {
			continue ;
		}

		QImage image ;
		if ( !image.load(fullPath) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(info.fileName())) ;
			continue ;
		}
		p->origImageW = image.width() ;
		p->origImageH = image.height() ;
		util::resizeImage(image) ;
		p->Image = image ;
		emit sig_modifiedImageFile(i) ;
	}
}
