#include "canm2d.h"


/************************************************************
*
* CAnm2DBin
*
************************************************************/
CAnm2DBin::CAnm2DBin()
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

	if ( !makeHeader(header, rEditData) ) {
		return false ;
	}

	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::ObjectGroup &objGroup = objList.at(i) ;
		QStandardItem *pObjItem = (QStandardItem *)objGroup.first ;
		const CObjectModel::LayerGroupList &layerGroupList = objGroup.second ;

		QByteArray objArray ;
		objArray.resize(sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int)) ;
		Anm2DObject *pObjData = (Anm2DObject *)objArray.data() ;
		memset( pObjData, 0, sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int) ) ;
		pObjData->header.nID   = ANM2D_ID_OBJECT ;
		pObjData->header.nSize = sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int) ;
		strcpy( pObjData->objName.name, pObjItem->text().toUtf8().data() ) ;
		pObjData->nLayerNum = layerGroupList.size() ;

		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(j) ;
			QStandardItem *pLayerItem = (QStandardItem *)layerGroup.first ;
			const CObjectModel::FrameDataList &frameDataList = layerGroup.second ;

			QByteArray layerArray ;
			layerArray.resize(sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int));
			Anm2DLayer *pLayerData = (Anm2DLayer *)layerArray.data() ;
			memset(pLayerData, 0, sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int)) ;
			pLayerData->header.nID   = ANM2D_ID_LAYER ;
			pLayerData->header.nSize = sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int) ;
			strcpy( pLayerData->layerName.name, pLayerItem->text().toUtf8().data() ) ;
			pLayerData->nLayerNo = layerList.size() ;
			pLayerData->nFrameDataNum = frameDataList.size() ;

			pObjData->nLayerNo[j] = pLayerData->nLayerNo ;	// オブジェクトが参照するレイヤ番号セット

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const CObjectModel::FrameData &data = frameDataList.at(k) ;

				QByteArray frameDataArray ;
				frameDataArray.resize(sizeof(Anm2DFrameData)) ;
				Anm2DFrameData *pFrameData = (Anm2DFrameData *)frameDataArray.data() ;
				memset(pFrameData, 0, sizeof(Anm2DFrameData)) ;
				pFrameData->header.nID		= ANM2D_ID_FRAMEDATA ;
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

				pLayerData->nFrameDataNo[k] = pFrameData->nFrameDataNo ;	// レイヤが参照するフレームデータ番号セット

				frameList << frameDataArray ;
			}

			layerList << layerArray ;
		}

		objectList << objArray ;
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
	if ( pHeader->header.nID != ANM2D_ID_HEADER ) {
		m_nError = kErrorNo_InvalidID ;
		return false ;
	}
	if ( pHeader->nFileSize != (unsigned int)m_Data.size() ) {
		m_nError = kErrorNo_InvalidFileSize ;
		return false ;
	}
	if ( pHeader->nVersion != ANM2D_VERSION ) {
		m_nError = kErrorNo_InvalidVersion ;
		return false ;
	}
	if ( !addObject(pHeader, rEditData) ) {
		return false ;
	}
	if ( !addLayer(pHeader, rEditData) ) {
		return false ;
	}
	if ( !addFrameData(pHeader, rEditData) ) {
		return false ;
	}
	if ( !addImageData(pHeader, rEditData) ) {
		return false ;
	}

	return true ;
}


// ヘッダ作成
bool CAnm2DBin::makeHeader( QByteArray &rData, CEditData &rEditData )
{
	int blockNum = 0 ;
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	blockNum += objList.size() ;	// オブジェクト数
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).second ;
		blockNum += layerGroupList.size() ;	// レイヤ数
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::FrameDataList &frameDataList = layerGroupList.at(j).second ;
			blockNum += frameDataList.size() ;	// フレームデータ数
		}
	}
	blockNum += rEditData.getImageDataSize() ;	// イメージデータ数

	rData.resize(sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int));
	Anm2DHeader *pHeader = (Anm2DHeader *)rData.data() ;
	memset( pHeader, 0, sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int) ) ;
	pHeader->header.nID		= ANM2D_ID_HEADER ;
	pHeader->header.nSize	= sizeof( Anm2DHeader ) ;
	pHeader->nVersion		= ANM2D_VERSION ;
	pHeader->nFileSize		= 0 ;	// 後で入れる
	pHeader->nBlockNum		= blockNum ;

	return true ;
}

