#ifndef GRIDLABEL_H
#define GRIDLABEL_H

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include "editdata.h"

class CGridLabel : public QLabel
{
	Q_OBJECT

public:
	explicit CGridLabel(CEditData *pEditData, int nTabIndex, QWidget *parent = 0) ;

	void setScale( int scale )			{ mScale		= scale ;	repaint() ; }
	void setGridSize( QPoint &size )	{ m_GridSize	= size ;	repaint() ; }
	void setCatchable( bool flag )		{ m_bCatchable = flag ; }

	bool IsRectMove( void )				{ return m_bRectMove ; }

	void setDrawCenter( bool flag ) ;

public slots:
	void slot_gridOnOff( bool flag ) ;

protected:
	void paintEvent(QPaintEvent *event) ;

	void mousePressEvent(QMouseEvent *ev) ;
	void mouseMoveEvent(QMouseEvent *ev) ;
	void mouseReleaseEvent(QMouseEvent *ev) ;

	void keyPressEvent(QKeyEvent *ev) ;
	void keyReleaseEvent(QKeyEvent *ev) ;

signals:
	void sig_moveCatchRect( QMouseEvent *ev ) ;
	void sig_releaseCatchRect( QMouseEvent *ev ) ;

	void sig_changeSelectLayerUV( CRectF rect ) ;
	void sig_changeCatchRect(CRectF rect) ;

private:
	void startDragAndDrop( QMouseEvent *ev ) ;

	void selectAll( void ) ;
	void deselect( void ) ;

private:
	int				mScale ;			// 倍率
	bool			bCatching ;			// 範囲選択中ならtrue
	bool			m_bRectMove ;		// 範囲移動してるならtrue
	QPoint			m_MovePos ;			// 範囲移動中の座標
	CEditData		*m_pEditData ;		// 編集データ

	QPoint			m_GridSize ;		// グリッドのサイズ
	bool			m_bCatchable ;		// 範囲選択できるならtrue

	bool			m_bDrawGrid ;		// グリッド表示するならtrue
	bool			m_bPressCtrl ;		// Ctrlキー押してたらtrue

	int				m_Index ;			// タブのインデックス

	bool			m_bDrawCenter ;		// 選択フレームデータのセンターを表示するならtrue
} ;

#endif // GRIDLABEL_H
