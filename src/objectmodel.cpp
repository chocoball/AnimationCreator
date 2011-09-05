#include "objectmodel.h"
#include "editdata.h"

// ---------------------------------------------------------------------------------
//
// CObjectModel
//
// ---------------------------------------------------------------------------------
CObjectModel::CObjectModel()
{
}

CObjectModel::~CObjectModel()
{
}

bool CObjectModel::isFrameDataInPos( const FrameData &data, QPoint pos )
{
#if 1
	QVector3D v[4] ;
	data.getVertexApplyMatrix(v);
	QVector3D tri[2][3] = {
		{
			v[0],
			v[2],
			v[1],
		},
		{
			v[2],
			v[1],
			v[3],
		}
	} ;
	for ( int i = 0 ; i < 3 ; i ++ ) {
		tri[0][i].setZ(0);
		tri[1][i].setZ(0);
	}

	QVector3D p = QVector3D(pos.x(), pos.y(), 0) ;
	for ( int i = 0 ; i < 2 ; i ++ ) {
		QVector3D p0 = tri[i][0] - p ;
		QVector3D p1 = tri[i][1] - p ;
		QVector3D p2 = tri[i][2] - p ;
		QVector3D c0 = QVector3D::crossProduct(p0, p1) ;
		QVector3D c1 = QVector3D::crossProduct(p1, p2) ;
		QVector3D c2 = QVector3D::crossProduct(p2, p0) ;
		if ( QVector3D::dotProduct(c0, c1) >= 0
		  && QVector3D::dotProduct(c1, c2) >= 0
		  && QVector3D::dotProduct(c2, c0) >= 0 ) {
			return true ;
		}
	}
#else
	Vertex v = data.getVertex() ;
	int left = v.x0<v.x1 ? v.x0 : v.x1 ;
	int right = v.x0<v.x1 ? v.x1 : v.x0 ;
	int top = v.y0<v.y1 ? v.y0 : v.y1 ;
	int bottom = v.y0<v.y1 ? v.y1 : v.y0 ;
	left += data.pos_x ;
	right += data.pos_x ;
	top += data.pos_y ;
	bottom += data.pos_y ;
	if ( left <= pos.x() && pos.x() <= right &&
		 top <= pos.y() && pos.y() <= bottom ) {
		return true ;
	}
#endif
	return false ;
}

bool CObjectModel::isFrameDataInRect( const FrameData &data, QRect rect )
{
	int i ;
	QVector3D v[4] ;
	data.getVertexApplyMatrix(v);

	for ( i = 0 ; i < 4 ; i ++ ) {
		if ( rect.contains(v[i].x(), v[i].y(), false) ) {
			return true ;
		}
	}
	QVector3D tri[2][3] = {
		{ v[0], v[2], v[1] },
		{ v[2], v[1], v[3] }
	} ;
	for ( i = 0 ; i < 3 ; i ++ ) {
		tri[0][i].setZ(0);
		tri[1][i].setZ(0);
	}
	QPoint pos[4] = {
		rect.topLeft(),
		rect.topRight(),
		rect.bottomLeft(),
		rect.bottomRight()
	} ;
	for ( i = 0 ; i < 4 ; i ++ ) {
		QVector3D vPos = QVector3D(pos[i]) ;
		vPos.setZ(0);

		for ( int j = 0 ; j < 2 ; j ++ ) {
			QVector3D p0 = tri[j][0] - vPos ;
			QVector3D p1 = tri[j][1] - vPos ;
			QVector3D p2 = tri[j][2] - vPos ;
			QVector3D c0 = QVector3D::crossProduct(p0, p1) ;
			QVector3D c1 = QVector3D::crossProduct(p1, p2) ;
			QVector3D c2 = QVector3D::crossProduct(p2, p0) ;
			if ( QVector3D::dotProduct(c0, c1) >= 0
			  && QVector3D::dotProduct(c1, c2) >= 0
			  && QVector3D::dotProduct(c2, c0) >= 0 ) {
				return true ;
			}
		}
	}

	return false ;
}

CObjectModel::ObjectGroup *CObjectModel::getObjectGroupFromID( typeID objID )
{
	for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
		if ( m_ObjectList.at(i).id != objID ) { continue ; }
		return &m_ObjectList[i] ;
	}
	return NULL ;
}

// pObj のレイヤグループﾘｽﾄを返す
CObjectModel::LayerGroupList *CObjectModel::getLayerGroupListFromID( typeID objID )
{
	for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
		if ( m_ObjectList.at(i).id != objID ) { continue ; }
		return &m_ObjectList[i].layerGroupList ;
	}
	return NULL ;
}

