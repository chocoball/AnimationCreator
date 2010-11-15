#include "command.h"

/**
  オブジェクト追加コマンド
  */
Command_AddObject::Command_AddObject(CEditData *pEditData, QString &str)
{
	m_strObjName = str ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_pTreeModel = pEditData->getTreeModel() ;
	m_pItem = NULL ;

	m_pItem = new QStandardItem(m_strObjName) ;
	QStandardItem *pRoot = m_pTreeModel->invisibleRootItem() ;
	m_ItemIndex = pRoot->rowCount() ;

	CObjectModel::ObjectList *pObjList = m_pObjModel->getObjectListPtr() ;
	m_objListIndex = pObjList->size() ;
	m_ObjGroup.first = m_pItem ;
}

void Command_AddObject::undo()
{
	qDebug() << "Command_AddObject::undo" ;

	QStandardItem *pRoot = m_pTreeModel->invisibleRootItem() ;
	QList<QStandardItem *> list = pRoot->takeRow(m_ItemIndex) ;
	if ( list.size() == 1 ) {
		m_pItem = list[0] ;
	}
	else {
		qDebug() << "ERROR:Command_AddObject::undo " << list.size() ;
	}

	CObjectModel::ObjectList *pObjList = m_pObjModel->getObjectListPtr() ;
	for ( int i = 0 ; i < pObjList->size() ; i ++ ) {
		if ( m_pItem != pObjList->at(i).first ) { continue ; }
		m_objListIndex = i ;
		m_ObjGroup = pObjList->takeAt(i) ;
		break ;
	}
}

void Command_AddObject::redo()
{
	qDebug() << "Command_AddObject::redo" ;

	QStandardItem *pRoot = m_pTreeModel->invisibleRootItem() ;
	pRoot->insertRow(m_ItemIndex, m_pItem);

	CObjectModel::ObjectList *pObjList = m_pObjModel->getObjectListPtr() ;
	pObjList->insert(m_objListIndex, m_ObjGroup);
}


/**
  オブジェクト削除コマンド
  */
Command_DelObject::Command_DelObject(CEditData *pEditData, QModelIndex index, QLabel *pDataMakerLabel)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_pTreeModel = pEditData->getTreeModel() ;
	m_ItemIndex = index ;
	m_pDataMarkerLabel = pDataMakerLabel ;
}

void Command_DelObject::undo()
{
	if ( m_Index < 0 ) {
		qDebug() << "ERROR:Command_DelObject::undo" ;
		return ;
	}

	QStandardItem *pParent = (QStandardItem *)m_ItemIndex.internalPointer() ;
	pParent->insertRow(m_ItemIndex.row(), m_ItemList);
	if ( m_ItemIndex.internalPointer() == m_pTreeModel->invisibleRootItem() ) {	// オブジェクト
		CObjectModel::ObjectList *pList = m_pObjModel->getObjectListPtr() ;

		pList->insert(m_Index, m_ObjGroup);
	}
	else {
		CObjectModel::LayerGroupList *pLayerGroupList = m_pObjModel->getLayerGroupListFromID(m_pTreeModel->itemFromIndex(m_ItemIndex.parent())) ;
		pLayerGroupList->insert(m_Index, m_LayerGroup);
	}
}

void Command_DelObject::redo()
{
	m_Index = -1 ;
	if ( m_ItemIndex.internalPointer() == m_pTreeModel->invisibleRootItem() ) {	// オブジェクト
		CObjectModel::ObjectList *pList = m_pObjModel->getObjectListPtr() ;
		for ( int i = 0 ; i < pList->size() ; i ++ ) {
			if ( m_pTreeModel->itemFromIndex(m_ItemIndex) != pList->at(i).first ) { continue ; }
			m_Index = i ;
			m_ObjGroup = pList->takeAt(i) ;
			break ;
		}
	}
	else {	// レイヤ
		CObjectModel::LayerGroupList *pLayerGroupList = m_pObjModel->getLayerGroupListFromID(m_pTreeModel->itemFromIndex(m_ItemIndex.parent())) ;
		for ( int i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			if ( m_pTreeModel->itemFromIndex(m_ItemIndex) != pLayerGroupList->at(i).first ) { continue ; }
			m_Index = i ;
			m_LayerGroup = pLayerGroupList->takeAt(i) ;
			break ;
		}
	}

	if ( m_Index < 0 ) {
		qDebug() << "ERROR:Command_DelObject:redo" ;
		return ;
	}

	QStandardItem *pParent = (QStandardItem *)m_ItemIndex.internalPointer() ;
	m_ItemList = pParent->takeRow(m_ItemIndex.row()) ;
}


/**
  レイヤ追加コマンド
  */
Command_AddLayer::Command_AddLayer(CEditData *pEditData,
								   QModelIndex objIndex,
								   QStandardItem *pAddItem,
								   CObjectModel::FrameData frameData,
								   QList<QWidget *> &updateWidget)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_pTreeModel = pEditData->getTreeModel() ;
	m_ObjIndex = objIndex ;
	m_pAddItem = pAddItem ;
	m_LayerGroup.first = pAddItem ;
	m_LayerGroup.second.append(frameData) ;
	m_UpdateWidgetList = updateWidget ;
}

