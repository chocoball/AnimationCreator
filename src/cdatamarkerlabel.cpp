#include <QtGui>
#include <QDebug>
#include "cdatamarkerlabel.h"

CDataMarkerLabel::CDataMarkerLabel(QWidget *parent) :
	QLabel(parent),
	m_value(0),
	m_frameStart(0),
	m_frameEnd(0),
	m_offset(0),
	m_pressFrame(-1),
	m_pressCurrentFrame(-1),
	m_moveMode(kMoveMode_None)
{
}

void CDataMarkerLabel::setValue(int val)
{
	if ( val == m_value ) { return ; }

	m_value = val ;
	emit sig_changeValue(val) ;
	repaint() ;
}

void CDataMarkerLabel::setScrollBarSize()
{
	int w = getX(m_frameEnd, false)+20 - width() ;
	if ( w > 0 ) {
		m_pHorizontalScrollBar->setMaximum(w) ;
		m_pHorizontalScrollBar->setEnabled(true) ;
	}
	else {
		m_pHorizontalScrollBar->setEnabled(false) ;
	}
}

void CDataMarkerLabel::slot_setFrameStart(int val)
{
	if ( val == m_frameStart ) { return ; }
	m_frameStart = val ;
	setScrollBarSize() ;
	slot_moveScrollBar(m_pHorizontalScrollBar->value()) ;
}

void CDataMarkerLabel::slot_setFrameEnd(int val)
{
	if ( val == m_frameEnd ) { return ; }
	m_frameEnd = val ;
	setScrollBarSize() ;
	slot_moveScrollBar(m_pHorizontalScrollBar->value()) ;
}

void CDataMarkerLabel::slot_moveScrollBar(int val)
{
	int w = getX(m_frameEnd, false)+20 - width() ;
	if ( w <= 0 ) {
		m_offset = 0 ;
		repaint() ;
		return ;
	}
	m_offset = -w * val / m_pHorizontalScrollBar->maximum() ;
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
	ObjectItem *obj = pModel->getObject(index) ;
	if ( !obj ) { return ; }

	QList<int> frames ;
	drawLayer(obj->getIndex(), painter, index, frames) ;
	if ( m_pressCurrentFrame >= 0 ) {
		int x0 = getX(m_pressCurrentFrame) ;
		int x1 = getX(m_pressCurrentFrame+1) ;
		int y0 = 0 ;
		QColor col = QColor(255, 0, 0, 128) ;
		if ( pModel->isObject(index) ) {
			y0 = height()/3 ;
			col.setRed(255) ;
		}
		painter.fillRect(x0, y0, x1-x0, height()/3, col) ;
	}
}

void CDataMarkerLabel::drawFrameBase(QPainter &painter)
{
	painter.setPen(QColor(0, 0, 0)) ;
	for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
		int x0 = getX(i) ;
		int x1 = getX(i + 1) ;
		if ( x1 < 0 ) { continue ; }

		if ( m_value == i ) {
			painter.fillRect(x0, 0, x1-x0, height(), QColor(32, 32, 32, 128)) ;
		}

		if ( !(i % 5) ) {
			painter.drawLine(x0, 0, x0, height()) ;
			painter.drawText(x0+2, 10, QString("%1").arg(i)) ;
		}
		else {
			int y = height()/3 ;
			if ( y < 10 ) { y = 10 ; }
			painter.drawLine(x0, y, x0, height()) ;
		}
	}
}

