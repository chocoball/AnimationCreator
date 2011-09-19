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
	m_parent = parent ;
}

void Command_AddItem::redo()
{
	QModelIndex index = m_pEditData->getObjectModel()->addItem(m_str, m_parent) ;
	m_row = index.row() ;
}

void Command_AddItem::undo()
{
	m_pEditData->setSelIndex(QModelIndex());
	ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(QModelIndex()) ;
	if ( p ) {
		m_pEditData->getObjectModel()->removeItem();
	}
}

/**
  アイテム削除 コマンド
  */
Command_DelItem::Command_DelItem(CEditData *pEditData, QModelIndex &index) :
	QUndoCommand(QObject::trUtf8("オブジェクト削除"))
{
	m_pEditData = pEditData ;
	m_index = index ;
	m_parentIndex = index.parent() ;
	m_row = index.row() ;
	m_pItem = NULL ;
}

void Command_DelItem::redo()
{
	ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(m_index) ;
	if ( pItem ) {
		if ( !m_pItem ) {
			m_pItem = new ObjectItem(pItem->getName(), pItem->parent()) ;
			m_pItem->copy(pItem) ;
			m_pEditData->getObjectModel()->removeItem(m_index) ;
		}
	}
}

void Command_DelItem::undo()
{
	if ( !m_pItem ) { return ; }

	m_index = m_pEditData->getObjectModel()->insertItem(m_row, m_pItem->getName(), m_parentIndex) ;
	ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(m_index) ;
	if ( p ) {
		p->copy(m_pItem);
		delete m_pItem ;
		m_pItem = NULL ;
	}
}


/**
  フレームデータ追加コマンド
  */
Command_AddFrameData::Command_AddFrameData(CEditData		*pEditData,
										   QModelIndex		&index,
										   FrameData		&data,
										   QList<QWidget *>	&updateWidget) :
	QUndoCommand("Command_AddFrameData")
{
	qDebug() << "Command_AddFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_index		= index ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_AddFrameData::redo()
{
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_AddFrameData::undo()
{
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
	QUndoCommand("Command_DelFrameData")
{
	qDebug() << "Command_DelFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_index		= index ;
	m_FrameData.frame = frame ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_DelFrameData::redo()
{
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_DelFrameData::undo()
{
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
	QUndoCommand("Command_EditFrameData")
{
	qDebug() << "Command_EditFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_index		= index ;
	m_Frame		= frame ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_EditFrameData::redo()
{
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
	QUndoCommand("Command_CopyObject")
{
	m_pEditData			= pEditData ;
	m_index				= index ;
	m_UpdateWidgetList	= updateWidget ;
}

void Command_CopyObject::redo()
{
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyObject::undo()
{
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

/**
  レイヤコピーコマンド
  */
Command_CopyLayer::Command_CopyLayer( CEditData *pEditData, QModelIndex &index, ObjectItem *pLayer, QList<QWidget *> &updateWidget ) :
	QUndoCommand("Command_CopyObject")
{
	m_pEditData			= pEditData ;
	m_index				= index ;
	m_UpdateWidgetList	= updateWidget ;
}

void Command_CopyLayer::redo()
{
}

void Command_CopyLayer::undo()
{
}







