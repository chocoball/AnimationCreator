#include <QtGui>
#include <QString>
#include "editimagedata.h"
#include "command.h"

CEditImageData::CEditImageData()
{
	m_pUndoStack = NULL ;
	initData() ;
}

CEditImageData::~CEditImageData()
{
	delete m_pObjectModel ;
	delete m_pTreeModel ;
	delete m_pUndoStack ;
}

void CEditImageData::resetData( void )
{
	for ( int i = 0 ; i < m_ImageData.size() ; i ++ ) {
		if ( !m_ImageData[i].nTexObj ) { continue ; }

		glDeleteTextures(1, &m_ImageData[i].nTexObj) ;
		m_ImageData[i].nTexObj = 0 ;
	}
	if ( m_pObjectModel ) {
		delete m_pObjectModel ;
	}
	if ( m_pTreeModel ) {
		delete m_pTreeModel ;
	}
	if ( m_pUndoStack ) {
		m_pUndoStack->clear();
	}

	initData() ;
}

void CEditImageData::initData( void )
{
	m_ImageData.clear();

	m_CatchRect			= QRect(0, 0, 0, 0) ;
	m_Center			= QPoint(0, 0) ;

	m_pObjectModel		= new CObjectModel ;
	m_pTreeModel		= new QStandardItemModel ;
	if ( !m_pUndoStack ) {
		m_pUndoStack		= new QUndoStack ;
	}
	else {
		m_pUndoStack->clear();
	}

	m_SelectObject		= 0 ;
	m_SelectLayer		= 0 ;
	m_SelectFrame		= 0 ;

	m_bPlayAnime		= false ;
	m_bPauseAnime		= false ;
	m_bDraggingImage	= false ;
}

// オブジェクト追加コマンド
CObjectModel::typeID CEditImageData::cmd_addNewObject( QString &str )
{
	Command_AddObject *p = new Command_AddObject(this, str) ;
	m_pUndoStack->push(p);
	return p->getNewItem() ;
}

// オブジェクト削除コマンド
void CEditImageData::cmd_delObject(QModelIndex index, QLabel *pMarkerLabel)
{
	m_pUndoStack->push(new Command_DelObject(this, index, pMarkerLabel));
}

// レイヤ追加コマンド
void CEditImageData::cmd_addNewLayer( QModelIndex index, QStandardItem *newItem, CObjectModel::FrameData data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push(new Command_AddLayer(this, index, newItem, data, updateWidget));
}

// フレームデータ追加コマンド
void CEditImageData::cmd_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_AddFrameData(this, objID, layerID, data, updateWidget));
}

// フレームデータ削除コマンド
void CEditImageData::cmd_delFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_DelFrameData(this, objID, layerID, frame, updateWidget));
}

// フレームデータ編集コマンド
void CEditImageData::cmd_editFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_EditFrameData(this, objID, layerID, frame, data, updateWidget));
}





