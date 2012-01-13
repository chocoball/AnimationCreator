#include "canm2d.h"


/************************************************************
*
* CAnm2DBin
*
************************************************************/
CAnm2DBin::CAnm2DBin()
	: CAnm2DBase()
{
}

// 作成中のデータをアニメデータに変換
bool CAnm2DBin::makeFromEditData( CEditData &rEditData )
{
	QByteArray header ;
	QList<QByteArray> objectList ;
	QList<QByteArray> layerList ;
	QList<QByteArray> frameList ;
	QList<QByteArray> imageList ;

	ObjectItem *pRoot = rEditData.getObjectModel()->getItemFromIndex(QModelIndex()) ;
	for ( int i = 0 ; i < pRoot->childCount() ; i ++ ) {
		if ( !makeObject(pRoot->child(i), objectList, layerList, frameList) ) {
			return false ;
		}
	}

	if ( !makeHeader(header, rEditData, objectList, layerList, frameList) ) {
		return false ;
	}
	if ( !makeImageList(imageList, rEditData) ) {
		return false ;
	}

	// ヘッダのオフセットを設定
	Anm2DHeader *pHeader = (Anm2DHeader *)header.data() ;
	unsigned int offset = 0 ;
	unsigned int blockCnt = 0 ;
	offset += header.size() ;
	for ( int i = 0 ; i < objectList.size() ; i ++ ) {
		pHeader->nBlockOffset[blockCnt ++] = offset ;
		offset += objectList[i].size() ;
	}
	for ( int i = 0 ; i < layerList.size() ; i ++ ) {
		pHeader->nBlockOffset[blockCnt ++] = offset ;
		offset += layerList[i].size() ;
	}
	for ( int i = 0 ; i < frameList.size() ; i ++ ) {
		pHeader->nBlockOffset[blockCnt ++] = offset ;
		offset += frameList[i].size() ;
	}
	for ( int i = 0 ; i < imageList.size() ; i ++ ) {
		pHeader->nBlockOffset[blockCnt ++] = offset ;
		offset += imageList[i].size() ;
	}
	pHeader->nFileSize = offset ;

	if ( pHeader->nBlockNum != blockCnt ) {
		m_nError = kErrorNo_BlockNumNotSame ;
		return false ;
	}

	// データセット
	m_Data.clear() ;
	m_Data += header ;
	for ( int i = 0 ; i < objectList.size() ; i ++ ) {
		m_Data += objectList[i] ;
	}
	for ( int i = 0 ; i < layerList.size() ; i ++ ) {
		m_Data += layerList[i] ;
	}
	for ( int i = 0 ; i < frameList.size() ; i ++ ) {
		m_Data += frameList[i] ;
	}
	for ( int i = 0 ; i < imageList.size() ; i ++ ) {
		m_Data += imageList[i] ;
	}
	return true ;
}

// アニメファイルから作成中データへ変換
bool CAnm2DBin::makeFromFile(QByteArray &data, CEditData &rEditData)
{
	m_Data = data ;

	Anm2DHeader *pHeader = (Anm2DHeader *)m_Data.data() ;
	if ( pHeader->header.nID != kANM2D_ID_HEADER ) {
		m_nError = kErrorNo_InvalidID ;
		return false ;
	}
	if ( pHeader->nFileSize != (unsigned int)m_Data.size() ) {
		m_nError = kErrorNo_InvalidFileSize ;
		return false ;
	}
	if ( pHeader->nVersion != kANM2D_VERSION ) {
		m_nError = kErrorNo_InvalidVersion ;
		return false ;
	}
	if ( !addList(pHeader) ) { return false ; }
	if ( !addModel(rEditData) ) { return false ; }
	if ( !addImageData(pHeader, rEditData) ) { return false ; }

	return true ;
}


// ヘッダ作成
bool CAnm2DBin::makeHeader( QByteArray &rData, CEditData &rEditData, QList<QByteArray> &objectList, QList<QByteArray> &layerList, QList<QByteArray> &frameList )
{
	int blockNum = 0 ;
	blockNum += objectList.size() ;
	blockNum += layerList.size() ;
	blockNum += frameList.size() ;
	blockNum += rEditData.getImageDataListSize() ;

	rData.resize(sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int));
	Anm2DHeader *pHeader = (Anm2DHeader *)rData.data() ;
	memset( pHeader, 0, sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int) ) ;
	pHeader->header.nID		= kANM2D_ID_HEADER ;
	pHeader->header.nSize	= sizeof( Anm2DHeader ) ;
	pHeader->nVersion		= kANM2D_VERSION ;
	pHeader->nFileSize		= 0 ;	// 後で入れる
	pHeader->nBlockNum		= blockNum ;
	return true ;
}

bool CAnm2DBin::makeObject(ObjectItem *pObj, QList<QByteArray> &objList, QList<QByteArray> &layerList, QList<QByteArray> &frameList)
{
	int layerNum = pObj->childCount() ;

	QByteArray objArray ;
	objArray.resize(sizeof(Anm2DObject) + (layerNum-1)*sizeof(unsigned int)) ;
	Anm2DObject *pObjData = (Anm2DObject *)objArray.data() ;
	memset( pObjData, 0, sizeof(Anm2DObject) + (layerNum-1)*sizeof(unsigned int) ) ;
	pObjData->header.nID	= kANM2D_ID_OBJECT ;
	pObjData->header.nSize	= sizeof(Anm2DObject) + (layerNum-1)*sizeof(unsigned int) ;
	strncpy( pObjData->objName.name, pObj->getName().toUtf8().data(), sizeof(Anm2DName) ) ;	// オブジェクト名
	pObjData->nLayerNum		= layerNum ;
	pObjData->nLoopNum		= pObj->getLoop() ;	// ループ回数(after ver 0.1.0)

	int layerNo = 0 ;
	for ( int i = 0 ; i < pObj->childCount() ; i ++ ) {
		if ( !makeLayer(pObj->child(i), &layerNo, pObjData, layerList, frameList, -1) ) {
			return false ;
		}
	}

	objList << objArray ;
	return true ;
}

