#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include "imagewindow.h"
#include "setting.h"
#include "editdata.h"
#include "animationform.h"
#include "cdropablemdiarea.h"
#include "cloupewindow.h"
#include "helpwindow.h"
#include "exportpngform.h"


QT_BEGIN_NAMESPACE
class QMdiArea ;
class QAction ;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	bool checkChangedFileSave( void ) ;

protected:
	void closeEvent(QCloseEvent *event) ;
	void keyPressEvent(QKeyEvent *event) ;
	void keyReleaseEvent(QKeyEvent *event);

public slots:
	void slot_open( void ) ;
	void slot_save( void ) ;
	void slot_saveAs( void ) ;
	void slot_dropFiles(QString fileName) ;
	void slot_checkFileModified( void ) ;
	void slot_checkDataModified(int index) ;
	void slot_help( void ) ;

	void slot_triggeredImageWindow( bool flag ) ;
	void slot_triggeredLoupeWindow( bool flag ) ;
	void slot_triggeredAnimeWindow( bool flag ) ;

	void slot_option( void ) ;

	void slot_exportPNG( void ) ;
	void slot_closeExportPNGForm( void ) ;

	void slot_portCheckDrawCenter(bool flag) ;
	void slot_portDragedImage(FrameData data) ;

	void slot_pushColorToolButton( void ) ;

	void slot_destroyAnmWindow( void ) ;
	void slot_destroyImgWindow( void ) ;
	void slot_destroyLoupeWindow( void ) ;

	void slot_reqFinished(QNetworkReply *reply) ;
	void slot_exportJSON() ;

#ifndef QT_NO_DEBUG
	void slot_dbgObjectDump( void ) ;
#endif

private:
	void readRootSetting( void ) ;
	void writeRootSetting( void ) ;

	void createActions( void ) ;
	void createMenus( void ) ;

	void createWindows( void ) ;
	void createImageWindow( void ) ;
	void createAnimationForm( void ) ;
	void setCurrentDir( QString &fileName ) ;

	void resizeImage( QImage &imageData ) ;

	bool fileOpen( QString fileName ) ;
	bool saveFile( QString fileName ) ;

	void makeImageWindow( void ) ;
	void makeLoupeWindow( void ) ;
	void makeAnimeWindow( void ) ;

signals:
	void sig_modifiedImageFile(int index) ;
	void sig_endedOption( void ) ;
	void sig_portCheckDrawCenter(bool) ;
	void sig_portDragedImage(FrameData) ;

private:
	CDropableMdiArea	*m_pMdiArea ;

	ImageWindow			*m_pImageWindow ;		// イメージウィンドウ
	CLoupeWindow		*m_pLoupeWindow ;		// ルーペウィンドウ
	AnimationForm		*m_pAnimationForm ;		// アニメーションフォーム
	ExportPNGForm		*m_pExportPNGForm ;		// PNG吐き出しフォーム
	QMdiSubWindow		*m_pExpngSubWindow ;

	// 編集データ ----
	QString				m_StrSaveFileName ;		// 保存ファイル名
	CEditData			m_EditData ;
	// ---- 編集データ

	// 設定 ----
	CSettings			setting ;
	// ---- 設定

	// アクション ----
	QAction				*m_pActOpen ;			// ファイルを開く
	QAction				*m_pActSave ;			// 保存
	QAction				*m_pActSaveAs ;			// 名前を付けて保存
	QAction				*m_pActExportPNG ;		// 連番PNGにエクスポート
	QAction				*m_pActExit ;			// 終了
	QAction				*m_pActUndo ;			// 戻す
	QAction				*m_pActRedo ;			// やり直す
	QAction				*m_pActImageWindow ;	// イメージウィンドウon/off
	QAction				*m_pActLoupeWindow ;	// ルーペウィンドウon/off
	QAction				*m_pActAnimeWindow ;	// アニメーションウィンドウon/off
	QAction				*m_pActOption ;			// オプション
	QAction				*m_pActHelp ;			// ヘルプ
	QAction				*m_pActAboutQt ;		// Qtについて
	QAction				*m_pActExportJson ;		// JSON吐き出し
	// ---- アクション

	QStringList			m_DragFileList ;

	int					m_UndoIndex ;

	QTimer				*m_pTimer ;

	bool				m_bSaveImage ;			// 画像データ保存するならtrue

	bool				m_bCtrl ;
#ifndef QT_NO_DEBUG
	// デバッグ用 ----
	QAction				*m_pActDbgDump ;
#endif

	QMdiSubWindow		*m_pSubWindow_Anm ;
	QMdiSubWindow		*m_pSubWindow_Img ;
	QMdiSubWindow		*m_pSubWindow_Loupe ;
};

#endif // MAINWINDOW_H
