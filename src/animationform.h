#ifndef ANIMATIONFORM_H
#define ANIMATIONFORM_H

#include <QtGui>
#include <QWidget>
#include "editdata.h"
#include "glwidget.h"
#include "cdatamarkerlabel.h"
#include "AnimationWindowSplitter.h"

#define LAYOUT_OWN

namespace Ui {
    class AnimationForm;
}

class MainWindow ;

class AnimationForm : public QWidget
{
    Q_OBJECT

public:
	explicit AnimationForm(CEditData *pImageData, CSettings *pSetting, QWidget *parent = 0);
    ~AnimationForm();

	void setBarCenter() ;
	void dbgDumpObject() ;

	AnimeGLWidget *getGLWidget( void )
	{
		return m_pGlWidget ;
	}

	void setSplitterPos(int pos, int index) ;

signals:
	void sig_imageChangeTab(int nImage) ;
	void sig_imageRepaint( void ) ;
	void sig_imageChangeRect(QRect rect) ;
	void sig_portCheckDrawCenter(bool) ;
	void sig_portDragedImage(FrameData) ;
	void sig_pushColorToolButton( void ) ;
	void sig_changeSelectLayer(QModelIndex) ;

public slots:
	void slot_createNewObject( void ) ;
	void slot_deleteObject( void ) ;
	void slot_deleteFrameData( void ) ;

	void slot_dropedImage( QRect rect, QPoint pos, int imageIndex ) ;

	void slot_frameChanged(int frame) ;
    void slot_selectLayerChanged( QModelIndex indexLayer ) ;

	void slot_setUI(FrameData data) ;

	void slot_changePosX( int val ) ;
	void slot_changePosY( int val ) ;
	void slot_changePosZ( int val ) ;
	void slot_changeRotX( int val ) ;
	void slot_changeRotY( int val ) ;
	void slot_changeRotZ( int val ) ;
	void slot_changeScaleX( double val ) ;
	void slot_changeScaleY( double val ) ;
	void slot_changeUvLeft( int val ) ;
	void slot_changeUvRight( int val ) ;
	void slot_changeUvTop( int val ) ;
	void slot_changeUvBottom( int val ) ;
	void slot_changeCenterX( int val ) ;
	void slot_changeCenterY( int val ) ;

	void slot_treeViewMenuReq(QPoint treeViewLocalPos) ;
	void slot_treeViewDoubleClicked(QModelIndex index) ;
	void slot_changeSelectObject(QModelIndex index) ;

	void slot_playAnimation( void ) ;
	void slot_pauseAnimation( void ) ;
	void slot_stopAnimation( void ) ;

	void slot_backwardFrameData( void ) ;
	void slot_forwardFrameData( void ) ;

	void slot_timerEvent( void ) ;
    void slot_addNewFrameData( QModelIndex indexLayer, int frame, FrameData data ) ;
	void slot_changeLayerDisp( void ) ;
	void slot_changeLayerLock( void ) ;

	void slot_changeSelectLayerUV( QRect rect ) ;
	void slot_changeAnimeSpeed(int index) ;

	void slot_addImage( int imageNo ) ;
	void slot_delImage( int imageNo ) ;

	void slot_changeImageIndex(QString index) ;

	void slot_changeUVAnime( bool flag ) ;

	void slot_modifiedImage(int index) ;

	void slot_endedOption( void ) ;

	void slot_frameDataMoveEnd( FrameData data ) ;

	void slot_clickedRadioPos( bool flag ) ;
	void slot_clickedRadioRot( bool flag ) ;
	void slot_clickedRadioCenter( bool flag ) ;
	void slot_clickedRadioScale( bool flag ) ;
	void slot_clickedRadioPath(bool flag) ;

	void slot_changeLoop( int val ) ;
	void slot_changeColorR(int val ) ;
	void slot_changeColorG(int val ) ;
	void slot_changeColorB(int val ) ;
	void slot_changeColorA(int val ) ;

	void slot_copyObject( void ) ;

	void slot_changeDrawFrame(bool flag) ;
	void slot_changeDrawCenter(bool flag) ;
	void slot_changeLinearFilter(bool flag) ;

	void slot_portDragedImage(FrameData data) ;

	void slot_clickPicker( void ) ;
	void slot_setColorFromPicker(QRgb rgba) ;

	void slot_splitterMoved(int pos, int index) ;
	void slot_copyIndex(int row, ObjectItem *pItem, QModelIndex index, Qt::DropAction action) ;

protected:
	FrameData *getNowSelectFrameData( void ) ;
	void addNewObject( QString str ) ;

	void addCommandEdit( FrameData data, FrameData *pOld = 0 ) ;

    bool setSelectFrameDataFromFrame( int frame, QModelIndex indexLayer ) ;

	void addNowSelectLayerAndFrame( void ) ;

	void keyPressEvent(QKeyEvent *event) ;
	void keyReleaseEvent(QKeyEvent *event);
	void copyFrameData( void ) ;
	void pasteFrameData( void ) ;
#ifdef LAYOUT_OWN
	void resizeEvent(QResizeEvent *event) ;
#endif
	void closeEvent(QCloseEvent *event) ;

	void dumpObjects(ObjectItem *p, int tab) ;

private:
	Ui::AnimationForm		*ui;

	AnimeGLWidget			*m_pGlWidget ;
	CEditData				*m_pEditData ;
	CSettings				*m_pSetting ;
	int						m_ObjIndex ;

	QAction					*m_pActTreeViewAdd ;
	QAction					*m_pActTreeViewCopy ;
	QAction					*m_pActTreeViewDel ;
	QAction					*m_pActTreeViewLayerDisp ;
	QAction					*m_pActPlay ;
	QAction					*m_pActStop ;
	QAction					*m_pActTreeViewLayerLock ;

	QTimer					*m_pTimer ;
	int						m_nMaxFrameNum ;

	CDataMarkerLabel		*m_pDataMarker ;

	AnimationWindowSplitter	*m_pSplitter ;
	bool					m_bDontSetData ;

	QSize					m_oldWinSize ;
};

#endif // ANIMATIONFORM_H
