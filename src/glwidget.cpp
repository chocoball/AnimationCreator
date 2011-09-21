#include "glwidget.h"
#include "util.h"

AnimeGLWidget::AnimeGLWidget(CEditData *editData, CSettings *pSetting, QWidget *parent) :
	QGLWidget(QGLFormat(QGL::AlphaChannel), parent)
{
	m_pEditData = editData ;
	m_pSetting = pSetting ;
	m_DrawWidth = m_DrawHeight = 0 ;
	m_bDrawGrid = true ;
	m_bPressCtrl = false ;
	setGridSpace( 16, 16 );

	setAcceptDrops(true) ;
//	setFocusPolicy(Qt::StrongFocus);

	m_pActDel = new QAction(this) ;
	m_pActDel->setText(trUtf8("Delete"));
	connect(m_pActDel, SIGNAL(triggered()), this, SLOT(slot_actDel())) ;

	m_editMode = kEditMode_Pos ;
	m_dragMode = kDragMode_None ;

	m_backImageTex = 0 ;
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

	for ( int i = 0 ; i < m_pEditData->getImageDataListSize() ; i ++ ) {
		CEditData::ImageData *p = m_pEditData->getImageData(i) ;
		if ( !p ) { continue ; }
		if ( p->nTexObj ) { continue ; }
		p->nTexObj = bindTexture(p->Image) ;
	}
}

void AnimeGLWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h) ;
}

void AnimeGLWidget::paintGL()
{
	QColor col = m_pSetting->getAnimeBGColor() ;
	if ( m_pEditData->isExportPNG() ) {
		col.setAlphaF(0);
	}
	glClearColor(col.redF(), col.greenF(), col.blueF(), col.alphaF()) ;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) ;

	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	glOrtho(-m_DrawWidth/2, m_DrawWidth/2, m_DrawHeight/2, -m_DrawHeight/2, -1000, 1000);

	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;

	// 背景画像描画
	if ( m_backImageTex ) {
		QRect rect ;
		QRectF uvF ;
		QColor col = QColor(255, 255, 255, 255) ;

		glEnable(GL_TEXTURE_2D) ;
		glBindTexture(GL_TEXTURE_2D, m_backImageTex) ;

		rect.setRect(-m_backImageW/2, m_backImageH/2, m_backImageW, -m_backImageH);
		uvF.setRect(0.0, (float)(m_BackImage.height()-m_backImageH)/m_BackImage.height(), (float)m_backImageW/m_BackImage.width(), (float)m_backImageH/m_BackImage.height());
		qDebug() << uvF ;
		drawRect(rect, uvF, -1, col) ;

		glBindTexture(GL_TEXTURE_2D, 0) ;
		glDisable(GL_TEXTURE_2D) ;
	}

	if ( m_pEditData ) {
		drawLayers() ;
	}

	if ( m_bDrawGrid ) {
		if ( !m_pEditData->isExportPNG() ) {
			drawGrid() ;
		}
	}

	// センター
	if ( m_pSetting->getDrawCenter() ) {
		if ( !m_pEditData->isExportPNG() ) {
			drawCenter() ;
		}
	}

	// PNG吐き出しモード
	if ( m_pEditData->getEditMode() == CEditData::kEditMode_ExportPNG && !m_pEditData->isExportPNG() ) {
		int rect[4] ;
		QColor col = QColor(255, 0, 0, 255) ;
		m_pEditData->getExportPNGRect(rect);
		glDisable(GL_DEPTH_TEST);
		drawLine(QPoint(rect[0], rect[1]), QPoint(rect[2], rect[1]), col, 1.0) ;
		drawLine(QPoint(rect[2], rect[1]), QPoint(rect[2], rect[3]), col, 1.0) ;
		drawLine(QPoint(rect[2], rect[3]), QPoint(rect[0], rect[3]), col, 1.0) ;
		drawLine(QPoint(rect[0], rect[3]), QPoint(rect[0], rect[1]), col, 1.0) ;
		glEnable(GL_DEPTH_TEST);
	}
}

