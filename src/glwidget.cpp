#include "glwidget.h"

AnimeGLWidget::AnimeGLWidget(CEditImageData *editData, QWidget *parent) :
    QGLWidget(parent)
{
	m_pEditImageData = editData ;
	m_DrawWidth = m_DrawHeight = 0 ;
	m_bDrawGrid = true ;
	m_bDragging = false ;
	m_bChangeUV = false ;
	m_bPressCtrl = 0 ;
	setGridSpace( 16, 16 );

	setAcceptDrops(true) ;
	setFocusPolicy(Qt::StrongFocus);

	m_pActDel = new QAction(this) ;
	m_pActDel->setText(trUtf8("Delete"));
	connect(m_pActDel, SIGNAL(triggered()), this, SLOT(slot_actDel())) ;

}

void AnimeGLWidget::slot_actDel( void )
{
	emit sig_deleteFrameData() ;
}

void AnimeGLWidget::slot_setDrawGrid(bool flag)
{
	m_bDrawGrid = flag ;
	update() ;
}

void AnimeGLWidget::initializeGL()
{
	glClearColor(0, 0, 0, 0) ;

	glEnable(GL_BLEND) ;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.5);

	glEnable(GL_DEPTH_TEST);

	for ( int i = 0 ; i < m_pEditImageData->getImageDataSize() ; i ++ ) {
		if ( m_pEditImageData->getTexObj(i) ) { continue ; }
		GLuint obj = bindTexture(m_pEditImageData->getImage(i)) ;
		m_pEditImageData->setTexObj(i, obj) ;
	}
}

void AnimeGLWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h) ;

}

void AnimeGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) ;

	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	glOrtho(-m_DrawWidth/2, m_DrawWidth/2, m_DrawHeight/2, -m_DrawHeight/2, -1000, 1000);

	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;

	if ( m_pEditImageData ) {
		drawLayers() ;
	}

	if ( m_bDrawGrid ) {
		drawGrid() ;
	}
}

void AnimeGLWidget::drawLayers( void )
{
	if ( !m_pEditImageData->getSelectObject() ) { return ; }

	glEnable(GL_TEXTURE_2D) ;

	if ( m_pEditImageData->isPlayAnime() ) {
		drawLayers_Anime() ;
	}
	else {
		drawLayers_Normal() ;
	}

	glDisable(GL_TEXTURE_2D) ;
	glBindTexture(GL_TEXTURE_2D, 0) ;
}

void AnimeGLWidget::drawLayers_Normal()
{
	CObjectModel			*pModel		= m_pEditImageData->getObjectModel() ;
	CObjectModel::typeID	objID		= m_pEditImageData->getSelectObject() ;
	CObjectModel::typeID	layerID		= m_pEditImageData->getSelectLayer() ;
	int						selFrame	= m_pEditImageData->getSelectFrame() ;

	if ( pModel->getLayerGroupListFromID(objID) == NULL ) { return ; }

	const CObjectModel::LayerGroupList &layerGroupList = *pModel->getLayerGroupListFromID(objID) ;
	for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
		QStandardItem *pLayerItem = layerGroupList[i].first ;
		if ( !pLayerItem->data(Qt::CheckStateRole).toBool() ) { continue ; }	// 非表示
		const CObjectModel::FrameDataList &frameDataList = layerGroupList[i].second ;
		for ( int j = 0 ; j < frameDataList.size() ; j ++ ) {
			const CObjectModel::FrameData &data = frameDataList.at(j) ;
			if ( data.frame != selFrame ) { continue ; }

			drawFrameData(data);
			if ( layerID == layerGroupList[i].first ) {	// 選択中のレイヤ
				QColor col ;
				if ( m_bPressCtrl ) {
					col = QColor(0, 255, 0, 255) ;
				}
				else {
					col = QColor(255, 0, 0, 255) ;
				}
				Vertex v = data.getVertex() ;

				glPushMatrix();
				glTranslatef(data.pos_x, data.pos_y, data.pos_z / 4096.0f);

				glDisable(GL_TEXTURE_2D) ;
				drawLine(QPoint(v.x0, v.y0), QPoint(v.x0, v.y1), col);
				drawLine(QPoint(v.x1, v.y0), QPoint(v.x1, v.y1), col);
				drawLine(QPoint(v.x0, v.y0), QPoint(v.x1, v.y0), col);
				drawLine(QPoint(v.x0, v.y1), QPoint(v.x1, v.y1), col);
				glEnable(GL_TEXTURE_2D) ;

				glPopMatrix();
			}
		}
	}

	// 前フレームのレイヤ
	for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
		QStandardItem *pLayerItem = layerGroupList[i].first ;
		if ( !pLayerItem->data(Qt::CheckStateRole).toBool() ) { continue ; }	// 非表示

		CObjectModel::FrameData *pData = pModel->getFrameDataFromPrevFrame(objID, layerGroupList[i].first, selFrame) ;
		if ( pData ) {
			CObjectModel::FrameData data = *pData ;
			data.pos_z -= 2048.0f ;
			drawFrameData(data, QColor(255, 255, 255, 128));
		}
	}
}

