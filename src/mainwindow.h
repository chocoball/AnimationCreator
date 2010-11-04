#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imagewindow.h"
#include "setting.h"
#include "editimagedata.h"
#include "animationform.h"
#include "cdropablemdiarea.h"

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

protected:
	void closeEvent(QCloseEvent *event) ;

private slots:
	void slot_open( void ) ;
	void slot_save( void ) ;
	void slot_saveAs( void ) ;
	void slot_dropFiles(QString fileName) ;

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

	void resizeImage( void ) ;

	bool fileOpen( QString fileName ) ;
	bool saveFile( QString fileName ) ;

	bool checkChangedFileSave( void ) ;

private:
	CDropableMdiArea	*m_pMdiArea ;

	ImageWindow			*m_pImageWindow ;		// イメージウィンドウ
	AnimationForm		*m_pAnimationForm ;		// アニメーションフォーム

	// 編集データ ----
	QString				m_StrSaveFileName ;		// 保存ファイル名
	QImage				imageData ;				// 開いている画像データ
	CEditImageData		m_EditImageData ;
	// ---- 編集データ

	// 設定 ----
	CSettings			setting ;
	// ---- 設定

	QAction				*m_pActOpen ;
	QAction				*m_pActSave ;
	QAction				*m_pActSaveAs ;
	QAction				*m_pActUndo ;
	QAction				*m_pActRedo ;

	QStringList			m_DragFileList ;

#ifndef QT_NO_DEBUG
	// デバッグ用 ----
	QAction				*m_pActDbgDump ;
#endif
};

#endif // MAINWINDOW_H
