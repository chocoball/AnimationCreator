#include <QtGui>
#include <QString>
#include "editdata.h"
#include "command.h"

CEditData::CEditData()
{
	m_pUndoStack = NULL ;
	initData() ;
}

CEditData::~CEditData()
{
	delete m_pObjectModel ;
	delete m_pTreeModel ;
	delete m_pUndoStack ;
}

void CEditData::resetData( void )
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

void CEditData::initData( void )
{
	m_editMode = kEditMode_Animation ;
	m_ImageData.clear();

	m_CatchRect			= QRect(0, 0, 0, 0) ;
	m_Center			= QPoint(0, 0) ;

	m_pObjectModel		= new CObjectModel ;
	m_pTreeModel		= new QStandardItemModel ;
	if ( !m_pUndoStack ) {
		m_pUndoStack	= new QUndoStack ;
	}
	else {
		m_pUndoStack->clear();
	}

	m_SelectObject		= 0 ;
	m_SelectFrame		= 0 ;
	m_SelectLayer.clear();

	m_bPlayAnime		= false ;
	m_bPauseAnime		= false ;
	m_bDraggingImage	= false ;
	m_bExportPNG		= false ;

	m_strExportPNGDir	= QString() ;

	m_exPngRect[0] =
	m_exPngRect[1] =
	m_exPngRect[2] =
	m_exPngRect[3] = 0 ;

	m_bCopyFrameData = false ;
	m_bCopyLayerGroup = false ;
}

// オブジェクト追加コマンド
CObjectModel::typeID CEditData::cmd_addNewObject( QString &str )
{
	Command_AddObject *p = new Command_AddObject(this, str) ;
	m_pUndoStack->push(p);
	return p->getNewItem() ;
}

// オブジェクト削除コマンド
void CEditData::cmd_delObject(QModelIndex index, QLabel *pMarkerLabel)
{
	m_pUndoStack->push(new Command_DelObject(this, index, pMarkerLabel));
}

// レイヤ追加コマンド
void CEditData::cmd_addNewLayer( QModelIndex index, QStandardItem *newItem, CObjectModel::LayerGroup &layerGroup, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push(new Command_AddLayer(this, index, newItem, layerGroup, updateWidget));
}

// フレームデータ追加コマンド
void CEditData::cmd_addNewFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, CObjectModel::FrameData &data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_AddFrameData(this, objID, layerID, data, updateWidget));
}

// フレームデータ削除コマンド
void CEditData::cmd_delFrameData( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_DelFrameData(this, objID, layerID, frame, updateWidget));
}

// フレームデータ編集コマンド
void CEditData::cmd_editFrameData( CObjectModel::typeID				objID,
								   QList<CObjectModel::typeID>		&layerIDs,
								   int								frame,
								   QList<CObjectModel::FrameData>	&datas,
								   QList<QWidget *>					&updateWidget )
{
	m_pUndoStack->push( new Command_EditFrameData(this, objID, layerIDs, frame, datas, updateWidget));
}

void CEditData::cmd_copyObject(CObjectModel::typeID objID, QList<QWidget *> &updateWidget)
{
	m_pUndoStack->push( new Command_CopyObject(this, objID, updateWidget) );
}

bool CEditData::getNowSelectFrameData(CObjectModel::FrameData &data)
{
	CObjectModel::typeID objID = getSelectObject() ;
	CObjectModel::typeID layerID = getSelectLayer() ;
	int frame = getSelectFrame() ;

	CObjectModel::FrameData *pPrev = m_pObjectModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
	if ( !pPrev ) {
		pPrev = m_pObjectModel->getFrameDataFromPrevFrame(objID, layerID, frame) ;
		if ( !pPrev ) { return false ; }
	}
	CObjectModel::FrameData *pNext = m_pObjectModel->getFrameDataFromNextFrame(objID, layerID, frame) ;
	data = pPrev->getInterpolation(pNext, frame) ;
	return true ;
}