// アニメ再生中
void AnimeGLWidget::drawLayers_Anime()
{
	CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
	int frame = m_pEditImageData->getSelectFrame() ;
	CObjectModel::typeID objID = m_pEditImageData->getSelectObject() ;
	if ( !pModel->getLayerGroupListFromID(objID) ) { return ; }

	const CObjectModel::LayerGroupList &layerGroupList = *pModel->getLayerGroupListFromID(objID) ;
	for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
		CObjectModel::typeID layerID = layerGroupList[i].first ;
		const CObjectModel::FrameData *pNow = pModel->getFrameDataFromPrevFrame(objID, layerID, frame+1) ;
		const CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;

		if ( !pNow ) { continue ; }
		if ( !pNext ) {	// 次フレームのデータがない
			drawFrameData(*pNow);
		}
		else {
			const CObjectModel::FrameData data = pNow->getInterpolation(pNext, frame) ;
			drawFrameData(data);
		}
	}
}

void AnimeGLWidget::drawFrameData( const CObjectModel::FrameData &data, QColor col )
{
	QImage &Image = m_pEditImageData->getImage(data.nImage) ;
	QRectF rect ;
	QRect uv = data.getRect() ;
	QRectF uvF ;

	glPushMatrix() ;
	glTranslatef(data.pos_x, data.pos_y, data.pos_z / 4096.0f);
	glRotatef(data.rot_x, 1, 0, 0);
	glRotatef(data.rot_y, 0, 1, 0);
	glRotatef(data.rot_z, 0, 0, 1);

	Vertex v = data.getVertex() ;
	rect.setLeft(v.x0);
	rect.setRight(v.x1) ;
	rect.setTop(v.y0);
	rect.setBottom(v.y1);

	uvF.setLeft((float)uv.left()/Image.width());
	uvF.setRight((float)uv.right()/Image.width());
	uvF.setTop((float)(Image.height()-uv.top())/Image.height());
	uvF.setBottom((float)(Image.height()-uv.bottom())/Image.height());

	glBindTexture(GL_TEXTURE_2D, m_pEditImageData->getTexObj(data.nImage)) ;

	drawRect(rect, uvF, data.pos_z / 4096.0f, col) ;

	glPopMatrix();
}

void AnimeGLWidget::drawGrid( void )
{
	glPushMatrix();
	glLoadIdentity();

	QColor colHalfWhite = QColor(255, 255, 255, 128) ;
	for ( int x = -m_DrawWidth/2 ; x < m_DrawWidth/2 ; x += m_GridWidth ) {
		drawLine(QPoint(x, -m_DrawHeight/2), QPoint(x, m_DrawHeight/2), colHalfWhite, 0.999f) ;
	}
	for ( int y = -m_DrawHeight/2 ; y < m_DrawHeight/2 ; y += m_GridHeight ) {
		drawLine(QPoint(-m_DrawWidth/2, y), QPoint(m_DrawWidth/2, y), colHalfWhite, 0.999f) ;
	}

	QColor colHalfYellow = QColor(255, 255, 0, 128) ;
	drawLine(QPoint(0, -m_DrawHeight/2), QPoint(0, m_DrawHeight/2), colHalfYellow) ;
	drawLine(QPoint(-m_DrawWidth/2, 0), QPoint(m_DrawWidth/2, 0), colHalfYellow) ;

	glPopMatrix();
}

void AnimeGLWidget::drawLine( QPoint pos0, QPoint pos1, QColor col, float z )
{
	glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

	glBegin(GL_LINES) ;
	glVertex3f(pos0.x(), pos0.y(), z) ;
	glVertex3f(pos1.x(), pos1.y(), z) ;
	glEnd() ;
}

void AnimeGLWidget::drawRect(QRectF rc, QRectF uv, float z, QColor col)
{
	glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

	glBegin(GL_TRIANGLE_STRIP) ;
		glTexCoord2f(uv.left(), uv.bottom());
		glVertex3f(rc.left(), rc.bottom(), z) ;
		glTexCoord2f(uv.right(), uv.bottom());
		glVertex3f(rc.right(), rc.bottom(), z) ;
		glTexCoord2f(uv.left(), uv.top());
		glVertex3f(rc.left(), rc.top(), z) ;
		glTexCoord2f(uv.right(), uv.top());
		glVertex3f(rc.right(), rc.top(), z) ;
	glEnd() ;
}

void AnimeGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if ( m_pEditImageData->getObjectModel()->getObjListSize() <= 0 ) {
		event->ignore();
		return ;
	}

	if ( event->mimeData()->hasFormat("editor/selected-image") ) {
		event->accept();
	}
	else {
		event->ignore();
	}
}

