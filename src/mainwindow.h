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
	void slot_checkFileModified( void ) ;
	void slot_checkDataModified(int index) ;

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

	bool checkChangedFileSave( void ) ;

signals:
	void sig_modifiedImageFile(int index) ;

private:
	CDropableMdiArea	*m_pMdiArea ;

	ImageWindow			*m_pImageWindow ;		// イメージウィンドウ
	AnimationForm		*m_pAnimationForm ;		// アニメーションフォーム

	// 編集データ ----
	QString				m_StrSaveFileName ;		// 保存ファイル名
	CEditImageData		m_EditImageData ;
	// ---- 編集データ

	// 設定 ----
	CSettings			setting ;
	// ---- 設定

	// アクション ----
	QAction				*m_pActOpen ;			// ファイルを開く
	QAction				*m_pActSave ;			// 保存
	QAction				*m_pActSaveAs ;			// 名前を付けて保存
	QAction				*m_pActUndo ;			// 戻す
	QAction				*m_pActRedo ;			// やり直す
	// ---- アクション

	QStringList			m_DragFileList ;

	int					m_UndoIndex ;

	QTimer				*m_pTimer ;

#ifndef QT_NO_DEBUG
	// デバッグ用 ----
	QAction				*m_pActDbgDump ;
#endif
};

#endif // MAINWINDOW_H
