#include "glwidget.h"

AnimeGLWidget::AnimeGLWidget(CEditData *editData, CSettings *pSetting, QWidget *parent) :
    QGLWidget(parent)
{
	m_pEditData = editData ;
	m_pSetting = pSetting ;
	m_DrawWidth = m_DrawHeight = 0 ;
	m_bDrawGrid = true ;
	m_bPressCtrl = false ;
	m_bPressShift = false ;
	setGridSpace( 16, 16 );

	setAcceptDrops(true) ;
	setFocusPolicy(Qt::StrongFocus);

	m_pActDel = new QAction(this) ;
	m_pActDel->setText(trUtf8("Delete"));
	connect(m_pActDel, SIGNAL(triggered()), this, SLOT(slot_actDel())) ;

	m_editMode = kEditMode_Pos ;
	m_dragMode = kDragMode_None ;
}

GLuint AnimeGLWidget::bindTexture(QImage &image)
{
	return QGLWidget::bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::InvertedYBindOption) ;
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
	glEnable(GL_BLEND) ;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

	glEnable(GL_DEPTH_TEST);

	for ( int i = 0 ; i < m_pEditData->getImageDataSize() ; i ++ ) {
		if ( m_pEditData->getTexObj(i) ) { continue ; }
		GLuint obj = bindTexture(m_pEditData->getImage(i)) ;
		m_pEditData->setTexObj(i, obj) ;
	}
}

void AnimeGLWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h) ;

}

void AnimeGLWidget::paintGL()
{
	QColor col = m_pSetting->getAnimeBGColor() ;
	glClearColor(col.redF(), col.greenF(), col.blueF(), col.alphaF()) ;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) ;

	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	glOrtho(-m_DrawWidth/2, m_DrawWidth/2, m_DrawHeight/2, -m_DrawHeight/2, -1000, 1000);

	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;

	if ( m_pEditData ) {
		drawLayers() ;
	}

	if ( m_bDrawGrid ) {
		drawGrid() ;
	}
}

void AnimeGLWidget::drawLayers( void )
{
	if ( !m_pEditData->getSelectObject() ) { return ; }

	glEnable(GL_TEXTURE_2D) ;

	if ( m_pEditData->isPlayAnime() || m_pEditData->isPauseAnime() ) {
		drawLayers_Anime() ;
	}
	else {
		drawLayers_Normal() ;
	}

	if ( !m_pEditData->isPlayAnime() || m_pEditData->isPauseAnime() ) {
		drawSelFrameInfo();
	}

	glDisable(GL_TEXTURE_2D) ;
	glBindTexture(GL_TEXTURE_2D, 0) ;
}

