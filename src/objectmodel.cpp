#include "objectmodel.h"

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
	return false ;
}

// pObj のレイヤグループﾘｽﾄを返す
CObjectModel::LayerGroupList *CObjectModel::getLayerGroupListFromID( typeID objID )
{
	for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
		if ( m_ObjectList.at(i).first != objID ) { continue ; }
		return &m_ObjectList[i].second ;
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

CObjectModel::typeID CObjectModel::getLayerIDFromFrameAndPos( typeID objID, int frame, QPoint pos )
{
	LayerGroupList *p = getLayerGroupListFromID(objID) ;
	if ( !p ) { return 0 ; }
	for ( int i = 0 ; i < p->size() ; i ++ ) {
		const FrameDataList &frameDataList = p->at(i).second ;
		for ( int j = 0 ; j < frameDataList.size() ; j ++ ) {
			const FrameData data = frameDataList.at(j) ;
			if ( data.frame != frame ) { continue ; }
			if ( !isFrameDataInPos(data, pos) ) { continue ; }
			return p->at(i).first ;
		}
	}
	return 0 ;
}

CObjectModel::FrameData *CObjectModel::getFrameDataFromPrevFrame( typeID objID, typeID layerID, int nowFrame )
{
	FrameData *pRet = NULL ;

	do {
		nowFrame -- ;
		pRet = getFrameDataFromIDAndFrame(objID, layerID, nowFrame) ;
		if ( pRet ) { return pRet ; }
	} while ( nowFrame >= 0 ) ;

	return NULL ;
}

CObjectModel::FrameData *CObjectModel::getFrameDataFromNextFrame( typeID objID, typeID layerID, int nowFrame )
{
	FrameData *pRet = NULL ;

	do {
		nowFrame ++ ;
		pRet = getFrameDataFromIDAndFrame(objID, layerID, nowFrame) ;
		if ( pRet ) { return pRet ; }
	} while ( nowFrame < 180 ) ;

	return NULL ;
}


