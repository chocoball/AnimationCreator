#ifndef ANIMATIONFORM_H
#define ANIMATIONFORM_H

#include <QtGui>
#include <QWidget>
#include "editdata.h"
#include "glwidget.h"
#include "cdatamarkerlabel.h"

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
#ifdef LAYOUT_OWN
	void resizeEvent(QResizeEvent *event) ;
#endif

	void setBarCenter() ;
	void dbgDumpObject() ;

signals:
	void sig_imageRepaint( void ) ;

public slots:
	void slot_createNewObject( void ) ;
	void slot_deleteObject( void ) ;
	void slot_deleteFrameData( void ) ;

	void slot_dropedImage( QRect rect, QPoint pos, int imageIndex ) ;

	void slot_frameChanged(int frame) ;
	void slot_selectLayerChanged( CObjectModel::typeID layerID ) ;

	void slot_setUI(CObjectModel::FrameData data) ;

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

	void slot_timerEvent( void ) ;
	void slot_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData data ) ;
	void slot_changeLayerDisp( void ) ;

	void slot_changeSelectLayerUV( QRect rect ) ;
	void slot_changeAnimeSpeed(int index) ;

	void slot_addImage( int imageNo ) ;
	void slot_delImage( int imageNo ) ;

	void slot_changeImageIndex(int index) ;

	void slot_changeUVAnime( bool flag ) ;

	void slot_modifiedImage(int index) ;

	void slot_endedOption( void ) ;

	void slot_frameDataMoveEnd(CObjectModel::FrameData*) ;

	void slot_clickedRadioPos( bool flag ) ;
	void slot_clickedRadioRot( bool flag ) ;
	void slot_clickedRadioCenter( bool flag ) ;
	void slot_clickedRadioScale( bool flag ) ;

protected:
	CObjectModel::FrameData *getNowSelectFrameData( void ) ;
	void addNewObject( QString str ) ;

	void addCommandEdit( CObjectModel::FrameData *pData, int id = 1 ) ;

private:
	Ui::AnimationForm	*ui;

	AnimeGLWidget		*m_pGlWidget ;

	CEditData			*m_pEditData ;
	int					m_ObjIndex ;

	QAction				*m_pActTreeViewAdd ;
	QAction				*m_pActTreeViewDel ;
	QAction				*m_pActTreeViewLayerDisp ;
	QAction				*m_pActPlay ;
	QAction				*m_pActStop ;

	QTimer				*m_pTimer ;
	int					m_nMaxFrameNum ;

	CDataMarkerLabel	*m_pDataMarker ;

	QSplitter			*m_pSplitter ;
};

#endif // ANIMATIONFORM_H
