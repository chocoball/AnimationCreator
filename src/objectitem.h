#ifndef OBJECTITEM_H
#define OBJECTITEM_H

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
		m_nFps		= 60 ;
	}

	~ObjectItem()
	{
		qDeleteAll(m_children) ;
	}

	// -----------------------------------------------
	// Model
	// -----------------------------------------------
	int row()						{ return m_index.row() ; }
	ObjectItem *parent()			{ return m_pParent ; }
	void setParent(ObjectItem *p)	{ m_pParent = p ; }
	int childCount()				{ return m_children.size() ; }
	ObjectItem *child(int row) ;
	void insertChild(int row, ObjectItem *p) ;
	void removeChild(ObjectItem *p) ;
	void flat() ;
	QList<ObjectItem *> getLayers(ObjectItem *pObj) ;

	// -----------------------------------------------
	// FRAMEDATA
	// -----------------------------------------------
	const QList<FrameData> &getFrameData() { return m_frameDatas ; }
	void setFrameDatas(const QList<FrameData> &data) { m_frameDatas = data ; }

	void addFrameData(const FrameData &data) ;
	void removeFrameData(int frame) ;
	int getFrameDataIndex(int frame) ;
	FrameData *getFrameDataPtr(int frame, bool bRecv = false) ;
	void sortFrameData() ;

	void copy(ObjectItem *p) ;
	int getMaxFrameNum(bool bRecv = true) ;

	QVariant data(int role) ;
	void setData(const QVariant &value, int role = Qt::UserRole + 1) ;

	FrameData getDisplayFrameData(int frame, bool *bValid = 0) ;
	QMatrix4x4 getDisplayMatrix(int frame, bool *bValid = 0) ;
	QMatrix4x4 getParentDispMatrix(int frame, bool *bValid = 0) ;
	FrameData *getFrameDataFromPrevFrame(int frame, bool bRepeat = false) ;
	FrameData *getFrameDataFromNextFrame(int frame) ;
	bool isContain(ObjectItem **ppRet, QPoint &pos, int frame, bool bChild = true, bool bCheckFlag = true) ;

	QPointF getBezierPos(int frame, bool *pValid) ;
	bool isUseImageRecv(int imageNo) ;

	bool validate(bool bRecv = true) ;

	kAccessor(QString, m_name, Name)
	kAccessor(int, m_nLoop, Loop)
	kAccessor(int, m_nCurrLoop, CurrLoop)
	kAccessor(int, m_nFps, Fps)
	kAccessor(QModelIndex, m_index, Index)

private:
	bool isContain(FrameData &displayData, QPoint &pos, const QMatrix4x4 &matDisp) ;

	void applyFrameDataFromParent() ;
	int getParentFrameMax() ;
	FrameData *getParentFrameDataPtr(int frame) ;

	int getAllChildNum(ObjectItem *root) ;

private:
	ObjectItem			*m_pParent ;
	QList<ObjectItem *>	m_children ;
	QList<FrameData>	m_frameDatas ;

	QVariant			m_checkStateData ;
};

#endif // OBJECTITEM_H