void AnimeGLWidget::drawLayers( void )
{
	if ( !m_pEditData->getObjectModel()->isObject(m_pEditData->getSelIndex())
		 && !m_pEditData->getObjectModel()->isLayer(m_pEditData->getSelIndex()) ) {
		return ;
	}

	glEnable(GL_TEXTURE_2D) ;

	switch ( m_pEditData->getEditMode() ) {
	case CEditData::kEditMode_Animation:
		drawLayers_Anime() ;
		break ;
	case CEditData::kEditMode_ExportPNG:
		if ( m_pEditData->isExportPNG() ) {
			drawLayers_Anime() ;
			writePNGFromFrameBuffer() ;
		}
		else {
			drawLayers_All() ;
		}
		break ;
	}

	drawSelFrameInfo();

	glDisable(GL_TEXTURE_2D) ;
	glBindTexture(GL_TEXTURE_2D, 0) ;
}

// アニメ再生中
void AnimeGLWidget::drawLayers_Anime()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;

	ObjectItem *pItem = pModel->getObject(m_pEditData->getSelIndex()) ;
	if ( !pItem ) { return ; }

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		drawLayers(pItem->child(i)) ;
	}
}

void AnimeGLWidget::drawLayers(ObjectItem *pLayerItem)
{
	for ( int i = 0 ; i < pLayerItem->childCount() ; i ++ ) {
		drawLayers(pLayerItem->child(i)) ;
	}

	int flag = pLayerItem->data(Qt::CheckStateRole).toInt() ;
	if ( !(flag & ObjectItem::kState_Disp) ) { return ; }

	FrameData d ;
	bool valid ;
	d = pLayerItem->getDisplayFrameData(m_pEditData->getSelectFrame(), &valid) ;
	if ( valid ) {
		drawFrameData(d, pLayerItem->getDisplayMatrix(m_pEditData->getSelectFrame())) ;

		if ( m_pSetting->getDrawFrame() && !m_pEditData->isExportPNG() ) {
			QColor col ;
			ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(m_pEditData->getSelIndex()) ;
			if ( pLayerItem == p )	{ col = QColor(255, 0, 0, 255) ; }
			else					{ col = QColor(64, 64, 64, 255) ; }
			drawFrame(pLayerItem, m_pEditData->getSelectFrame(), col) ;
		}
	}
}

// 全フレーム描画
void AnimeGLWidget::drawLayers_All( void )
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;

	ObjectItem *pItem = pModel->getObject(m_pEditData->getSelIndex()) ;
	if ( !pItem ) { return ; }

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		drawLayer_All(pItem->child(i), -1) ;
	}
}

void AnimeGLWidget::drawLayer_All(ObjectItem *pLayerItem, int frame)
{
	if ( frame >= 0 ) {
		if ( !pLayerItem->getFrameDataPtr(frame) ) {
			bool valid ;
			FrameData data = pLayerItem->getDisplayFrameData(frame, &valid) ;
			if ( valid ) {
				drawFrameData(data, pLayerItem->getDisplayMatrix(frame)) ;
			}
		}
		for ( int i = 0 ; i < pLayerItem->childCount() ; i ++ ) {
			drawLayer_All(pLayerItem->child(i), frame) ;
		}
	}
	else {
		for ( int i = 0 ; i < pLayerItem->childCount() ; i ++ ) {
			drawLayer_All(pLayerItem->child(i), -1) ;
		}

		const QList<FrameData> &datas = pLayerItem->getFrameData() ;
		for ( int i = 0 ; i < datas.size() ; i ++ ) {
			const FrameData &data = datas.at(i) ;
			drawFrameData(data, pLayerItem->getDisplayMatrix(data.frame)) ;

			for ( int i = 0 ; i < pLayerItem->childCount() ; i ++ ) {
				drawLayer_All(pLayerItem->child(i), data.frame) ;
			}
		}
	}
}

// 選択中フレーム
void AnimeGLWidget::drawSelFrameInfo( void )
{
	QModelIndex index = m_pEditData->getSelIndex() ;
	if ( !m_pEditData->getObjectModel()->isLayer(index) ) { return ; }

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

	ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	FrameData data = pItem->getDisplayFrameData(m_pEditData->getSelectFrame()) ;
	if ( data.frame == m_pEditData->getSelectFrame() ) {
		switch ( m_editMode ) {
			case kEditMode_Rot:
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(2, 0x3333);
				{
					QMatrix4x4 mat = pItem->getDisplayMatrix(m_pEditData->getSelectFrame()) ;

					QPoint c = QPoint(mat.column(3).x(), mat.column(3).y()) ;
					QPoint p0 = m_DragOffset - QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) ;

					float len = QVector2D(p0-c).length() ;
					drawLine(c, p0, col, 0);
					glBegin(GL_LINES);
					for ( int i = 0 ; i < 40 ; i ++ ) {
						float rad = i * M_PI*2.0f / 40.0f ;
						float x = cosf(rad) * len ;
						float y = sinf(rad) * len ;
						glVertex2f(x+c.x(), y+c.y());
					}
					glEnd() ;
				}
				glDisable(GL_LINE_STIPPLE);
				break ;
		}
	}

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D) ;
}

