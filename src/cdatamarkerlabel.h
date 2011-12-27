#ifndef CDATAMARKERLABEL_H
#define CDATAMARKERLABEL_H

#include <QLabel>
#include <QScrollBar>
#include "include.h"
#include "editdata.h"

class CDataMarkerLabel : public QLabel
{
    Q_OBJECT
public:
	explicit CDataMarkerLabel(QWidget *parent = 0);

	void setValue(int val) ;
	void setScrollBarSize() ;

	int value() { return m_value ; }
	void setHorizontalBar(QScrollBar *pBar) { m_pHorizontalScrollBar = pBar ; }

signals:
	void sig_changeValue(int val) ;
	void sig_moveFrameData(int prevFrame, int nextFrame) ;

public slots:
	void slot_setFrameStart(int val) ;
	void slot_setFrameEnd(int val) ;
	void slot_moveScrollBar(int val) ;

protected:
	void paintEvent(QPaintEvent *event);
	void drawFrameBase(QPainter &painter) ;
	void drawLayer( const QModelIndex &index, QPainter &painter, const QModelIndex selIndex, QList<int> &frames ) ;

	int getX(int frame, bool bAddOffset = true) ;

	void mousePressEvent(QMouseEvent *ev) ;
	void mouseMoveEvent(QMouseEvent *ev) ;
	void mouseReleaseEvent(QMouseEvent *ev) ;

	void fixOffset(void) ;

	kAccessor(CEditData*, m_pEditData, EditData)

private:
	int			m_value ;
	int			m_frameStart, m_frameEnd ;
	int			m_offset ;
	QPoint		m_oldMousePos ;
	QScrollBar	*m_pHorizontalScrollBar ;

	bool		m_bPressLeft ;
	bool		m_bMouseMove ;

	int			m_pressFrame, m_pressCurrentFrame ;
} ;

#endif // CDATAMARKERLABEL_H
