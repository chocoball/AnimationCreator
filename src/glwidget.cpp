#include "glwidget.h"
#include "util.h"

AnimeGLWidget::AnimeGLWidget(CEditData *editData, CSettings *pSetting, QWidget *parent) :
    QGLWidget(parent)
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

	QGLFormat f = format() ;
	f.setAlpha(true) ;
	setFormat(f);
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
#if 1
	for ( int i = 0 ; i < m_pEditData->getImageDataListSize() ; i ++ ) {
		CEditData::ImageData *p = m_pEditData->getImageData(i) ;
		if ( !p ) { continue ; }
		if ( p->nTexObj ) { continue ; }
		p->nTexObj = bindTexture(p->Image) ;
	}
#else
	for ( int i = 0 ; i < m_pEditData->getImageDataSize() ; i ++ ) {
		if ( m_pEditData->getTexObj(i) ) { continue ; }
		GLuint obj = bindTexture(m_pEditData->getImage(i)) ;
		m_pEditData->setTexObj(i, obj) ;
	}
#endif
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
	if ( !m_pEditData->getSelectObject() ) { return ; }

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
	int frame = m_pEditData->getSelectFrame() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !pModel->getLayerGroupListFromID(objID) ) { return ; }

	QList<CObjectModel::FrameData> sort ;
	QList<bool> select ;

	const CObjectModel::LayerGroupList &layerGroupList = *pModel->getLayerGroupListFromID(objID) ;
	for ( int i = 0 ; i < layerGroupList.size() ; i ++ ) {
		CObjectModel::typeID layerID = layerGroupList[i].first ;
		QStandardItem *pLayerItem = layerID ;
		if ( !(pLayerItem->data(Qt::CheckStateRole).toInt() & 0x01) ) { continue ; }	// 非表示

		const CObjectModel::FrameData *pNow = pModel->getFrameDataFromPrevFrame(objID, layerID, frame+1) ;
		const CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;

		if ( !pNow ) { continue ; }
		const CObjectModel::FrameData data = pNow->getInterpolation(pNext, frame) ;
		sort.append(data);

		select.append(m_pEditData->getSelectLayer() == layerID) ;
	}
	for ( int i = 0 ; i < sort.size() ; i ++ ) {
		for ( int j = i + 1 ; j < sort.size() ; j ++ ) {
			if ( sort[i].pos_z > sort[j].pos_z ) {
				sort.swap(i, j);
				select.swap(i, j);
			}
		}
	}
	for ( int i = 0 ; i < sort.size() ; i ++ ) {
		drawFrameData(sort[i]);

		if ( m_pSetting->getDrawFrame() && !m_pEditData->isExportPNG() ) {
			QColor col ;
			if ( select[i] )	{ col = QColor(255, 0, 0, 255) ; }
			else				{ col = QColor(64, 64, 64, 255) ; }
			drawFrame(sort[i], col) ;
		}
	}
}