void AnimeGLWidget::drawLayers_Normal()
{
	int i, j ;
	CObjectModel			*pModel		= m_pEditData->getObjectModel() ;
	CObjectModel::typeID	objID		= m_pEditData->getSelectObject() ;
	int						selFrame	= m_pEditData->getSelectFrame() ;

	if ( pModel->getLayerGroupListFromID(objID) == NULL ) { return ; }

	QList<CObjectModel::FrameData *> sort ;
	CObjectModel::LayerGroupList &layerGroupList = *pModel->getLayerGroupListFromID(objID) ;
	for ( i = 0 ; i < layerGroupList.size() ; i ++ ) {
		QStandardItem *pLayerItem = layerGroupList[i].first ;
		if ( !pLayerItem->data(Qt::CheckStateRole).toBool() ) { continue ; }	// 非表示
#if 1
		CObjectModel::FrameDataList &frameDataList = layerGroupList[i].second ;
		for ( j = 0 ; j < frameDataList.size() ; j ++ ) {
			CObjectModel::FrameData *data = &frameDataList[j] ;
			if ( data->frame != selFrame ) { continue ; }
			sort.append(data);
		}
#else
		for ( j = 0 ; j < m_pEditData->getSelectLayerNum() ; j ++ ) {
			if ( pLayerItem != m_pEditData->getSelectLayer(j) ) { continue ; }
			const CObjectModel::FrameData *data = m_pEditData->getSelectFrameData(j) ;
			if ( !data ) { continue ; }
			if ( data->frame != selFrame ) { continue ; }
			drawFrameData(*data) ;
			break ;
		}
		if ( j != m_pEditData->getSelectLayerNum() ) { continue ; }

		const CObjectModel::FrameDataList &frameDataList = layerGroupList[i].second ;
		for ( j = 0 ; j < frameDataList.size() ; j ++ ) {
			const CObjectModel::FrameData &data = frameDataList.at(j) ;
			if ( data.frame != selFrame ) { continue ; }

			drawFrameData(data);
		}
#endif
	}
#if 1
	for ( int i = 0 ; i < sort.size() ; i ++ ) {
		for ( int j = i + 1 ; j < sort.size() ; j ++ ) {
			if ( sort[i]->pos_z > sort[j]->pos_z ) {
				sort.swap(i, j);
			}
		}
	}
	for ( int i = 0 ; i < sort.size() ; i ++ ) {
		drawFrameData(*sort[i]);
	}
#endif
	// 前フレームのレイヤ
	for ( i = 0 ; i < layerGroupList.size() ; i ++ ) {
		QStandardItem *pLayerItem = layerGroupList[i].first ;
		if ( !pLayerItem->data(Qt::CheckStateRole).toBool() ) { continue ; }	// 非表示

		CObjectModel::FrameData *pData = pModel->getFrameDataFromPrevFrame(objID, layerGroupList[i].first, selFrame, !(pModel->getFrameDataFromIDAndFrame(objID, layerGroupList[i].first, selFrame))) ;
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
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	int frame = m_pEditData->getSelectFrame() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
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

// 選択中フレーム
void AnimeGLWidget::drawSelFrameInfo( void )
{
	glDisable(GL_TEXTURE_2D) ;
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);

	// 枠
	QColor col ;
	if ( m_bPressCtrl ) {
		col = QColor(0, 255, 0, 255) ;
	}
	else {
		col = QColor(255, 0, 0, 255) ;
	}

	for ( int i = 0 ; i < m_pEditData->getSelectFrameDataNum() ; i ++ ) {
		const CObjectModel::FrameData *pData = m_pEditData->getSelectFrameData(i) ;
		if ( !pData ) { continue ; }

		Vertex v = pData->getVertex() ;

		glPushMatrix();
			glTranslatef(pData->pos_x, pData->pos_y, pData->pos_z / 4096.0f);
			glRotatef(pData->rot_x, 1, 0, 0);
			glRotatef(pData->rot_y, 0, 1, 0);
			glRotatef(pData->rot_z, 0, 0, 1);

			drawLine(QPoint(v.x0, v.y0), QPoint(v.x0, v.y1), col, 0);
			drawLine(QPoint(v.x1, v.y0), QPoint(v.x1, v.y1), col, 0);
			drawLine(QPoint(v.x0, v.y0), QPoint(v.x1, v.y0), col, 0);
			drawLine(QPoint(v.x0, v.y1), QPoint(v.x1, v.y1), col, 0);
		glPopMatrix();

		if ( (m_dragMode != kDragMode_Edit) || m_bPressCtrl ) {
			continue ;
		}

		switch ( m_editMode ) {
		case kEditMode_Rot:
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(2, 0x3333);
			{
				QPoint c = QPoint(pData->pos_x, pData->pos_y) ;
				QPoint p0 = m_DragOffset - QPoint(512, 512) ;

				float len = QVector2D(p0-c).length() ;
				drawLine(c, p0, col, 0);
				glBegin(GL_LINES);
				for ( int i = 0 ; i < 40 ; i ++ ) {
					float rad = i * M_PI*2.0f / 40.0f ;
					float x = cosf(rad) * len ;
					float y = sinf(rad) * len ;
					glVertex2f(x+pData->pos_x, y+pData->pos_y);
				}
				glEnd() ;
			}

			glDisable(GL_LINE_STIPPLE);
			break ;
		}
	}

	// レイヤ複数選択中のマウス枠
	if ( m_dragMode == kDragMode_SelPlural ) {
		drawLine(QPoint(m_SelPluralStartPos.x(), m_SelPluralStartPos.y()),
				 QPoint(m_SelPluralStartPos.x(), m_SelPluralEndPos.y()), col, 0);
		drawLine(QPoint(m_SelPluralStartPos.x(), m_SelPluralStartPos.y()),
				 QPoint(m_SelPluralEndPos.x(), m_SelPluralStartPos.y()), col, 0);
		drawLine(QPoint(m_SelPluralEndPos.x(), m_SelPluralEndPos.y()),
				 QPoint(m_SelPluralEndPos.x(), m_SelPluralStartPos.y()), col, 0);
		drawLine(QPoint(m_SelPluralEndPos.x(), m_SelPluralEndPos.y()),
				 QPoint(m_SelPluralStartPos.x(), m_SelPluralEndPos.y()), col, 0);
	}


	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D) ;
}