bool CAnm2DBin::makeLayer(ObjectItem		*pLayer,
						  int				*pLayerNo,
						  Anm2DObject		*pObjData,
						  QList<QByteArray>	&layerList,
						  QList<QByteArray>	&frameList,
						  int				parentNo)
{
	const QList<FrameData> &frameDatas = pLayer->getFrameData() ;
	QByteArray layerArray ;
	layerArray.resize(sizeof(Anm2DLayer) + (frameDatas.size()-1)*sizeof(unsigned int));
	Anm2DLayer *pLayerData = (Anm2DLayer *)layerArray.data() ;
	memset(pLayerData, 0, sizeof(Anm2DLayer) + (frameDatas.size()-1)*sizeof(unsigned int)) ;
	pLayerData->header.nID   = kANM2D_ID_LAYER ;
	pLayerData->header.nSize = sizeof(Anm2DLayer) + (frameDatas.size()-1)*sizeof(unsigned int) ;
	strncpy( pLayerData->layerName.name, pLayer->getName().toUtf8().data(), sizeof(Anm2DName) ) ;	// レイヤ名
	pLayerData->nLayerNo = layerList.size() ;
	pLayerData->nFrameDataNum = frameDatas.size() ;
	pLayerData->nParentNo = parentNo ;

	if ( pObjData ) {
		pObjData->nLayerNo[*pLayerNo] = pLayerData->nLayerNo ;	// オブジェクトが参照するレイヤ番号セット
		*pLayerNo += 1 ;
	}

	for ( int i = 0 ; i < frameDatas.size() ; i ++ ) {
		const FrameData &data = frameDatas.at(i) ;

		QByteArray frameDataArray ;
		frameDataArray.resize(sizeof(Anm2DFrameData)) ;
		Anm2DFrameData *pFrameData = (Anm2DFrameData *)frameDataArray.data() ;
		memset(pFrameData, 0, sizeof(Anm2DFrameData)) ;
		pFrameData->header.nID		= kANM2D_ID_FRAMEDATA ;
		pFrameData->header.nSize	= sizeof(Anm2DFrameData) ;
		pFrameData->nFrameDataNo	= frameList.size() ;
		pFrameData->nFrame			= data.frame ;
		pFrameData->pos_x			= data.pos_x ;
		pFrameData->pos_y			= data.pos_y ;
		pFrameData->pos_z			= data.pos_z ;
		pFrameData->rot_x			= data.rot_x ;
		pFrameData->rot_y			= data.rot_y ;
		pFrameData->rot_z			= data.rot_z ;
		pFrameData->cx				= data.center_x ;
		pFrameData->cy				= data.center_y ;
		pFrameData->nImageNo		= data.nImage ;
		pFrameData->uv[0]			= data.left ;
		pFrameData->uv[1]			= data.right ;
		pFrameData->uv[2]			= data.top ;
		pFrameData->uv[3]			= data.bottom ;
		pFrameData->fScaleX			= data.fScaleX ;
		pFrameData->fScaleY			= data.fScaleY ;
		pFrameData->bFlag			= (data.bUVAnime) ? 1 : 0 ;
		// 頂点色(after ver 0.1.0)
		pFrameData->rgba[0]			= data.rgba[0] ;
		pFrameData->rgba[1]			= data.rgba[1] ;
		pFrameData->rgba[2]			= data.rgba[2] ;
		pFrameData->rgba[3]			= data.rgba[3] ;

		pLayerData->nFrameDataNo[i] = pFrameData->nFrameDataNo ;	// レイヤが参照するフレームデータ番号セット

		frameList << frameDataArray ;
	}

	layerList << layerArray ;

	for ( int i = 0 ; i < pLayer->childCount() ; i ++ ) {
		if ( !makeLayer(pLayer->child(i), pLayerNo, NULL, layerList, frameList, pLayerData->nLayerNo) ) {
			return false ;
		}
	}
	return true ;
}

// イメージデータ作成
bool CAnm2DBin::makeImageList( QList<QByteArray> &rData, CEditData &rEditData )
{
	int imageNum = rEditData.getImageDataListSize() ;

	for ( int i = 0 ; i < imageNum ; i ++ ) {
		CEditData::ImageData *p = rEditData.getImageData(i) ;
		if ( !p ) { continue ; }

		QImage img = p->Image ;
		unsigned int size = sizeof(Anm2DImage) + img.width() * img.height() * 4 * sizeof(unsigned char) - 1 ;
		size = (size+0x03) & ~0x03 ;

		QByteArray imgArray ;
		imgArray.resize(size);
		Anm2DImage *pImage = (Anm2DImage *)imgArray.data() ;
		memset( pImage, 0, size ) ;
		pImage->header.nID		= kANM2D_ID_IMAGE ;
		pImage->header.nSize	= size ;
		pImage->nWidth			= img.width() ;
		pImage->nHeight			= img.height() ;
		pImage->nImageNo		= p->nNo ;
		QString relPath = getRelativePath(m_filePath, p->fileName) ;
		strncpy(pImage->fileName, relPath.toUtf8().data(), 255) ;
		memcpy(pImage->data, img.bits(), img.width()*img.height()*4) ;

		rData << imgArray ;
	}
	return true ;
}

bool CAnm2DBin::addList(Anm2DHeader *pHeader)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
			case kANM2D_ID_OBJECT:
				m_ObjPtrList.append((Anm2DObject *)p) ;
				break ;
			case kANM2D_ID_LAYER:
				m_LayerPtrList.append((Anm2DLayer *)p) ;
				break ;
			case kANM2D_ID_FRAMEDATA:
				m_FrameDataPtrList.append((Anm2DFrameData *)p) ;
				break ;
			case kANM2D_ID_IMAGE:
				continue ;

			default:
				m_nError = kErrorNo_InvalidID ;
				return false ;
		}
	}
	return true ;
}

bool CAnm2DBin::addModel(CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	for ( int i = 0 ; i < m_ObjPtrList.size() ; i ++ ) {
		const Anm2DObject *pObj = m_ObjPtrList.at(i) ;
		QModelIndex index = pModel->addItem(QString(pObj->objName.name), QModelIndex()) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		pItem->setLoop(pObj->nLoopNum) ;

		for ( int j = 0 ; j < pObj->nLayerNum ; j ++ ) {
			if ( !addModel_Layer(index, pObj->nLayerNo[j], pModel) ) { return false ; }
		}
	}
	return true ;
}

bool CAnm2DBin::addModel_Layer(QModelIndex &parent, int layerNo, CObjectModel *pModel)
{
	if ( layerNo < 0 || layerNo >= m_LayerPtrList.size() ) {
		m_nError = kErrorNo_InvalidLayerNum ;
		return false ;
	}

	const Anm2DLayer *pLayer = m_LayerPtrList.at(layerNo) ;
	QModelIndex index = pModel->addItem(QString(pLayer->layerName.name), parent) ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	pItem->setData(ObjectItem::kState_Disp, Qt::CheckStateRole) ;

	for ( int i = 0 ; i < pLayer->nFrameDataNum ; i ++ ) {
		const Anm2DFrameData *pFrame = m_FrameDataPtrList.at(pLayer->nFrameDataNo[i]) ;
		FrameData data ;
		data.frame = pFrame->nFrame ;
		data.pos_x = pFrame->pos_x ;
		data.pos_y = pFrame->pos_y ;
		data.pos_z = pFrame->pos_z ;
		data.rot_x = pFrame->rot_x ;
		data.rot_y = pFrame->rot_y ;
		data.rot_z = pFrame->rot_z ;
		data.center_x = pFrame->cx ;
		data.center_y = pFrame->cy ;
		data.nImage = pFrame->nImageNo ;
		data.left = pFrame->uv[0] ;
		data.right = pFrame->uv[1] ;
		data.top = pFrame->uv[2] ;
		data.bottom = pFrame->uv[3] ;
		data.fScaleX = pFrame->fScaleX ;
		data.fScaleY = pFrame->fScaleY ;
		data.bUVAnime = pFrame->bFlag ? true : false ;
		data.rgba[0] = pFrame->rgba[0] ;
		data.rgba[1] = pFrame->rgba[1] ;
		data.rgba[2] = pFrame->rgba[2] ;
		data.rgba[3] = pFrame->rgba[3] ;

		pItem->addFrameData(data) ;
	}

	QList<int> childList ;
	for ( int i = 0 ; i < m_LayerPtrList.size() ; i ++ ) {
		if ( m_LayerPtrList.at(i)->nParentNo == layerNo ) {
			childList << i ;
		}
	}
	for ( int i = 0 ; i < childList.size() ; i ++ ) {
		if ( !addModel_Layer(index, childList[i], pModel) ) { return false ; }
	}
	return true ;
}

