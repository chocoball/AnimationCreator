#include <qglobal.h>
#include <QtGui>
#include "mainwindow.h"
#include "imagewindow.h"
#include "canm2d.h"
#include "optiondialog.h"

#define FILE_EXT_ANM2D_XML	".xml"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	m_pMdiArea = new CDropableMdiArea ;
	m_pMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(m_pMdiArea);

	m_pTimer = new QTimer(this) ;
	m_pTimer->start(1000);

	createActions() ;
	createMenus() ;

	readRootSetting() ;

	setWindowTitle(tr("Animation Creator"));
	setUnifiedTitleAndToolBarOnMac(true);

	connect(m_pMdiArea, SIGNAL(dropFiles(QString)), this, SLOT(slot_dropFiles(QString))) ;
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_checkFileModified())) ;
	QUndoStack *pUndoStack = m_EditData.getUndoStack() ;
	connect(pUndoStack, SIGNAL(indexChanged(int)), this, SLOT(slot_checkDataModified(int))) ;

	m_UndoIndex = 0 ;
	m_bSaveImage = false ;

	m_pImageWindow = NULL ;
	m_pLoupeWindow = NULL ;
	m_pAnimationForm = NULL ;

	setObjectName("AnimationCreator MainWindow");
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
	}
	else {
		writeRootSetting() ;
		event->accept() ;
	}
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
											setting.getCurrentDir(),
											tr("All Files (*);;Image Files (*.png *.bmp *.jpg);;Text Anm Files (*"FILE_EXT_ANM2D_XML")")) ;
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
											   setting.getCurrentSaveDir(),
											   tr("Text Anm Files (*"FILE_EXT_ANM2D_XML")")) ;
	if ( str.isEmpty() ) { return ; }

	if ( saveFile(str) ) {
		m_StrSaveFileName = str ;
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

// 読み込んでるイメージデータの最終更新日時をチェック
void MainWindow::slot_checkFileModified( void )
{
	for ( int i = 0 ; i < m_EditData.getImageDataSize() ; i ++ ) {
		QString fullPath = m_EditData.getImageFileName(i) ;
		QFileInfo info(fullPath) ;
		if ( !info.isFile() ) { continue ; }
		if ( !info.lastModified().isValid() ) { continue ; }
		if ( info.lastModified().toUTC() <= m_EditData.getImageDataLastModified(i) ) { continue ; }

		QDateTime time = info.lastModified().toUTC() ;
		m_EditData.setImageDataLastModified(i, time);

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
		m_EditData.setImage(i, image) ;
		emit sig_modifiedImageFile(i) ;
	}
}

// データを編集したか
void MainWindow::slot_checkDataModified(int index)
{
	if ( m_UndoIndex == index ) {
		setWindowTitle(tr("Animation Creator[%1]").arg(m_StrSaveFileName));
	}
	else {	// 編集してる
		setWindowTitle(tr("Animation Creator[%1]*").arg(m_StrSaveFileName));
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

#ifndef QT_NO_DEBUG
void MainWindow::slot_dbgObjectDump( void )
{
	if ( !m_pAnimationForm ) { return ; }

	m_pAnimationForm->dbgDumpObject() ;
}
#endif
// 設定を復元
void MainWindow::readRootSetting( void )
{
#if 1
	QSettings settings(qApp->applicationDirPath() + "/settnig.ini", QSettings::IniFormat) ;
	qDebug() << "readRootSetting\n" << settings.allKeys() ;
	qDebug() << "file:" << qApp->applicationDirPath() + "/settnig.ini" ;
#else
	QSettings settings("Editor", "rootSettings") ;
#endif
	settings.beginGroup("Global");
	QString dir =
#if defined(Q_OS_WIN32)
	settings.value("cur_dir", QString(".\\")).toString() ;
#elif defined(Q_OS_MAC)
	settings.value("cur_dir", QString("/Users/")).toString() ;
#elif defined(Q_OS_LINUX)
	settings.value("cur_dir", QString("/home/")).toString() ;
#else
	#error OSが定義されてないよ
#endif
	QRgb col ;
	col = settings.value("anime_color", 0).toUInt() ;
	QColor animeCol = QColor(qRed(col), qGreen(col), qBlue(col), qAlpha(col)) ;
	col = settings.value("image_color", 0).toUInt() ;
	QColor imageCol = QColor(qRed(col), qGreen(col), qBlue(col), qAlpha(col)) ;
	bool bSaveImage = settings.value("save_image", false).toBool() ;
	settings.endGroup();

	settings.beginGroup("MainWindow");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint() ;
	QSize size = settings.value("size", QSize(400, 400)).toSize() ;
	settings.endGroup();

	move(pos) ;
	resize(size) ;
	setting.setCurrentDir(dir) ;
	setting.setAnimeBGColor(animeCol);
	setting.setImageBGColor(imageCol);
	setting.setSaveImage(bSaveImage);
}

// 設定を保存
void MainWindow::writeRootSetting( void )
{
#if 1
	QSettings settings(qApp->applicationDirPath() + "/settnig.ini", QSettings::IniFormat) ;
	qDebug() << "writeRootSetting writable:" << settings.isWritable() ;
	qDebug() << "file:" << qApp->applicationDirPath() + "/settnig.ini" ;
#else
	QSettings settings("Editor", "rootSettings") ;
#endif
	settings.beginGroup("Global");
	settings.setValue("cur_dir", setting.getCurrentDir()) ;
	settings.setValue("anime_color", setting.getAnimeBGColor().rgba());
	settings.setValue("image_color", setting.getImageBGColor().rgba());
	settings.setValue("save_image", setting.getSaveImage());
	settings.endGroup();

	settings.beginGroup("MainWindow");
	settings.setValue("pos", pos()) ;
	settings.setValue("size", size()) ;
	settings.endGroup();

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
#endif
}

// 「開く」での現在のディレクトリを設定
void MainWindow::setCurrentDir( QString &fileName )
{
	QString tmp(fileName) ;
	int index = tmp.lastIndexOf( '/' ) ;
	if ( index < 0 ) { return ; }

	tmp.remove( index + 1, tmp.size() ) ;
	setting.setCurrentDir(tmp) ;
}

// ウィンドウ達を作成
void MainWindow::createWindows( void )
{
	makeAnimeWindow() ;
	makeImageWindow() ;
	makeLoupeWindow() ;
}

// imageDataのサイズを2の累乗に修正
void MainWindow::resizeImage( QImage &imageData )
{
	int origW = imageData.width() ;
	int origH = imageData.height() ;
	int fixW = 0, fixH = 0 ;

	int i = 1 ;

	while ( i < 1024 ) {
		if ( origW > i ) {
			i <<= 1 ;
		}
		else {
			fixW = i ;
			break ;
		}
	}
	if ( fixW == 0 ) { fixW = 1024 ; }

	i = 1 ;
	while ( i < 1024 ) {
		if ( origH > i ) {
			i <<= 1 ;
		}
		else {
			fixH = i ;
			break ;
		}
	}
	if ( fixH == 0 ) { fixH = 1024 ; }

	qDebug("orig %d/%d fix %d/%d", origW, origH, fixW, fixH) ;

	if ( fixW == origW && fixH == origH ) { return ; }

	if ( origW > 1024 ) { origW = 1024 ; }
	if ( origH > 1024 ) { origH = 1024 ; }

	QImage tmp = QImage(fixW, fixH, imageData.format()) ;
	for ( int y = 0 ; y < fixH ; y ++ ) {
		for ( int x = 0 ; x < fixW ; x ++ ) {
			tmp.setPixel(x, y, qRgba(0, 0, 0, 0));
		}
	}
	for ( int y = 0 ; y < origH ; y ++ ) {
		for ( int x = 0 ; x < origW ; x ++ ) {
			tmp.setPixel(x, y, imageData.pixel(x, y));
		}
	}
	imageData = tmp ;
}

// ファイル開く
bool MainWindow::fileOpen( QString fileName )
{
	setCurrentDir( fileName ) ;		// カレントディレクトリを設定

	if ( fileName.toLower().indexOf(".png")	<= 0
	  && fileName.toLower().indexOf(".bmp")	<= 0
	  && fileName.toLower().indexOf(".jpg")	<= 0
	  && fileName.indexOf(FILE_EXT_ANM2D_XML)	<= 0 ) {
		QMessageBox::warning(this, tr("warning"), trUtf8("対応していないファイルです") ) ;
		return false ;
	}

	if ( m_pMdiArea->currentSubWindow() ) {
		m_pMdiArea->closeAllSubWindows() ;	// 全部閉じる
	}
	m_EditData.resetData();
	m_UndoIndex = 0 ;

	// アニメファイル
	if ( fileName.indexOf(FILE_EXT_ANM2D_XML) > 0 ) {
		CAnm2DXml data(setting.getSaveImage()) ;
//		QByteArray dataArray ;

		QFile file(fileName) ;
		if ( !file.open(QFile::ReadOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			return false ;
		}
#if 1
		QDomDocument xml ;
		xml.setContent(&file) ;
		data.setFilePath(fileName);
		if ( !data.makeFromFile(xml, m_EditData) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorString()) )  ;
			return false ;
		}
#else
		QDataStream in(&file) ;
		dataArray.resize(file.size());
		in.readRawData(dataArray.data(), file.size()) ;
		if ( !data.makeFromFile(dataArray, m_EditData) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorNo()) )  ;
			return false ;
		}
#endif
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
		resizeImage(imageData) ;

		data.Image			= imageData ;
		data.fileName		= fileName ;
		data.nTexObj		= 0 ;
		data.lastModified	= QDateTime::currentDateTimeUtc() ;
		m_EditData.addImageData(data);

		m_StrSaveFileName = QString() ;
	}

	setWindowTitle(tr("Animation Creator[%1]").arg(m_StrSaveFileName));

	createWindows() ;

	return true ;
}