// フレームデータ描画
void AnimeGLWidget::drawFrameData( const CObjectModel::FrameData &data, QColor col )
{
	QImage &Image = m_pEditData->getImage(data.nImage) ;
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

	glBindTexture(GL_TEXTURE_2D, m_pEditData->getTexObj(data.nImage)) ;

	col.setRed( col.red()		* data.rgba[0] / 255 );
	col.setGreen( col.green()	* data.rgba[1] / 255 );
	col.setBlue( col.blue()		* data.rgba[2] / 255 );
	col.setAlpha( col.alpha()	* data.rgba[3] / 255 );

	drawRect(rect, uvF, data.pos_z / 4096.0f, col) ;

	glPopMatrix();
}

// グリッド描画
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

// ライン描画
void AnimeGLWidget::drawLine( QPoint pos0, QPoint pos1, QColor col, float z )
{
	glColor4ub(col.red(), col.green(), col.blue(), col.alpha());

	glBegin(GL_LINES) ;
	glVertex3f(pos0.x(), pos0.y(), z) ;
	glVertex3f(pos1.x(), pos1.y(), z) ;
	glEnd() ;
}

// 矩形描画
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

// ドラッグ進入イベント
void AnimeGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if ( m_pEditData->getObjectModel()->getObjListSize() <= 0 ) {
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

// ドロップイベント
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
	m_dragMode = kDragMode_None ;
	if ( m_pEditData->isPlayAnime() ) { return ; }

	if ( event->button() == Qt::LeftButton ) {	// 左ボタン
		QPoint localPos = event->pos() - QPoint(512, 512) ;
		m_DragOffset = event->pos() ;

		CObjectModel *pModel = m_pEditData->getObjectModel() ;
		CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
		int frame = m_pEditData->getSelectFrame() ;

		CObjectModel::typeID layerID = pModel->getLayerIDFromFrameAndPos(objID, frame, localPos) ;

		if ( !layerID ) {
			// 前フレームを調べる
			CObjectModel::LayerGroupList *pLGList = pModel->getLayerGroupListFromID(objID) ;
			if ( pLGList ) {
				for ( int i = 0 ; i < pLGList->size() ; i ++ ) {
					// 既に現在のフレームにデータがあったら調べない
					if ( pModel->getFrameDataFromIDAndFrame(objID, pLGList->at(i).first, frame) ) { continue ; }

					CObjectModel::FrameData *data = pModel->getFrameDataFromPrevFrame(objID, pLGList->at(i).first, frame, true) ;
					if ( !data ) { continue ; }
					if ( !pModel->isFrameDataInPos(*data, localPos) ) { continue ; }

					layerID = pLGList->at(i).first ;

					emit sig_selectPrevLayer(objID, layerID, frame, *data) ;
					break ;
				}
			}
		}

		if ( layerID ) {
			CObjectModel::FrameData *p = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
			if ( p ) {
				m_rotStart = (float)p->rot_z*M_PI/180.0f ;
			}
			if ( m_pEditData->getSelectFrameDataNum() <= 1 || !m_pEditData->isSelectedLayer(layerID) ) {
				QList<CObjectModel::typeID> layers ;
				layers << layerID ;
				emit sig_selectLayerChanged(layers);
			}

			m_dragMode = kDragMode_Edit ;
		}
#if 0
		else {
			m_dragMode = kDragMode_SelPlural ;
			m_SelPluralStartPos = m_SelPluralEndPos = event->pos() - QPoint(512, 512) ;
		}
#endif
	}
}

// マウス移動中イベント
void AnimeGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	int frame = m_pEditData->getSelectFrame() ;

	switch ( m_dragMode ) {
	case kDragMode_Edit:
		if ( !m_pEditData->getSelectLayerNum() ) {
			return ;
		}

		{
			CObjectModel::FrameData *pData = NULL ;
			QPoint old = m_DragOffset ;
			for ( int i = 0 ; i < m_pEditData->getSelectFrameDataNum() ; i ++ ) {

				pData = m_pEditData->getSelectFrameData(i) ;
				QPoint ret = editData(pData, event->pos(), old) ;
				if ( !i && pData ) {
					m_DragOffset = ret ;
					emit sig_dragedImage(*pData) ;
				}
			}
		}

		update() ;
		break ;
	case kDragMode_SelPlural:
		{
			m_SelPluralEndPos = event->pos() - QPoint(512, 512) ;
			QRect rc ;
			rc.setLeft(m_SelPluralStartPos.x()<m_SelPluralEndPos.x()?m_SelPluralStartPos.x():m_SelPluralEndPos.x());
			rc.setRight(m_SelPluralStartPos.x()<m_SelPluralEndPos.x()?m_SelPluralEndPos.x():m_SelPluralStartPos.x());
			rc.setTop(m_SelPluralStartPos.y()<m_SelPluralEndPos.y()?m_SelPluralStartPos.y():m_SelPluralEndPos.y());
			rc.setBottom(m_SelPluralStartPos.y()<m_SelPluralEndPos.y()?m_SelPluralEndPos.y():m_SelPluralStartPos.y());
			QList<CObjectModel::typeID> selLayers = pModel->getFrameDatasFromRect(objID, frame, rc) ;
			m_pEditData->setSelectLayer(selLayers);
			update() ;
		}
		break ;
	}

}