// 編集データにイメージを追加
bool CAnm2DBin::addImageData(Anm2DHeader *pHeader, CEditData &rEditData)
{
	QList<CEditData::ImageData> data ;
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case kANM2D_ID_IMAGE:
			{
				Anm2DImage *pImage = (Anm2DImage *)p ;

				QImage image ;
				QString fileName = QString::fromUtf8(pImage->fileName) ;
				QString path = getAbsolutePath(m_filePath, fileName) ;
				if ( !image.load(path) ) {
					qDebug() << "not load:" << path << fileName ;
					image = QImage(pImage->nWidth, pImage->nHeight, QImage::Format_ARGB32) ;
					unsigned int *pCol = (unsigned int *)pImage->data ;
					for ( int y = 0 ; y < pImage->nHeight ; y ++ ) {
						for ( int x = 0 ; x < pImage->nWidth ; x ++ ) {
							image.setPixel(x, y, *pCol);
							pCol ++ ;
						}
					}
				}

				qDebug("w:%d h:%d, %d %d", image.width(), image.height(), pImage->nWidth, pImage->nHeight) ;

				CEditData::ImageData ImageData ;
				ImageData.Image = image ;
				ImageData.nTexObj = 0 ;
				ImageData.fileName = path ;
				ImageData.lastModified = QDateTime::currentDateTimeUtc() ;
				ImageData.nNo = pImage->nImageNo ;
				data.insert(pImage->nImageNo, ImageData);
			}
			break ;
		case kANM2D_ID_OBJECT:
		case kANM2D_ID_LAYER:
		case kANM2D_ID_FRAMEDATA:
			continue ;

		default:
			m_nError = kErrorNo_InvalidID ;
			return false ;
		}
	}
	rEditData.setImageData(data);
	return true ;
}

// 名前からオブジェクトを探す
Anm2DObject *CAnm2DBin::search2DObjectFromName(Anm2DHeader *pHeader, QString name)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DObject *p = (Anm2DObject *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		if ( p->header.nID != kANM2D_ID_OBJECT ) { continue ; }
		if ( name != QString::fromUtf8(p->objName.name) ) { continue ; }

		return p ;
	}
	return NULL ;
}

// 名前からレイヤを探す
Anm2DLayer *CAnm2DBin::search2DLayerFromName(Anm2DHeader *pHeader, QString name)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DLayer *p = (Anm2DLayer *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		if ( p->header.nID != kANM2D_ID_LAYER ) { continue ; }
		if ( name != QString::fromUtf8(p->layerName.name) ) { continue ; }

		return p ;
	}
	return NULL ;
}


/************************************************************
*
* CAnm2DXml
*
************************************************************/
CAnm2DXml::CAnm2DXml(bool bSaveImage)
	: CAnm2DBase()
{
	m_pProgress = NULL ;
	m_bSaveImage = bSaveImage ;
}

// 作成中のデータをアニメデータに変換
bool CAnm2DXml::makeFromEditData( CEditData &rEditData )
{
	QDomDocument doc(kAnmXML_ID_Anm2D) ;
	QDomElement root = doc.createElement(kAnmXML_ID_Root) ;

	setProgMaximum(m_pProgress, rEditData) ;

	if ( !makeHeader(root, doc, rEditData) ) {
		return false ;
	}
	if ( !makeObject(root, doc, rEditData) ) {
		return false ;
	}
	if ( !makeImage(root, doc, rEditData) ) {
		return false ;
	}

	doc.appendChild(root) ;

	m_Data = doc ;
	return true ;
}

// アニメファイルから作成中データへ変換
bool CAnm2DXml::makeFromFile(QDomDocument &xml, CEditData &rEditData)
{
	m_Data = xml ;
	if ( m_Data.doctype().name() != kAnmXML_ID_Anm2D ) {
		m_nError = kErrorNo_InvalidID ;
		return false ;
	}

	QDomElement root = m_Data.documentElement() ;
	if ( root.isNull() ) {
		m_nError = kErrorNo_InvalidNode ;
		return false ;
	}
	if ( root.nodeName() != kAnmXML_ID_Root ) {
		m_nError = kErrorNo_InvalidID ;
		return false ;
	}

	QDomNamedNodeMap nodeMap = root.attributes() ;
	if ( nodeMap.isEmpty() ) {
		m_nError = kErrorNo_InvalidNode ;
		return false ;
	}
	if ( nodeMap.namedItem(kAnmXML_Attr_Version).isNull() ) {
		m_nError = kErrorNo_InvalidVersion ;
		return false ;
	}
	int version = nodeMap.namedItem(kAnmXML_Attr_Version).toAttr().value().toInt() ;
	if ( version == 0x00001000 ) {
		if ( nodeMap.namedItem(kAnmXML_Attr_ObjNum).isNull() ) {
			m_nError = kErrorNo_InvalidObjNum ;
			return false ;
		}
		if ( nodeMap.namedItem(kAnmXML_Attr_ImageNum).isNull() ) {
			m_nError = kErrorNo_InvalidImageNum ;
			return false ;
		}
		m_ObjNum = nodeMap.namedItem(kAnmXML_Attr_ObjNum).toAttr().value().toInt() ;
		m_ImageNum = nodeMap.namedItem(kAnmXML_Attr_ImageNum).toAttr().value().toInt() ;

//		qDebug("objNum:%d imageNum:%d", m_ObjNum, m_ImageNum) ;

		QDomNode n = root.firstChild() ;
		if ( !addElement_00001000(n, rEditData) ) {
			return false ;
		}
	}
	else if ( version >= 0x01000000 ) {
		if ( nodeMap.namedItem(kAnmXML_Attr_ObjNum).isNull() ) {
			m_nError = kErrorNo_InvalidObjNum ;
			return false ;
		}
		if ( nodeMap.namedItem(kAnmXML_Attr_ImageNum).isNull() ) {
			m_nError = kErrorNo_InvalidImageNum ;
			return false ;
		}
		m_ObjNum = nodeMap.namedItem(kAnmXML_Attr_ObjNum).toAttr().value().toInt() ;
		m_ImageNum = nodeMap.namedItem(kAnmXML_Attr_ImageNum).toAttr().value().toInt() ;

//		qDebug("objNum:%d imageNum:%d", m_ObjNum, m_ImageNum) ;

		QDomNode n = root.firstChild() ;
		if ( !addElement_01000000(n, rEditData) ) {
			return false ;
		}
	}
	else {
		return false ;
	}

	return true ;
}

// ヘッダエレメント作成
bool CAnm2DXml::makeHeader(QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
Q_UNUSED(doc) ;

	CObjectModel *pModel = rEditData.getObjectModel() ;
	ObjectItem *pRoot = pModel->getItemFromIndex(QModelIndex()) ;

	element.setAttribute(kAnmXML_Attr_Version, kAnmXML_Version);
	element.setAttribute(kAnmXML_Attr_ObjNum, pRoot->childCount());
	element.setAttribute(kAnmXML_Attr_ImageNum, rEditData.getImageDataListSize());

	return true ;
}

// オブジェクトエレメント作成
bool CAnm2DXml::makeObject(QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;
	ObjectItem *pRoot = pModel->getItemFromIndex(QModelIndex()) ;
	for ( int i = 0 ; i < pRoot->childCount() ; i ++ ) {
		ObjectItem *obj = pRoot->child(i) ;

		QDomElement elmObj = doc.createElement(kAnmXML_ID_Object) ;
		elmObj.setAttribute(kAnmXML_Attr_Name, QString(obj->getName().toUtf8()));
		elmObj.setAttribute(kAnmXML_Attr_No, i);
		elmObj.setAttribute(kAnmXML_Attr_LayerNum, obj->childCount()) ;
		elmObj.setAttribute(kAnmXML_Attr_LoopNum, obj->getLoop());	// ループ回数(after ver 0.1.0)
		elmObj.setAttribute(kAnmXML_Attr_FpsNum, obj->getFps()) ;	// FPS(after ver 1.0.1)
		element.appendChild(elmObj) ;

		for ( int j = 0 ; j < obj->childCount() ; j ++ ) {
			if ( !makeLayer(obj->child(j), elmObj, doc, rEditData) ) {
				return false ;
			}
		}
	}
	return true ;
}

