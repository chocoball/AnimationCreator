#include "command.h"
#include "animationform.h"
#include "debug.h"

void CommandBase::updateAllWidget()
{
	foreach ( QWidget *widget, qApp->allWidgets() ) {
		widget->update() ;
	}
}

void CommandBase::error(QString title, QString text)
{
	if ( qApp->activeWindow() ) {
		QMessageBox::warning(qApp->activeWindow(), title.toUtf8(), text.toUtf8()) ;
	}
	else {
		QString t = title + ":" + text ;
		qDebug() << t ;
	}
}

/**
  アイテム追加 コマンド
  */
Command_AddItem::Command_AddItem(CEditData *pEditData, QString &str, QModelIndex &parent) :
	CommandBase(QObject::trUtf8("オブジェクト追加"))
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

	updateAllWidget();
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

	updateAllWidget();
}

/**
  アイテム削除 コマンド
  */
Command_DelItem::Command_DelItem(CEditData *pEditData, QModelIndex &index) :
	CommandBase(QObject::trUtf8("オブジェクト削除"))
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

			m_pEditData->getObjectModel()->removeItem(index) ;
			m_pEditData->getObjectModel()->updateIndex() ;
			qDebug() << "Command_DelItem redo exec" ;
		}
	}

	updateAllWidget();
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

	updateAllWidget();
}


/**
  フレームデータ追加コマンド
  */
Command_AddFrameData::Command_AddFrameData(CEditData		*pEditData,
										   QModelIndex		&index,
										   FrameData		&data) :
	CommandBase(QObject::trUtf8("オブジェクト追加"))
{
	qDebug() << "Command_AddFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_frameData			= data ;
	m_flag				= ObjectItem::kState_Disp ;
}

void Command_AddFrameData::redo()
{
	qDebug() << "Command_AddFrameData::redo()" ;

	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	qDebug() << "layer name:" << pItem->data(Qt::DisplayRole) ;

	FrameData *p = pItem->getFrameDataPtr(m_frameData.frame) ;
	if ( p ) {
		error("エラー 00", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
		return ;
	}
	if ( m_frameData.frame > 9999 ) {
		error("エラー 01", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
		return ;
	}

	pItem->addFrameData(m_frameData);
	pItem->setData(m_flag, Qt::CheckStateRole);

	updateAllWidget();
}

void Command_AddFrameData::undo()
{
	qDebug() << "Command_AddFrameData::undo()" ;
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	qDebug() << "layer name:" << pItem->data(Qt::DisplayRole) ;
	m_flag = pItem->data(Qt::CheckStateRole).toInt() ;
	pItem->removeFrameData(m_frameData.frame) ;

	updateAllWidget();
}



/**
  フレームデータ削除コマンド
  */
Command_DelFrameData::Command_DelFrameData(CEditData			*pEditData,
										   QModelIndex			&index,
										   int					frame) :
	CommandBase(QObject::trUtf8("フレームデータ削除"))
{
	qDebug() << "Command_DelFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_FrameData.frame	= frame ;
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

	updateAllWidget();
}

void Command_DelFrameData::undo()
{
	if ( m_FrameData.frame > 9999 ) {
		error("エラー 10", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
		return ;
	}

	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
		pItem->addFrameData(m_FrameData) ;
	}

	updateAllWidget();
}



/**
  フレームデータ編集コマンド
  */
Command_EditFrameData::Command_EditFrameData(CEditData			*pEditData,
											 QModelIndex		&index,
											 int				frame,
											 FrameData			&data,
											 FrameData			*pOld,
											 QWidget			*pAnimeWidget) :
	CommandBase(QObject::trUtf8("フレームデータ編集"))
{
	qDebug() << "Command_EditFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_row		= m_pObjModel->getRow(index) ;
	m_frame		= frame ;
	m_FrameData = data ;
	m_pAnimeWidget = pAnimeWidget ;

	if ( pOld ) {
		m_OldFrameData = *pOld ;
		m_bSetOld = true ;
	}
}

void Command_EditFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( !p ) { return ; }

	if ( !m_bSetOld ) {
		m_OldFrameData = *p ;
	}
	m_bSetOld = false ;
	*p = m_FrameData ;

	static_cast<AnimationForm *>(m_pAnimeWidget)->slot_setUI(m_FrameData);

	updateAllWidget();
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

	static_cast<AnimationForm *>(m_pAnimeWidget)->slot_setUI(m_FrameData);
	updateAllWidget();
}


/**
  オブジェクトコピーコマンド
  */
Command_CopyObject::Command_CopyObject( CEditData *pEditData, QModelIndex &index ) :
	CommandBase(QObject::trUtf8("オブジェクトコピー"))
{
	m_pEditData			= pEditData ;
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

	updateAllWidget();
}

void Command_CopyObject::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( m_row >= 0 ) {
		QModelIndex index = pModel->getIndex(m_row) ;
		pModel->removeItem(index) ;
		m_row = -1 ;
	}
	updateAllWidget();
}

