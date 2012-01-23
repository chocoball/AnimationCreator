#include <QtGui>
#include "gridlabel.h"


CGridLabel::CGridLabel(CEditData *pEditData, int nTabIndex, QWidget *parent)
	: QLabel(parent)
{
	m_pEditData = pEditData ;
	mScale = 1 ;
	bCatching = false ;
	if ( parent ) {
		resize(parent->size()) ;
	}
	m_GridSize		= QPoint(16, 16) ;
	m_bCatchable	= true ;
	m_bDrawGrid		= true ;
	m_bPressCtrl	= false ;
	m_Index			= nTabIndex ;
	m_bDrawCenter	= false ;

//	setFocusPolicy(Qt::StrongFocus);
}

void CGridLabel::setDrawCenter( bool flag )
{
	m_bDrawCenter = flag ;
	update() ;
}

void CGridLabel::slot_gridOnOff( bool flag )
{
	m_bDrawGrid = flag ;
	repaint();
}

// 描画イベント
void CGridLabel::paintEvent(QPaintEvent *event)
{
	QPainter painter(this) ;
	QPen pen, penCenter ;
	pen.setColor(QColor(64, 64, 64, 255));
	pen.setWidth(mScale);
	penCenter.setColor(QColor(255, 255, 0, 255));
	penCenter.setWidth(mScale);

	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(m_Index) ;
	if ( !p ) { return ; }

	if ( m_pEditData ) {
		QSize size = p->Image.size()*mScale ;
		size += QSize(mScale, mScale) ;
		resize(size) ;
	}

	if ( m_bDrawGrid ) {
		painter.setPen(pen);
		for ( int x = 0 ; x <= size().width() ; x += m_GridSize.x() * mScale ) {
			if ( x == size().width()/2 ) {
				painter.setPen(penCenter);
			}
			painter.drawLine(x, event->rect().top(), x, event->rect().bottom());
			if ( x == size().width()/2 ) {
				painter.setPen(pen);
			}
		}
		for ( int y = 0 ; y <= size().height() ; y += m_GridSize.y() * mScale ) {
			if ( y == size().height()/2 ) {
				painter.setPen(penCenter);
			}
			painter.drawLine(event->rect().left(), y, event->rect().right(), y);
			if ( y == size().height()/2 ) {
				painter.setPen(pen);
			}
		}
	}

	// 選択中範囲
	if ( m_pEditData && m_bCatchable ) {
		if ( m_bPressCtrl ) {
			pen.setColor(QColor(0, 255, 0, 255));
		}
		else {
			pen.setColor(QColor(255, 0, 0, 255));
		}
		painter.setPen(pen);

		CRectF rect = m_pEditData->getCatchRect() ;
		QRect r = QRect(rect.left(), rect.top(), rect.right()-rect.left(), rect.bottom()-rect.top()) ;
		painter.drawRect(r) ;
	}

	FrameData data ;
	if ( m_pEditData->getNowSelectFrameData(data) ) {
		if ( data.nImage != m_Index ) { return ; }
	}
	else {
		return ;
	}

	// センター表示
	if ( m_bDrawCenter ) {
		pen.setColor(QColor(0, 0, 255, 255));
		painter.setPen(pen);

		int x = data.center_x + data.left ;
		int y = data.center_y + data.top - 1 ;
		QSize size = p->Image.size() ;
		painter.drawLine(QPointF(0, y), QPointF(size.width(), y));
		painter.drawLine(QPointF(x, 0), QPointF(x, size.height()));
	}
}

// 範囲選択開始
void CGridLabel::mousePressEvent(QMouseEvent *ev)
{
	if ( !m_pEditData || !m_bCatchable ) { return ; }
	if ( ev->button() != Qt::LeftButton ) { return ; }

	bCatching = true ;
	m_bRectMove = false ;

	if ( m_pEditData->getCatchRect().contains( ev->pos() / mScale ) ) {	// 範囲内選択した場合
		m_bRectMove = true ;

		if ( m_bPressCtrl ) {	// Ctrl押してたら
			m_MovePos = ev->pos() ;
			repaint() ;
		}
		else {
			startDragAndDrop(ev) ;
		}
	}
	else {
		int x = ev->pos().x() / mScale ;
		int y = ev->pos().y() / mScale ;
		CRectF r = CRectF(x, y, x+1, y+1) ;
		m_pEditData->setCatchRect(r);
		repaint() ;

		emit sig_changeCatchRect(r) ;
	}
}