// イメージデータ作成
bool CAnm2DBin::makeImageList( QList<QByteArray> &rData, CEditData &rEditData )
{
	int imageNum = rEditData.getImageDataSize() ;

	for ( int i = 0 ; i < imageNum ; i ++ ) {
		QImage img = rEditData.getImage(i) ;
		unsigned int size = sizeof(Anm2DImage) + img.width() * img.height() * 4 * sizeof(unsigned char) - 1 ;
		size = (size+0x03) & ~0x03 ;

		QByteArray imgArray ;
		imgArray.resize(size);
		Anm2DImage *pImage = (Anm2DImage *)imgArray.data() ;
		memset( pImage, 0, size ) ;
		pImage->header.nID = ANM2D_ID_IMAGE ;
		pImage->header.nSize = size ;
		pImage->nWidth = img.width() ;
		pImage->nHeight = img.height() ;
		pImage->nImageNo = i ;
		strncpy(pImage->fileName, rEditData.getImageFileName(i).toStdString().c_str(), 255) ;
		memcpy(pImage->data, img.bits(), img.width()*img.height()*4) ;

		rData << imgArray ;
	}
	return true ;
}

// 編集データにオブジェクトを追加
bool CAnm2DBin::addObject(Anm2DHeader *pHeader, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;
	QStandardItemModel *pTreeModel = rEditData.getTreeModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_OBJECT:
			{
				Anm2DObject *pObj = (Anm2DObject *)p ;

				QStandardItem *pRoot = pTreeModel->invisibleRootItem() ;
				QStandardItem *newItem = new QStandardItem(QString(pObj->objName.name)) ;
				pRoot->appendRow(newItem);

				CObjectModel::ObjectGroup objGroup ;
				objGroup.first = newItem ;
				pModel->addObject(objGroup);
			}
			break ;
		case ANM2D_ID_LAYER:
		case ANM2D_ID_FRAMEDATA:
		case ANM2D_ID_IMAGE:
			continue ;

		default:
			m_nError = kErrorNo_InvalidID ;
			return false ;
		}
	}
	return true ;
}

// 編集データにレイヤを追加
bool CAnm2DBin::addLayer(Anm2DHeader *pHeader, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_LAYER:
			{
				Anm2DLayer *pLayer = (Anm2DLayer *)p ;

				CObjectModel::ObjectList &objList = *pModel->getObjectListPtr() ;
				for ( int j = 0 ; j < objList.size() ; j ++ ) {
					QStandardItem *pObjItem = (QStandardItem *)objList.at(j).first ;
					Anm2DObject *pObj = search2DObjectFromName(pHeader, pObjItem->text()) ;
					if ( !pObj ) {
						m_nError = 0xffffffff ;
						return false ;
					}
					for ( int k = 0 ; k < pObj->nLayerNum ; k ++ ) {
						if ( pObj->nLayerNo[k] != pLayer->nLayerNo ) { continue ; }

						QStandardItem *newItem = new QStandardItem(pLayer->layerName.name) ;
						newItem->setData(true, Qt::CheckStateRole);
						pObjItem->appendRow(newItem);

						CObjectModel::LayerGroup layerGroup = qMakePair(newItem, CObjectModel::FrameDataList()) ;
						objList[j].second.append(layerGroup);
					}
				}
			}
			break ;
		case ANM2D_ID_OBJECT:
		case ANM2D_ID_FRAMEDATA:
		case ANM2D_ID_IMAGE:
			continue ;

		default:
			m_nError = kErrorNo_InvalidID ;
			return false ;
		}
	}
	return true ;
}

