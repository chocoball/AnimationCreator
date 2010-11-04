#include "canm2d.h"


CAnm2D::CAnm2D()
{
	m_nError = kErrorNo_NoError ;
}

// 作成中のデータをアニメデータに変換
bool CAnm2D::makeFromEditImageData( CEditImageData &rEditImageData )
{
	QByteArray header ;
	QList<QByteArray> objectList ;
	QList<QByteArray> layerList ;
	QList<QByteArray> frameList ;
	QList<QByteArray> imageList ;

	if ( !makeHeader(header, rEditImageData) ) {
		return false ;
	}

	CObjectModel *pModel = rEditImageData.getObjectModel() ;
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
			pLayerData->nImageNo = 0 ;		// TODO:複数画像対応
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
				pFrameData->uv[0]			= data.left ;
				pFrameData->uv[1]			= data.right ;
				pFrameData->uv[2]			= data.top ;
				pFrameData->uv[3]			= data.bottom ;
				pFrameData->fScaleX			= data.fScaleX ;
				pFrameData->fScaleY			= data.fScaleY ;

				pLayerData->nFrameDataNo[k] = pFrameData->nFrameDataNo ;	// レイヤが参照するフレームデータ番号セット

				frameList << frameDataArray ;
			}

			layerList << layerArray ;
		}

		objectList << objArray ;
	}

	if ( !makeImageList(imageList, rEditImageData) ) {
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

// ヘッダ作成
bool CAnm2D::makeHeader( QByteArray &rData, CEditImageData &rEditImageData )
{
	int blockNum = 0 ;
	CObjectModel *pModel = rEditImageData.getObjectModel() ;
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
	blockNum += 1 ;	// イメージデータ数 TODO:複数画像対応

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
bool CAnm2D::makeImageList( QList<QByteArray> &rData, CEditImageData &rEditImageData )
{
	int imageNum = 1 ;

	for ( int i = 0 ; i < imageNum ; i ++ ) {
		QImage img = rEditImageData.getImage() ;
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
		pImage->nImageNo = 0 ;
		memcpy(pImage->data, img.bits(), img.width()*img.height()*4) ;

		rData << imgArray ;
	}
	return true ;
}

// アニメファイルから作成中データへ変換
bool CAnm2D::makeFromFile(QByteArray &data, QImage &imageData, CEditImageData &rEditImageData)
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
	if ( !addObject(pHeader, rEditImageData) ) {
		return false ;
	}
	if ( !addLayer(pHeader, rEditImageData) ) {
		return false ;
	}
	if ( !addFrameData(pHeader, rEditImageData) ) {
		return false ;
	}
	if ( !addImageData(pHeader, imageData) ) {
		return false ;
	}

	return true ;
}

// オブジェクトを追加
bool CAnm2D::addObject(Anm2DHeader *pHeader, CEditImageData &rEditImageData)
{
	CObjectModel *pModel = rEditImageData.getObjectModel() ;
	QStandardItemModel *pTreeModel = rEditImageData.getTreeModel() ;

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

// レイヤを追加
bool CAnm2D::addLayer(Anm2DHeader *pHeader, CEditImageData &rEditImageData)
{
	CObjectModel *pModel = rEditImageData.getObjectModel() ;

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

// フレームデータを追加
bool CAnm2D::addFrameData(Anm2DHeader *pHeader, CEditImageData &rEditImageData)
{
	CObjectModel *pModel = rEditImageData.getObjectModel() ;

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

// イメージを追加
bool CAnm2D::addImageData(Anm2DHeader *pHeader, QImage &image)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		switch ( p->nID ) {
		case ANM2D_ID_IMAGE:
			{
				Anm2DImage *pImage = (Anm2DImage *)p ;

				image = QImage(pImage->nWidth, pImage->nHeight, QImage::Format_ARGB32) ;
				unsigned int *pCol = (unsigned int *)pImage->data ;
				for ( int y = 0 ; y < pImage->nHeight ; y ++ ) {
					for ( int x = 0 ; x < pImage->nWidth ; x ++ ) {
						image.setPixel(x, y, *pCol);
						pCol ++ ;
					}
				}

				qDebug("w:%d h:%d, %d %d", image.width(), image.height(), pImage->nWidth, pImage->nHeight) ;
				return true ;
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
	return true ;
}

// 名前からオブジェクトを探す
Anm2DObject *CAnm2D::search2DObjectFromName(Anm2DHeader *pHeader, QString name)
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
Anm2DLayer *CAnm2D::search2DLayerFromName(Anm2DHeader *pHeader, QString name)
{
	for ( int i = 0 ; i < pHeader->nBlockNum ; i ++ ) {
		Anm2DLayer *p = (Anm2DLayer *)LP_ADD(pHeader, pHeader->nBlockOffset[i]) ;
		if ( p->header.nID != ANM2D_ID_LAYER ) { continue ; }
		if ( name != QString(p->layerName.name) ) { continue ; }

		return p ;
	}
	return NULL ;
}

