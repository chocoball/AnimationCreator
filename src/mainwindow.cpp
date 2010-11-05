#include <qglobal.h>
#include <QtGui>
#include "mainwindow.h"
#include "imagewindow.h"
#include "canm2d.h"
#include "cloupewindow.h"

#define FILE_EXT_ANM2D	".anm2d"

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
	QUndoStack *pUndoStack = m_EditImageData.getUndoStack() ;
	connect(pUndoStack, SIGNAL(indexChanged(int)), this, SLOT(slot_checkDataModified(int))) ;

	m_UndoIndex = 0 ;
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
											tr("Images (*.png *.bmp *.jpg *"FILE_EXT_ANM2D")")) ;
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
	QString str = QFileDialog::getSaveFileName(this, trUtf8("名前を付けて保存"), setting.getCurrentSaveDir(), tr("*.anm2d")) ;
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
	for ( int i = 0 ; i < m_EditImageData.getImageDataSize() ; i ++ ) {
		QString fullPath = m_EditImageData.getImageFileName(i) ;
		QFileInfo info(fullPath) ;
		if ( !info.isFile() ) { continue ; }
		if ( !info.lastModified().isValid() ) { continue ; }
		if ( info.lastModified().toUTC() <= m_EditImageData.getImageDataLastModified(i) ) { continue ; }

		QDateTime time = info.lastModified().toUTC() ;
		m_EditImageData.setImageDataLastModified(i, time);

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
		m_EditImageData.setImage(i, image) ;
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
	QSettings settings("Editor", "rootSettings") ;
	QPoint pos = settings.value("pos_root", QPoint(200, 200)).toPoint() ;
	QSize size = settings.value("size_root", QSize(400, 400)).toSize() ;

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

	move(pos) ;
	resize(size) ;
	setting.setCurrentDir(dir) ;

	pos = settings.value("pos_imgwindow", QPoint(100, 100)).toPoint() ;
	size = settings.value("size_imgwindow", QSize(100, 100)).toSize() ;
	setting.setImgWinPos(pos) ;
	setting.setImgWinSize(size) ;
}

// 設定を保存
void MainWindow::writeRootSetting( void )
{
	QSettings settings("Editor", "rootSettings") ;
	settings.setValue("pos_root", pos()) ;
	settings.setValue("size_root", size()) ;
	settings.setValue("cur_dir", setting.getCurrentDir()) ;

	settings.setValue("pos_imgwindow", setting.getImgWinPos());
	settings.setValue("size_imgwindow", setting.getImgWinSize());
}

// アクションを作成
void MainWindow::createActions( void )
{
	m_pActOpen = new QAction(trUtf8("&Open..."), this) ;
	m_pActOpen->setShortcuts(QKeySequence::Open) ;
	m_pActOpen->setStatusTip(trUtf8("ファイルを開きます")) ;
	connect(m_pActOpen, SIGNAL(triggered()), this, SLOT(slot_open())) ;

	m_pActSave = new QAction(trUtf8("&Save"), this) ;
	m_pActSave->setShortcuts(QKeySequence::Save) ;
	m_pActSave->setStatusTip(trUtf8("ファイルを保存します")) ;
	connect(m_pActSave, SIGNAL(triggered()), this, SLOT(slot_save())) ;

	m_pActSaveAs = new QAction(trUtf8("Save &As..."), this) ;
	m_pActSaveAs->setShortcuts(QKeySequence::SaveAs) ;
	m_pActSaveAs->setStatusTip(trUtf8("ファイルを保存します")) ;
	connect(m_pActSaveAs, SIGNAL(triggered()), this, SLOT(slot_saveAs())) ;

	QUndoStack *pStack = m_EditImageData.getUndoStack() ;
	m_pActUndo = pStack->createUndoAction(this, trUtf8("&Undo")) ;
	m_pActUndo->setShortcuts(QKeySequence::Undo);

	m_pActRedo = pStack->createRedoAction(this, trUtf8("&Redo")) ;
	m_pActRedo->setShortcuts(QKeySequence::Redo);

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


	pMenu = menuBar()->addMenu(trUtf8("&Edit")) ;
	pMenu->addAction(m_pActUndo) ;
	pMenu->addAction(m_pActRedo) ;

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
	// アニメーションフォーム
	m_pAnimationForm = new AnimationForm( &m_EditImageData, m_pMdiArea ) ;
	m_pMdiArea->addSubWindow( m_pAnimationForm ) ;
	m_pAnimationForm->show();
	m_pAnimationForm->setBarCenter();

	// イメージウィンドウ
	m_pImageWindow = new ImageWindow( &setting, &m_EditImageData, m_pAnimationForm, m_pMdiArea ) ;
	m_pMdiArea->addSubWindow(m_pImageWindow) ;
	m_pImageWindow->show();

	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pImageWindow, SLOT(slot_modifiedImage(int))) ;
	connect(this, SIGNAL(sig_modifiedImageFile(int)), m_pAnimationForm, SLOT(slot_modifiedImage(int))) ;

	// ルーペウィンドウ
	CLoupeWindow *pLoupe = new CLoupeWindow(&m_EditImageData, m_pMdiArea) ;
	m_pMdiArea->addSubWindow( pLoupe ) ;
	pLoupe->show();
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
	  && fileName.indexOf(FILE_EXT_ANM2D)	<= 0 ) {
		QMessageBox::warning(this, tr("warning"), trUtf8("対応していないファイルです") ) ;
		return false ;
	}

	if ( m_pMdiArea->currentSubWindow() ) {
		m_pMdiArea->closeAllSubWindows() ;	// 全部閉じる
	}
	m_EditImageData.resetData();
	m_UndoIndex = 0 ;

	// アニメファイル
	if ( fileName.indexOf(FILE_EXT_ANM2D) > 0 ) {
		CAnm2D data ;
		QByteArray dataArray ;

		QFile file(fileName) ;
		if ( !file.open(QFile::ReadOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(fileName)) ;
			return false ;
		}
		QDataStream in(&file) ;
		dataArray.resize(file.size());
		in.readRawData(dataArray.data(), file.size()) ;

		if ( !data.makeFromFile(dataArray, m_EditImageData) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorNo()) )  ;
			return false ;
		}
		m_StrSaveFileName = fileName ;
	}
	// 画像ファイル
	else {
		CEditImageData::ImageData data ;
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
		m_EditImageData.addImageData(data);

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

	if ( fileName.indexOf(FILE_EXT_ANM2D) > 0 ) {
		CAnm2D data ;
		QApplication::setOverrideCursor(Qt::WaitCursor);
		if ( !data.makeFromEditImageData(m_EditImageData) ) {
			QMessageBox::warning(this, trUtf8("エラー"),
								 trUtf8("コンバート失敗 %1:\n%2").arg(fileName).arg(data.getErrorNo())) ;
			QApplication::restoreOverrideCursor();
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

		m_UndoIndex = m_EditImageData.getUndoStack()->index() ;
		setWindowTitle(tr("Animation Creator[%1]").arg(fileName));
		return true ;
	}
	else if ( fileName.indexOf(".png") ) {
	}

	return false ;
}

// 変更したファイルを保存するか
// キャンセルならfalse
bool MainWindow::checkChangedFileSave( void )
{
	QUndoStack *p = m_EditImageData.getUndoStack() ;
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