bool CAnm2DXml::makeLayer(ObjectItem *root, QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
	QDomElement elmLayer = doc.createElement(kAnmXML_ID_Layer) ;
	elmLayer.setAttribute(kAnmXML_Attr_Name, QString(root->getName().toUtf8()));
	elmLayer.setAttribute(kAnmXML_Attr_FrameNum, root->getFrameData().size()) ;
	elmLayer.setAttribute(kAnmXML_Attr_ChildNum, root->childCount()) ;
	elmLayer.setAttribute(kAnmXML_Attr_State, root->data(Qt::CheckStateRole).toInt()) ;	// レイヤ状態(after ver 1.0.3)
	element.appendChild(elmLayer) ;

	const QList<FrameData> &datas = root->getFrameData() ;
	for ( int i = 0 ; i < datas.size() ; i ++ ) {
		const FrameData &data = datas.at(i) ;

		QDomElement elmFrameData = doc.createElement(kAnmXML_ID_FrameData) ;
		QDomElement elmTmp ;
		QDomText text ;

		elmTmp = doc.createElement("frame") ;
		text = doc.createTextNode(QString("%1").arg(data.frame)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("pos") ;
		text = doc.createTextNode(QString("%1 %2 %3").arg(data.pos_x).arg(data.pos_y).arg(data.pos_z)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("rot") ;
		text = doc.createTextNode(QString("%1 %2 %3").arg(data.rot_x).arg(data.rot_y).arg(data.rot_z)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("center") ;
		text = doc.createTextNode(QString("%1 %2").arg(data.center_x).arg(data.center_y)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("UV") ;
		text = doc.createTextNode(QString("%1 %2 %3 %4").arg(data.left).arg(data.top).arg(data.right).arg(data.bottom)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("ImageNo") ;
		text = doc.createTextNode(QString("%1").arg(data.nImage)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("scale") ;
		text = doc.createTextNode(QString("%1 %2").arg(data.fScaleX).arg(data.fScaleY)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		elmTmp = doc.createElement("UVAnime") ;
		text = doc.createTextNode(QString("%1").arg(data.bUVAnime)) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		// 頂点色(after ver 0.1.0)
		elmTmp = doc.createElement("Color") ;
		text = doc.createTextNode(QString("%1 %2 %3 %4").arg(data.rgba[0]).arg(data.rgba[1]).arg(data.rgba[2]).arg(data.rgba[3])) ;
		elmTmp.appendChild(text) ;
		elmFrameData.appendChild(elmTmp) ;

		// パス(after ver 1.0.2)
		for ( int j = 0 ; j < 2 ; j ++ ) {
			if ( data.path[j].bValid ) {
				elmTmp = doc.createElement(QString("Path%1").arg(j)) ;
				text = doc.createTextNode(QString("%1 %2").arg(data.path[j].v.x()).arg(data.path[j].v.y())) ;
				elmTmp.appendChild(text) ;
				elmFrameData.appendChild(elmTmp) ;
			}
		}

		elmLayer.appendChild(elmFrameData) ;

		if ( m_pProgress ) {
			m_pProgress->setValue(m_pProgress->value()+1);

			if ( m_pProgress->wasCanceled() ) {
				m_nError = kErrorNo_Cancel ;
				return false ;
			}
		}
	}

	for ( int i = 0 ; i < root->childCount() ; i ++ ) {
		makeLayer(root->child(i), elmLayer, doc, rEditData) ;
	}
	return true ;
}

// イメージエレメント作成
bool CAnm2DXml::makeImage( QDomElement &element, QDomDocument &doc, CEditData &rEditData )
{
	for ( int i = 0 ; i < rEditData.getImageDataListSize() ; i ++ ) {
#if 1
		CEditData::ImageData *p = rEditData.getImageData(i) ;
		if ( !p ) { continue ; }
		QString imgFilePath = p->fileName ;
		QImage image = p->Image ;
#else
		QString imgFilePath = rEditData.getImageFileName(i) ;
		QImage image = rEditData.getImage(i) ;
#endif
		QDomElement elmImage = doc.createElement(kAnmXML_ID_Image) ;
		elmImage.setAttribute(kAnmXML_Attr_No, p->nNo);

		QDomElement elmTmp ;
		QDomText text ;

		elmTmp = doc.createElement("FilePath") ;
		QString relPath = getRelativePath(m_filePath, imgFilePath) ;
		text = doc.createTextNode(relPath.toUtf8()) ;
		elmTmp.appendChild(text) ;
		elmImage.appendChild(elmTmp) ;

		int w, h ;
		if ( m_bSaveImage ) {
			w = image.width() ;
			h = image.height() ;
		}
		else {
#if 1
			w = p->origImageW ;
			h = p->origImageH ;
#else
			rEditData.getOriginalImageSize(i, w, h) ;
#endif
		}

		elmTmp = doc.createElement("Size") ;
		text = doc.createTextNode(QString("%1 %2").arg(w).arg(h)) ;
		elmTmp.appendChild(text) ;
		elmImage.appendChild(elmTmp) ;

		if ( m_bSaveImage ) {
			elmTmp = doc.createElement("Data") ;
			QString str ;
			str.reserve(image.height()*image.width()*9+image.height()*15);
			for ( int y = 0 ; y < image.height() ; y ++ ) {
				for ( int x = 0 ; x < image.width() ; x ++ ) {
					if ( !(x%16) ) {
						str.append(QString("\n            ")) ;
					}
					str.append(QString::number(image.pixel(x, y), 16)) ;
					str.append(" ") ;

					if ( m_pProgress ) {
						m_pProgress->setValue(m_pProgress->value()+1);
						if ( m_pProgress->wasCanceled() ) {
							m_nError = kErrorNo_Cancel ;
							return false ;
						}
					}
				}
			}
			str.append(QString("\n        ")) ;
			text = doc.createTextNode(str) ;
			elmTmp.appendChild(text) ;
			elmImage.appendChild(elmTmp) ;
		}

		element.appendChild(elmImage) ;
	}

	return true ;
}

// プログレスバーの最大値セット
void CAnm2DXml::setProgMaximum( QProgressDialog *pProg, CEditData &rEditData )
{
	if ( !pProg ) { return ; }
#if 0
	TODO
	int max = 0 ;
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).layerGroupList ;
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			max += layerGroupList.at(j).second.size() ;
		}
	}

	if ( m_bSaveImage ) {
		for ( int i = 0 ; i < rEditData.getImageDataListSize() ; i ++ ) {
#if 1
			CEditData::ImageData *p = rEditData.getImageData(i) ;
			if ( !p ) { continue ; }
			max += p->Image.height() * p->Image.width() ;
#else
			max += rEditData.getImage(i).height() * rEditData.getImage(i).width() ;
#endif
		}
	}
	qDebug() << "max:" << max ;
	pProg->setMaximum(max);
#endif
}

// エレメント追加
bool CAnm2DXml::addElement_00001000( QDomNode &node, CEditData &rEditData )
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	QList<CEditData::ImageData> ImageData ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_Object ) {	// オブジェクト
			QString name ;
			int layerNum = 0 ;
			int no = 0 ;
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_Name).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LayerNum).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_No).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LoopNum).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt() ;
			no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;
			int loopNum = nodeMap.namedItem(kAnmXML_Attr_LoopNum).toAttr().value().toInt() ;

			QModelIndex index = pModel->addItem(name, QModelIndex()) ;
			ObjectItem *pObj = pModel->getItemFromIndex(index) ;
			pObj->setLoop(loopNum) ;
			pObj->setFps(60);

			QDomNode child = node.firstChild() ;
			if ( !addLayer_00001000(child, pObj, layerNum, rEditData) ) {
				return false ;
			}
		}
		else if ( node.nodeName() == kAnmXML_ID_Image ) {	// イメージ
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_No).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			int no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;

			CEditData::ImageData data ;
			QDomNode child = node.firstChild() ;
			if ( !addImage(child, data) ) {
				return false ;
			}
			data.lastModified = QDateTime::currentDateTimeUtc() ;
			data.nTexObj = 0 ;
			data.nNo = no ;
			ImageData.insert(no, data);
		}
		node = node.nextSibling() ;
	}

	if ( pModel->getItemFromIndex(QModelIndex())->childCount() != m_ObjNum
	  || ImageData.size() != m_ImageNum ) {
		m_nError = kErrorNo_InvalidObjNum ;
		return false ;
	}

	rEditData.setImageData(ImageData);
	return true ;
}

