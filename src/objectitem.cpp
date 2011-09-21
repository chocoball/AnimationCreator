#include "objectitem.h"
#include "editdata.h"

ObjectItem *ObjectItem::child(int row)
{
	if ( row < 0 || row >= m_children.size() ) { return NULL ; }
	return m_children[row] ;
}
void ObjectItem::insertChild(int row, ObjectItem *p)
{
	m_children.insert(row, p) ;
}
void ObjectItem::removeChild(ObjectItem *p)
{
	int index = m_children.indexOf(p) ;
	if ( index < 0 ) { return ; }
	m_children.removeAt(index) ;
	delete p ;
}

void ObjectItem::addFrameData(FrameData &data)
{
	FrameData *p = getFrameDataPtr(data.frame) ;
	if ( p ) {
		*p = data ;
	}
	else {
		m_frameDatas.append(data) ;
	}
}
void ObjectItem::removeFrameData(int frame)
{
	int index = getFrameDataIndex(frame) ;
	if ( index < 0 ) { return ; }
	m_frameDatas.removeAt(index) ;
}

int ObjectItem::getFrameDataIndex(int frame)
{
	for ( int i = 0 ; i < m_frameDatas.size() ; i ++ ) {
		if ( frame == m_frameDatas.at(i).frame ) { return i ; }
	}
	return -1 ;
}

FrameData *ObjectItem::getFrameDataPtr(int frame)
{
	for ( int i = 0 ; i < m_frameDatas.size() ; i ++ ) {
		if ( frame == m_frameDatas.at(i).frame ) { return &m_frameDatas[i] ; }
	}
	return NULL ;
}

void ObjectItem::sortFrameData()
{
	for ( int i = 0 ; i < m_frameDatas.size() ; i ++ ) {
		for ( int j = 0 ; j < i ; j ++ ) {
			if ( m_frameDatas.at(i).frame < m_frameDatas.at(j).frame ) {
				m_frameDatas.swap(i, j) ;
			}
		}
	}
}

void ObjectItem::copy(ObjectItem *p)
{
	this->m_nLoop = p->m_nLoop ;
	this->m_nCurrLoop = p->m_nCurrLoop ;
	this->m_frameDatas = p->m_frameDatas ;
	this->m_checkStateData = p->m_checkStateData ;
	this->m_foregroundData = p->m_foregroundData ;

	for ( int i = 0 ; i < p->m_children.size() ; i ++ ) {
		insertChild(i, new ObjectItem(p->m_children[i]->m_name, this)) ;
		this->m_children[i]->copy(p->m_children[i]);
	}
}

int ObjectItem::getMaxFrameNum(bool bRecv)
{
	int ret = 0 ;

	for ( int i = 0 ; i < m_frameDatas.size() ; i ++ ) {
		if ( ret < m_frameDatas.at(i).frame ) {
			ret = m_frameDatas.at(i).frame ;
		}
	}
	if ( bRecv ) {
		for ( int i = 0 ; i < m_children.size() ; i ++ ) {
			int tmp = m_children[i]->getMaxFrameNum() ;
			if ( ret < tmp ) { ret = tmp ; }
		}
	}
	return ret ;
}

QVariant ObjectItem::data(int role)
{
	switch ( role ) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			return getName() ;
		case Qt::CheckStateRole:
			return m_checkStateData ;
		case Qt::ForegroundRole:
			return m_foregroundData ;
	}
	return QVariant() ;
}

void ObjectItem::setData(const QVariant &value, int role)
{
	switch ( role ) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			setName(value.toString()) ;
		case Qt::CheckStateRole:
			m_checkStateData = value ;
			break ;
		case Qt::ForegroundRole:
			m_foregroundData = value ;
			break ;
	}
}

FrameData ObjectItem::getDisplayFrameData(int frame, bool *bValid)
{
	FrameData d ;
	FrameData *pPrev = getFrameDataFromPrevFrame(frame+1) ;
	if ( pPrev ) {
		FrameData *pNext = getFrameDataFromNextFrame(frame) ;
		d = pPrev->getInterpolation(pNext, frame) ;

		if ( bValid ) { *bValid = true ; }
	}
	else {
		if ( bValid ) { *bValid = false ; }
	}
	return d ;
}

QMatrix4x4 ObjectItem::getDisplayMatrix(int frame, bool *bValid)
{
	QMatrix4x4 parent, mat ;
	if ( m_pParent ) {
		parent = m_pParent->getDisplayMatrix(frame) ;
	}

	bool valid ;
	FrameData d = getDisplayFrameData(frame, &valid) ;
	if ( valid ) {
		mat.translate(d.pos_x, d.pos_y, d.pos_z/4096.0f) ;
		mat.rotate(d.rot_x, 1, 0, 0) ;
		mat.rotate(d.rot_y, 0, 1, 0) ;
		mat.rotate(d.rot_z, 0, 0, 1) ;
		mat.scale(d.fScaleX, d.fScaleY) ;
		mat = parent * mat ;

		if ( bValid ) { *bValid = true ; }
	}
	else {
		if ( bValid ) { *bValid = false ; }
	}
	return mat ;
}

FrameData *ObjectItem::getFrameDataFromPrevFrame(int frame, bool bRepeat)
{
	int old = frame ;
	frame -- ;
	while ( old != frame ) {
		if ( frame < 0 ) {
			if ( !bRepeat ) { return NULL ; }

			frame += CEditData::kMaxFrame ;
		}
		FrameData *pData = getFrameDataPtr(frame) ;
		if ( pData ) { return pData ; }
		frame -- ;
	}
	return NULL ;
}

FrameData *ObjectItem::getFrameDataFromNextFrame(int frame)
{
	int max = getMaxFrameNum(false) ;

	for ( int i = frame + 1 ; i < max+1 ; i ++ ) {
		FrameData *pData = getFrameDataPtr(i) ;
		if ( pData ) { return pData ; }
	}
	return NULL ;
}

bool ObjectItem::isContain(ObjectItem **ppRet, QPoint &pos, int frame, bool bChild)
{
	if ( bChild ) {
		for ( int i = 0 ; i < childCount() ; i ++ ) {
			if ( child(i)->isContain(ppRet, pos, frame, true) ) { return true ; }
		}
	}

	bool valid ;
	FrameData d = getDisplayFrameData(frame, &valid) ;
	if (valid && isContain(d, pos, getDisplayMatrix(frame)) ) {
		*ppRet = this ;
		return true ;
	}
	return false ;
}

bool ObjectItem::isContain(FrameData &displayData, QPoint &pos, const QMatrix4x4 &matDisp)
{
	QVector3D v[4] ;
	displayData.getVertexApplyMatrix(v, matDisp) ;
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

	return false ;
}


