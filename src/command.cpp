#include "command.h"
#include "animationform.h"
#include "debug.h"

/**
  アイテム追加 コマンド
  */
Command_AddItem::Command_AddItem(CEditData *pEditData, QString &str, QModelIndex &parent) :
	QUndoCommand("Command_AddItem")
{
	// TODO
}

Command_AddItem::redo()
{
}

Command_AddItem::undo()
{
}

/**
  アイテム削除 コマンド
  */
Command_DelItem::Command_DelItem(CEditData *pEditData, QModelIndex &index) :
	QUndoCommand("Command_DelItem")
{
	// TODO
}

Command_DelItem::redo()
{
}

Command_DelItem::undo()
{
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
	CObjectModel::FrameDataList *pFrameDataList = m_pObjModel->getFrameDataListFromID(m_objID, m_layerID) ;
	if ( !pFrameDataList ) {
		qDebug() << "ERROR:Command_AddFrameData::redo" ;
		return ;
	}
	for ( int i = 0 ; i < pFrameDataList->size() ; i ++ ) {
		if ( pFrameDataList->at(i).frame == m_FrameData.frame ) {
			QUndoStack *pStack = m_pEditData->getUndoStack() ;
			for ( int j = 0 ; j < pStack->count() ; j ++ ) {
				const QUndoCommand *cmd = pStack->command(j) ;
				QString str = "[" + QVariant(j).toString() + "]:" + cmd->text() + "\n" ;
				WriteLogFile(str) ;
			}
			QMessageBox::warning(NULL, QObject::tr("Error"), QObject::trUtf8("同じフレームデータがあります。プログラマに報告してください") ) ;
			return ;
		}
	}
	pFrameDataList->append(m_FrameData) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}

}

void Command_AddFrameData::undo()
{
	CObjectModel::FrameDataList *pFrameDataList = m_pObjModel->getFrameDataListFromID(m_objID, m_layerID) ;
	if ( !pFrameDataList ) {
		qDebug() << "ERROR:Command_AddFrameData::undo" ;
		return ;
	}
	m_FrameData = pFrameDataList->takeLast() ;

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
	CObjectModel::FrameDataList *pFrameDataList = m_pObjModel->getFrameDataListFromID(m_objID, m_layerID) ;
	if ( !pFrameDataList ) {
		qDebug() << "ERROR:Command_DelFrameData::redo 0" ;
		return ;
	}
	m_Index = -1 ;
	for ( int i = 0 ; i < pFrameDataList->size() ; i ++ ) {
		if ( m_FrameData.frame != pFrameDataList->at(i).frame ) { continue ; }
		m_Index = i ;
		m_FrameData = pFrameDataList->takeAt(i) ;
	}
	if ( m_Index < 0 ) {
		qDebug() << "ERROR:Command_DelFrameData::redo 1" ;
		return ;
	}

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_DelFrameData::undo()
{
	CObjectModel::FrameDataList *pFrameDataList = m_pObjModel->getFrameDataListFromID(m_objID, m_layerID) ;
	if ( !pFrameDataList ) {
		qDebug() << "ERROR:Command_DelFrameData::undo 0" ;
		return ;
	}
	if ( m_Index < 0 ) {
		qDebug() << "ERROR:Command_DelFrameData::undo 1" ;
		return ;
	}
	for ( int i = 0 ; i < pFrameDataList->size() ; i ++ ) {
		if ( pFrameDataList->at(i).frame == m_FrameData.frame ) {
			QUndoStack *pStack = m_pEditData->getUndoStack() ;
			for ( int j = 0 ; j < pStack->count() ; j ++ ) {
				const QUndoCommand *cmd = pStack->command(j) ;
				QString str = "[" + QVariant(j).toString() + "]:" + cmd->text() + "\n" ;
				WriteLogFile(str) ;
			}
			QMessageBox::warning(NULL, QObject::tr("Error"), QObject::trUtf8("同じフレームデータがあります。プログラマに報告してください") ) ;
			return ;
		}
	}
	pFrameDataList->insert(m_Index, m_FrameData) ;

	m_pEditData->updateSelectData();

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
											 QList<FrameData>	&datas,
											 QList<QWidget *>	&updateWidget) :
	QUndoCommand("Command_EditFrameData")
{
	qDebug() << "Command_EditFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_index		= index ;
	m_Frame		= frame ;
	m_FrameData = datas ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_EditFrameData::redo()
{
	for ( int i = 0 ; i < m_layerIDs.size() ; i ++ ) {
		CObjectModel::FrameData *p = m_pObjModel->getFrameDataFromIDAndFrame(m_objID, m_layerIDs[i], m_Frame) ;
		if ( !p ) {
			qDebug() << "ERROR:Command_EditFrameData::redo 0" << i ;
			continue ;
		}
		m_OldFrameData.insert(i, *p) ;
		if ( i >= m_FrameData.size() ) {
			qDebug() << "ERROR:Command_EditFrameData::redo 1" << i ;
			continue ;
		}
		*p = m_FrameData[i] ;
	}

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_FrameData[0]);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}

void Command_EditFrameData::undo()
{
	for ( int i = 0 ; i < m_layerIDs.size() ; i ++ ) {
		CObjectModel::FrameData *p = m_pObjModel->getFrameDataFromIDAndFrame(m_objID, m_layerIDs[i], m_Frame) ;
		if ( !p ) {
			qDebug() << "ERROR:Command_EditFrameData::undo" << i ;
			continue ;
		}
		*p = m_OldFrameData[i] ;
	}

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_OldFrameData[0]);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}