// レイヤデータを追加
bool CAnm2DXml::addLayer_00001000( QDomNode &node, ObjectItem *pRoot, int maxLayerNum, CEditData &rEditData )
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_Layer ) {
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_Name).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_FrameNum).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_No).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			QString name ;
			int frameDataNum = 0 ;
			int no = 0 ;

			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			frameDataNum = nodeMap.namedItem(kAnmXML_Attr_FrameNum).toAttr().value().toInt() ;
			no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;

			QModelIndex index = pModel->addItem(name, pRoot->getIndex()) ;
			ObjectItem *pItem = pModel->getItemFromIndex(index) ;
			pItem->setData(ObjectItem::kState_Disp, Qt::CheckStateRole) ;

			QDomNode child = node.firstChild() ;
			if ( !addFrameData_00001000(child, pItem, frameDataNum) ) {
				return false ;
			}
		}
		node = node.nextSibling() ;
	}

	if ( pRoot->childCount() != maxLayerNum ) {
		m_nError = kErrorNo_InvalidLayerNum ;
		return false ;
	}
	return true ;
}

// フレームデータを追加
bool CAnm2DXml::addFrameData_00001000( QDomNode &node, ObjectItem *pItem, int maxFrameDataNum )
{
	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_FrameData ) {
			FrameData data ;

			QDomNode dataNode = node.firstChild() ;
			while ( !dataNode.isNull() ) {
				if ( dataNode.nodeName() == "frame" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QString frame = dataNode.firstChild().toText().nodeValue() ;
					if ( frame.isEmpty() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.frame = frame.toInt() ;
				}
				else if ( dataNode.nodeName() == "pos" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QStringList pos = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( pos.size() != 3 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.pos_x = pos[0].toFloat() ;
					data.pos_y = pos[1].toFloat() ;
					data.pos_z = pos[2].toFloat() ;
				}
				else if ( dataNode.nodeName() == "rot" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QStringList rot = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( rot.size() != 3 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.rot_x = rot[0].toFloat() ;
					data.rot_y = rot[1].toFloat() ;
					data.rot_z = rot[2].toFloat() ;
				}
				else if ( dataNode.nodeName() == "center" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QStringList center = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( center.size() != 2 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.center_x = center[0].toFloat() ;
					data.center_y = center[1].toFloat() ;
				}
				else if ( dataNode.nodeName() == "UV" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QStringList uv = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( uv.size() != 4 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.left	= uv[0].toFloat() ;
					data.top	= uv[1].toFloat() ;
					data.right	= uv[2].toFloat() ;
					data.bottom	= uv[3].toFloat() ;
				}
				else if ( dataNode.nodeName() == "ImageNo" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QString image = dataNode.firstChild().toText().nodeValue() ;
					if ( image.isEmpty() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.nImage = image.toInt() ;
				}
				else if ( dataNode.nodeName() == "scale" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QStringList scale = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( scale.size() != 2 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.fScaleX = scale[0].toFloat() ;
					data.fScaleY = scale[1].toFloat() ;
				}
				else if ( dataNode.nodeName() == "UVAnime" ) {
					if ( !dataNode.hasChildNodes() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					QString anime = dataNode.firstChild().toText().nodeValue() ;
					if ( anime.isEmpty() ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.bUVAnime = anime.toInt() ? true : false ;
				}
				else if ( dataNode.nodeName() == "Color" ) {	// 頂点色(after ver 0.1.0)
					QStringList color = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( color.size() != 4 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.rgba[0] = color[0].toInt() ;
					data.rgba[1] = color[1].toInt() ;
					data.rgba[2] = color[2].toInt() ;
					data.rgba[3] = color[3].toInt() ;
				}
				else if ( dataNode.nodeName() == "Path0"
						|| dataNode.nodeName() == "Path1" ) {	// パス(after ver 1.0.2)
					int index = (dataNode.nodeName() == "Path0") ? 0 : 1 ;
					QStringList vec = dataNode.firstChild().toText().nodeValue().split(" ") ;
					if ( vec.size() != 2 ) {
						m_nError = kErrorNo_InvalidNode ;
						return false ;
					}
					data.path[index].bValid = true ;
					data.path[index].v.setX(vec[0].toFloat()) ;
					data.path[index].v.setY(vec[1].toFloat()) ;
				}

				dataNode = dataNode.nextSibling() ;
			}

			pItem->addFrameData(data) ;
		}
		node = node.nextSibling() ;
	}

	if ( pItem->getFrameData().size() != maxFrameDataNum ) {
		qDebug("%d %d", pItem->getFrameData().size(), maxFrameDataNum) ;
		m_nError = kErrorNo_InvalidFrameDataNum ;
		return false ;
	}
	return true ;
}

bool CAnm2DXml::addElement_01000000(QDomNode &node, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	QList<CEditData::ImageData> ImageData ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_Object ) {	// オブジェクト
			QString name ;
			int layerNum = 0 ;
			int no = 0 ;
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_Name).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LayerNum).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_No).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LoopNum).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt() ;
			no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;
			int loopNum = nodeMap.namedItem(kAnmXML_Attr_LoopNum).toAttr().value().toInt() ;
			int fps = 60 ;
			if ( !nodeMap.namedItem(kAnmXML_Attr_FpsNum).isNull() ) {
				fps = nodeMap.namedItem(kAnmXML_Attr_FpsNum).toAttr().value().toInt() ;
			}

			QModelIndex index = pModel->addItem(name, QModelIndex()) ;
			ObjectItem *pObj = pModel->getItemFromIndex(index) ;
			pObj->setLoop(loopNum) ;
			pObj->setFps(fps) ;

			QDomNode child = node.firstChild() ;
			if ( !addLayer_01000000(child, pObj, layerNum, rEditData) ) {
				return false ;
			}
		}
		else if ( node.nodeName() == kAnmXML_ID_Image ) {	// イメージ
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_No).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			int no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;

			CEditData::ImageData data ;
			QDomNode child = node.firstChild() ;
			if ( !addImage(child, data) ) {
				return false ;
			}
			data.lastModified = QDateTime::currentDateTimeUtc() ;
			data.nTexObj = 0 ;
			data.nNo = no ;
			ImageData.insert(no, data);
		}
		node = node.nextSibling() ;
	}

	if ( pModel->getItemFromIndex(QModelIndex())->childCount() != m_ObjNum
	  || ImageData.size() != m_ImageNum ) {
		m_nError = kErrorNo_InvalidObjNum ;
		return false ;
	}

	rEditData.setImageData(ImageData);
	return true ;
}

bool CAnm2DXml::addLayer_01000000( QDomNode &node, ObjectItem *pRoot, int maxLayerNum, CEditData &rEditData )
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_Layer ) {
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_Name).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_FrameNum).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_ChildNum).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			QString name ;
			int frameDataNum = 0 ;
			int childNum = 0 ;
			int state = ObjectItem::kState_Disp ;

			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			frameDataNum = nodeMap.namedItem(kAnmXML_Attr_FrameNum).toAttr().value().toInt() ;
			childNum = nodeMap.namedItem(kAnmXML_Attr_ChildNum).toAttr().value().toInt() ;
			if ( !nodeMap.namedItem(kAnmXML_Attr_State).isNull() ) {
				state = nodeMap.namedItem(kAnmXML_Attr_State).toAttr().value().toInt() ;
			}

			QModelIndex index = pModel->addItem(name, pRoot->getIndex()) ;
			ObjectItem *pItem = pModel->getItemFromIndex(index) ;
			pItem->setData(state, Qt::CheckStateRole) ;

			QDomNode child = node.firstChild() ;
			if ( !addFrameData_00001000(child, pItem, frameDataNum) ) {
				return false ;
			}
			QDomNode layers = node.firstChild() ;
			if ( !addLayer_01000000(layers, pItem, childNum, rEditData) ) {
				return false ;
			}
		}
		node = node.nextSibling() ;
	}

	if ( pRoot->childCount() != maxLayerNum ) {
		m_nError = kErrorNo_InvalidLayerNum ;
		return false ;
	}
	return true ;
}

