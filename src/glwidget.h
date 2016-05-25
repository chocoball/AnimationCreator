#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include "editdata.h"
#include "setting.h"
#include "texturecachemanager.h"


class AnimeGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
	// 編集モード
	enum {
		kEditMode_Pos = 0,		///< 位置編集中
		kEditMode_Rot,			///< 回転角編集中
		kEditMode_Center,		///< 中心位置編集中
		kEditMode_Scale,		///< 拡縮編集中
		kEditMode_Path			///< パス編集中
	} ;

	// ドラッグモード
	enum {
		kDragMode_None = 0,		///< なんもない
        kDragMode_Edit			///< 編集モード
	} ;

	typedef struct {
		QMatrix4x4	mat ;
		FrameData	data ;
		QColor		frameCol ;
	} DRAW_FRAMEDATA ;

public:
	explicit AnimeGLWidget(CEditData *editData, CSettings *pSetting, QWidget *parent = 0);
    virtual ~AnimeGLWidget();

	GLuint bindTexture(QImage &image) ;
    void deleteTexture(GLuint texId);

	void setPressWindowMove(bool f) { m_bPressWindowMove = f ; }

signals:
	void sig_dropedImage(CRectF rect, QPoint pos, int index) ;
    void sig_selectLayerChanged( QModelIndex indexLayer ) ;
    void sig_dragedImage( FrameData data ) ;
	void sig_deleteFrameData( void ) ;
    void sig_selectPrevLayer( QModelIndex indexLayer, int frame, FrameData data ) ;
	void sig_frameDataMoveEnd( FrameData data ) ;
	void sig_exportPNGRectChange( void ) ;
	void sig_scrollWindow(QPoint move) ;

public slots:
	void slot_actDel( void ) ;
	void slot_setDrawGrid(bool flag) ;

protected:
    void initializeGL() Q_DECL_OVERRIDE ;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE ;
    void paintGL() Q_DECL_OVERRIDE ;

    bool initShaders(QOpenGLShaderProgram &program, QString prefix);

	void drawLayers( void ) ;
	void drawLayers_Anime( void ) ;
	void drawLayers(ObjectItem *pLayerItem) ;
	void drawLayers_All( void ) ;
	void drawLayer_All(ObjectItem *pLayerItem, int frame) ;

	void drawSelFrameInfo( void ) ;
	void drawFrameData( const FrameData &data, QMatrix4x4 mat, QColor col = QColor(255, 255, 255, 255) ) ;
	void drawFrame(const FrameData &data, QMatrix4x4 mat, QColor col) ;
	void drawGrid( void ) ;
	void drawCenter( void ) ;

	void drawLine( QPoint pos0, QPoint pos1, QColor col, float z = 1.0f ) ;
	void drawRect( CRectF rc, CRectF uv, float z, QColor col ) ;
	void drawCircle(QPoint p, float length, int div) ;

	void drawBezierLine(ObjectItem *pLayerItem, int prevFrame, int nextFrame) ;

	void dragEnterEvent(QDragEnterEvent *event) ;
	void dropEvent(QDropEvent *event) ;

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void contextMenuEvent(QContextMenuEvent *event) ;

	QPoint editData(FrameData *pData, QPoint nowPos, QPoint oldPos, QMatrix4x4 mat, ObjectItem *pItem, int frame) ;

	void writePNGFromFrameBuffer( void ) ;
	int getDigit(int num) ;

	void sortDrawList() ;
	void drawList() ;

public:
	void setDrawArea( int w, int h ) ;
	void setGridSpace( int w, int h )
	{
		m_GridWidth = w ;
		m_GridHeight = h ;
	}
	void setEditMode( int mode )
	{
		m_editMode = mode ;
	}
	void setPressCtrl( bool flag )
	{
		m_bPressCtrl = flag ;
	}
	bool getPressCtrl( void )
	{
		return m_bPressCtrl ;
	}

	int getDragMode( void ) { return m_dragMode ; }

	void setBackImage( QString path ) ;

private:
	CEditData			*m_pEditData ;
	CSettings			*m_pSetting ;
    TextureCacheManager *m_pTextureCacheManager;
	GLint				m_DrawWidth, m_DrawHeight ;
	GLint				m_GridWidth, m_GridHeight ;

	bool				m_bDrawGrid ;				///< グリッド描画するならtrue
	bool				m_bPressCtrl ;				///< Ctrlが押されてたらtrue
	bool				m_bPressWindowMove ;		///< WindowMoveショートカットが押されてたらtrue

	QPoint				m_DragOffset ;
	QPoint				m_SelPluralStartPos ;		///< レイヤ複数選択開始位置
	QPoint				m_SelPluralEndPos ;			///< レイヤ複数選択終了位置

	QAction				*m_pActDel ;

	int					m_editMode ;				///< 編集モード kEditMode_~
	int					m_dragMode ;				///< ドラッグモード kDragMode_~

	int					m_dragPathIndex ;			///< ドラッグしてるパスインデックス(-1で無効)
	QPoint				m_dragStart ;				///< マウス開始ポス

	unsigned int		m_backImageTex ;
	QImage				m_BackImage ;
	int					m_backImageW, m_backImageH ;

	FrameData			m_editFrameDataOld ;
    QOpenGLShaderProgram m_textureShaderProgram;
    QOpenGLShaderProgram m_lineShaderProgram;

    QMatrix4x4          m_matProj;

	QList<DRAW_FRAMEDATA>	m_drawList ;
};

#endif // GLWIDGET_H