// 編集データにフレームデータを追加
bool CAnm2DBin::addFrameData(Anm2DHeader *pHeader, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_FRAMEDATA:
			{
				Anm2DFrameData *pFrame = (Anm2DFrameData *)p ;

				CObjectModel::ObjectList &objList = *pModel->getObjectListPtr() ;
				for ( int objNum = 0 ; objNum < objList.size() ; objNum ++ ) {
					CObjectModel::LayerGroupList &layerGroupList = objList[objNum].second ;
					for ( int layerGroupNum = 0 ; layerGroupNum < layerGroupList.size() ; layerGroupNum ++ ) {
						QStandardItem *pLayerItem = (QStandardItem *)layerGroupList[layerGroupNum].first ;
						CObjectModel::FrameDataList &frameDataList = layerGroupList[layerGroupNum].second ;
						Anm2DLayer *pLayer = search2DLayerFromName(pHeader, pLayerItem->text()) ;
						if ( !pLayer ) {
							m_nError = 0xfffffffe ;
							return false ;
						}
						for ( int j = 0 ; j < pLayer->nFrameDataNum ; j ++ ) {
							if ( pLayer->nFrameDataNo[j] != pFrame->nFrameDataNo ) { continue ; }

							CObjectModel::FrameData data ;
							data.frame		= pFrame->nFrame ;
							data.pos_x		= pFrame->pos_x ;
							data.pos_y		= pFrame->pos_y ;
							data.pos_z		= pFrame->pos_z ;
							data.rot_x		= pFrame->rot_x ;
							data.rot_y		= pFrame->rot_y ;
							data.rot_z		= pFrame->rot_z ;
							data.center_x	= pFrame->cx ;
							data.center_y	= pFrame->cy ;
							data.left		= pFrame->uv[0] ;
							data.right		= pFrame->uv[1] ;
							data.top		= pFrame->uv[2] ;
							data.bottom		= pFrame->uv[3] ;
							data.fScaleX	= pFrame->fScaleX ;
							data.fScaleY	= pFrame->fScaleY ;
							data.nImage		= pFrame->nImageNo ;
							data.bUVAnime	= (pFrame->bFlag & 0x01) ;

							frameDataList.append(data) ;
						}
					}
				}
			}
			break ;
		case ANM2D_ID_OBJECT:
		case ANM2D_ID_LAYER:
		case ANM2D_ID_IMAGE:
			continue ;

		default:
			m_nError = kErrorNo_InvalidID ;
			return false ;
		}
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
		case ANM2D_ID_IMAGE:
			{
				Anm2DImage *pImage = (Anm2DImage *)p ;

				QImage image ;
				if ( !image.load(pImage->fileName) ) {
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
				ImageData.fileName = pImage->fileName ;
				ImageData.lastModified = QDateTime::currentDateTimeUtc() ;
				data.insert(pImage->nImageNo, ImageData);
			}
			break ;
		case ANM2D_ID_OBJECT:
		case ANM2D_ID_LAYER:
		case ANM2D_ID_FRAMEDATA:
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
		if ( p->header.nID != ANM2D_ID_OBJECT ) { continue ; }
		if ( name != QString(p->objName.name) ) { continue ; }

		return p ;
	}
	return NULL ;
}

// 名前からレイヤを探す
Anm2DLayer *CAnm2DBin::search2DLayerFromName(Anm2DHeader *pHeader, QString name)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DLayer *p = (Anm2DLayer *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		if ( p->header.nID != ANM2D_ID_LAYER ) { continue ; }
		if ( name != QString(p->layerName.name) ) { continue ; }

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
	if ( nodeMap.namedItem(kAnmXML_Attr_Version).isNull()
	  || nodeMap.namedItem(kAnmXML_Attr_Version).toAttr().value().toInt() != kAnmXML_Version ) {
		m_nError = kErrorNo_InvalidVersion ;
		return false ;
	}
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

	qDebug("objNum:%d imageNum:%d", m_ObjNum, m_ImageNum) ;

	QDomNode n = root.firstChild() ;
	if ( !addElement(n, rEditData) ) {
		return false ;
	}

	return true ;
}

// ヘッダエレメント作成
bool CAnm2DXml::makeHeader(QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
Q_UNUSED(doc) ;

	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	element.setAttribute(kAnmXML_Attr_Version, kAnmXML_Version);
	element.setAttribute(kAnmXML_Attr_ObjNum, objList.size());
	element.setAttribute(kAnmXML_Attr_ImageNum, rEditData.getImageDataSize());
	return true ;
}