bool CAnm2DXml::addFrameData_01000000( QDomNode &node, ObjectItem *pItem, int maxFrameDataNum )
{
	return false ;
}

// イメージ追加
bool CAnm2DXml::addImage( QDomNode &node, CEditData::ImageData &data )
{
	bool bFindData = false ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == "FilePath" ) {
			if ( !node.hasChildNodes() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			data.fileName = node.firstChild().toText().nodeValue() ;
		}
		else if ( node.nodeName() == "Size" ) {
			if ( !node.hasChildNodes() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			QStringList size = node.firstChild().toText().nodeValue().split(" ") ;
			if ( size.size() != 2 ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			data.Image = QImage(size[0].toInt(), size[1].toInt(), QImage::Format_ARGB32) ;
		}
		else if ( node.nodeName() == "Data" ) {
			bFindData = true ;
			if ( !node.hasChildNodes() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			QStringList image = node.firstChild().toText().nodeValue().split(QRegExp("( |\n|\r)"), QString::SkipEmptyParts) ;
			qDebug("size:%d w:%d h:%d = %d", image.size(), data.Image.width(), data.Image.height(), data.Image.width()*data.Image.height()) ;
			if ( image.size() != data.Image.width()*data.Image.height() ) {
				for ( int i = 0 ; i < 32 ; i ++ ) {
					if ( i >= image.size() ) {
						break ;
					}
					qDebug() << image[i] ;
				}
				m_nError = kErrorNo_InvalidImageData ;
				return false ;
			}
			int cnt = 0 ;
			for ( int y = 0 ; y < data.Image.height() ; y ++ ) {
				for ( int x = 0 ; x < data.Image.width() ; x ++ ) {
					data.Image.setPixel(x, y, image[cnt].toUInt(NULL, 16));
					cnt ++ ;
				}
			}
		}
		node = node.nextSibling() ;
	}

	if ( !bFindData ) {	// Data要素が無い場合はファイルパスから読み込む
		if ( data.fileName.isEmpty() ) {	// ファイルパスもない
			m_nError = kErrorNo_InvalidFilePath ;
			return false ;
		}
		QString path = getAbsolutePath(m_filePath, data.fileName) ;
		data.Image = QImage(path) ;
		data.fileName = path ;

		if ( data.Image.isNull() ) {
			m_nError = kErrorNo_InvalidFilePath ;
			return false ;
		}
	}
	return true ;
}


/************************************************************
*
* CAnm2DJson
*
************************************************************/
CAnm2DJson::CAnm2DJson(bool bFlat)
	: CAnm2DBase()
{
	m_bFlat		= bFlat ;
	m_pModel	= NULL ;
}

CAnm2DJson::~CAnm2DJson()
{
	if ( m_bFlat && m_pModel ) {
		delete m_pModel ;
		m_pModel = NULL ;
	}
}

bool CAnm2DJson::makeFromEditData( CEditData &rEditData )
{
	m_pModel = rEditData.getObjectModel() ;
	if ( m_bFlat ) {
		CObjectModel *p = new CObjectModel() ;
		p->copy(m_pModel) ;
		p->flat();
		m_pModel = p ;
	}

	addString("{\n") ;
	if ( !makeObject(rEditData) ) { return false ; }
	addString("}") ;

	m_Data.replace(QRegExp("( |\n)"), "") ;
	return true ;
}

void CAnm2DJson::addString(QString str, int tab)
{
	QString t ;
	for ( int i = 0 ; i < tab ; i ++ ) {
		t += "  " ;
	}
	m_Data += t + str ;
}

bool CAnm2DJson::makeObject( CEditData &rEditData )
{
	ObjectItem *pRoot = m_pModel->getItemFromIndex(QModelIndex()) ;
	int tab = 1 ;
	for ( int i = 0 ; i < pRoot->childCount() ; i ++ ) {
		ObjectItem *pObj = pRoot->child(i) ;

		addString("\"" + QString(pObj->getName().toUtf8()) + "\": {\n", tab) ;
		tab ++ ;
		addString("\"animeTime\": " + QVariant((int)(pObj->getMaxFrameNum()*(100.0/6.0))).toString() + ",\n", tab) ;
		addString("\"loopNum\": " + QVariant(pObj->getLoop()).toString() + ",\n", tab) ;
		addString("\"speed\": " + QVariant((double)pObj->getFps() / 60.0).toString() + ",\n", tab) ;
		addString("\"layer\": [\n", tab) ;
		for ( int j = 0 ; j < pObj->childCount() ; j ++ ) {
			makeLayer(pObj->child(j), rEditData, tab + 1) ;
			if ( j < pObj->childCount()-1 ) {
				addString(",") ;
			}
			addString("\n") ;
		}
		addString("]\n", tab) ;
		tab -- ;
		addString("}", tab) ;
		if ( i < pRoot->childCount()-1 ) { addString(",") ; }
		addString("\n") ;
	}

	return true ;
}

bool CAnm2DJson::makeLayer(ObjectItem *pItem, CEditData &rEditData, int tab)
{
	const QList<FrameData> &datas = pItem->getFrameData() ;

	addString("{\n", tab) ;
	tab ++ ;
	addString(QString("\"name\": \"%1\",\n").arg(pItem->getName()), tab) ;
	addString("\"frame\": [\n", tab) ;
	tab ++ ;
	for ( int i = 0 ; i < datas.size() ; i ++ ) {
		const FrameData &data = datas.at(i) ;

		CEditData::ImageData *pImageData = rEditData.getImageDataFromNo(data.nImage) ;
		if ( !pImageData ) { continue ; }

		int extPos = pImageData->fileName.lastIndexOf("/") ;
		QString path ;
		path = pImageData->fileName.right(pImageData->fileName.size() - extPos - 1) ;
		double anchor[2], uv[4] ;
		int w, h ;
		w = data.right - data.left ;
		h = data.bottom - data.top ;
		anchor[0] = w==0 ? 0 : (double)data.center_x / (double)w ;
		anchor[1] = h==0 ? 0 : (double)data.center_y / (double)h ;
		uv[0] = (double)data.left / (double)pImageData->origImageW ;
		uv[1] = (double)data.top / (double)pImageData->origImageH ;
		uv[2] = (double)w / (double)pImageData->origImageW ;
		uv[3] = (double)h / (double)pImageData->origImageH ;

		addString("{\n", tab) ;
		tab ++ ;
		addString("\"frame\": " + QVariant((int)(data.frame*(100.0/6.0))).toString() + ",\n", tab) ;
		addString("\"pos\": [" + QVariant(data.pos_x).toString() + ", " + QVariant(data.pos_y).toString() + ", " + QVariant(data.pos_z).toString() + "],\n", tab) ;
		addString("\"rot\": [" + QVariant(data.rot_x).toString() + ", " + QVariant(data.rot_y).toString() + ", " + QVariant(data.rot_z).toString() + "],\n", tab) ;
		addString("\"sca\": [" + QVariant(data.fScaleX).toString() + ", " + QVariant(data.fScaleY).toString() + "],\n", tab) ;
		addString("\"color\": [" + QVariant((double)data.rgba[0]/255.0).toString() + ", " + QVariant((double)data.rgba[1]/255.0).toString() + ", " + QVariant((double)data.rgba[2]/255.0).toString() + ", " + QVariant((double)data.rgba[3]/255.0).toString() + "],\n", tab) ;
		addString("\"uvAnime\": " + QVariant((int)data.bUVAnime).toString() + ",\n", tab) ;
		addString("\"image\": {\n", tab) ;
		tab ++ ;
		addString("\"path\": \"" + path + "\",\n", tab) ;
		addString("\"size\": [" + QVariant(w).toString() + ", " + QVariant(h).toString() + "],\n", tab) ;
		addString("\"center\": [" + QVariant(anchor[0]).toString() + ", " + QVariant(anchor[1]).toString() + "],\n", tab) ;
		addString("\"uvrect\": [" + QVariant(uv[0]).toString() + ", " + QVariant(uv[1]).toString() + ", " + QVariant(uv[2]).toString() + ", " + QVariant(uv[3]).toString() + "]\n", tab) ;
		tab -- ;
		addString("}\n", tab) ;
		tab -- ;
		addString("}", tab) ;
		if ( i < datas.size()-1 ) { addString(",") ; }
		addString("\n") ;
	}
	tab -- ;
	addString("]", tab) ;
	if ( pItem->childCount() ) {
		addString(",\n") ;
		addString("\"layer\": [\n", tab) ;
		for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
			makeLayer(pItem->child(i), rEditData, tab + 1) ;
			if ( i < pItem->childCount()-1 ) {
				addString(",") ;
			}
			addString("\n") ;
		}
		addString("]", tab) ;
	}
	addString("\n") ;
	tab -- ;
	addString("}", tab) ;
	return true ;
}


/*-----------------------------------------------------------*//**
  @brief	.asm インターフェース
  @author	Kenji Nishida
  @date		2011/11/24
*//*------------------------------------------------------------*/
CAnm2DAsm::CAnm2DAsm(bool bFlat)
	: CAnm2DBase()
{
	m_bFlat		= bFlat;
	m_pModel	= NULL;
}

CAnm2DAsm::~CAnm2DAsm()
{
	if(m_bFlat && m_pModel){
		delete m_pModel;
		m_pModel = NULL;
	}
}

QVector4D makeFromEditDataArea(ObjectItem *pObj, QVector4D qv4Area)
{
	QMatrix4x4	mat = pObj->getDisplayMatrix(0);
	FrameData	frameData = pObj->getDisplayFrameData(0);
	QVector3D	v[4];
	frameData.getVertexApplyMatrix(v, mat);
	for(int i=0; i<4; i++){
		if(qv4Area.x() == 0.0f && qv4Area.y() == 0.0f && qv4Area.z() == 0.0f && qv4Area.w() == 0.0f){
			qv4Area.setX(v[i].x());
			qv4Area.setY(v[i].y());
			qv4Area.setZ(v[i].x());
			qv4Area.setW(v[i].y());
		} else {
			if(qv4Area.x() > v[i].x()) qv4Area.setX(v[i].x());
			if(qv4Area.y() > v[i].y()) qv4Area.setY(v[i].y());
			if(qv4Area.z() < v[i].x()) qv4Area.setZ(v[i].x());
			if(qv4Area.w() < v[i].y()) qv4Area.setW(v[i].y());
		}
	}
	
	if(pObj->childCount()){
		for(int i=0; i<pObj->childCount(); i++){
			ObjectItem	*pChild = pObj->child(i);
			qv4Area = makeFromEditDataArea(pChild, qv4Area);
		}
	}
	
	return qv4Area;
}

void CAnm2DAsm::makeFromEditDataTip(QString qsLabel, ObjectItem *pObj)
{
	addString(";---------------------------------------------------------------- ANM_TIP\n");
	addString(qsLabel + ":\n");
	addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->getMaxFrameNum(false)) + "\t\t; nKeyFrame\n"); 
	addString("\t\t\tdd\t\t.key\t\t; pKey\n");
	addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
	if(pObj->childCount()){
		addString("\t\t\tdd\t\t.tips\t\t; papTip\n");
	} else {
		addString("\t\t\tdd\t\tNO_READ\t\t; papTip\n");
	}
	addString("\t\n");
	
	// キーフレーム
	addString("\t.key:\n");
	for(int i=0; i<=pObj->getMaxFrameNum(false); i++){
		bool		valid;
		FrameData	frameData = pObj->getDisplayFrameData(i, &valid);
		if(valid == false) continue;
		addString("\t\t\t; frame " + QString("%1").arg(i) + " --------------------------------\n");
		addString("\t\t\tdd\t\t1\t\t; [NORMAL]\n");
		addString("\t\t\tdw\t\t" + QString("%1").arg(frameData.frame) + "\t\t; uTime\n");
		addString("\t\t\tdw\t\t" + m_aqsVramID[frameData.nImage] + "\t\t; uVramID\n");
		addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2), F32(%3)").arg(frameData.pos_x, 0, 'f').arg(frameData.pos_y, 0, 'f').arg(-frameData.pos_z, 0, 'f') + "\t\t; fvPos\n");
		addString("\t\t\tdd\t\t" + QString("F32(%1)").arg(frameData.rot_z * M_PI / 180.0f, 0, 'f') + "\t\t; fRot\n");
		addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2)").arg(frameData.fScaleX, 0, 'f').arg(frameData.fScaleY, 0, 'f') + "\t\t; fvSca\n");
		addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2)").arg(frameData.center_x, 0, 'f').arg(frameData.center_y, 0, 'f') + "\t\t; fvCenter\n");
		addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2), F32(%3), F32(%4)").arg(frameData.left, 0, 'f').arg(frameData.top, 0, 'f').arg(frameData.right, 0, 'f').arg(frameData.bottom, 0, 'f') + "\t\t; fvUV\n");
		addString("\t\t\tdb\t\t" + QString("%1, %2, %3, %4").arg(frameData.rgba[0]).arg(frameData.rgba[1]).arg(frameData.rgba[2]).arg(frameData.rgba[3]) + "\t\t; bvRGBA\n");
		addString("\t\t\t\n");
	}
	addString("\t\t\tdd\t\t0\t\t; [TERM]\n");
	addString("\n");
	
	// 子供の処理
	if(pObj->childCount()){
		addString("\t.tips:\n");
		for(int i=0; i<pObj->childCount(); i++){
			addString("\t\t\tdd\t\t" + qsLabel + "_" + QString("%1").arg(i) + "\n");
		}
		addString("\n");
		for(int i=0; i<pObj->childCount(); i++){
			QString		qsLabelChild = qsLabel + QString("_%1").arg(i);
			ObjectItem	*pChild = pObj->child(i);
			makeFromEditDataTip(qsLabelChild, pChild);
		}
	}
}

