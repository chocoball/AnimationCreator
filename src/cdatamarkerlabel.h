#ifndef CDATAMARKERLABEL_H
#define CDATAMARKERLABEL_H

#include <QLabel>
#include "include.h"
#include "editdata.h"

class CDataMarkerLabel : public QLabel
{
    Q_OBJECT
public:
	explicit CDataMarkerLabel(QWidget *parent = 0);

	int value() { return m_value ; }
	void setValue(int val) ;

signals:
	void sig_changeValue(int val) ;

public slots:
	void slot_setFrameStart(int val) ;
	void slot_setFrameEnd(int val) ;

protected:
	void paintEvent(QPaintEvent *event);
	void drawFrameBase(QPainter &painter) ;
	void drawLine( const QModelIndex &index, QPainter &painter, const QModelIndex selIndex ) ;

	int getX(int frame) ;

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

	bool		m_bPressLeft ;
	bool		m_bMouseMove ;
} ;

#endif // CDATAMARKERLABEL_H
