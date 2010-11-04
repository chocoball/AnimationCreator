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

	createActions() ;
	createMenus() ;

	readRootSetting() ;

	setWindowTitle(tr("Animation Creator"));
	setUnifiedTitleAndToolBarOnMac(true);

	connect(m_pMdiArea, SIGNAL(dropFiles(QString)), this, SLOT(slot_dropFiles(QString))) ;
}

MainWindow::~MainWindow()
{
	delete m_pMdiArea ;
}

// ウィンドウ閉じイベント
void MainWindow::closeEvent(QCloseEvent *event)
{
	printf( "closeEvent\n" ) ;
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
	// イメージウィンドウ
	m_pImageWindow = new ImageWindow( &setting, &m_EditImageData, m_pMdiArea ) ;
	m_pMdiArea->addSubWindow(m_pImageWindow) ;
	m_pImageWindow->show();

	// アニメーションフォーム
	m_pAnimationForm = new AnimationForm( &m_EditImageData, m_pMdiArea ) ;
	m_pMdiArea->addSubWindow( m_pAnimationForm ) ;
	m_pAnimationForm->show();
	m_pAnimationForm->setBarCenter();

	// ルーペウィンドウ
	CLoupeWindow *pLoupe = new CLoupeWindow(&m_EditImageData, m_pMdiArea) ;
	m_pMdiArea->addSubWindow( pLoupe ) ;
	pLoupe->show();

	connect(m_pAnimationForm, SIGNAL(sig_imageRepaint()), (QWidget *)m_pImageWindow->getGridLabel(), SLOT(update())) ;
	connect((QWidget *)m_pImageWindow->getGridLabel(),	SIGNAL(sig_changeSelectLayerUV(QRect)),
			m_pAnimationForm,							SLOT(slot_changeSelectLayerUV(QRect))) ;

}

// imageDataのサイズを2の累乗に修正
void MainWindow::resizeImage( void )
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

	// 画像ファイル
	if ( fileName.toLower().indexOf(".png") > 0
	  || fileName.toLower().indexOf(".bmp") > 0
	  || fileName.toLower().indexOf(".jpg") > 0 ) {
		if ( !imageData.load(fileName) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました") ) ;
			return false ;
		}
		resizeImage() ;
		m_StrSaveFileName = QString() ;
	}
	// アニメファイル
	else if ( fileName.indexOf(FILE_EXT_ANM2D) > 0 ) {
		CAnm2D data ;
		QByteArray dataArray ;

		QFile file(fileName) ;
		if ( !file.open(QFile::ReadOnly) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました") ) ;
			return false ;
		}
		QDataStream in(&file) ;
		dataArray.resize(file.size());
		in.readRawData(dataArray.data(), file.size()) ;

		if ( !data.makeFromFile(dataArray, imageData, m_EditImageData) ) {
			QMessageBox::warning(this, trUtf8("エラー"), trUtf8("読み込みに失敗しました:%1").arg(data.getErrorNo()) )  ;
			return false ;
		}
		m_StrSaveFileName = fileName ;
	}

	m_EditImageData.setImage( imageData );

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
	if ( 0 ) {
		QMessageBox::StandardButton reply = QMessageBox::question(this, trUtf8("確認"), trUtf8("保存しますか？"),
																  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) ;
		if ( reply == QMessageBox::Yes ) {		// 保存する
			slot_save();
		}
		else if ( reply == QMessageBox::Cancel ) {	// キャンセル
			return false ;
		}
	}
	return true ;
}