// ファイル保存
bool MainWindow::saveFile( QString fileName )
{
	qDebug() << "SaveFile:" << fileName ;

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
				QMessageBox::warning(this, trUtf8("エラー"),
									 trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
			}
			QApplication::restoreOverrideCursor();
			return false ;
		}
		prog.setValue(prog.maximum());

		QFile file(fileName) ;
		if ( !file.open(QFile::WriteOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"),
								 trUtf8("保存失敗 %1:\n%2").arg(fileName).arg(file.errorString())) ;
			QApplication::restoreOverrideCursor();
			return false ;
		}
#if 1
		file.write(data.getData().toString(4).toAscii()) ;
#else
		QDataStream out(&file) ;
		out.writeRawData(data.getData().data(), data.getData().size()) ;
#endif
		QApplication::restoreOverrideCursor();

		m_UndoIndex = m_EditData.getUndoStack()->index() ;
		setWindowTitle(tr("Animation Creator[%1]").arg(fileName));
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
	m_pMdiArea->addSubWindow(m_pImageWindow) ;
	m_pImageWindow->show();

	m_pActImageWindow->setEnabled(true);
	m_pActImageWindow->setChecked(true);

	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pImageWindow, SLOT(slot_modifiedImage(int))) ;
	connect(this, SIGNAL(sig_endedOption()), m_pImageWindow, SLOT(slot_endedOption())) ;
}

// ルーペウィンドウ作成
void MainWindow::makeLoupeWindow( void )
{
	m_pLoupeWindow = new CLoupeWindow(&m_EditData, this, m_pMdiArea) ;
	m_pMdiArea->addSubWindow( m_pLoupeWindow ) ;
	m_pLoupeWindow->show();

	m_pActLoupeWindow->setEnabled(true);
	m_pActLoupeWindow->setChecked(true);
}

// アニメーションフォーム作成
void MainWindow::makeAnimeWindow( void )
{
	m_pAnimationForm = new AnimationForm( &m_EditData, &setting, m_pMdiArea ) ;
	m_pMdiArea->addSubWindow( m_pAnimationForm ) ;
	m_pAnimationForm->show();
	m_pAnimationForm->setBarCenter();

	m_pActAnimeWindow->setEnabled(true);
	m_pActAnimeWindow->setChecked(true);

	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pAnimationForm, SLOT(slot_modifiedImage(int))) ;
	connect(this, SIGNAL(sig_endedOption()), m_pAnimationForm, SLOT(slot_endedOption())) ;
}

