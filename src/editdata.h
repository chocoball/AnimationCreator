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
		GLuint			nTexObj ;
		QString			fileName ;
		QDateTime		lastModified ;	// fileNameの最終更新時間
	} ImageData ;

public:
	CEditData() ;
	~CEditData() ;

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

	void	setCatchRect( QRect &rect )	{ m_CatchRect = rect ; }
	QRect	&getCatchRect( void )		{ return m_CatchRect ; }

	void	setCenter( QPoint &cen )	{ m_Center = cen ; }
	QPoint	&getCenter( void )			{ return m_Center ; }

	void setSelectObject( CObjectModel::typeID objID )	{ m_SelectObject = objID ; }
	CObjectModel::typeID getSelectObject( void )		{ return m_SelectObject ; }

	void setSelectLayer( CObjectModel::typeID layerID ) { m_SelectLayer = layerID ; }
	CObjectModel::typeID getSelectLayer( void )			{ return m_SelectLayer ; }

	void	setSelectFrame( int frame )	{ m_SelectFrame = frame ; }
	int		getSelectFrame( void )		{ return m_SelectFrame ; }

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
	void cmd_addNewLayer( QModelIndex index, QStandardItem *newItem, CObjectModel::FrameData data, QList<QWidget *> &updateWidget ) ;
	void cmd_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget ) ;
	void cmd_delFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, QList<QWidget *> &updateWidget ) ;
	void cmd_editFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget, int id ) ;

private:
	QList<ImageData>		m_ImageData ;

	QRect					m_CatchRect ;
	QPoint					m_Center ;

	CObjectModel::typeID	m_SelectObject ;
	CObjectModel::typeID	m_SelectLayer ;
	int						m_SelectFrame ;

	CObjectModel			*m_pObjectModel ;
	QStandardItemModel		*m_pTreeModel ;
	QUndoStack				*m_pUndoStack ;

	bool					m_bPlayAnime ;
	bool					m_bPauseAnime ;
	bool					m_bDraggingImage ;
} ;

#endif // EDITDATA_H