// オブジェクトエレメント作成
bool CAnm2DXml::makeObject(QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::ObjectGroup &objGroup = objList.at(i) ;
		QStandardItem *pObjID = objGroup.first ;
		const CObjectModel::LayerGroupList &layerGroupList = objGroup.second ;

		QDomElement elmObj = doc.createElement(kAnmXML_ID_Object) ;
		elmObj.setAttribute(kAnmXML_Attr_Name, QString(pObjID->text().toUtf8()));
		elmObj.setAttribute(kAnmXML_Attr_No, i);
		elmObj.setAttribute(kAnmXML_Attr_LayerNum, layerGroupList.size());
		element.appendChild(elmObj) ;

		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(j) ;
			QStandardItem *pLayerID = layerGroup.first ;
			const CObjectModel::FrameDataList &frameDataList = layerGroup.second ;

			QDomElement elmLayer = doc.createElement(kAnmXML_ID_Layer) ;
			elmLayer.setAttribute(kAnmXML_Attr_Name, QString(pLayerID->text().toUtf8()));
			elmLayer.setAttribute(kAnmXML_Attr_No, j);
			elmLayer.setAttribute(kAnmXML_Attr_FrameNum, frameDataList.size());
			elmObj.appendChild(elmLayer) ;

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const CObjectModel::FrameData &data = frameDataList.at(k) ;

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

				elmLayer.appendChild(elmFrameData) ;

				if ( m_pProgress ) {
					m_pProgress->setValue(m_pProgress->value()+1);

					if ( m_pProgress->wasCanceled() ) {
						m_nError = kErrorNo_Cancel ;
						return false ;
					}
				}
			}
		}
	}

	return true ;
}

// イメージエレメント作成
bool CAnm2DXml::makeImage( QDomElement &element, QDomDocument &doc, CEditData &rEditData )
{
	for ( int i = 0 ; i < rEditData.getImageDataSize() ; i ++ ) {
		QString imgFilePath = rEditData.getImageFileName(i) ;
		QImage image = rEditData.getImage(i) ;

		QDomElement elmImage = doc.createElement(kAnmXML_ID_Image) ;
		elmImage.setAttribute(kAnmXML_Attr_No, i);

		QDomElement elmTmp ;
		QDomText text ;

		elmTmp = doc.createElement("FilePath") ;
#if 1
		QString relPath = getRelativePath(m_filePath, imgFilePath) ;
		text = doc.createTextNode(relPath.toUtf8()) ;
#else
		text = doc.createTextNode(imgFilePath.toUtf8()) ;
#endif
		elmTmp.appendChild(text) ;
		elmImage.appendChild(elmTmp) ;

		elmTmp = doc.createElement("Size") ;
		text = doc.createTextNode(QString("%1 %2").arg(image.width()).arg(image.height())) ;
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

	int max = 0 ;
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).second ;
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			max += layerGroupList.at(j).second.size() ;
		}
	}

	if ( m_bSaveImage ) {
		for ( int i = 0 ; i < rEditData.getImageDataSize() ; i ++ ) {
			max += rEditData.getImage(i).height() * rEditData.getImage(i).width() ;
		}
	}
	qDebug() << "max:" << max ;
	pProg->setMaximum(max);
}

// エレメント追加
bool CAnm2DXml::addElement( QDomNode &node, CEditData &rEditData )
{
	CObjectModel *pModel = rEditData.getObjectModel() ;
	CObjectModel::ObjectList *pObjList = pModel->getObjectListPtr() ;
	QStandardItemModel *pTreeModel = rEditData.getTreeModel() ;
	QStandardItem *pTreeRoot = pTreeModel->invisibleRootItem() ;

	QList<CEditData::ImageData> ImageData ;

	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_Object ) {	// オブジェクト
			QString name ;
			int layerNum = 0 ;
			int no = 0 ;
			QDomNamedNodeMap nodeMap = node.attributes() ;
			if ( nodeMap.namedItem(kAnmXML_Attr_Name).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LayerNum).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_No).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt() ;
			no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;

			CObjectModel::ObjectGroup objGroup ;
			QStandardItem *pObjItem = new QStandardItem(name) ;

			pTreeRoot->insertRow(no, pObjItem);
			objGroup.first = pObjItem ;

			QDomNode child = node.firstChild() ;
			if ( !addLayer(child, objGroup.second, pObjItem, layerNum) ) {
				return false ;
			}

			pObjList->insert(no, objGroup) ;
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
			ImageData.insert(no, data);
		}
		node = node.nextSibling() ;
	}

	if ( pObjList->size() != m_ObjNum
	  || ImageData.size() != m_ImageNum ) {
		m_nError = kErrorNo_InvalidObjNum ;
		return false ;
	}

	rEditData.setImageData(ImageData);
	return true ;
}