/**
  オブジェクトコピーコマンド
  */
Command_CopyObject::Command_CopyObject( CEditData *pEditData, CObjectModel::typeID objID, QList<QWidget *> &updateWidget ) :
	QUndoCommand("Command_CopyObject")
{
	m_pEditData			= pEditData ;
	m_objID				= objID ;
	m_UpdateWidgetList	= updateWidget ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::ObjectGroup *pSrcObj = pModel->getObjectGroupFromID(m_objID) ;
	if ( !pSrcObj ) {
		qDebug() << "failed copy object 00" ;
		return ;
	}
	m_objGroup.id = new QStandardItem(objID->text() + "_copy") ;
	m_objGroup.nCurrentLoop = 0 ;
	m_objGroup.nLoop = pSrcObj->nLoop ;

	CObjectModel::LayerGroupList addLayerGroupList ;
	for ( int i = 0 ; i < pSrcObj->layerGroupList.size() ; i ++ ) {
		CObjectModel::LayerGroup *pLayerGroup = &pSrcObj->layerGroupList[i] ;
		CObjectModel::LayerGroup newLayerGroup ;
		newLayerGroup.first = new QStandardItem(pLayerGroup->first->text()) ;
		newLayerGroup.first->setData(pLayerGroup->first->data(Qt::CheckStateRole), Qt::CheckStateRole) ;
		newLayerGroup.second = pLayerGroup->second ;
		addLayerGroupList.append(newLayerGroup) ;
	}
	m_objGroup.layerGroupList = addLayerGroupList ;
}

void Command_CopyObject::redo()
{
	QStandardItem *pRootItem = m_pEditData->getTreeModel()->invisibleRootItem() ;
	pRootItem->appendRow(m_objGroup.id) ;
	for ( int i = 0 ; i < m_objGroup.layerGroupList.size() ; i ++ ) {
		m_objGroup.id->appendRow(m_objGroup.layerGroupList.at(i).first) ;
	}

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->addObject(m_objGroup) ;

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyObject::undo()
{
	QStandardItem *pItem = m_objGroup.id ;
	QStandardItem *pRootItem = m_pEditData->getTreeModel()->invisibleRootItem() ;
	if ( !pRootItem ) {
		qDebug() << "Command_CopyObject::undo pRootItem==NULLpo!!" ;
		return ;
	}
	pRootItem->takeRow(pItem->index().row()) ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->delObjectGroup(m_objGroup.id) ;

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}







