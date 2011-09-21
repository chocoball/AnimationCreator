#include "command.h"
#include "animationform.h"
#include "debug.h"

/**
  アイテム追加 コマンド
  */
Command_AddItem::Command_AddItem(CEditData *pEditData, QString &str, QModelIndex &parent) :
	QUndoCommand(QObject::trUtf8("オブジェクト追加"))
{
	m_pEditData = pEditData ;
	m_str = str ;
	m_parentRow = m_pEditData->getObjectModel()->getRow(parent) ;
	m_row = -1 ;
}

void Command_AddItem::redo()
{
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_parentRow) ;
	m_index = m_pEditData->getObjectModel()->addItem(m_str, index) ;
	m_row = m_pEditData->getObjectModel()->getRow(m_index) ;
}

void Command_AddItem::undo()
{
	if ( m_row < 0 ) { return ; }

//	m_pEditData->setSelIndex(QModelIndex());
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
		m_str = pItem->getName() ;
		m_pEditData->getObjectModel()->removeItem(index) ;
	}
	m_row = -1 ;
}

/**
  アイテム削除 コマンド
  */
Command_DelItem::Command_DelItem(CEditData *pEditData, QModelIndex &index) :
	QUndoCommand(QObject::trUtf8("オブジェクト削除"))
{
	m_pEditData = pEditData ;
	m_relRow = index.row() ;
	m_row = m_pEditData->getObjectModel()->getRow(index) ;
	m_parentRow = m_pEditData->getObjectModel()->getRow(index.parent()) ;
	m_pItem = NULL ;

	qDebug() << "Command_DelItem relRow:" << m_relRow << " row:" << m_row << " parent row:" << m_parentRow ;
}

void Command_DelItem::redo()
{
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
		if ( !m_pItem ) {
			m_pItem = new ObjectItem(pItem->getName(), pItem->parent()) ;
			m_pItem->copy(pItem) ;
			m_pEditData->getObjectModel()->updateIndex() ;

			m_pEditData->getObjectModel()->removeItem(index) ;
			qDebug() << "Command_DelItem redo exec" ;
		}
	}
}

void Command_DelItem::undo()
{
	if ( !m_pItem ) { return ; }

	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_parentRow) ;
	index = m_pEditData->getObjectModel()->insertItem(m_relRow, m_pItem->getName(), index) ;
	ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	if ( p ) {
		p->copy(m_pItem);
		m_pEditData->getObjectModel()->updateIndex() ;
		delete m_pItem ;
		m_pItem = NULL ;
		qDebug() << "Command_DelItem undo exec" ;
	}
}


/**
  フレームデータ追加コマンド
  */
Command_AddFrameData::Command_AddFrameData(CEditData		*pEditData,
										   QModelIndex		&index,
										   FrameData		&data,
										   QList<QWidget *>	&updateWidget) :
	QUndoCommand(QObject::trUtf8("オブジェクト追加"))
{
	qDebug() << "Command_AddFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_frameData			= data ;
	m_UpdateWidgetList	= updateWidget ;
	m_flag				= ObjectItem::kState_Disp ;
}

void Command_AddFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	pItem->addFrameData(m_frameData);
	pItem->setData(m_flag, Qt::CheckStateRole);

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_AddFrameData::undo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	m_flag = pItem->data(Qt::CheckStateRole).toInt() ;
	pItem->removeFrameData(m_frameData.frame) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}



/**
  フレームデータ削除コマンド
  */
Command_DelFrameData::Command_DelFrameData(CEditData			*pEditData,
										   QModelIndex			&index,
										   int					frame,
										   QList<QWidget *>		&updateWidget) :
	QUndoCommand(QObject::trUtf8("フレームデータ削除"))
{
	qDebug() << "Command_DelFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_FrameData.frame	= frame ;
	m_UpdateWidgetList	= updateWidget ;
}

void Command_DelFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_FrameData.frame) ;
	if ( !p ) { return ; }

	m_FrameData = *p ;
	pItem->removeFrameData(m_FrameData.frame);

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_DelFrameData::undo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
		pItem->addFrameData(m_FrameData) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}



/**
  フレームデータ編集コマンド
  */
Command_EditFrameData::Command_EditFrameData(CEditData			*pEditData,
											 QModelIndex		&index,
											 int				frame,
											 FrameData			&data,
											 QList<QWidget *>	&updateWidget) :
	QUndoCommand(QObject::trUtf8("フレームデータ編集"))
{
	qDebug() << "Command_EditFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_row		= m_pObjModel->getRow(index) ;
	m_frame		= frame ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_EditFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( !p ) { return ; }

	m_OldFrameData = *p ;
	*p = m_FrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_FrameData);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}

void Command_EditFrameData::undo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( !p ) { return ; }

	m_FrameData = *p ;
	*p = m_OldFrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_OldFrameData);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}


/**
  オブジェクトコピーコマンド
  */
Command_CopyObject::Command_CopyObject( CEditData *pEditData, QModelIndex &index, QList<QWidget *> &updateWidget ) :
	QUndoCommand(QObject::trUtf8("オブジェクトコピー"))
{
	m_pEditData			= pEditData ;
	m_UpdateWidgetList	= updateWidget ;
	m_pObject			= NULL ;
	m_row				= -1 ;

	ObjectItem *p = pEditData->getObjectModel()->getObject(index) ;
	if ( p ) {
		m_pObject = new ObjectItem(p->getName() + "_copy", NULL) ;
		m_pObject->copy(p) ;
	}
}

void Command_CopyObject::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( m_pObject ) {
		QModelIndex index = pModel->addItem(m_pObject->getName(), QModelIndex()) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		pItem->copy(m_pObject) ;
		pModel->updateIndex() ;
		m_row = pModel->getRow(index) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyObject::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( m_row >= 0 ) {
		QModelIndex index = pModel->getIndex(m_row) ;
		pModel->removeItem(index) ;
		m_row = -1 ;
	}
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

/**
  レイヤコピー
  */
Command_CopyIndex::Command_CopyIndex( CEditData *pEditData, int row, ObjectItem *pLayer, QModelIndex parent, QList<QWidget *> &updateWidget ) :
	QUndoCommand(QObject::trUtf8("レイヤコピー"))
{
	m_pEditData			= pEditData ;
	m_relRow			= row ;
	m_parentRow			= m_pEditData->getObjectModel()->getRow(parent) ;
	m_UpdateWidgetList	= updateWidget ;

	m_pLayer = new ObjectItem(pLayer->getName() + QString("_copy"), NULL) ;
	m_pLayer->copy(pLayer) ;
}

void Command_CopyIndex::redo()
{
	if ( m_pLayer ) {
		CObjectModel *pModel = m_pEditData->getObjectModel() ;
		QModelIndex index ;

		index = pModel->getIndex(m_parentRow) ;
		if ( m_relRow < 0 ) {
			index = pModel->addItem(m_pLayer->getName(), index) ;
		}
		else {
			index = pModel->insertItem(m_relRow, m_pLayer->getName(), index) ;
		}
		if ( index.isValid() ) {
			ObjectItem *pItem = pModel->getItemFromIndex(index) ;
			pItem->copy(m_pLayer) ;
			pModel->updateIndex() ;
			m_row = pModel->getRow(index) ;
		}

//		m_pEditData->setSelIndex(index) ;
	}
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyIndex::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem ;

		delete m_pLayer ;

		pItem = pModel->getItemFromIndex(index) ;
		m_pLayer = new ObjectItem(pItem->getName(), NULL) ;
		m_pLayer->copy(pItem) ;
		pModel->updateIndex() ;
		pModel->removeItem(index) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}







