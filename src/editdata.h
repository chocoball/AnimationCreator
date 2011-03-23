#ifndef EDITDATA_H
#define EDITDATA_H

#include <QImage>
#include <QtOpenGL>
#include <QUndoStack>
#include "objectmodel.h"

class CEditData
{
public:
	typedef struct {
		QImage			Image ;
		int				origImageW, origImageH ;
		GLuint			nTexObj ;
		QString			fileName ;
		QDateTime		lastModified ;	// fileNameの最終更新時間
	} ImageData ;

	enum {
		kMaxFrame	= 180,		// 最大アニメーションフレーム数
	} ;

	// edit mode
	enum {
		kEditMode_Animation = 0,
		kEditMode_ExportPNG,

		kEditMode_Num,
	} ;

public:
	CEditData() ;
	~CEditData() ;
	
	void setEditMode( int mode )
	{
		m_editMode = mode ;
	}
	int getEditMode( void )
	{
		return m_editMode ;
	}

	void setImageData( QList<ImageData> &data )
	{
		m_ImageData = data ;
	}
	void addImageData( ImageData &data )
	{
		m_ImageData.append(data) ;
	}

	void	addImage( QImage &image )
	{
		m_ImageData.append(ImageData());
		m_ImageData.last().Image = image ;
	}
	void	setImage( int index, QImage &image )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		m_ImageData[index].Image = image ;
	}

	QImage	&getImage( int index )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			static QImage image ;
			return image ;
		}

		return m_ImageData[index].Image ;
	}
	void setImageFileName( int index, QString &name )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		m_ImageData[index].fileName = name ;
	}
	QString &getImageFileName( int index )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			static QString str ;
			return str ;
		}
		return m_ImageData[index].fileName ;
	}

	void setImageDataLastModified( int index, QDateTime &time )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		m_ImageData[index].lastModified = time ;
	}
	QDateTime &getImageDataLastModified( int index )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			static QDateTime time ;
			return time ;
		}
		return m_ImageData[index].lastModified ;
	}

	int getImageDataSize( void ) { return m_ImageData.size() ; }

	void removeImageData( int index )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		if ( m_ImageData[index].nTexObj ) {
			glDeleteTextures(1, &m_ImageData[index].nTexObj);
		}
		m_ImageData.removeAt(index);
	}

	void	setTexObj( int index, GLuint texObj )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		m_ImageData[index].nTexObj = texObj ;
	}
	GLuint	getTexObj( int index )
	{
		if ( index < 0 || index >= m_ImageData.size() ) { return 0 ; }
		return m_ImageData[index].nTexObj ;
	}
	
	void setOriginalImageSize( int index, int w, int h )
	{
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		m_ImageData[index].origImageW = w ;
		m_ImageData[index].origImageH = h ;
	}
	void getOriginalImageSize( int index, int &w, int &h )
	{
		w = 0 ;
		h = 0 ;
		if ( index < 0 || index >= m_ImageData.size() ) {
			return ;
		}
		w = m_ImageData[index].origImageW ;
		h = m_ImageData[index].origImageH ;
	}

	void	setCatchRect( QRect &rect )	{ m_CatchRect = rect ; }
	QRect	&getCatchRect( void )		{ return m_CatchRect ; }

	void	setCenter( QPoint &cen )	{ m_Center = cen ; }
	QPoint	&getCenter( void )			{ return m_Center ; }

	void setSelectObject( CObjectModel::typeID objID )
	{
		m_SelectObject = objID ;
	}
	CObjectModel::typeID getSelectObject( void )		{ return m_SelectObject ; }

	void setSelectLayer( QList<CObjectModel::typeID> &layerID )
	{
		m_SelectLayer = layerID ;
		updateSelectData();
	}

	CObjectModel::typeID getSelectLayer( int index = 0 )
	{
		if ( index >= m_SelectLayer.size() ) {
			return 0 ;
		}
		return m_SelectLayer[index] ;
	}
	QList<CObjectModel::typeID> &getSelectLayers( void )
	{
		return m_SelectLayer ;
	}

	int getSelectLayerNum( void )
	{
		return m_SelectLayer.size() ;
	}
	bool isSelectedLayer( CObjectModel::typeID layerID )
	{
		return m_SelectLayer.indexOf(layerID) >= 0 ;
	}

	void	setSelectFrame( int frame )	{ m_SelectFrame = frame ; }
	int		getSelectFrame( void )		{ return m_SelectFrame ; }

	void updateSelectData( void )
	{
		m_SelectData.clear();
		for ( int i = 0 ; i < m_SelectLayer.size() ; i ++ ) {
			CObjectModel::FrameData *p = m_pObjectModel->getFrameDataFromIDAndFrame(m_SelectObject,
																					m_SelectLayer[i],
																					m_SelectFrame) ;
			if ( !p ) { continue ; }
			m_SelectData.insert(i, *p);
		}
	}
	CObjectModel::FrameData *getSelectFrameData( int index )
	{
		if ( index >= m_SelectData.size() ) {
			return NULL ;
		}
		return &m_SelectData[index] ;
	}
	int getSelectFrameDataNum( void )
	{
		return m_SelectData.size() ;
	}

	void	setPlayAnime( bool flag )	{ m_bPlayAnime = flag ; }
	bool	isPlayAnime( void )			{ return m_bPlayAnime ; }

	void	setPauseAnime( bool flag )	{ m_bPauseAnime = flag ; }
	bool	isPauseAnime( void )		{ return m_bPauseAnime ; }

	void	setDraggingImage( bool f )	{ m_bDraggingImage = f ; }
	bool	isDraggingImage( void )		{ return m_bDraggingImage ; }

	CObjectModel		*getObjectModel( void )	{ return m_pObjectModel ; }
	QStandardItemModel	*getTreeModel( void )	{ return m_pTreeModel ; }
	QUndoStack			*getUndoStack( void )	{ return m_pUndoStack ; }

	void	resetData( void ) ;
	void	initData( void ) ;

	CObjectModel::typeID cmd_addNewObject( QString &str ) ;
	void cmd_delObject(QModelIndex index, QLabel *pMarkerLabel) ;
	void cmd_addNewLayer( QModelIndex index, QStandardItem *newItem, CObjectModel::LayerGroup &layerGroup, QList<QWidget *> &updateWidget ) ;
	void cmd_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget ) ;
	void cmd_delFrameData( CObjectModel::typeID	objID,
						   CObjectModel::typeID	layerID,
						   int					frame,
						   QList<QWidget *>		&updateWidget ) ;
	void cmd_editFrameData( CObjectModel::typeID			objID,
							QList<CObjectModel::typeID>		&layerIDs,
							int								frame,
							QList<CObjectModel::FrameData>	&datas,
							QList<QWidget *>				&updateWidget ) ;
	void cmd_copyObject(CObjectModel::typeID objID, QList<QWidget *> &updateWidget) ;

	void setCurrLoopNum( int num )
	{
		if ( getSelectObject() == 0 ) { return ; }
		CObjectModel::ObjectGroup *p = m_pObjectModel->getObjectGroupFromID(getSelectObject()) ;
		if ( !p ) { return ; }
		p->nCurrentLoop = num ;
	}
	bool addCurrLoopNum( int num )
	{
		if ( getSelectObject() == 0 ) { return true ; }
		CObjectModel::ObjectGroup *p = m_pObjectModel->getObjectGroupFromID(getSelectObject()) ;
		if ( !p ) { return true ; }
		p->nCurrentLoop += num ;
		if ( p->nLoop < 0 ) { return false ; }	// 無限ループ
		return (p->nCurrentLoop > p->nLoop) ;
	}

	// 連番PNG保存関連 ----------------------
	void startExportPNG( QString dir )
	{
		m_nExportEndFrame = 0 ;
		m_strExportPNGDir = dir ;
		m_bExportPNG = true ;
	}

	void endExportPNG( void )
	{
		m_bExportPNG = false ;
	}

	QString getExportPNGDir( void )
	{
		return m_strExportPNGDir ;
	}

	bool isExportPNG( void )
	{
		return m_bExportPNG ;
	}

	void getExportPNGRect( int ret[4] )
	{
		ret[0] = m_exPngRect[0] ;
		ret[1] = m_exPngRect[1] ;
		ret[2] = m_exPngRect[2] ;
		ret[3] = m_exPngRect[3] ;
	}

	void setExportPNGRect( int rect[4] )
	{
		m_exPngRect[0] = rect[0] ;
		m_exPngRect[1] = rect[1] ;
		m_exPngRect[2] = rect[2] ;
		m_exPngRect[3] = rect[3] ;
	}

	int getExportEndFrame( void )
	{
		return m_nExportEndFrame ;
	}
	void setExportEndFrame( int frame )
	{
		m_nExportEndFrame = frame ;
	}
	// --------------------------------------

	// フレームデータ コピー関連 ------------
	void setCopyFrameData( CObjectModel::FrameData data )
	{
		m_CopyFrameData = data ;
		m_bCopyFrameData = true ;
	}
	CObjectModel::FrameData getCopyFrameData( void )
	{
		return m_CopyFrameData ;
	}
	bool isCopyData( void )
	{
		return m_bCopyFrameData ;
	}
	// --------------------------------------

	// レイヤコピー関連 ----------------------
	void setCopyLayer( CObjectModel::LayerGroup layerGroup )
	{
		m_CopyLayerGroup = layerGroup ;
		m_bCopyLayerGroup = true ;
	}
	CObjectModel::LayerGroup getCopyLayer( void )
	{
		return m_CopyLayerGroup ;
	}
	bool isCopyLayer( void )
	{
		return m_bCopyLayerGroup ;
	}
	// --------------------------------------

	bool getNowSelectFrameData(CObjectModel::FrameData &data) ;

private:
	int								m_editMode ;

	QList<ImageData>				m_ImageData ;

	QRect							m_CatchRect ;
	QPoint							m_Center ;

	CObjectModel::typeID			m_SelectObject ;
	QList<CObjectModel::typeID>		m_SelectLayer ;
	int								m_SelectFrame ;

	QList<CObjectModel::FrameData>	m_SelectData ;

	CObjectModel					*m_pObjectModel ;
	QStandardItemModel				*m_pTreeModel ;
	QUndoStack						*m_pUndoStack ;

	bool							m_bPlayAnime ;
	bool							m_bPauseAnime ;
	bool							m_bDraggingImage ;

	bool							m_bExportPNG ;
	QString							m_strExportPNGDir ;
	int								m_exPngRect[4] ;	// [0]left, [1]top, [2]right, [3]bottom
	int								m_nExportEndFrame ;	// 吐き出し終わったフレーム

	bool							m_bCopyFrameData ;
	CObjectModel::FrameData			m_CopyFrameData ;

	bool							m_bCopyLayerGroup ;
	CObjectModel::LayerGroup		m_CopyLayerGroup ;
} ;

#endif // EDITDATA_H