// フレームデータ描画
void AnimeGLWidget::drawFrameData( const FrameData &data, const QMatrix4x4 &mat, QColor col )
{
	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(data.nImage) ;
	if ( !p ) { return ; }

	QImage &Image = p->Image ;
	QRectF rect ;
	QRect uv = data.getRect() ;
	QRectF uvF ;

	glPushMatrix() ;
	{
		multMatrix(mat) ;

		Vertex v = data.getVertex() ;
		rect.setLeft(v.x0);
		rect.setRight(v.x1) ;
		rect.setTop(v.y0);
		rect.setBottom(v.y1);

		uvF.setLeft((float)uv.left()/Image.width());
		uvF.setRight((float)uv.right()/Image.width());
		uvF.setTop((float)(Image.height()-uv.top())/Image.height());
		uvF.setBottom((float)(Image.height()-uv.bottom())/Image.height());

		glBindTexture(GL_TEXTURE_2D, p->nTexObj) ;

		col.setRed( col.red()		* data.rgba[0] / 255 );
		col.setGreen( col.green()	* data.rgba[1] / 255 );
		col.setBlue( col.blue()		* data.rgba[2] / 255 );
		col.setAlpha( col.alpha()	* data.rgba[3] / 255 );

		drawRect(rect, uvF, data.pos_z / 4096.0f, col) ;
	}
	glPopMatrix() ;
}

