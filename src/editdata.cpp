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
        // TODO:
//		glDeleteTextures(1, &m_ImageDataList[i].nTexObj) ;
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

	m_catchRect			= CRectF(0, 0, 0, 0) ;
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
	m_bCopyAllFrame = false ;
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
void CEditData::cmd_addFrameData( QModelIndex &index, FrameData &data )
{
	m_pUndoStack->push( new Command_AddFrameData(this, index, data));
}

// フレームデータ削除コマンド
void CEditData::cmd_delFrameData( QModelIndex &index, int frame )
{
	m_pUndoStack->push( new Command_DelFrameData(this, index, frame));
}

// フレームデータ編集コマンド
void CEditData::cmd_editFrameData( QModelIndex		index,
								 int				frame,
								 FrameData		&data,
								 FrameData		*pOld,
								 QWidget		*animeWidget )
{
	m_pUndoStack->push( new Command_EditFrameData(this, index, frame, data, pOld, animeWidget));
}

// オブジェクトコピー コマンド
void CEditData::cmd_copyObject(QModelIndex &index)
{
	m_pUndoStack->push( new Command_CopyObject(this, index) );
}

// レイヤ 親子移動
void CEditData::cmd_copyIndex(int row, ObjectItem *pItem, QModelIndex parent)
{
	m_pUndoStack->push(new Command_CopyIndex(this, row, pItem, parent)) ;
}

// フレームデータ移動
void CEditData::cmd_moveFrameData(QModelIndex &index, int prevFrame, int nextFrame)
{
	m_pUndoStack->push(new Command_MoveFrameData(this, index, prevFrame, nextFrame)) ;
}

// 全フレームデータ移動
void CEditData::cmd_moveAllFrameData(QModelIndex &index, int prevFrame, int nextFrame)
{
	m_pUndoStack->push(new Command_MoveAllFrameData(this, index, prevFrame, nextFrame)) ;
}

// ツリーアイテム上に移動
void CEditData::cmd_moveItemUp(const QModelIndex &index)
{
	m_pUndoStack->push(new Command_MoveItemUp(this, index)) ;
}

// ツリーアイテム下に移動
void CEditData::cmd_moveItemDown(const QModelIndex &index)
{
	m_pUndoStack->push(new Command_MoveItemDown(this, index)) ;
}

// UVスケール
void CEditData::cmd_changeUvScale(double scale)
{
	m_pUndoStack->push(new Command_ScaleUv(this, scale)) ;
}

// フレームスケール
void CEditData::cmd_changeFrameDataScale(double scale)
{
	m_pUndoStack->push(new Command_ScaleFrame(this, scale)) ;
}

// 現在フレームの全フレームデータペースト
void CEditData::cmd_pasteAllFrame(QModelIndex index, int frame)
{
	m_pUndoStack->push(new Command_PasteAllFrame(this, index, frame)) ;
}

// 現在フレームの全フレームデータ削除
void CEditData::cmd_deleteAllFrame(QModelIndex index, int frame)
{
	m_pUndoStack->push(new Command_DeleteAllFrame(this, index, frame)) ;
}

// レイヤペースト
void CEditData::cmd_pasteLayer(QModelIndex index, ObjectItem *pLayer)
{
	m_pUndoStack->push(new Command_PasteLayer(this, index, pLayer)) ;
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
	ObjectItem *pRoot = getObjectModel()->getItemFromIndex(QModelIndex()) ;
	sortFrameDatas(pRoot) ;
}

void CEditData::sortFrameDatas(ObjectItem *pItem)
{
	pItem->sortFrameData() ;
	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		sortFrameDatas(pItem->child(i)) ;
	}
}
