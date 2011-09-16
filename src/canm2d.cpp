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

	if ( !makeHeader(header, rEditData) ) {
		return false ;
	}
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::ObjectGroup &objGroup = objList.at(i) ;
		QStandardItem *pObjItem = (QStandardItem *)objGroup.id ;
		const CObjectModel::LayerGroupList &layerGroupList = objGroup.layerGroupList ;

		QByteArray objArray ;
		objArray.resize(sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int)) ;
		Anm2DObject *pObjData = (Anm2DObject *)objArray.data() ;
		memset( pObjData, 0, sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int) ) ;
		pObjData->header.nID   = ANM2D_ID_OBJECT ;
		pObjData->header.nSize = sizeof(Anm2DObject) + (layerGroupList.size()-1)*sizeof(unsigned int) ;
		strncpy( pObjData->objName.name, pObjItem->text().toUtf8().data(), sizeof(Anm2DName) ) ;	// オブジェクト名
		pObjData->nLayerNum = layerGroupList.size() ;
		pObjData->nLoopNum = objGroup.nLoop ;	// ループ回数(after ver 0.1.0)

		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(j) ;
			QStandardItem *pLayerItem = (QStandardItem *)layerGroup.first ;
			const FrameDataList &frameDataList = layerGroup.second ;

			QByteArray layerArray ;
			layerArray.resize(sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int));
			Anm2DLayer *pLayerData = (Anm2DLayer *)layerArray.data() ;
			memset(pLayerData, 0, sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int)) ;
			pLayerData->header.nID   = ANM2D_ID_LAYER ;
			pLayerData->header.nSize = sizeof(Anm2DLayer) + (frameDataList.size()-1)*sizeof(unsigned int) ;
			strncpy( pLayerData->layerName.name, pLayerItem->text().toUtf8().data(), sizeof(Anm2DName) ) ;	// レイヤ名
			pLayerData->nLayerNo = layerList.size() ;
			pLayerData->nFrameDataNum = frameDataList.size() ;

			pObjData->nLayerNo[j] = pLayerData->nLayerNo ;	// オブジェクトが参照するレイヤ番号セット

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const FrameData &data = frameDataList.at(k) ;

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
				// 頂点色(after ver 0.1.0)
				pFrameData->rgba[0]			= data.rgba[0] ;
				pFrameData->rgba[1]			= data.rgba[1] ;
				pFrameData->rgba[2]			= data.rgba[2] ;
				pFrameData->rgba[3]			= data.rgba[3] ;

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
#endif
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
#if 0
	TODO
	int blockNum = 0 ;
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	blockNum += objList.size() ;	// オブジェクト数
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).layerGroupList ;
		blockNum += layerGroupList.size() ;	// レイヤ数
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const FrameDataList &frameDataList = layerGroupList.at(j).second ;
			blockNum += frameDataList.size() ;	// フレームデータ数
		}
	}
	blockNum += rEditData.getImageDataListSize() ;	// イメージデータ数

	rData.resize(sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int));
	Anm2DHeader *pHeader = (Anm2DHeader *)rData.data() ;
	memset( pHeader, 0, sizeof(Anm2DHeader) + (blockNum-1) * sizeof(unsigned int) ) ;
	pHeader->header.nID		= ANM2D_ID_HEADER ;
	pHeader->header.nSize	= sizeof( Anm2DHeader ) ;
	pHeader->nVersion		= ANM2D_VERSION ;
	pHeader->nFileSize		= 0 ;	// 後で入れる
	pHeader->nBlockNum		= blockNum ;
#endif
	return true ;
}

// イメージデータ作成
bool CAnm2DBin::makeImageList( QList<QByteArray> &rData, CEditData &rEditData )
{
	int imageNum = rEditData.getImageDataListSize() ;

	for ( int i = 0 ; i < imageNum ; i ++ ) {
#if 1
		CEditData::ImageData *p = rEditData.getImageData(i) ;
		if ( !p ) { continue ; }

		QImage img = p->Image ;
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
		pImage->nImageNo = p->nNo ;
		QString relPath = getRelativePath(m_filePath, p->fileName) ;
		strncpy(pImage->fileName, relPath.toUtf8().data(), 255) ;
		memcpy(pImage->data, img.bits(), img.width()*img.height()*4) ;
#else
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
		QString relPath = getRelativePath(m_filePath, rEditData.getImageFileName(i)) ;
		strncpy(pImage->fileName, relPath.toUtf8().data(), 255) ;
		memcpy(pImage->data, img.bits(), img.width()*img.height()*4) ;
#endif
		rData << imgArray ;
	}
	return true ;
}