// フレームデータの枠描画
void AnimeGLWidget::drawFrame( ObjectItem *pItem, int frame, QColor col)
{
	FrameData data = pItem->getDisplayFrameData(frame) ;
	Vertex v = data.getVertex() ;

	bool bDepth = glIsEnabled(GL_DEPTH_TEST) ;
	bool bTex = glIsEnabled(GL_TEXTURE_2D) ;
	glDisable(GL_DEPTH_TEST) ;
	glDisable(GL_TEXTURE_2D) ;

	glPushMatrix();
	{
		multMatrix(pItem->getDisplayMatrix(frame)) ;

		drawLine(QPoint(v.x0, v.y0), QPoint(v.x0, v.y1), col, 0);
		drawLine(QPoint(v.x1, v.y0), QPoint(v.x1, v.y1), col, 0);
		drawLine(QPoint(v.x0, v.y0), QPoint(v.x1, v.y0), col, 0);
		drawLine(QPoint(v.x0, v.y1), QPoint(v.x1, v.y1), col, 0);
	}
	glPopMatrix();

	if ( bDepth ) { glEnable(GL_DEPTH_TEST) ; }
	if ( bTex ) { glEnable(GL_TEXTURE_2D) ; }
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

// 選択フレームデータのセンター表示
void AnimeGLWidget::drawCenter( void )
{
    FrameData data ;
	if ( !m_pEditData->getNowSelectFrameData(data) ) { return ; }

	QMatrix4x4 m = m_pEditData->getNowSelectMatrix() ;
#if 0
	glPushMatrix() ;
	{
		multMatrix(m) ;

		QColor col = QColor(0, 0, 255, 255) ;
		glDisable(GL_DEPTH_TEST);
		drawLine(QPoint(-(CEditData::kGLWidgetSize/2), 0), QPoint((CEditData::kGLWidgetSize/2), 0), col, 1.0) ;
		drawLine(QPoint(0, -(CEditData::kGLWidgetSize/2)), QPoint(0, (CEditData::kGLWidgetSize/2)), col, 1.0) ;
		glEnable(GL_DEPTH_TEST);
	}
	glPopMatrix() ;
#else
	QColor col = QColor(0, 0, 255, 255) ;
	glDisable(GL_DEPTH_TEST);
	drawLine(QPoint(-(CEditData::kGLWidgetSize/2), m.column(3).y()), QPoint((CEditData::kGLWidgetSize/2), m.column(3).y()), col, 1.0) ;
	drawLine(QPoint(m.column(3).x(), -(CEditData::kGLWidgetSize/2)), QPoint(m.column(3).x(), (CEditData::kGLWidgetSize/2)), col, 1.0) ;
	glEnable(GL_DEPTH_TEST);
#endif
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
	if ( !m_pEditData->getSelIndex().isValid() ) {
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
	if ( m_pEditData->getPlayAnime() ) { return ; }

	if ( event->button() == Qt::LeftButton ) {	// 左ボタン
		// 連番PNG吐き出し時
		if ( m_pEditData->getEditMode() == CEditData::kEditMode_ExportPNG ) {
			int rect[4] = { 0, 0, 0, 0 } ;
			rect[0] = rect[2] = event->pos().x()-(CEditData::kGLWidgetSize/2) ;
			rect[1] = rect[3] = event->pos().y()-(CEditData::kGLWidgetSize/2)+1 ;
			m_pEditData->setExportPNGRect(rect);
			emit sig_exportPNGRectChange() ;
			update() ;
			return ;
		}

		QPoint localPos = event->pos() - QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) ;
		m_DragOffset = event->pos() ;

		CObjectModel *pModel = m_pEditData->getObjectModel() ;
		int frame = m_pEditData->getSelectFrame() ;

		QModelIndex index = m_pEditData->getSelIndex() ;
		ObjectItem *pItem = pModel->getObject(index) ;
		if ( !pItem ) {
			// オブジェクトが選択されていない
			qDebug() << "mousePressEvent not sel object" ;
			return ;
		}

		if ( pItem->isContain(&pItem, localPos, frame) ) {
			int flag = pItem->data(Qt::CheckStateRole).toInt() ;
			if ( !(flag & ObjectItem::kState_Disp) || (flag & ObjectItem::kState_Lock) ) {	// 非表示
				qDebug() << "mousePressEvent not display:" << flag ;
				event->ignore();
				return ;
			}

			if ( !pItem->getFrameDataPtr(frame) ) {
				// データがないので追加
				FrameData data = pItem->getDisplayFrameData(frame) ;
				emit sig_selectPrevLayer(pItem->getIndex(), frame, data) ;
			}

			FrameData *p = pItem->getFrameDataPtr(frame) ;
			if ( p ) {
				m_rotStart = (float)p->rot_z * M_PI / 180.0f ;
			}
			emit sig_selectLayerChanged(pItem->getIndex()) ;
			m_dragMode = kDragMode_Edit ;
		}
		else {
			qDebug() << "mousePressEvent not contain" << localPos ;
		}
	}
}

// マウス移動中イベント
void AnimeGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	int frame = m_pEditData->getSelectFrame() ;

	// 連番PNG吐き出し時
	if ( m_pEditData->getEditMode() == CEditData::kEditMode_ExportPNG ) {
		int rect[4] = { 0, 0, 0, 0 } ;
		m_pEditData->getExportPNGRect(rect);
		rect[2] = event->pos().x()-(CEditData::kGLWidgetSize/2) ;
		rect[3] = event->pos().y()-(CEditData::kGLWidgetSize/2)+1 ;
		m_pEditData->setExportPNGRect(rect);
		emit sig_exportPNGRectChange() ;
		update() ;
		return ;
	}

	switch ( m_dragMode ) {
	case kDragMode_Edit:
		{
			QModelIndex index = m_pEditData->getSelIndex() ;
			if ( !pModel->isLayer(index) ) { return ; }
			ObjectItem *pItem = pModel->getItemFromIndex(index) ;
			if ( !pItem ) { return ; }

			FrameData *pData = pItem->getFrameDataPtr(frame) ;
			if ( !pData ) { return ; }
			QPoint ret = editData(pData, event->pos(), m_DragOffset) ;
			m_DragOffset = ret ;
			emit sig_dragedImage(*pData) ;
			update() ;
		}
		break ;
	}
}

// マウスボタン離し直後イベント
void AnimeGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event) ;

	switch ( m_dragMode ) {
	case kDragMode_Edit:
		emit sig_frameDataMoveEnd() ;
		break ;
	}
	m_dragMode = kDragMode_None ;
	update() ;
}