// 全フレーム描画
void AnimeGLWidget::drawLayers_All( void )
{
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
	if ( !objID ) { return ; }
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	const CObjectModel::LayerGroupList *pLayerGroupList = pModel->getLayerGroupListFromID(objID) ;
	if ( !pLayerGroupList ) { return ; }

	int maxFrame = pModel->getMaxFrameFromSelectObject(objID) ;
	for ( int frame = 0 ; frame <= maxFrame ; frame ++ ) {
		for ( int i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			CObjectModel::typeID layerID = pLayerGroupList->at(i).first ;
			const CObjectModel::FrameData *pNow = pModel->getFrameDataFromPrevFrame(objID, layerID, frame+1) ;
			const CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, layerID, frame) ;

			if ( !pNow ) { continue ; }
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

		m_centerPos = QPoint(pData->pos_x, pData->pos_y) ;

		if ( m_editMode != kEditMode_Center ) {
			if ( (m_dragMode != kDragMode_Edit) || m_bPressCtrl ) {
				continue ;
			}
		}

		switch ( m_editMode ) {
		case kEditMode_Rot:
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(2, 0x3333);
			{
				QPoint c = QPoint(pData->pos_x, pData->pos_y) ;
				QPoint p0 = m_DragOffset - QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) ;

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
#if 0
		case kEditMode_Center:
			{
				m_bDrawCenter = true ;
				m_centerPos = QPoint(pData->pos_x, pData->pos_y) ;
			}
			break ;
#endif
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
	CEditData::ImageData *p = m_pEditData->getImageDataFromNo(data.nImage) ;
	if ( !p ) { return ; }

	QImage &Image = p->Image ;
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

	glBindTexture(GL_TEXTURE_2D, p->nTexObj) ;

	col.setRed( col.red()		* data.rgba[0] / 255 );
	col.setGreen( col.green()	* data.rgba[1] / 255 );
	col.setBlue( col.blue()		* data.rgba[2] / 255 );
	col.setAlpha( col.alpha()	* data.rgba[3] / 255 );

	drawRect(rect, uvF, data.pos_z / 4096.0f, col) ;

	glPopMatrix();
}

// フレームデータの枠描画
void AnimeGLWidget::drawFrame( const CObjectModel::FrameData &data, QColor col)
{
	Vertex v = data.getVertex() ;

	bool bDepth = glIsEnabled(GL_DEPTH_TEST) ;
	bool bTex = glIsEnabled(GL_TEXTURE_2D) ;
	glDisable(GL_DEPTH_TEST) ;
	glDisable(GL_TEXTURE_2D) ;

	glPushMatrix();
		glTranslatef(data.pos_x, data.pos_y, data.pos_z / 4096.0f);
		glRotatef(data.rot_x, 1, 0, 0);
		glRotatef(data.rot_y, 0, 1, 0);
		glRotatef(data.rot_z, 0, 0, 1);

		drawLine(QPoint(v.x0, v.y0), QPoint(v.x0, v.y1), col, 0);
		drawLine(QPoint(v.x1, v.y0), QPoint(v.x1, v.y1), col, 0);
		drawLine(QPoint(v.x0, v.y0), QPoint(v.x1, v.y0), col, 0);
		drawLine(QPoint(v.x0, v.y1), QPoint(v.x1, v.y1), col, 0);
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

void AnimeGLWidget::drawCenter( void )
{
	CObjectModel::FrameData data ;
	if ( !m_pEditData->getNowSelectFrameData(data) ) { return ; }

	QColor col = QColor(0, 0, 255, 255) ;
	glDisable(GL_DEPTH_TEST);
	drawLine(QPoint(-(CEditData::kGLWidgetSize/2), data.pos_y), QPoint((CEditData::kGLWidgetSize/2), data.pos_y), col, 1.0) ;
	drawLine(QPoint(data.pos_x, -(CEditData::kGLWidgetSize/2)), QPoint(data.pos_x, (CEditData::kGLWidgetSize/2)), col, 1.0) ;
	glEnable(GL_DEPTH_TEST);
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
		CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
		int frame = m_pEditData->getSelectFrame() ;

		CObjectModel::typeID layerID = pModel->getLayerIDFromFrameAndPos(objID, frame, localPos) ;

		if ( !layerID ) {
			// 前フレームを調べる
			CObjectModel::LayerGroupList *pLGList = pModel->getLayerGroupListFromID(objID) ;
			if ( pLGList ) {
				for ( int i = 0 ; i < pLGList->size() ; i ++ ) {
					const CObjectModel::typeID tmpLayerID = pLGList->at(i).first ;
					// 既に現在のフレームにデータがあったら調べない
					if ( pModel->getFrameDataFromIDAndFrame(objID, tmpLayerID, frame) ) { continue ; }

					CObjectModel::FrameData *pPrev = pModel->getFrameDataFromPrevFrame(objID, tmpLayerID, frame, false) ;
					if ( !pPrev ) { continue ; }
					CObjectModel::FrameData *pNext = pModel->getFrameDataFromNextFrame(objID, tmpLayerID, frame) ;
					CObjectModel::FrameData data = pPrev->getInterpolation(pNext, frame) ;
					if ( !pModel->isFrameDataInPos(data, localPos) ) { continue ; }

					layerID = tmpLayerID ;

					emit sig_selectPrevLayer(objID, layerID, frame, data) ;
					break ;
				}
			}
		}

		if ( layerID ) {
			int flag = layerID->data(Qt::CheckStateRole).toInt() ;
			if ( !(flag & 0x01) || (flag & 0x02) ) {	// 非表示
				event->ignore();
				return ;
			}

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
	}
}

// マウス移動中イベント
void AnimeGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::typeID objID = m_pEditData->getSelectObject() ;
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

			if ( pData ) {
				CObjectModel::typeID layerID = m_pEditData->getSelectLayer() ;
				CObjectModel::FrameData *p = pModel->getFrameDataFromIDAndFrame(objID, layerID, frame) ;
				*p = *pData ;
			}
		}

		update() ;
		break ;
	case kDragMode_SelPlural:
		{
			m_SelPluralEndPos = event->pos() - QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) ;
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
			m_SelPluralEndPos = event->pos() - QPoint((CEditData::kGLWidgetSize/2), (CEditData::kGLWidgetSize/2)) ;
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

/**
  データ編集
  */
QPoint AnimeGLWidget::editData(CObjectModel::FrameData *pData, QPoint nowPos, QPoint oldPos)
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

	if ( !m_BackImage.load(path) ) { return ; }
	m_backImageW = m_BackImage.width() ;
	m_backImageH = m_BackImage.height() ;

	util::resizeImage(m_BackImage) ;

	m_backImageTex = this->bindTexture(m_BackImage) ;
}