// 編集データにオブジェクトを追加
bool CAnm2DBin::addObject(Anm2DHeader *pHeader, CEditData &rEditData)
{
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;
	QStandardItemModel *pTreeModel = rEditData.getTreeModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_OBJECT:
			{
				Anm2DObject *pObj = (Anm2DObject *)p ;

				QStandardItem *pRoot = pTreeModel->invisibleRootItem() ;
				QStandardItem *newItem = new QStandardItem(QString::fromUtf8(pObj->objName.name)) ;
				pRoot->appendRow(newItem);

				CObjectModel::ObjectGroup objGroup ;
				objGroup.id = newItem ;
				objGroup.nLoop = pObj->nLoopNum ;	// ループ回数(after ver 0.1.0)
				pModel->addObject(objGroup);

				m_ObjPtrList.append(pObj) ;
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
#endif
	return true ;
}

// 編集データにレイヤを追加
bool CAnm2DBin::addLayer(Anm2DHeader *pHeader, CEditData &rEditData)
{
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_LAYER:
			{
				Anm2DLayer *pLayer = (Anm2DLayer *)p ;

				CObjectModel::ObjectList &objList = *pModel->getObjectListPtr() ;
				if ( m_ObjPtrList.size() != objList.size() ) {
					qDebug() << "ObjPtrList != objList" ;
					m_nError = kErrorNo_InvalidObjNum ;
					return false ;
				}
				for ( int j = 0 ; j < objList.size() ; j ++ ) {
					QStandardItem *pObjItem = (QStandardItem *)objList.at(j).id ;
					Anm2DObject *pObj = m_ObjPtrList[j] ;
					if ( !pObj ) {
					}
					for ( int k = 0 ; k < pObj->nLayerNum ; k ++ ) {
						if ( pObj->nLayerNo[k] != pLayer->nLayerNo ) { continue ; }

						QStandardItem *newItem = new QStandardItem(QString::fromUtf8(pLayer->layerName.name)) ;
						newItem->setData(true, Qt::CheckStateRole);
						pObjItem->appendRow(newItem);

						CObjectModel::LayerGroup layerGroup = qMakePair(newItem, FrameDataList()) ;
						objList[j].layerGroupList.append(layerGroup);
					}
				}
				m_LayerPtrList.append(pLayer) ;

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
#endif
	return true ;
}

// 編集データにフレームデータを追加
bool CAnm2DBin::addFrameData(Anm2DHeader *pHeader, CEditData &rEditData)
{
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;

	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_FRAMEDATA:
			{
				Anm2DFrameData *pFrame = (Anm2DFrameData *)p ;

				CObjectModel::ObjectList &objList = *pModel->getObjectListPtr() ;

				if ( m_ObjPtrList.size() != objList.size() ) {
					qDebug() << "ObjPtrList != objList" ;
					m_nError = kErrorNo_InvalidObjNum ;
					return false ;
				}
				for ( int objNum = 0 ; objNum < objList.size() ; objNum ++ ) {
					Anm2DObject *pObj = m_ObjPtrList[objNum] ;
					CObjectModel::LayerGroupList &layerGroupList = objList[objNum].layerGroupList ;
					for ( int layerNum = 0 ; layerNum < pObj->nLayerNum ; layerNum ++ ) {
						Anm2DLayer *pLayer = m_LayerPtrList[ pObj->nLayerNo[layerNum] ] ;
						if ( !pLayer ) {
							qDebug() << "pLayer == NULLpo" ;
							m_nError = kErrorNo_InvalidLayerNum ;
							return false ;
						}

						FrameDataList &frameDataList = layerGroupList[layerNum].second ;
						for ( int frameNum = 0 ; frameNum < pLayer->nFrameDataNum ; frameNum ++ ) {
							if ( pLayer->nFrameDataNo[frameNum] != pFrame->nFrameDataNo ) { continue ; }

							FrameData data ;
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
							// 頂点色(after ver 0.1.0)
							data.rgba[0]	= pFrame->rgba[0] ;
							data.rgba[1]	= pFrame->rgba[1] ;
							data.rgba[2]	= pFrame->rgba[2] ;
							data.rgba[3]	= pFrame->rgba[3] ;

							frameDataList.append(data) ;
						}
					}
				}
				m_FrameDataPtrList.append(pFrame) ;

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
#endif
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
		if ( p->header.nID != ANM2D_ID_LAYER ) { continue ; }
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
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	element.setAttribute(kAnmXML_Attr_Version, kAnmXML_Version);
	element.setAttribute(kAnmXML_Attr_ObjNum, objList.size());
	element.setAttribute(kAnmXML_Attr_ImageNum, rEditData.getImageDataListSize());
#endif
	return true ;
}

// オブジェクトエレメント作成
bool CAnm2DXml::makeObject(QDomElement &element, QDomDocument &doc, CEditData &rEditData)
{
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::ObjectGroup &objGroup = objList.at(i) ;
		QStandardItem *pObjID = objGroup.id ;
		const CObjectModel::LayerGroupList &layerGroupList = objGroup.layerGroupList ;

		QDomElement elmObj = doc.createElement(kAnmXML_ID_Object) ;
		elmObj.setAttribute(kAnmXML_Attr_Name, QString(pObjID->text().toUtf8()));
		elmObj.setAttribute(kAnmXML_Attr_No, i);
		elmObj.setAttribute(kAnmXML_Attr_LayerNum, layerGroupList.size());
		elmObj.setAttribute(kAnmXML_Attr_LoopNum, objGroup.nLoop);	// ループ回数(after ver 0.1.0)
		element.appendChild(elmObj) ;

		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(j) ;
			QStandardItem *pLayerID = layerGroup.first ;
			const FrameDataList &frameDataList = layerGroup.second ;

			QDomElement elmLayer = doc.createElement(kAnmXML_ID_Layer) ;
			elmLayer.setAttribute(kAnmXML_Attr_Name, QString(pLayerID->text().toUtf8()));
			elmLayer.setAttribute(kAnmXML_Attr_No, j);
			elmLayer.setAttribute(kAnmXML_Attr_FrameNum, frameDataList.size());
			elmObj.appendChild(elmLayer) ;

			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const FrameData &data = frameDataList.at(k) ;

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
#endif
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
bool CAnm2DXml::addElement( QDomNode &node, CEditData &rEditData )
{
#if 0
	TODO
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
			  || nodeMap.namedItem(kAnmXML_Attr_No).isNull()
			  || nodeMap.namedItem(kAnmXML_Attr_LoopNum).isNull() ) {
				m_nError = kErrorNo_InvalidNode ;
				return false ;
			}
			name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value() ;
			layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt() ;
			no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;
			int loopNum = nodeMap.namedItem(kAnmXML_Attr_LoopNum).toAttr().value().toInt() ;

			CObjectModel::ObjectGroup objGroup ;
			QStandardItem *pObjItem = new QStandardItem(name) ;

			pTreeRoot->insertRow(no, pObjItem);
			objGroup.id = pObjItem ;
			objGroup.nLoop = loopNum ;		// ループ回数(after ver 0.1.0)

			QDomNode child = node.firstChild() ;
//TODO			if ( !addLayer(child, objGroup.layerGroupList, pObjItem, layerNum) ) {
				return false ;
//TODO			}

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
			data.nNo = no ;
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
#endif
	return true ;
}
#if 0
TODO
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
bool CAnm2DXml::addFrameData( QDomNode &node, FrameDataList &frameDataList, int maxFrameDataNum )
{
	while ( !node.isNull() ) {
		if ( node.nodeName() == kAnmXML_ID_FrameData ) {
			FrameData data ;
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
#endif
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
CAnm2DJson::CAnm2DJson()
	: CAnm2DBase()
{
}

bool CAnm2DJson::makeFromEditData( CEditData &rEditData )
{
	addString("{\n") ;
	if ( !makeObject(rEditData) ) { return false ; }
	if ( !makeImage(rEditData) ) { return false ; }
	addString("}") ;
	return true ;
}

bool CAnm2DJson::makeObject( CEditData &rEditData )
{
#if 0
	TODO
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;

	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::ObjectGroup &objGroup = objList.at(i) ;
		QStandardItem *pObjID = objGroup.id ;
		const CObjectModel::LayerGroupList &layerGroupList = objGroup.layerGroupList ;

		addString("  \"" + QString(pObjID->text().toUtf8()) + "\": {\n") ;
		addString("    \"animeTime\": " + QVariant((int)(pModel->getMaxFrameFromSelectObject(pObjID)*(100.0/6.0))).toString() + ",\n") ;
		addString("    \"loopNum\": " + QVariant(objGroup.nLoop).toString() + ",\n") ;
		addString("    \"layer\": [\n") ;

		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			const CObjectModel::LayerGroup &layerGroup = layerGroupList.at(j) ;
			const FrameDataList &frameDataList = layerGroup.second ;

			addString("      {\n") ;
			addString("        \"frame\": [\n") ;
			for ( int k = 0 ; k < frameDataList.size() ; k ++ ) {
				const FrameData &data = frameDataList.at(k) ;
				CEditData::ImageData *pImageData = rEditData.getImageDataFromNo(data.nImage) ;
				int extPos = pImageData->fileName.lastIndexOf("/") ;
				QString path ;
				path = pImageData->fileName.right(pImageData->fileName.size() - extPos - 1) ;
				float anchor[2], uv[4] ;
				anchor[0] = (float)data.center_x / (float)(data.right-data.left) ;
				anchor[1] = (float)data.center_y / (float)(data.bottom-data.top) ;
				uv[0] = (float)data.left / (float)pImageData->origImageW ;
				uv[1] = (float)data.top / (float)pImageData->origImageW ;
				uv[2] = (float)(data.right-data.left) / (float)pImageData->origImageW ;
				uv[3] = (float)(data.bottom-data.top) / (float)pImageData->origImageW ;

				addString("          {\n") ;
				addString("            \"frame\": " + QVariant((int)(data.frame*(100.0/6.0))).toString() + ",\n") ;
				addString("            \"pos\": [" + QVariant(data.pos_x).toString() + ", " + QVariant(data.pos_y).toString() + ", " + QVariant(data.pos_z).toString() + "],\n") ;
				addString("            \"rot\": [" + QVariant(data.rot_x).toString() + ", " + QVariant(data.rot_y).toString() + ", " + QVariant(data.rot_z).toString() + "],\n") ;
				addString("            \"sca\": [" + QVariant(data.fScaleX).toString() + ", " + QVariant(data.fScaleY).toString() + "],\n") ;
				addString("            \"color\": [" + QVariant((float)data.rgba[0]/255.0f).toString() + ", " + QVariant((float)data.rgba[1]/255.0f).toString() + ", " + QVariant((float)data.rgba[2]/255.0f).toString() + ", " + QVariant((float)data.rgba[3]/255.0f).toString() + "],\n") ;
				addString("            \"uvAnime\": " + QVariant((int)data.bUVAnime).toString() + ",\n") ;
				addString("            \"image\": {\n") ;
				addString("              \"path\": \"" + path + "\",\n") ;
				addString("              \"size\": [" + QVariant(pImageData->origImageW).toString() + ", " + QVariant(pImageData->origImageH).toString() + "],\n") ;
				addString("              \"center\": [" + QVariant(anchor[0]).toString() + ", " + QVariant(anchor[1]).toString() + "],\n") ;
				addString("              \"uvrect\": [" + QVariant(uv[0]).toString() + ", " + QVariant(uv[1]).toString() + ", " + QVariant(uv[2]).toString() + ", " + QVariant(uv[3]).toString() + "]\n") ;
				addString("            }\n") ;
				addString("          }") ;
				if ( k < frameDataList.size()-1 ) { addString(",") ; }
				addString("\n") ;
			}
			addString("        ]\n") ;
			addString("      }") ;
			if ( j < layerGroupList.size()-1 ) { addString(",") ; }
			addString("\n") ;
		}
		addString("    ]\n") ;
		addString("  }") ;
		if ( i < objList.size()-1 ) { addString(",") ; }
		addString("\n") ;
	}
#endif
	return true ;
}

bool CAnm2DJson::makeImage( CEditData &rEditData )
{
	return true ;
}



bool CAnm2DJson::makeFromFile(QString &str, CEditData &rEditData)
{
	return false ;
}