/**
  レイヤコピー
  */
Command_CopyIndex::Command_CopyIndex( CEditData *pEditData, int row, ObjectItem *pLayer, QModelIndex parent ) :
	CommandBase(QObject::trUtf8("レイヤコピー"))
{
	m_pEditData			= pEditData ;
	m_relRow			= row ;
	m_parentRow			= m_pEditData->getObjectModel()->getRow(parent) ;

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
	updateAllWidget();
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

	updateAllWidget();
}


/**
  フレームデータ移動
  */
Command_MoveFrameData::Command_MoveFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame) :
	CommandBase(QObject::trUtf8("フレームデータ移動"))
{
	m_pEditData			= pEditData ;
	m_row				= m_pEditData->getObjectModel()->getRow(index) ;
	m_srcFrame			= prevFrame ;
	m_dstFrame			= nextFrame ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex i = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(i) ;
	FrameData *pData ;
	pData = pItem->getFrameDataPtr(m_srcFrame) ;
	if ( pData ) { m_srcData = *pData ; }
	pData = pItem->getFrameDataPtr(m_dstFrame) ;
	if ( pData ) { m_dstData = *pData ; }
}

void Command_MoveFrameData::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	// 移動先にコピー
	FrameData *pData = pItem->getFrameDataPtr(m_dstFrame) ;
	m_srcData.frame = m_dstFrame ;
	if ( pData ) { *pData = m_srcData ; }
	else {
		if ( m_srcData.frame > 9999 ) {
			error("エラー 20", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			return ;
		}
		pItem->addFrameData(m_srcData) ;
	}

	// 移動元を消す
	pItem->removeFrameData(m_srcFrame) ;

	updateAllWidget();
}

void Command_MoveFrameData::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	// 移動元に戻す
	m_srcData.frame = m_srcFrame ;
	FrameData *pData = pItem->getFrameDataPtr(m_srcFrame) ;
	if ( pData ) {	// 移動元にデータがあるのはおかしい
		error("エラー 21", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
		return ;
	}
	pItem->addFrameData(m_srcData) ;

	if ( m_dstData.frame != 0xffff ) {	// 元から移動先のデータがあった場合は戻す
		pData = pItem->getFrameDataPtr(m_dstFrame) ;
		if ( !pData ) {	// 移動先にデータがないのはおかしい
			error("エラー 22", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			return ;
		}
		*pData = m_dstData ;
	}
	else {	// ない場合は消す
		pItem->removeFrameData(m_dstFrame) ;
	}

	updateAllWidget();
}


/**
  全フレームデータ移動
  */
Command_MoveAllFrameData::Command_MoveAllFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame) :
	CommandBase(QObject::trUtf8("全フレームデータ移動"))
{
	m_pEditData			= pEditData ;
	m_row				= m_pEditData->getObjectModel()->getRow(index) ;
	m_srcFrame			= prevFrame ;
	m_dstFrame			= nextFrame ;

	qDebug() << "moveAllFrameData" << "prev" << prevFrame << "next" << nextFrame ;
}

void Command_MoveAllFrameData::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	m_dstDatas.clear() ;
	save_frameData(pItem, m_srcFrame, m_dstFrame) ;

	if ( !pItem->validate() ) {
		error("エラー 30", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
	}

	updateAllWidget();
}

void Command_MoveAllFrameData::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	restore_frameData(pItem, m_srcFrame, m_dstFrame) ;

	if ( !pItem->validate() ) {
		error("エラー 31", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
	}

	updateAllWidget();
}

void Command_MoveAllFrameData::save_frameData(ObjectItem *pItem, int srcFrame, int dstFrame)
{
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	FrameData *pSrc = pItem->getFrameDataPtr(srcFrame) ;
	FrameData *pDst = pItem->getFrameDataPtr(dstFrame) ;
	if ( pSrc ) {
		if ( pDst ) {
			m_dstDatas.append(qMakePair(row, *pDst)) ;
			pItem->removeFrameData(dstFrame) ;
		}
		pSrc->frame = dstFrame ;
		qDebug() << "save_frameData move" << pItem->getName() ;
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_frameData(pItem->child(i), srcFrame, dstFrame) ;
	}
}

