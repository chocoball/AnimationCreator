#include "objectitem.h"

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

FrameData ObjectItem::getDisplayFrameData(int frame)
{
	FrameData d, parent ;
	if ( m_pParent ) {
		parent = m_pParent->getDisplayFrameData(frame) ;
	}

	FrameData *pPrev = getFrameDataFromPrevFrame(frame) ;
	if ( pPrev ) {
		FrameData *pNext = getFrameDataFromNextFrame(frame) ;
		d = pPrev->getInterpolation(pNext) ;
		d.fromParent(parent) ;
	}
	return d ;
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
		FrameData *pData = getFrameDataPtr(frame) ;
		if ( pData ) { return pData ; }
	}
	return NULL ;
}