void AnimeGLWidget::dropEvent(QDropEvent *event)
{
	if ( event->mimeData()->hasFormat("editor/selected-image") ) {
		QPixmap pix ;
		QRect rect ;
		int scale ;
		QPoint pos ;
		int index ;

		QByteArray itemData = event->mimeData()->data("editor/selected-image");
		QDataStream stream( &itemData, QIODevice::ReadOnly ) ;
		stream >> rect >> scale >> index ;

		pos = event->pos() ;

		event->accept();

		emit(sig_dropedImage(rect, pos, index)) ;
	}
	else {
		event->ignore();
	}
}

// マウス押し直後イベント
void AnimeGLWidget::mousePressEvent(QMouseEvent *event)
{
	m_bDragging = false ;
	m_bChangeUV = false ;
	if ( m_pEditImageData->isPlayAnime() ) { return ; }

	if ( event->button() == Qt::LeftButton ) {	// 左ボタン
		QPoint localPos = event->pos() - QPoint(512, 512) ;
		CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
		CObjectModel::typeID objID = m_pEditImageData->getSelectObject() ;
		int frame = m_pEditImageData->getSelectFrame() ;
		CObjectModel::typeID layerID = pModel->getLayerIDFromFrameAndPos(objID, frame, localPos) ;
		if ( layerID ) {
			m_bDragging = true ;
			m_DragOffset = event->pos() ;
		}
		else {
			// 前フレームを調べる
			CObjectModel::LayerGroupList *pLGList = pModel->getLayerGroupListFromID(objID) ;
			if ( pLGList ) {
				for ( int i = 0 ; i < pLGList->size() ; i ++ ) {
					// 現在のフレームにデータがあったら調べない
					if ( pModel->getFrameDataFromIDAndFrame(objID, pLGList->at(i).first, frame) ) { continue ; }

					CObjectModel::FrameData *data = pModel->getFrameDataFromPrevFrame(objID, pLGList->at(i).first, frame) ;
					if ( !data ) { continue ; }
					if ( !pModel->isFrameDataInPos(*data, localPos) ) { continue ; }

					layerID = pLGList->at(i).first ;
					emit sig_selectPrevLayer(objID, layerID, frame, *data) ;
					break ;
				}
			}
		}
		emit sig_selectLayerChanged(layerID) ;
	}
}

// マウス移動中イベント
void AnimeGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if ( m_bDragging ) {	// ドラッグ中
		CObjectModel::typeID layerID = m_pEditImageData->getSelectLayer() ;
		if ( !layerID ) { return ; }

		CObjectModel *pModel = m_pEditImageData->getObjectModel() ;
		CObjectModel::typeID objID = m_pEditImageData->getSelectObject() ;
		int frame = m_pEditImageData->getSelectFrame() ;
		CObjectModel::FrameData *pData = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
		if ( !pData ) { return ; }

		QPoint sub = event->pos() - m_DragOffset ;
		if ( m_bPressCtrl ) {	// UV操作
			QSize imageSize = m_pEditImageData->getImage(pData->nImage).size() ;
			pData->left		+= sub.x() ;
			pData->right	+= sub.x() ;
			pData->top		+= sub.y() ;
			pData->bottom	+= sub.y() ;
			if ( pData->left < 0 ) {
				pData->right -= pData->left ;
				pData->left = 0 ;
			}
			if ( pData->right > imageSize.width()-1 ) {
				pData->left -= pData->right-(imageSize.width()-1) ;
				pData->right = imageSize.width()-1 ;
			}
			if ( pData->top < 0 ) {
				pData->bottom -= pData->top ;
				pData->top = 0 ;
			}
			if ( pData->bottom > imageSize.height()-1 ) {
				pData->top -= pData->bottom-(imageSize.height()-1) ;
				pData->bottom = imageSize.height()-1 ;
			}
		}
		else {
			pData->pos_x += sub.x() ;
			pData->pos_y += sub.y() ;
		}
		m_DragOffset = event->pos() ;
		update() ;

		emit sig_dragedImage(*pData) ;
	}
}

// マウスボタン離し直後イベント
void AnimeGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event) ;

	m_bDragging = false ;
}

// 右クリックメニューイベント
void AnimeGLWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if ( m_pEditImageData->getSelectLayer() == 0 ) {
		return ;
	}
	qDebug() << "select layer:" << m_pEditImageData->getSelectLayer() ;

	QMenu menu(this) ;
	menu.addAction(m_pActDel) ;
	menu.exec(event->globalPos()) ;
}

// キー押しイベント
void AnimeGLWidget::keyPressEvent(QKeyEvent *event)
{
	if ( event->key() == Qt::Key_Control ) {
		m_bPressCtrl = true ;
		update() ;
	}
}

// キー離しイベント
void AnimeGLWidget::keyReleaseEvent(QKeyEvent *event)
{
	if ( event->key() == Qt::Key_Control ) {
		m_bPressCtrl = false ;
		update() ;
	}
}

// 描画エリア設定
void AnimeGLWidget::setDrawArea(int w, int h)
{
	m_DrawWidth = w ;
	m_DrawHeight = h ;
}