void Command_MoveAllFrameData::restore_frameData(ObjectItem *pItem, int srcFrame, int dstFrame)
{
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	FrameData *pSrc = pItem->getFrameDataPtr(srcFrame) ;
	FrameData *pDst = pItem->getFrameDataPtr(dstFrame) ;
	if ( pDst ) {
		if ( pSrc ) {
			error("エラー 32", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			return ;
		}
		pDst->frame = srcFrame ;

		for ( int i = 0 ; i < m_dstDatas.size() ; i ++ ) {
			if ( m_dstDatas.at(i).first != row ) { continue ; }
			pItem->addFrameData(m_dstDatas.at(i).second) ;
			break ;
		}
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		restore_frameData(pItem->child(i), srcFrame, dstFrame) ;
	}
}


/**
  ツリーアイテム上に移動
  */
Command_MoveItemUp::Command_MoveItemUp(CEditData *pEditData, const QModelIndex &index) :
	CommandBase(QObject::trUtf8("アイテム上移動"))
{
	m_pEditData	= pEditData ;
	m_index		= index ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	m_pItem = new ObjectItem(pItem->getName(), NULL) ;
	m_pItem->copy(pItem) ;
}

void Command_MoveItemUp::redo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()-1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;

	updateAllWidget();
}

void Command_MoveItemUp::undo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()+1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;

	updateAllWidget();
}


/**
  ツリーアイテム下に移動
  */
Command_MoveItemDown::Command_MoveItemDown(CEditData *pEditData, const QModelIndex &index) :
	CommandBase(QObject::trUtf8("アイテム下移動"))
{
	m_pEditData	= pEditData ;
	m_index		= index ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	m_pItem = new ObjectItem(pItem->getName(), NULL) ;
	m_pItem->copy(pItem) ;
}

void Command_MoveItemDown::redo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()+1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;

	updateAllWidget();
}

void Command_MoveItemDown::undo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()-1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;

	updateAllWidget();
}


/**
  UVスケール変更
  */
Command_ScaleUv::Command_ScaleUv(CEditData *pEditData, double scale) :
	CommandBase(QObject::trUtf8("UVスケール変更"))
{
	m_pEditData = pEditData ;
	m_scale = scale ;
}

void Command_ScaleUv::redo()
{
	m_changeFrameDatas.clear();

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < pModel->rowCount(QModelIndex()) ; i ++ ) {
		QModelIndex index = pModel->index(i) ;
		ObjectItem *pObj = pModel->getObject(index) ;
		if ( !pObj ) { continue ; }
		save_framedata(pObj) ;
	}

	updateAllWidget();
}

void Command_ScaleUv::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_changeFrameDatas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_changeFrameDatas.at(i) ;
		QModelIndex index = pModel->getIndex(d.first) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		if ( !pItem ) { continue ; }
		FrameData *pFrameData = pItem->getFrameDataPtr(d.second.frame) ;
		if ( !pFrameData ) {
			qDebug() << "Command_ScaleUv pFrameData==NULLpo" ;
			continue ;
		}
		*pFrameData = d.second ;
	}

	updateAllWidget();
}

void Command_ScaleUv::save_framedata(ObjectItem *pItem)
{
	int i ;
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	int frameNum = pItem->getMaxFrameNum(false) ;
	for ( i = 0 ; i <= frameNum ; i ++ ) {
		FrameData *p = pItem->getFrameDataPtr(i) ;
		if ( !p ) { continue ; }
//		if ( p->nImage != m_imageNo ) { continue ; }

		QPair<int, FrameData> d = qMakePair(row, *p) ;
		m_changeFrameDatas.append(d) ;

		p->left *= m_scale ;
		p->right *= m_scale ;
		p->top *= m_scale ;
		p->bottom *= m_scale ;
		p->pos_x *= m_scale ;
		p->pos_y *= m_scale ;
		p->center_x *= m_scale ;
		p->center_y *= m_scale ;
//		p->fScaleX *= m_scale ;
//		p->fScaleY *= m_scale ;
	}

	for ( i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_framedata(pItem->child(i)) ;
	}
}


/**
  フレームスケール変更
  */
Command_ScaleFrame::Command_ScaleFrame(CEditData *pEditData, double scale) :
	CommandBase(QObject::trUtf8("フレームスケール変更"))
{
	m_pEditData = pEditData ;
	m_scale = scale ;
}

void Command_ScaleFrame::redo()
{
	m_changeFrameDatas.clear();

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = m_pEditData->getSelIndex() ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	save_framedata(pItem) ;
	if ( !pItem->validate() ) {
		error("エラー 40", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
	}

	updateAllWidget();
}

void Command_ScaleFrame::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_changeFrameDatas.size() ; i ++ ) {
		const QPair<int, QList<FrameData> > d = m_changeFrameDatas.at(i) ;
		QModelIndex index = pModel->getIndex(d.first) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		if ( !pItem ) { continue ; }
		pItem->setFrameDatas(d.second) ;
	}

	updateAllWidget();
}

