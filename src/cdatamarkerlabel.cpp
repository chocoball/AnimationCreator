#include <QtGui>
#include "cdatamarkerlabel.h"

CDataMarkerLabel::CDataMarkerLabel(CEditData *pEditData, QWidget *parent) :
    QLabel(parent)
{
	m_pEditData = pEditData ;
}


void CDataMarkerLabel::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this) ;

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

void CDataMarkerLabel::drawLine( const QModelIndex &index, QPainter &painter, const QModelIndex selIndex )
{
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	if ( selIndex.isValid() ) {
		if ( index == selIndex ) {
			painter.setPen(QPen(QColor(255, 0, 0)));
		}
		else {
			painter.setPen(QPen(QColor(0, 0, 255, 128)));
		}
	}

	const QList<FrameData> &datas = pItem->getFrameData() ;
	for ( int i = 0 ; i < datas.size() ; i ++ ) {
		const FrameData data = datas.at(i) ;

		int x = (width()-1) * data.frame / CEditData::kMaxFrame ;
		painter.drawLine(x, 0, x, height());
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		QModelIndex child = m_pEditData->getObjectModel()->index(i, 0, index) ;
		drawLine(child, painter, selIndex) ;
	}
}