bool CAnm2DAsm::makeFromEditData(CEditData &rEditData)
{
	m_pModel = rEditData.getObjectModel();
	if(m_bFlat){
		CObjectModel	*p = new CObjectModel();
		p->copy(m_pModel);
		p->flat();
		m_pModel = p;
	}
	
	ObjectItem	*pRoot = m_pModel->getItemFromIndex(QModelIndex()) ;
	ObjectItem	*pObj = pRoot->child(0);
	
	addString(";----------------------------------------------------------------\n");
	addString("; @kamefile\t" + pRoot->getName().toUtf8() + "\n");
	addString(";---------------------------------------------------------------- HEADER\n");
	addString("\t\t\%include\t\"../imageid.inc\"\n");
	addString("\n");
	addString(";---------------------------------------------------------------- DATA\n");
	addString("%define\t\tNO_READ\t\t0\n");
	addString("%define\t\tF32(f32)\t\t__float32__(f32)\n");
	addString("data:\n");
	addString(";---------------------------------------------------------------- ANM_HEAD\n");
	addString("\t\t\tdb\t\t'ANM0'\t\t; ANM0\n");
	addString("\t\t\tdd\t\t00000003h\t\t; uVersion\n");
	addString("\t\t\tdd\t\t" + QString("%1").arg(rEditData.getImageDataListSize()) + "\t\t; nVram\n");
	addString("\t\t\tdd\t\t.vram\t\t; pauVram\n");
	addString("\t\t\tdd\t\t" + QString("%1").arg(pRoot->childCount()) + "\t\t; nObject\n");
	addString("\t\t\tdd\t\t.object\t\t; paObj\n");
	addString("\t\n");
	addString("\t.vram:\n");
	for(int i=0; i<rEditData.getImageDataListSize(); i++){
		if(i >= KM_VRAM_MAX){
			return false;
		}
		CEditData::ImageData	*p = rEditData.getImageData(i);
		if(!p) continue;
		QFileInfo	fi(p->fileName);
		QString		sImageLabel = fi.fileName();
		sImageLabel = QString("ID_") + sImageLabel.replace(".", "_").toUpper();
		sImageLabel = sImageLabel.toUtf8();
		m_aqsVramID[i] = sImageLabel;
		addString("\t\t\tdd\t\t" + m_aqsVramID[i] + "\n");
	}
	addString("\t\n");
	addString("\t.object:\n");
	for(int i=0; i<pRoot->childCount(); i++){
		addString("\t\t\tdd\t\tanmobj" + QString("%1").arg(i) + "\n");
	}
	addString("\n");
	for(int i=0; i<pRoot->childCount(); i++){
		ObjectItem	*pObj = pRoot->child(i);
		QVector4D	qv4Area = QVector4D(0, 0, 0, 0);
		addString(";---------------------------------------------------------------- ANM_OBJ\n");
		addString("; " + QString(pObj->getName().toUtf8()) + "\n");
		addString("anmobj" + QString("%1").arg(i) + ":\n");
		if(pObj->childCount()){
			if(pObj->getLoop()){
				addString("\t\t\tdd\t\t00000001h\t\t; bFlag\n");
			} else {
				addString("\t\t\tdd\t\t00000000h\t\t; bFlag\n");
			}
			// 最小矩形算出
			for(int j=0; j<pObj->childCount(); j++){
				ObjectItem	*pChild = pObj->child(j);
				qv4Area = makeFromEditDataArea(pChild, qv4Area);
			}
			addString("\t\t\tdd\t\t" + QString("%1, %2, %3, %4").arg((int)qv4Area.x()).arg((int)qv4Area.y()).arg((int)qv4Area.z()).arg((int)qv4Area.w()) + "\t\t; ivArea\n");
			addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
			addString("\t\t\tdd\t\t.tips\t\t; papTip\n");
			addString("\t\n");
			addString("\t.tips:\n");
			for(int j=0; j<pObj->childCount(); j++){
				addString("\t\t\tdd\t\tobj" + QString("%1").arg(i) + "tip" + QString("%1").arg(j) + "\n");
			}
			addString("\n");
			for(int j=0; j<pObj->childCount(); j++){
				QString		qsLabel = "obj" + QString("%1").arg(i) + "tip" + QString("%1").arg(j);
				ObjectItem	*pChild = pObj->child(j);
				makeFromEditDataTip(qsLabel, pChild);
			}
		} else {
			addString("\t\t\tdd\t\t00000000h\t\t; bFlag\n");
			addString("\t\t\tdd\t\t0, 0, 0, 0\t\t; svArea\n");
			addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
			addString("\t\t\tdd\t\tNO_READ\t\t; papTip\n");
		}
	}
	addString("\n");

	return true;
}

