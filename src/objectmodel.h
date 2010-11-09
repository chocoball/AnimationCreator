#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QtGui>
#include <QPair>

typedef struct {
	float		x0, y0 ;
	float		x1, y1 ;
} Vertex ;

class CObjectModel
{
public:
	typedef struct _tagFrameData {
		unsigned short	frame ;
		short			pos_x, pos_y, pos_z ;		///< global position
		short			rot_x, rot_y, rot_z ;		///< rotation
		short			center_x, center_y ;		///< local center position
		short			left, right, top, bottom ;	///< UV
		short			nImage ;					///< Image No.
		float			fScaleX, fScaleY ;			///< scale
		bool			bUVAnime ;					///< UVアニメするならtrue

		void setRect( QRect &rect )
		{
			left	= rect.left() ;
			right	= rect.right() ;
			top		= rect.top() ;
			bottom	= rect.bottom() ;
		}
		QRect getRect() const
		{
			return QRect(left, top, right-left, bottom-top) ;
		}
		int width() const { return right - left ; }
		int height() const { return bottom - top ; }
		Vertex getVertex() const
		{
			Vertex v ;
			v.x0 = -center_x * fScaleX ;
			v.y0 = -center_y * fScaleY ;
			v.x1 = v.x0 + width()*fScaleX ;
			v.y1 = v.y0 + height()*fScaleY ;
			return v ;
		}
		struct _tagFrameData getInterpolation(const struct _tagFrameData *pNext, int nowFrame) const
		{
			struct _tagFrameData data = *this ;
			int frameNow = nowFrame - frame ;
			int frameAll = pNext->frame - frame ;
			data.pos_x		+= (pNext->pos_x - pos_x)*frameNow/frameAll ;
			data.pos_y		+= (pNext->pos_y - pos_y)*frameNow/frameAll ;
			data.pos_z		+= (pNext->pos_z - pos_z)*frameNow/frameAll ;
			data.rot_x		+= (pNext->rot_x - rot_x)*frameNow/frameAll ;
			data.rot_y		+= (pNext->rot_y - rot_y)*frameNow/frameAll ;
			data.rot_z		+= (pNext->rot_z - rot_z)*frameNow/frameAll ;
			data.center_x	+= (pNext->center_x - center_x)*frameNow/frameAll ;
			data.center_y	+= (pNext->center_y - center_y)*frameNow/frameAll ;

			if ( pNext->bUVAnime ) {
				data.left		+= (pNext->left - left)*frameNow/frameAll ;
				data.right		+= (pNext->right - right)*frameNow/frameAll ;
				data.top		+= (pNext->top - top)*frameNow/frameAll ;
				data.bottom		+= (pNext->bottom - bottom)*frameNow/frameAll ;
			}

			data.fScaleX	+= (pNext->fScaleX - fScaleX)*frameNow/frameAll ;
			data.fScaleY	+= (pNext->fScaleY - fScaleY)*frameNow/frameAll ;
			return data ;
		}
	} FrameData ;

	typedef QStandardItem*							typeID ;
	typedef QList<FrameData>						FrameDataList ;
	typedef QPair<typeID, FrameDataList>			LayerGroup ;		// レイヤID, フレームデータリスト
	typedef QList<LayerGroup>						LayerGroupList ;
	typedef QPair<typeID, LayerGroupList>			ObjectGroup ;		// オブジェクトID, レイヤグループリスト
	typedef QList<ObjectGroup>						ObjectList ;

public:
	explicit CObjectModel() ;
	virtual ~CObjectModel() ;

	bool isFrameDataInPos( const FrameData &data, QPoint pos ) ;

	const ObjectList &getObjectList( void ) { return m_ObjectList ; }
	ObjectList		*getObjectListPtr( void ) { return &m_ObjectList ; }
	LayerGroupList	*getLayerGroupListFromID( typeID objID ) ;
	FrameDataList	*getFrameDataListFromID( typeID objID, typeID layerID ) ;
	FrameData		*getFrameDataFromIDAndFrame(typeID objID, typeID layerID, int frame) ;
	FrameData		*getMaxFrameDataFromID(typeID objID, typeID layerID) ;
	typeID			getLayerIDFromFrameAndPos( typeID objID, int frame, QPoint pos ) ;
	FrameData		*getFrameDataFromPrevFrame( typeID objID, typeID layerID, int nowFrame, bool bRepeat = false ) ;
	FrameData		*getFrameDataFromNextFrame( typeID objID, typeID layerID, int nowFrame ) ;

	int getObjListSize() { return m_ObjectList.size() ; }

	ObjectGroup *searchObjectGroup( typeID objID )
	{
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( objID != m_ObjectList[i].first ) {
				continue ;
			}
			return &m_ObjectList[i] ;
		}
		return NULL ;
	}

	LayerGroupList *searchLayerGroupList( typeID objID )
	{
		ObjectGroup *p = searchObjectGroup(objID) ;
		if ( !p ) { return NULL ; }
		return &p->second ;
	}

	LayerGroup *searchLayerGroup( LayerGroupList &layerGroupList, typeID layerID )
	{
		for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
			if ( layerID != layerGroupList[i].first ) {
				continue ;
			}
			return &layerGroupList[i] ;
		}
		return NULL ;
	}

	void addObject( ObjectGroup &obj )
	{
		m_ObjectList.append(obj);
	}

	void delObjectGroup( typeID objID )
	{
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( m_ObjectList.at(i).first != objID ) { continue ; }

			m_ObjectList.removeAt(i);
			return ;
		}
	}
	void delLayerGroup( typeID objID, typeID layerID )
	{
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( m_ObjectList.at(i).first != objID ) { continue ; }

			LayerGroupList &layerGroupList = m_ObjectList[i].second ;
			for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
				if ( layerGroupList.at(j).first != layerID ) { continue ; }

				layerGroupList.removeAt(j);
				return ;
			}
		}
	}
	bool delLayerData( typeID objID, typeID layerID, int frame )
	{
		FrameDataList *pList = getFrameDataListFromID(objID, layerID) ;
		if ( !pList ) { return false ; }
		for ( int i = 0 ; i < pList->size() ; i ++ ) {
			const FrameData &data = pList->at(i) ;
			if ( data.frame != frame ) { continue ; }
			pList->removeAt(i) ;
			return true ;
		}
		return false ;
	}

	int getMaxFrame( void )
	{
		int ret = 0 ;
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			const LayerGroupList &layerGroup = m_ObjectList.at(i).second ;
			for ( int j = 0 ; j < layerGroup.size() ; j ++ ) {
				const FrameDataList &dataList = layerGroup.at(j).second ;
				for ( int k = 0 ; k < dataList.size() ; k ++ ) {
					const FrameData &data = dataList.at(k) ;
					if ( ret < data.frame ) {
						ret = data.frame ;
					}
				}
			}
		}
		return ret ;
	}

private:
	ObjectList		m_ObjectList ;
};

#endif // OBJECTMODEL_H
