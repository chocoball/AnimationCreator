#ifndef CURVEGRAPHLABEL_H
#define CURVEGRAPHLABEL_H

#include <QLabel>
#include <QModelIndex>
#include "include.h"
#include "editdata.h"

class CurveGraphLabel : public QLabel
{
    Q_OBJECT
public:
	explicit CurveGraphLabel(CEditData *pEditData, QWidget *parent = 0);

	enum {
		kDispType_None = -1,
		kDispType_PosX = 0,
		kDispType_PosY,
		kDispType_PosZ,
		kDispType_RotX,
		kDispType_RotY,
		kDispType_RotZ,
		kDispType_CenterX,
		kDispType_CenterY,
		kDispType_ScaleX,
		kDispType_ScaleY,
		kDispType_UvTop,
		kDispType_UvLeft,
		kDispType_UvRight,
		kDispType_UvBottom,
		kDispType_ColorR,
		kDispType_ColorG,
		kDispType_ColorB,
		kDispType_ColorA
	} ;

	void adjustSize() ;
signals:

public slots:

protected:
	void paintEvent(QPaintEvent *event) ;

	kAccessor(QModelIndex, m_currIndex, CurrentIndex)
	kAccessor(int, m_currDispType, CurrentDispType)
	kAccessor(float, m_currMag, CurrentMag)

private:
	QList< QPair<int, float> > getDatasFromCurrentType(ObjectItem *pLayer) ;
	QPair<float, float> getDataSubMaxMin(const QList< QPair<int, float> > &datas) ;
	void drawFrameNum(QPainter &painter, int max) ;

private:
	CEditData		*m_pEditData ;
};

#endif // CURVEGRAPHLABEL_H
