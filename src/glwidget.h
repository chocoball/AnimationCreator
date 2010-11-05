#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include "editimagedata.h"

class AnimeGLWidget : public QGLWidget
{
    Q_OBJECT
public:
	explicit AnimeGLWidget(CEditImageData *editData, QWidget *parent = 0);

signals:
	void sig_dropedImage(QRect rect, QPoint pos, int index) ;
	void sig_selectLayerChanged( CObjectModel::typeID layerID ) ;
	void sig_dragedImage( CObjectModel::FrameData data ) ;
	void sig_deleteFrameData( void ) ;
	void sig_selectPrevLayer( CObjectModel::typeID objID, CObjectModel::typeID layerID, int frame, CObjectModel::FrameData data ) ;

public slots:
	void slot_actDel( void ) ;
	void slot_setDrawGrid(bool flag) ;

protected:
	void initializeGL() ;
	void resizeGL(int w, int h) ;
	void paintGL() ;

	void drawLayers( void ) ;
	void drawLayers_Normal( void ) ;
	void drawLayers_Anime( void ) ;
	void drawFrameData( const CObjectModel::FrameData &data, QColor col = QColor(255, 255, 255, 255) ) ;
	void drawGrid( void ) ;

	void drawLine( QPoint pos0, QPoint pos1, QColor col, float z = 1.0f ) ;
	void drawRect( QRectF rc, QRectF uv, float z, QColor col ) ;

	void dragEnterEvent(QDragEnterEvent *event) ;
	void dropEvent(QDropEvent *event) ;

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void contextMenuEvent(QContextMenuEvent *event) ;

	void keyPressEvent(QKeyEvent *event) ;
	void keyReleaseEvent(QKeyEvent *event);

public:
	void setDrawArea( int w, int h ) ;
	void setGridSpace( int w, int h )
	{
		m_GridWidth = w ;
		m_GridHeight = h ;
	}

private:
	CEditImageData	*m_pEditImageData ;
	GLint			m_DrawWidth, m_DrawHeight ;
	GLint			m_GridWidth, m_GridHeight ;

	bool			m_bDrawGrid ;				///< グリッド描画するならtrue
	bool			m_bDragging ;				///< ドラッグ中ならtrue
	bool			m_bChangeUV ;				///< UV変更中ならtrue

	bool			m_bPressCtrl ;				///< Ctrlが押されてたらtrue

	QPoint			m_DragOffset ;

	QAction			*m_pActDel ;
};

#endif // GLWIDGET_H