// マウスボタン離し直後イベント
void AnimeGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event) ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	int frame = m_pEditData->getSelectFrame() ;
	QList<CObjectModel::typeID> selLayers ;

	switch ( m_dragMode ) {
	case kDragMode_Edit:
		emit sig_frameDataMoveEnd() ;
		break ;
	case kDragMode_SelPlural:
		{
			m_SelPluralEndPos = event->pos() - QPoint(512, 512) ;
			QRect rc ;
			rc.setLeft(m_SelPluralStartPos.x()<m_SelPluralEndPos.x()?m_SelPluralStartPos.x():m_SelPluralEndPos.x());
			rc.setRight(m_SelPluralStartPos.x()<m_SelPluralEndPos.x()?m_SelPluralEndPos.x():m_SelPluralStartPos.x());
			rc.setTop(m_SelPluralStartPos.y()<m_SelPluralEndPos.y()?m_SelPluralStartPos.y():m_SelPluralEndPos.y());
			rc.setBottom(m_SelPluralStartPos.y()<m_SelPluralEndPos.y()?m_SelPluralEndPos.y():m_SelPluralStartPos.y());
			selLayers = pModel->getFrameDatasFromRect(objID, frame, rc) ;
			emit sig_selectLayerChanged(selLayers) ;
		}
		break ;
	}
	m_dragMode = kDragMode_None ;
	update() ;
}

// 右クリックメニューイベント
void AnimeGLWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if ( m_pEditData->getSelectFrameDataNum() == 0 ) {
		return ;
	}
	qDebug() << "select layer:" << m_pEditData->getSelectLayer() ;

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

/**
  データ編集
  */
QPoint AnimeGLWidget::editData(CObjectModel::FrameData *pData, QPoint nowPos, QPoint oldPos)
{
	if ( !pData ) {
		return QPoint(0, 0) ;
	}

	QPoint sub = nowPos - oldPos ;
	QSize imageSize = m_pEditData->getImage(pData->nImage).size() ;
	QPoint ret = oldPos ;
	if ( m_bPressCtrl ) {	// UV操作
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
		ret = nowPos ;
	}
	else {
		switch ( m_editMode ) {
			case kEditMode_Pos:
				pData->pos_x += sub.x() ;
				pData->pos_y += sub.y() ;
				break ;
			case kEditMode_Rot:
				{
					QVector2D vOld = QVector2D(oldPos-QPoint(512, 512) - QPoint(pData->pos_x, pData->pos_y)) ;
					QVector2D vNow = QVector2D(nowPos-QPoint(512, 512) - QPoint(pData->pos_x, pData->pos_y)) ;
					vOld.normalize();
					vNow.normalize();
					float old = atan2(vOld.y(), vOld.x()) ;
					float now = atan2(vNow.y(), vNow.x()) ;
					now -= old ;
					if ( now >= M_PI*2 ) { now -= M_PI*2 ; }
					if ( now < -M_PI*2 ) { now += M_PI*2 ; }
					now += m_rotStart ;
					if ( now >= M_PI*2 ) { now -= M_PI*2 ; }
					if ( now < -M_PI*2 ) { now += M_PI*2 ; }
					pData->rot_z = now*180.0f/M_PI ;
				}
				break ;
			case kEditMode_Center:
				pData->center_x += sub.x() ;
				pData->center_y += sub.y() ;
				break ;
			case kEditMode_Scale:
				pData->fScaleX += (float)sub.x() * 0.01f ;
				pData->fScaleY += (float)sub.y() * 0.01f ;
				break ;
		}
		if ( m_editMode != kEditMode_Rot ) {
			ret = nowPos ;
		}
	}
	return ret ;
}

// 描画エリア設定
void AnimeGLWidget::setDrawArea(int w, int h)
{
	m_DrawWidth = w ;
	m_DrawHeight = h ;
}





