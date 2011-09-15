#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include <QRect>
#include <QList>
#include "framedata.h"
#include "include.h"


class ObjectItem
{
public:
	ObjectItem(QString name, ObjectItem *parent)
	{
		m_name		= name ;
		m_pParent	= parent ;
		m_nLoop		= 0 ;
		m_nCurrLoop	= 0 ;
	}

	~ObjectItem()
	{
		qDeleteAll(m_children) ;
	}

	int childCount() { return m_children.size() ; }
	ObjectItem *child(int row)
	{
		if ( row < 0 || row >= m_children.size() ) { return NULL ; }
		return m_children[row] ;
	}
	void insertChild(int row, ObjectItem *p)
	{
		m_children.insert(row, p) ;
	}
	void removeChild(ObjectItem *p)
	{
		int index = m_children.indexOf(p) ;
		if ( index < 0 ) { return ; }
		m_children.removeAt(index) ;
		delete p ;
	}

	FrameData *getFrameDataPtr(int col)
	{
		if ( col < 0 || col >= m_frameDatas.size() ) { return NULL ; }
		return &m_frameDatas[col] ;
	}

	void copy(ObjectItem *p)
	{
		this->m_name = p->m_name ;
		this->m_nLoop = p->m_nLoop ;
		this->m_nCurrLoop = p->m_nCurrLoop ;
		for ( int i = 0 ; i < p->m_children.size() ; i ++ ) {
			insertChild(i, new ObjectItem(p->m_children[i]->m_name, this)) ;
			this->m_children[i]->copy(p->m_children[i]);
		}
		this->m_frameDatas = p->m_frameDatas ;
	}


	kAccessor(QString, m_name, Name)
	kAccessor(int, m_nLoop, Loop)
	kAccessor(int, m_nCurrLoop, CurrLoop)

private:
	ObjectItem			*m_pParent ;
	QList<ObjectItem *>	m_children ;
	QList<FrameData>	m_frameDatas ;
};

#endif // OBJECTITEM_H
