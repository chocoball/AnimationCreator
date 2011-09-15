#include <QtGui>
#include <QString>
#include "editdata.h"
#include "command.h"

CEditData::CEditData()
{
	m_pUndoStack = NULL ;
	m_pCopyLayer = NULL ;
	initData() ;
}

CEditData::~CEditData()
{
	delete m_pObjectModel ;
	delete m_pTreeModel ;
	delete m_pUndoStack ;

	if ( m_pCopyLayer ) {
		delete m_pCopyLayer ;
	}
}

void CEditData::resetData( void )
{
	for ( int i = 0 ; i < m_ImageDataList.size() ; i ++ ) {
		if ( !m_ImageDataList[i].nTexObj ) { continue ; }

		glDeleteTextures(1, &m_ImageDataList[i].nTexObj) ;
		m_ImageDataList[i].nTexObj = 0 ;
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
	if ( m_pCopyLayer ) {
		delete m_pCopyLayer ;
		m_pCopyLayer = NULL ;
	}

	initData() ;
}

void CEditData::initData( void )
{
	m_editMode = kEditMode_Animation ;
	m_ImageDataList.clear();

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

// アイテム追加 コマンド
QModelIndex CEditData::cmd_addItem(QString &str, QModelIndex &parent)
{
	Command_AddItem *p = new Command_AddItem(this, str, parent) ;
	m_pUndoStack->push(p) ;
	return p->getIndex() ;
}

// アイテム削除 コマンド
void CEditData::cmd_delItem(QModelIndex &index)
{
	m_pUndoStack->push(new Command_DelItem(this, index)) ;
}

// フレームデータ追加コマンド
void CEditData::cmd_addNewFrameData( QModelIndex &index, FrameData &data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_AddFrameData(this, index, data, updateWidget));
}

// フレームデータ削除コマンド
void CEditData::cmd_delFrameData( QModelIndex &index, int frame, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_DelFrameData(this, index, frame, updateWidget));
}

// フレームデータ編集コマンド
void CEditData::cmd_editFrameData( QModelIndex		&index,
								   int				frame,
								   QList<FrameData>	&datas,
								   QList<QWidget *>	&updateWidget )
{
	m_pUndoStack->push( new Command_EditFrameData(this, index, frame, datas, updateWidget));
}

// オブジェクトコピー コマンド
void CEditData::cmd_copyObject(CObjectModel::typeID objID, QList<QWidget *> &updateWidget)
{
	m_pUndoStack->push( new Command_CopyObject(this, objID, updateWidget) );
}

// 選択しているフレームデータ取得
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

// フレームデータをフレーム数順に並び替え
void CEditData::sortFrameDatas( void )
{
	CObjectModel::ObjectList &objList = *m_pObjectModel->getObjectListPtr() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		CObjectModel::ObjectGroup &objGroup = objList[i] ;

		for ( int j = 0 ; j < objGroup.layerGroupList.size() ; j ++ ) {
			CObjectModel::FrameDataList &frameDataList = objGroup.layerGroupList[j].second ;

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				for ( int l = 0 ; l < k ; l ++ ) {
					if ( frameDataList[k].frame < frameDataList[l].frame ) {
						frameDataList.swap(k, l) ;
					}
				}
			}
		}
	}
}