// レイヤデータを追加
bool CAnm2DXml::addLayer( QDomNode &node, CObjectModel::LayerGroupList &layerGroupList, QStandardItem *pParentItem, int maxLayerNum )
{
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

			QStandardItem *layerItem = new QStandardItem(name) ;
			layerItem->setData(true, Qt::CheckStateRole);
			pParentItem->insertRow(no, layerItem) ;

			CObjectModel::LayerGroup layerGroup ;
			layerGroup.first = layerItem ;

			QDomNode child = node.firstChild() ;
			if ( !addFrameData(child, layerGroup.second, frameDataNum) ) {
				return false ;
			}

			layerGroupList.insert(no, layerGroup) ;
		}
		node = node.nextSibling() ;
	}

	if ( layerGroupList.size() != maxLayerNum ) {
		m_nError = kErrorNo_InvalidLayerNum ;
		return false ;
	}
	return true ;
}

// フレームデータを追加
bool CAnm2DXml::addFrameData( QDomNode &node, CObjectModel::FrameDataList &frameDataList, int maxFrameDataNum )
{
	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_FrameData ) {
			CObjectModel::FrameData data ;
			memset( &data, 0, sizeof(data) ) ;

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
					data.pos_x = pos[0].toInt() ;
					data.pos_y = pos[1].toInt() ;
					data.pos_z = pos[2].toInt() ;
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
					data.rot_x = rot[0].toInt() ;
					data.rot_y = rot[1].toInt() ;
					data.rot_z = rot[2].toInt() ;
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
					data.center_x = center[0].toInt() ;
					data.center_y = center[1].toInt() ;
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
					data.left	= uv[0].toInt() ;
					data.top	= uv[1].toInt() ;
					data.right	= uv[2].toInt() ;
					data.bottom	= uv[3].toInt() ;
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

				dataNode = dataNode.nextSibling() ;
			}

			frameDataList.append(data);
		}
		node = node.nextSibling() ;
	}

	if ( frameDataList.size() != maxFrameDataNum ) {
		qDebug("%d %d", frameDataList.size(), maxFrameDataNum) ;
		m_nError = kErrorNo_InvalidFrameDataNum ;
		return false ;
	}
	return true ;
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
		qDebug() << data.fileName ;
		if ( data.fileName.isEmpty() ) {	// ファイルパスもない
			m_nError = kErrorNo_InvalidFilePath ;
			return false ;
		}
#if 1
		QString path = getAbsolutePath(m_filePath, data.fileName) ;
		data.Image = QImage(path) ;
#else
		data.Image = QImage(data.fileName) ;
#endif
		if ( data.Image.isNull() ) {
			m_nError = kErrorNo_InvalidFilePath ;
			return false ;
		}
	}
	return true ;
}

QString CAnm2DXml::getRelativePath(QString &src, QString &dest)
{
	QString path = src ;
	if ( path.at(path.count()-1) != '/' ) {
		path.chop(path.count()-path.lastIndexOf("/")-1) ;
	}
	QDir dir(path) ;
	return dir.relativeFilePath(dest) ;
}

QString CAnm2DXml::getAbsolutePath(QString &src, QString &dest)
{
	QString path = src ;
	if ( path.at(path.count()-1) != '/' ) {
		path.chop(path.count()-path.lastIndexOf("/")-1) ;
	}
	QDir dir(path) ;
	return dir.absoluteFilePath(dest) ;
}