// 右クリックメニューイベント
void AnimeGLWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if ( !m_pEditData->getObjectModel()->isLayer(m_pEditData->getSelIndex()) ) {
		return ;
	}

	QMenu menu(this) ;
	menu.addAction(m_pActDel) ;
	menu.exec(event->globalPos()) ;
}

/**
  データ編集
  */
QPoint AnimeGLWidget::editData(FrameData *pData, QPoint nowPos, QPoint oldPos)
{
	if ( !pData ) {
		return QPoint(0, 0) ;
	}

	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(pData->nImage) ;
	if ( !p ) {
		return QPoint(0, 0) ;
	}

	QPoint sub = nowPos - oldPos ;
	QSize imageSize = p->Image.size() ;
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
					QVector2D vOld = QVector2D(oldPos-QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) - QPoint(pData->pos_x, pData->pos_y)) ;
					QVector2D vNow = QVector2D(nowPos-QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) - QPoint(pData->pos_x, pData->pos_y)) ;
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

// フレームバッファをPNGに吐き出す
void AnimeGLWidget::writePNGFromFrameBuffer( void )
{
	QString dir = m_pEditData->getExportPNGDir() ;
	int frame = m_pEditData->getSelectFrame() ;
	QString name = "" ;
	int allDigit = 4 ;
	int currDigit = getDigit(frame) ;

	for ( int i = 0 ; i < allDigit-currDigit ; i ++ ) {
		name += "0" ;
	}
	name += QVariant(frame).toString() ;

	name = dir + "/" + name + ".png" ;
	int rect[4] ;
	m_pEditData->getExportPNGRect(rect);
	rect[0] += (CEditData::kGLWidgetSize/2) ;
	rect[1] += (CEditData::kGLWidgetSize/2) ;
	rect[2] += (CEditData::kGLWidgetSize/2) ;
	rect[3] += (CEditData::kGLWidgetSize/2) ;
	if ( rect[0] > rect[2] ) {
		int tmp = rect[0] ;
		rect[0] = rect[2] ;
		rect[2] = tmp ;
	}
	if ( rect[1] > rect[3] ) {
		int tmp = rect[1] ;
		rect[1] = rect[3] ;
		rect[3] = tmp ;
	}

	qDebug("l:%d t:%d r:%d b:%d", rect[0], rect[1], rect[2], rect[3]) ;
	qDebug() << "name:" << name ;
	int x = rect[0] ;
	int y = rect[1] ;
	int w = rect[2]-x ;
	int h = rect[3]-y ;

	QImage img = grabFrameBuffer(true).copy(x, y, w, h) ;
	img.save(name) ;
	m_pEditData->setExportEndFrame(frame) ;
}

int AnimeGLWidget::getDigit(int num)
{
	if ( num == 0 ) { return 1 ; }

	int ret = 0 ;
	while ( num ) {
		ret ++ ;
		num /= 10 ;
	}
	return ret ;
}

void AnimeGLWidget::multMatrix(const QMatrix4x4 &mat)
{
	double m[16] ;
	convMat(m, mat) ;
	glMultMatrixd(m) ;
}

void AnimeGLWidget::convMat(double *ret, const QMatrix4x4 &mat)
{
	const qreal *p = mat.data() ;
	for ( int i = 0 ; i < 16 ; i ++ ) {
		ret[i] = p[i] ;
	}
}

// 描画エリア設定
void AnimeGLWidget::setDrawArea(int w, int h)
{
	m_DrawWidth = w ;
	m_DrawHeight = h ;
}

// 背景画像設定
void AnimeGLWidget::setBackImage( QString path )
{
	if ( m_backImageTex ) {
		this->deleteTexture(m_backImageTex);
	}

	qDebug() << "back image path:" << path << " " << path.isEmpty() ;
	m_backImageTex = 0 ;
	if ( path.isEmpty() ) { return ; }

	if ( !m_BackImage.load(path) ) {
		qDebug() << path << " load failed" ;
		return ;
	}
	m_backImageW = m_BackImage.width() ;
	m_backImageH = m_BackImage.height() ;

	util::resizeImage(m_BackImage) ;

	m_backImageTex = this->bindTexture(m_BackImage) ;
}



