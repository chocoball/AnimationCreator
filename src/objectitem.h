#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include <QRect>
#include <QList>
#include <QModelIndex>
#include <QBrush>
#include "framedata.h"
#include "include.h"


class ObjectItem
{
public:
	enum {
		kState_Disp = 0x01,		// 表示設定
		kState_Lock = 0x02		// ロック
	};
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

	int row() { return m_index.row() ; }
	ObjectItem *parent() { return m_pParent ; }
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

	FrameData *getFrameDataPtr(int frame)
	{
		for ( int i = 0 ; i < m_frameDatas.size() ; i ++ ) {
			if ( frame == m_frameDatas.at(i).frame ) {
				return &m_frameDatas[i] ;
			}
		}
		return NULL ;
	}
	const QList<FrameData> getFrameData()
	{
		return m_frameDatas ;
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

	int getMaxFrameNum(bool bRecv = true)
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

	QVariant data(int role) ;
	void setData(const QVariant &value, int role = Qt::UserRole + 1) ;
	FrameData getDisplayFrameData(int frame, bool *bValid = 0) ;
	FrameData *getFrameDataFromPrevFrame(int frame, bool bRepeat = false) ;
	FrameData *getFrameDataFromNextFrame(int frame) ;
	bool isContain(ObjectItem *pRet, QPoint &pos, int frame, bool bChild = true) ;

	kAccessor(QString, m_name, Name)
	kAccessor(int, m_nLoop, Loop)
	kAccessor(int, m_nCurrLoop, CurrLoop)
	kAccessor(QModelIndex, m_index, Index)

private:
	bool isContain(FrameData &displayData, QPoint &pos) ;

private:
	ObjectItem			*m_pParent ;
	QList<ObjectItem *>	m_children ;
	QList<FrameData>	m_frameDatas ;

	QVariant			m_checkStateData ;
	QVariant			m_foregroundData ;
};

#endif // OBJECTITEM_H
