#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QtGui>
#include <QPair>
#include <QMatrix4x4>

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
		unsigned char	rgba[4] ;					///< RGBA

		bool operator == (const struct _tagFrameData &r) const
		{
			if ( frame != r.frame )			{ return false ; }
			if ( pos_x != r.pos_x )			{ return false ; }
			if ( pos_y != r.pos_y )			{ return false ; }
			if ( pos_z != r.pos_z )			{ return false ; }
			if ( rot_x != r.rot_x )			{ return false ; }
			if ( rot_y != r.rot_y )			{ return false ; }
			if ( rot_z != r.rot_z )			{ return false ; }
			if ( center_x != r.center_x )	{ return false ; }
			if ( center_y != r.center_y )	{ return false ; }
			if ( left != r.left )			{ return false ; }
			if ( right != r.right )			{ return false ; }
			if ( top != r.top )				{ return false ; }
			if ( bottom != r.bottom )		{ return false ; }
			if ( nImage != r.nImage )		{ return false ; }
			if ( fScaleX != r.fScaleX )		{ return false ; }
			if ( fScaleY != r.fScaleY )		{ return false ; }
			if ( bUVAnime != r.bUVAnime )	{ return false ; }
			if ( rgba[0] != r.rgba[0] )		{ return false ; }
			if ( rgba[1] != r.rgba[1] )		{ return false ; }
			if ( rgba[2] != r.rgba[2] )		{ return false ; }
			if ( rgba[3] != r.rgba[3] )		{ return false ; }

			return true ;
		}
		bool operator != (const struct _tagFrameData &r) const
		{
			if ( *this == r ) {
				return false ;
			}
			return true ;
		}

		void setRect( QRect &rect )
		{
			left	= rect.left() ;
			right	= rect.right() ;
			top		= rect.top() ;
			bottom	= rect.bottom() ;
		}
		QRect getRect() const
		{
			return QRect(left, top, right-left+1, bottom-top+1) ;
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

		void getVertexApplyMatrix(QVector3D ret[4]) const
		{
			Vertex v = getVertex() ;
			ret[0] = QVector3D(v.x0, v.y0, 0) ;		// left-up
			ret[1] = QVector3D(v.x1, v.y0, 0) ;		// right-up
			ret[2] = QVector3D(v.x0, v.y1, 0) ;		// left-down
			ret[3] = QVector3D(v.x1, v.y1, 0) ;		// right-down

			QMatrix4x4 m ;
			m.setToIdentity();
			m.translate(pos_x, pos_y, pos_z/4096.0f);
			m.rotate(rot_x, 1, 0, 0);
			m.rotate(rot_y, 0, 1, 0);
			m.rotate(rot_z, 0, 0, 1);
			for ( int i = 0 ; i < 4 ; i ++ ) {
				ret[i] = m * ret[i] ;
			}
		}

		struct _tagFrameData getInterpolation(const struct _tagFrameData *pNext, int nowFrame) const
		{
			struct _tagFrameData data = *this ;
			if ( pNext ) {
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

				for ( int i = 0 ; i < 4 ; i ++ ) {
					data.rgba[i] += (pNext->rgba[i] - rgba[i])*frameNow/frameAll ;
				}
			}
			return data ;
		}
	} FrameData ;

	typedef QStandardItem*							typeID ;
	typedef QList<FrameData>						FrameDataList ;
	typedef QPair<typeID, FrameDataList>			LayerGroup ;		// レイヤID, フレームデータリスト
	typedef QList<LayerGroup>						LayerGroupList ;
#if 1
	typedef struct {
		typeID			id ;						// オブジェクトID
		LayerGroupList	layerGroupList ;			// レイヤグループリスト
		int				nLoop ;						// ループ回数
		int				nCurrentLoop ;				// 現在のループ回数(アニメーション再生中)
	} ObjectGroup ;
#else
	typedef QPair<typeID, LayerGroupList>			ObjectGroup ;		// オブジェクトID, レイヤグループリスト
#endif
	typedef QList<ObjectGroup>						ObjectList ;

public:
	explicit CObjectModel() ;
	virtual ~CObjectModel() ;

	bool isFrameDataInPos( const FrameData &data, QPoint pos ) ;
	bool isFrameDataInRect( const FrameData &data, QRect rect ) ;

	const ObjectList &getObjectList( void ) { return m_ObjectList ; }
	ObjectList		*getObjectListPtr( void ) { return &m_ObjectList ; }
	ObjectGroup		*getObjectGroupFromID( typeID objID ) ;
	LayerGroupList	*getLayerGroupListFromID( typeID objID ) ;
	FrameDataList	*getFrameDataListFromID( typeID objID, typeID layerID ) ;
	FrameData		*getFrameDataFromIDAndFrame(typeID objID, typeID layerID, int frame) ;
	FrameData		*getMaxFrameDataFromID(typeID objID, typeID layerID) ;
	typeID			getLayerIDFromFrameAndPos( typeID objID, int frame, QPoint pos ) ;
	FrameData		*getFrameDataFromPrevFrame( typeID objID, typeID layerID, int nowFrame, bool bRepeat = false ) ;
	FrameData		*getFrameDataFromNextFrame( typeID objID, typeID layerID, int nowFrame ) ;
	QList<typeID>	getFrameDatasFromRect( typeID objID, int frame, QRect rect ) ;

	int getObjListSize() { return m_ObjectList.size() ; }

	ObjectGroup *searchObjectGroup( typeID objID )
	{
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( objID != m_ObjectList[i].id ) {
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
		return &p->layerGroupList ;
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
			if ( m_ObjectList.at(i).id != objID ) { continue ; }

			m_ObjectList.removeAt(i);
			return ;
		}
	}
	void delLayerGroup( typeID objID, typeID layerID )
	{
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( m_ObjectList.at(i).id != objID ) { continue ; }

			LayerGroupList &layerGroupList = m_ObjectList[i].layerGroupList ;
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

	int getMaxFrameFromSelectObject( typeID objID )
	{
		int ret = 0 ;
		for ( int i = 0 ; i < m_ObjectList.size() ; i ++ ) {
			if ( objID != m_ObjectList.at(i).id ) { continue ; }

			const LayerGroupList &layerGroup = m_ObjectList.at(i).layerGroupList ;
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