void CAnm2DAsm::makeFromEditData2IncTip(QString qsLabel, ObjectItem *pObj)
{
	addString("%define\t\t" + qsLabel + pObj->getName().replace(" ", "_").toUpper().toUtf8() + QString("\t\t%1\n").arg(m_nCnt++));
	
	// 子供の処理
	if(pObj->childCount()){
		for(int i=0; i<pObj->childCount(); i++){
			ObjectItem	*pChild = pObj->child(i);
			makeFromEditData2IncTip(qsLabel, pChild);
		}
	}
}

bool CAnm2DAsm::makeFromEditData2Inc(CEditData &rEditData, QString qsFname)
{
	qsFname = QFileInfo(qsFname).baseName().toUpper();
	m_pModel = rEditData.getObjectModel();
	if(m_bFlat){
		CObjectModel	*p = new CObjectModel();
		p->copy(m_pModel);
		p->flat();
		m_pModel = p;
	}
	
	ObjectItem	*pRoot = m_pModel->getItemFromIndex(QModelIndex()) ;
	ObjectItem	*pObj = pRoot->child(0);
	
	addString("; このファイルはAnimationCreatorにより生成されました。\n");
	addString("\n");
	for(int i=0; i<pRoot->childCount(); i++){
		ObjectItem	*pObj = pRoot->child(i);
		addString("%define\t\tACO_" + qsFname + "__" + pObj->getName().replace(" ", "_").toUpper().toUtf8() + QString("\t\t%1").arg(i) + "\n");
	}
	addString("\n");
	for(int i=0; i<pRoot->childCount(); i++){
		ObjectItem	*pObj = pRoot->child(i);
		QString		qsLabel = "ACL_" + qsFname + "__" + pObj->getName().replace(" ", "_").toUpper().toUtf8() + "__";
		addString("%define\t\t" + qsLabel + "ROOT\t\t0\n");
		m_nCnt = 1;
		for(int j=0; j<pObj->childCount(); j++){
			ObjectItem	*pChild = pObj->child(j);
			makeFromEditData2IncTip(qsLabel, pChild);
		}
		addString("\n");
	}

	return true;
}

void CAnm2DAsm::addString(QString str, int tab)
{
	QString	t;
	for(int i=0; i<tab; i++){
		t += "  ";
	}
	m_Data += t + str;
}


