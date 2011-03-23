#include "command.h"
#include "animationform.h"

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
	m_ObjGroup.id = m_pItem ;
	m_ObjGroup.nLoop = 0 ;
	m_ObjGroup.nCurrentLoop = 0 ;
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
		if ( m_pItem != pObjList->at(i).id ) { continue ; }
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
			if ( m_pTreeModel->itemFromIndex(m_ItemIndex) != pList->at(i).id ) { continue ; }
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
Command_AddLayer::Command_AddLayer(CEditData				*pEditData,
								   QModelIndex				objIndex,
								   QStandardItem			*pAddItem,
								   CObjectModel::LayerGroup	&layerGroup,
								   QList<QWidget *>			&updateWidget)
{
	m_pObjModel = pEditData->getObjectModel() ;
	m_pTreeModel = pEditData->getTreeModel() ;
	m_ObjIndex = objIndex ;
	m_pAddItem = pAddItem ;
	m_LayerGroup = layerGroup ;
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
Command_AddFrameData::Command_AddFrameData(CEditData				*pEditData,
										   CObjectModel::typeID		objID,
										   CObjectModel::typeID		layerID,
										   CObjectModel::FrameData	&data,
										   QList<QWidget *>			&updateWidget)
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
Command_DelFrameData::Command_DelFrameData(CEditData			*pEditData,
										   CObjectModel::typeID	objID,
										   CObjectModel::typeID	layerID,
										   int					frame,
										   QList<QWidget *>		&updateWidget)
{
	m_pEditData = pEditData ;
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

	m_pEditData->updateSelectData();

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

	m_pEditData->updateSelectData();

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  フレームデータ編集コマンド
  */
Command_EditFrameData::Command_EditFrameData(CEditData						*pEditData,
											 CObjectModel::typeID			objID,
											 QList<CObjectModel::typeID>	&layerIDs,
											 int							frame,
											 QList<CObjectModel::FrameData>	&datas,
											 QList<QWidget *>				&updateWidget)
{
	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_objID		= objID ;
	m_layerIDs	= layerIDs ;
	m_Frame		= frame ;
	m_FrameData = datas ;
	m_UpdateWidgetList = updateWidget ;
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

/**
  オブジェクトコピーコマンド
  */
Command_CopyObject::Command_CopyObject( CEditData *pEditData, CObjectModel::typeID objID, QList<QWidget *> &updateWidget )
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






