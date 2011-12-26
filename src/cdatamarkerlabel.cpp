#include <QtGui>
#include <QDebug>
#include "cdatamarkerlabel.h"

CDataMarkerLabel::CDataMarkerLabel(QWidget *parent) :
	QLabel(parent),
	m_value(0),
	m_frameStart(0),
	m_frameEnd(0),
	m_offset(0)
{
}

void CDataMarkerLabel::setValue(int val)
{
	if ( val == m_value ) { return ; }

	m_value = val ;
	repaint() ;
}

void CDataMarkerLabel::slot_setFrameStart(int val)
{
	if ( val == m_frameStart ) { return ; }
	m_frameStart = val ;
	repaint() ;
}

void CDataMarkerLabel::slot_setFrameEnd(int val)
{
	if ( val == m_frameEnd ) { return ; }
	m_frameEnd = val ;
	repaint() ;
}

void CDataMarkerLabel::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this) ;
	painter.drawRect(0, 0, width()-1, height()-1) ;

	if ( m_frameEnd-m_frameStart < 1 ) { return ; }

	drawFrameBase(painter) ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = m_pEditData->getSelIndex() ;
	if ( pModel->isObject(index) ) {
		painter.setPen(QPen(QColor(255, 0, 0)));
		drawLine(index, painter, QModelIndex()) ;
	}
	else if ( pModel->isLayer(index) ) {
		drawLine(index, painter, index) ;
	}
}

void CDataMarkerLabel::drawFrameBase(QPainter &painter)
{
	painter.setPen(QColor(0, 0, 0)) ;
	for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
		int x = getX(i) ;
		if ( x < -20 ) { continue ; }

		if ( m_value == i ) {
			painter.fillRect(x, 0, getX(i+1)-x, height(), QColor(32, 32, 32, 128)) ;
		}

		if ( !(i % 5) ) {
			painter.drawLine(x, height()-30, x, height()) ;
			painter.drawText(x+2, height()-20, QString("%1").arg(i)) ;
		}
		else {
			painter.drawLine(x, height()-20, x, height()) ;
		}
	}
}

void CDataMarkerLabel::drawLine( const QModelIndex &index, QPainter &painter, const QModelIndex selIndex )
{
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	bool bMine = false ;
	if ( selIndex.isValid() ) {
		if ( index == selIndex ) {
			painter.setPen(QColor(255, 0, 0)) ;
			bMine = true ;
		}
		else {
			painter.setPen(QColor(0, 0, 255, 128)) ;
		}
	}

	const QList<FrameData> &datas = pItem->getFrameData() ;
	for ( int i = 0 ; i < datas.size() ; i ++ ) {
		const FrameData data = datas.at(i) ;
		if ( data.frame < m_frameStart || data.frame > m_frameEnd ) { continue ; }

		int x = getX(data.frame) ;
		if ( x < 0 ) { continue ; }
		if ( bMine ) {
			painter.drawLine(x, 0, x, height()-10) ;
		}
		else {
			painter.drawLine(x, height()-10, x, height()-20) ;
		}
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		QModelIndex child = m_pEditData->getObjectModel()->index(i, 0, index) ;
		drawLine(child, painter, selIndex) ;
	}
}

int CDataMarkerLabel::getX(int frame)
{
#if 0
	return 5 + m_offset + (frame-m_frameStart) * 20 ;
#else
	if ( m_frameEnd - m_frameStart < 1 ) { return -1 ; }

	int w = width()-(5+20) ;
	return 5 + w*frame/(m_frameEnd-m_frameStart) ;
#endif
}

void CDataMarkerLabel::mousePressEvent(QMouseEvent *ev)
{
	m_bMouseMove = false ;
	m_bPressLeft = false ;
	if ( ev->button() != Qt::LeftButton ) {
		ev->ignore();
		return ;
	}
	m_oldMousePos = ev->pos() ;
	m_bPressLeft = true ;
}

void CDataMarkerLabel::mouseMoveEvent(QMouseEvent *ev)
{
	if ( m_bPressLeft ) {
#if 0
		m_offset += ev->pos().x() - m_oldMousePos.x() ;
		fixOffset() ;

		repaint() ;
		m_oldMousePos = ev->pos() ;
#endif
		m_bMouseMove = true ;
	}
}

void CDataMarkerLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if ( m_bPressLeft ) {
		if ( !m_bMouseMove ) {
			for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
				if ( ev->pos().x() >= getX(i) && ev->pos().x() < getX(i+1) ) {
					setValue(i) ;
					emit sig_changeValue(i) ;
					break ;
				}
			}
		}
	}
	m_bPressLeft = false ;
}

void CDataMarkerLabel::fixOffset(void)
{
	int endPos = 25 + (m_frameEnd-m_frameStart) * 20 ;
	if ( endPos > width() ) {
		if ( m_offset < -(endPos-width()) ) {
			m_offset = -(endPos-width()) ;
		}
	}
	else {
		m_offset = 0 ;
		return ;
	}
	if ( m_offset > 0 ) { m_offset = 0 ; }
}