void CDataMarkerLabel::drawLayer( const QModelIndex &index, QPainter &painter, const QModelIndex selIndex, QList<int> &frames )
{
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	bool bMine = false ;
	if ( selIndex.isValid() && index == selIndex ) {
		bMine = true ;
	}

	const QList<FrameData> &datas = pItem->getFrameData() ;
	for ( int i = 0 ; i < datas.size() ; i ++ ) {
		const FrameData data = datas.at(i) ;

		if ( data.frame < m_frameStart || data.frame > m_frameEnd ) { continue ; }

		int x0 = getX(data.frame) ;
		int x1 = getX(data.frame+1) ;
		if ( x1 < 0 ) { continue ; }
		if ( bMine ) {
			if ( m_pressFrame == data.frame ) { continue ; }
			painter.fillRect(x0, 0, x1-x0, height()/3, QColor(255, 0, 0, 128)) ;
		}
		else {
			if ( frames.indexOf(data.frame) >= 0 ) { continue ; }
			frames.append(data.frame) ;

			QColor col = QColor(0, 0, 255, 128) ;
			if ( m_pEditData->getObjectModel()->isObject(m_pEditData->getSelIndex()) ) {
				col.setRed(255) ;
			}
			painter.fillRect(x0, height()/3, x1-x0, height()/3, col) ;
		}
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		QModelIndex child = m_pEditData->getObjectModel()->index(i, 0, index) ;
		drawLayer(child, painter, selIndex, frames) ;
	}
}

int CDataMarkerLabel::getX(int frame, bool bAddOffset)
{
	if ( m_frameEnd - m_frameStart < 1 ) { return -1 ; }

	int w = width()-(5+20) ;
	float w1 = (float)w/(m_frameEnd-m_frameStart) ;
	if ( w1 < 8 ) {
		w1 = 8 ;
	}
	return 5 + w1*(frame-m_frameStart) + (bAddOffset?m_offset:0) ;
}

void CDataMarkerLabel::mousePressEvent(QMouseEvent *ev)
{
	m_bMouseMove = false ;
	m_bPressLeft = false ;
	m_moveMode = kMoveMode_None ;
	if ( ev->button() != Qt::LeftButton ) {
		ev->ignore();
		return ;
	}
	m_oldMousePos = ev->pos() ;
	m_bPressLeft = true ;
	m_pressFrame = m_pressCurrentFrame = -1 ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = m_pEditData->getSelIndex() ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;

	// 移動予定のフレームデータ取得
	if ( ev->y() < height()/3 ) {
		if ( pModel->isLayer(index) ) {
			for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
				if ( ev->pos().x() >= getX(i) && ev->pos().x() < getX(i+1) ) {
					if ( pItem->getFrameDataPtr(i) ) {
						m_pressCurrentFrame = m_pressFrame = i ;
						m_moveMode = kMoveMode_Layer ;
					}
					break ;
				}
			}
		}
	}
	else if ( pModel->isObject(index) && ev->y() < height()*2/3 ) {
		for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
			if ( ev->pos().x() >= getX(i) && ev->pos().x() < getX(i+1) ) {
				if ( pItem->getFrameDataPtr(i, true) ) {
					m_pressCurrentFrame = m_pressFrame = i ;
					m_moveMode = kMoveMode_Object ;
				}
				break ;
			}
		}
	}
}

void CDataMarkerLabel::mouseMoveEvent(QMouseEvent *ev)
{
	if ( m_bPressLeft ) {
		m_bMouseMove = true ;

		if ( m_pressFrame >= 0 ) {
			int old = m_pressCurrentFrame ;
			for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
				if ( ev->pos().x() >= getX(i) && ev->pos().x() < getX(i+1) ) {
					m_pressCurrentFrame = i ;
					break ;
				}
			}
			if ( m_pressCurrentFrame != old ) {
				repaint() ;
			}
		}
	}
}

void CDataMarkerLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if ( m_bPressLeft ) {
		for ( int i = m_frameStart ; i <= m_frameEnd ; i ++ ) {
			if ( ev->pos().x() >= getX(i) && ev->pos().x() < getX(i+1) ) {
				if ( !m_bMouseMove ) {
					setValue(i) ;
					emit sig_changeValue(i) ;
				}
				else if ( m_pressFrame >= 0 ){
					if ( m_pressFrame != i ) {
						if ( m_moveMode == kMoveMode_Layer ) {
							emit sig_moveFrameData(m_pressFrame, i) ;	// フレームデータ移動
						}
						else if ( m_moveMode == kMoveMode_Object ) {
							emit sig_moveAllFrameData(m_pressFrame, i) ;
						}
					}
				}
				break ;
			}
		}
	}
	m_bPressLeft = false ;
	m_pressFrame = m_pressCurrentFrame = -1 ;
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