CObjectModel::FrameDataList *CObjectModel::getFrameDataListFromID( typeID objID, typeID layerID )
{
	LayerGroupList *p = getLayerGroupListFromID(objID) ;
	if ( !p ) { return NULL ; }

	LayerGroupList::iterator it ;
	for ( it = p->begin() ; it != p->end() ; it ++ ) {
		if ( it->first != layerID ) { continue ; }
		return &it->second ;
	}

	return NULL ;
}

CObjectModel::FrameData *CObjectModel::getFrameDataFromIDAndFrame(typeID objID, typeID layerID, int frame)
{
	FrameDataList *pList = getFrameDataListFromID(objID, layerID) ;
	if ( !pList ) { return NULL ; }

	for ( int i = 0 ; i < pList->size() ; i ++ ) {
		if ( pList->at(i).frame != frame ) { continue ; }
		return &((*pList)[i]) ;
	}
	return NULL ;
}

CObjectModel::FrameData *CObjectModel::getMaxFrameDataFromID(typeID objID, typeID layerID)
{
	FrameDataList *pList = getFrameDataListFromID(objID, layerID) ;
	if ( !pList ) { return NULL ; }

	int frame = -1 ;
	FrameData *pRet = NULL ;
	for ( int i = 0 ; i < pList->size() ; i ++ ) {
		if ( !pRet || (frame < pList->at(i).frame) ) {
			pRet = &((*pList)[i]) ;
			frame = pList->at(i).frame ;
		}
	}
	return pRet ;
}

CObjectModel::typeID CObjectModel::getLayerIDFromFrameAndPos( typeID objID, int frame, QPoint pos )
{
	LayerGroupList *p = getLayerGroupListFromID(objID) ;
	typeID ret = 0 ;
	int z = 0 ;
	if ( !p ) { return 0 ; }
	for ( int i = 0 ; i < p->size() ; i ++ ) {
		const int flag = p->at(i).first->data(Qt::CheckStateRole).toInt() ;
		if ( !(flag & 0x01) || (flag & 0x02) ) { continue ; }	// hide or Lock

		const FrameDataList &frameDataList = p->at(i).second ;
		for ( int j = 0 ; j < frameDataList.size() ; j ++ ) {
			const FrameData data = frameDataList.at(j) ;
			if ( data.frame != frame ) { continue ; }
			if ( !isFrameDataInPos(data, pos) ) { continue ; }

			if ( !ret || z < data.pos_z ) {
				ret = p->at(i).first ;
				z = data.pos_z ;
			}
		}
	}
	return ret ;
}

CObjectModel::FrameData *CObjectModel::getFrameDataFromPrevFrame( typeID objID, typeID layerID, int nowFrame, bool bRepeat )
{
	FrameData *pRet = NULL ;
	int prevFrame = nowFrame ;

	do {
		nowFrame -- ;
		if ( nowFrame < 0 ) {
			FrameData *p = getMaxFrameDataFromID(objID, layerID) ;
			if ( bRepeat && p ) {
				nowFrame = p->frame ;
			}
			else {
				return NULL ;
			}
		}
		pRet = getFrameDataFromIDAndFrame(objID, layerID, nowFrame) ;
		if ( pRet ) { return pRet ; }
	} while ( prevFrame != nowFrame ) ;

	return NULL ;
}

CObjectModel::FrameData *CObjectModel::getFrameDataFromNextFrame( typeID objID, typeID layerID, int nowFrame )
{
	FrameData *pRet = NULL ;

	do {
		nowFrame ++ ;
		pRet = getFrameDataFromIDAndFrame(objID, layerID, nowFrame) ;
		if ( pRet ) { return pRet ; }
	} while ( nowFrame < CEditData::kMaxFrame ) ;

	return NULL ;
}

QList<CObjectModel::typeID> CObjectModel::getFrameDatasFromRect( typeID objID, int frame, QRect rect )
{
	QList<typeID> list ;
	const LayerGroupList *pLayerGroupList = getLayerGroupListFromID(objID) ;

	if ( pLayerGroupList ) {
		for ( int i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			const LayerGroup &layerGroup = pLayerGroupList->at(i) ;
			const FrameData *pData = getFrameDataFromIDAndFrame(objID, layerGroup.first, frame) ;
			if ( !pData ) { continue ; }
			if ( !isFrameDataInRect(*pData, rect) ) { continue ; }

			list << layerGroup.first ;
		}
	}
	return list ;
}