// 範囲選択中
void CGridLabel::mouseMoveEvent(QMouseEvent *ev)
{
	if ( !m_pEditData || !m_bCatchable ) { return ; }
	if ( !bCatching ) { return ; }

	CRectF r = m_pEditData->getCatchRect() ;
	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(m_Index) ;
	if ( !p ) { return ; }

	int img_w = p->Image.width() ;
	int img_h = p->Image.height() ;
	int x = ev->pos().x() / mScale ;
	int y = ev->pos().y() / mScale ;

	if ( m_bRectMove ) {	// 範囲移動中
		if ( !m_bPressCtrl ) {
			return ;
		}
		// Ctrlキー押してたら
		QPoint add = ev->pos() - m_MovePos ;
		r.setLeft(r.left()+add.x());
		r.setRight(r.right()+add.x());
		r.setTop(r.top()+add.y());
		r.setBottom(r.bottom()+add.y());
		if ( r.left() < 0 ) {
			r.setRight(r.right()-r.left());
			r.setLeft(0);
		}
		if ( r.right() > img_w-1 ) {
			r.setLeft(r.left()-(r.right()-(img_w-1)));
			r.setRight(img_w-1);
		}
		if ( r.top() < 0 ) {
			r.setBottom(r.bottom()-r.top());
			r.setTop(0);
		}
		if ( r.bottom() > img_h-1 ) {
			r.setTop(r.top()-(r.bottom()-(img_h-1)));
			r.setBottom(img_h-1);
		}
		emit sig_changeSelectLayerUV(r) ;

		m_MovePos = ev->pos() ;
	}
	else {
		if ( x < 0 ) { x = 0 ; }
		if ( x >= img_w ) { x = img_w ; }
		if ( x < r.left() ) { x = r.left() ; }
		if ( y < 0 ) { y = 0 ; }
		if ( y >= img_h ) { y = img_h ; }
		if ( y < r.top() ) { y = r.top() ; }

		r.setRight(x);
		r.setBottom(y);
	}

	m_pEditData->setCatchRect(r);
	emit sig_changeCatchRect(r) ;

	repaint() ;
}

// 範囲選択終了
void CGridLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if ( !m_pEditData || !m_bCatchable ) { return ; }
	if ( !bCatching ) { return ; }

	bCatching = false ;
	if ( m_bRectMove ) {	// 範囲移動中
		m_bRectMove = false ;
		return ;
	}
	m_bRectMove = false ;

	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(m_Index) ;
	if ( !p ) { return ; }

	int img_w = p->Image.width() ;
	int img_h = p->Image.height() ;

	CRectF r = m_pEditData->getCatchRect() ;

	float x = ev->pos().x() / mScale ;
	float y = ev->pos().y() / mScale ;

	if ( x < 0 ) { x = 0 ; }
	if ( x > img_w ) { x = img_w ; }
	if ( x < r.left() ) { x = r.left() ; }
	if ( y < 0 ) { y = 0 ; }
	if ( y > img_h ) { y = img_h ; }
	if ( y < r.top() ) { y = r.top() ; }

	r.setRight(x);
	r.setBottom(y);
	if ( r.width() <= 1 || r.height() <= 1 ) {
		r.setLeft(-2);
		r.setRight(-2);
		r.setTop(-1);
		r.setBottom(-1);
	}
	m_pEditData->setCatchRect(r);
	emit sig_changeCatchRect(r) ;

	repaint() ;
}

void CGridLabel::keyPressEvent(QKeyEvent *ev)
{
	if ( ev->key() == Qt::Key_Control ) {
		m_bPressCtrl = true ;
		update() ;
	}

	if ( m_bPressCtrl ) {
		if ( ev->key() == Qt::Key_A ) {
			selectAll() ;
		}
		else if ( ev->key() == Qt::Key_D ) {
			deselect() ;
		}
	}
}

void CGridLabel::keyReleaseEvent(QKeyEvent *ev)
{
	if ( ev->key() == Qt::Key_Control ) {
		m_bPressCtrl = false ;
		update() ;
	}
}

// ドラッグアンドドロップ開始
void CGridLabel::startDragAndDrop( QMouseEvent *ev )
{
	Q_UNUSED(ev) ;

	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(m_Index) ;
	if ( !p ) { return ; }

	QImage img = p->Image.copy(m_pEditData->getCatchRect().toRect()) ;
	QPixmap pix = QPixmap::fromImage(img);

	QByteArray itemData ;
	QDataStream stream(&itemData, QIODevice::WriteOnly) ;
	CRectF rect = m_pEditData->getCatchRect() ;
//	stream << rect << mScale << m_Index ;
	stream << rect.left() << rect.top() << rect.right() << rect.bottom() << mScale << m_Index ;

	QMimeData *mimeData = new QMimeData ;
	mimeData->setData("editor/selected-image", itemData);

	QPainter painter ;
	painter.begin(&pix) ;
	painter.fillRect(pix.rect(), QColor(127, 127, 127, 127));
	painter.end() ;

	QDrag *drag = new QDrag(this) ;
	drag->setMimeData(mimeData);
	drag->setPixmap(pix);
	drag->setHotSpot(QPoint((rect.right()-rect.left())/2, (rect.bottom()-rect.top())/2));

//	qDebug() << "x:" << ev->pos().x() << " y:" << ev->pos().y() ;

	m_pEditData->setDraggingImage(true);
	drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) ;
	m_pEditData->setDraggingImage(false);
}

void CGridLabel::selectAll( void )
{
	CRectF r ;
	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(m_Index) ;
	if ( !p ) { return ; }

	int img_w = p->Image.width() ;
	int img_h = p->Image.height() ;

	r.setLeft(0);
	r.setRight(img_w);
	r.setTop(0);
	r.setBottom(img_h);
	m_pEditData->setCatchRect(r);
	update() ;
}

void CGridLabel::deselect( void )
{
	CRectF r ;
	r.setLeft(-2);
	r.setRight(-2);
	r.setTop(-1);
	r.setBottom(-1);
	m_pEditData->setCatchRect(r);
	update() ;
}


