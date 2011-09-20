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

	m_catchRect			= QRect(0, 0, 0, 0) ;
	m_center			= QPoint(0, 0) ;

	m_pObjectModel		= new CObjectModel() ;
	if ( !m_pUndoStack )	{ m_pUndoStack = new QUndoStack ; }
	else					{ m_pUndoStack->clear(); }

	m_selectFrame		= 0 ;

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
	m_bCopyLayer = false ;
}

// アイテム追加 コマンド
QModelIndex CEditData::cmd_addItem(QString str, QModelIndex parent)
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
void CEditData::cmd_addFrameData( QModelIndex &index, FrameData &data, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_AddFrameData(this, index, data, updateWidget));
}

// フレームデータ削除コマンド
void CEditData::cmd_delFrameData( QModelIndex &index, int frame, QList<QWidget *> &updateWidget )
{
	m_pUndoStack->push( new Command_DelFrameData(this, index, frame, updateWidget));
}

// フレームデータ編集コマンド
void CEditData::cmd_editFrameData( QModelIndex		index,
								   int				frame,
								   FrameData		&data,
								   QList<QWidget *>	&updateWidget )
{
	m_pUndoStack->push( new Command_EditFrameData(this, index, frame, data, updateWidget));
}

// オブジェクトコピー コマンド
void CEditData::cmd_copyObject(QModelIndex &index, QList<QWidget *> &updateWidget)
{
	m_pUndoStack->push( new Command_CopyObject(this, index, updateWidget) );
}

// レイヤコピー コマンド
void CEditData::cmd_copyLayer(QModelIndex &index, ObjectItem *pLayer, QList<QWidget *> &updateWidget)
{
	m_pUndoStack->push(new Command_CopyLayer(this, index, pLayer, updateWidget)) ;
}

// レイヤ 親子移動
void CEditData::cmd_moveIndex(int row, ObjectItem *pItem, QModelIndex parent, QList<QWidget *> &updateWidget)
{
	m_pUndoStack->push(new Command_MoveIndex(this, row, pItem, parent, updateWidget)) ;
}

// 選択しているフレームデータ取得
bool CEditData::getNowSelectFrameData(FrameData &ret)
{
	QModelIndex index = getSelIndex() ;
	if ( !getObjectModel()->isLayer(index) ) { return false ; }

	int frame = getSelectFrame() ;
	ObjectItem *pItem = getObjectModel()->getItemFromIndex(index) ;
	bool valid ;
	ret = pItem->getDisplayFrameData(frame, &valid) ;
	return valid ;
}

QMatrix4x4 CEditData::getNowSelectMatrix()
{
	QModelIndex index = getSelIndex() ;
	if ( !getObjectModel()->isLayer(index) ) { return QMatrix4x4() ; }

	int frame = getSelectFrame() ;
	ObjectItem *pItem = getObjectModel()->getItemFromIndex(index) ;
	return pItem->getDisplayMatrix(frame) ;
}

// フレームデータをフレーム数順に並び替え
void CEditData::sortFrameDatas( void )
{
#if 0
	TODO
	CObjectModel::ObjectList &objList = *m_pObjectModel->getObjectListPtr() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		CObjectModel::ObjectGroup &objGroup = objList[i] ;

		for ( int j = 0 ; j < objGroup.layerGroupList.size() ; j ++ ) {
			FrameDataList &frameDataList = objGroup.layerGroupList[j].second ;

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				for ( int l = 0 ; l < k ; l ++ ) {
					if ( frameDataList[k].frame < frameDataList[l].frame ) {
						frameDataList.swap(k, l) ;
					}
				}
			}
		}
	}
#endif
}