void Command_AddLayer::undo()
{
	QStandardItem *pParentItem = m_pTreeModel->itemFromIndex(m_ObjIndex) ;
	if ( !pParentItem ) {
		qDebug() << "ERROR:Command_AddLayer::undo 0" ;
		return ;
	}

	pParentItem->takeRow(m_ItemIndex) ;

	CObjectModel::LayerGroupList *pLayerGroupList = m_pObjModel->getLayerGroupListFromID(pParentItem) ;
	if ( !pLayerGroupList ) {
		qDebug() << "ERROR:Command_AddLayer::undo 1" ;
		return ;
	}
	pLayerGroupList->takeLast() ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_AddLayer::redo()
{
	QStandardItem *pParentItem = m_pTreeModel->itemFromIndex(m_ObjIndex) ;
	if ( !pParentItem ) {
		qDebug() << "ERROR:Command_AddLayer::redo 0" ;
		return ;
	}

	pParentItem->appendRow(m_pAddItem);
	m_ItemIndex = m_pAddItem->index().row() ;

	CObjectModel::LayerGroupList *pLayerGroupList = m_pObjModel->getLayerGroupListFromID(pParentItem) ;
	if ( !pLayerGroupList ) {
		qDebug() << "ERROR:Command_AddLayer::redo 1" ;
		return ;
	}
	pLayerGroupList->append(m_LayerGroup) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  フレームデータ追加コマンド
  */
Command_AddFrameData::Command_AddFrameData(CEditData *pEditData,
										   CObjectModel::typeID objID,
										   CObjectModel::typeID layerID,
										   CObjectModel::FrameData &data,
										   QList<QWidget *> &updateWidget)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_objID = objID ;
	m_layerID = layerID ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;
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

void Command_AddFrameData::redo()
{
	CObjectModel::FrameDataList *pFrameDataList = m_pObjModel->getFrameDataListFromID(m_objID, m_layerID) ;
	if ( !pFrameDataList ) {
		qDebug() << "ERROR:Command_AddFrameData::redo" ;
		return ;
	}
	pFrameDataList->append(m_FrameData) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  フレームデータ削除コマンド
  */
Command_DelFrameData::Command_DelFrameData(CEditData *pEditData,
					 CObjectModel::typeID objID,
					 CObjectModel::typeID layerID,
					 int frame,
					 QList<QWidget *> &updateWidget)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_objID = objID ;
	m_layerID = layerID ;
	m_FrameData.frame = frame ;
	m_UpdateWidgetList = updateWidget ;
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
	pFrameDataList->insert(m_Index, m_FrameData) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
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

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  フレームデータ編集コマンド
  */
Command_EditFrameData::Command_EditFrameData(CEditData				*pEditData,
											 CObjectModel::typeID		objID,
											 CObjectModel::typeID		layerID,
											 int						frame,
											 CObjectModel::FrameData	&data,
											 QList<QWidget *>			&updateWidget,
											 int						id)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_objID		= objID ;
	m_layerID	= layerID ;
	m_Frame		= frame ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;
	m_ID = 1 ;
}

void Command_EditFrameData::undo()
{
	CObjectModel::FrameData *p = m_pObjModel->getFrameDataFromIDAndFrame(m_objID, m_layerID, m_Frame) ;
	if ( !p ) {
		qDebug() << "ERROR:Command_EditFrameData::undo" ;
		return ;
	}
	*p = m_OldFrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_EditFrameData::redo()
{
	CObjectModel::FrameData *p = m_pObjModel->getFrameDataFromIDAndFrame(m_objID, m_layerID, m_Frame) ;
	if ( !p ) {
		qDebug() << "ERROR:Command_EditFrameData::redo" ;
		return ;
	}
	m_OldFrameData = *p ;
	*p = m_FrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

bool Command_EditFrameData::mergeWith(const QUndoCommand *other)
{
#if 1
	if ( other->id() < 0 ) {
		return false ;
	}
	Command_EditFrameData *p = (Command_EditFrameData *)other ;
	if ( *this != *p ) {
		return false ;
	}

	m_pObjModel			= p->m_pObjModel ;
	m_objID				= p->m_objID ;
	m_layerID			= p->m_layerID ;
	m_Frame				= p->m_Frame ;
	m_FrameData			= p->m_FrameData ;
	m_UpdateWidgetList	= p->m_UpdateWidgetList ;
	return true ;
#else
	if ( id() != other->id() ) {
		return false ;
	}
	Command_EditFrameData *p = (Command_EditFrameData *)other ;
	m_pObjModel = p->m_pObjModel ;
	m_objID = p->m_objID ;
	m_layerID = p->m_layerID ;
	m_Frame = p->m_Frame ;
	m_FrameData = p->m_FrameData ;
	m_UpdateWidgetList = p->m_UpdateWidgetList ;
	return true ;
#endif
}