void Command_ScaleFrame::save_framedata(ObjectItem *pItem)
{
	int i ;
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	pItem->sortFrameData() ;
	const QList<FrameData> &olds = pItem->getFrameData() ;
	m_changeFrameDatas.append(qMakePair(row, olds)) ;

	QList<FrameData> news ;
	for ( i = 0 ; i < olds.size() ; i ++ ) {
		FrameData d = olds[i] ;
		d.frame *= m_scale ;
		for ( int j = 0 ; j < news.size() ; j ++ ) {
			if ( news.at(j).frame == d.frame ) {
				d.frame ++ ;
				j = -1 ;
			}
		}
		news.append(d) ;
	}
	pItem->setFrameDatas(news) ;

	for ( i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_framedata(pItem->child(i)) ;
	}
}


/**
  全フレームデータペースト
  */
Command_PasteAllFrame::Command_PasteAllFrame(CEditData *pEditData, QModelIndex index, int frame) :
	CommandBase(QObject::trUtf8("全フレームデータペースト"))
{
	m_pEditData = pEditData ;
	m_objRow = m_pEditData->getObjectModel()->getRow(index) ;
	m_frame = frame ;

	m_copyObjRow = m_pEditData->getAllFrameDataObjRow() ;
	m_copyDatas = m_pEditData->getAllFrameDatas() ;
}

void Command_PasteAllFrame::redo()
{
	m_changeFrameDatas.clear() ;

	if ( m_copyObjRow != m_objRow ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;

	for ( int i = 0 ; i < m_copyDatas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_copyDatas.at(i) ;
		ObjectItem *pItem = pModel->getItemFromIndex(pModel->getIndex(d.first)) ;
		if ( !pItem ) { continue ; }
		FrameData *pFrame = pItem->getFrameDataPtr(m_frame) ;
		if ( pFrame ) {
			m_changeFrameDatas.append(qMakePair(d.first, *pFrame)) ;
			*pFrame = d.second ;
			pFrame->frame = m_frame ;
		}
		else {
			FrameData data = d.second ;
			data.frame = m_frame ;
			pItem->addFrameData(data) ;
			qDebug() << "Command_PasteAllFrame add frame:" << m_frame << " name:" << pItem->getName() ;
		}
	}
	updateAllWidget();
}

void Command_PasteAllFrame::undo()
{
	if ( m_copyObjRow != m_objRow ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_copyDatas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_copyDatas.at(i) ;
		ObjectItem *pItem = pModel->getItemFromIndex(pModel->getIndex(d.first)) ;
		if ( !pItem ) { continue ; }
		pItem->removeFrameData(m_frame) ;
	}

	for ( int i = 0 ; i < m_changeFrameDatas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_changeFrameDatas.at(i) ;
		ObjectItem *pItem = pModel->getItemFromIndex(pModel->getIndex(d.first)) ;
		if ( !pItem ) {
			error("エラー 50", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			continue ;
		}
		if ( pItem->getFrameDataPtr(m_frame) ) {
			error("エラー 51", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			pItem->removeFrameData(m_frame) ;
		}
		pItem->addFrameData(d.second) ;
	}
	updateAllWidget();
}


/**
  全フレームデータ削除
  */
Command_DeleteAllFrame::Command_DeleteAllFrame(CEditData *pEditData, QModelIndex index, int frame) :
	CommandBase(QObject::trUtf8("全フレームデータ削除"))
{
	m_pEditData = pEditData ;
	m_objRow = m_pEditData->getObjectModel()->getRow(index) ;
	m_frame = frame ;
}

void Command_DeleteAllFrame::redo()
{
	m_datas.clear() ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_objRow) ;
	ObjectItem *pObj = pModel->getObject(index) ;
	if ( !pObj ) { return ; }

	save_framedata(pObj) ;
	updateAllWidget();
}

void Command_DeleteAllFrame::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_datas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_datas.at(i) ;
		ObjectItem *pItem = pModel->getItemFromIndex(pModel->getIndex(d.first)) ;
		if ( !pItem ) { continue ; }

		if ( pItem->getFrameDataPtr(m_frame) ) {
			error("エラー 60", "不正なフレームデータが登録されました。直ちにプログラマに相談してください") ;
			pItem->removeFrameData(m_frame) ;
		}
		pItem->addFrameData(d.second) ;
	}
	updateAllWidget();
}

void Command_DeleteAllFrame::save_framedata(ObjectItem *pItem)
{
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( p ) {
		int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;
		m_datas.append(qMakePair(row, *p)) ;

		pItem->removeFrameData(m_frame) ;
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_framedata(pItem->child(i)) ;
	}
}







