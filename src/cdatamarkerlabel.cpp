#include <QtGui>
#include "cdatamarkerlabel.h"

CDataMarkerLabel::CDataMarkerLabel(CEditData *pEditData, QWidget *parent) :
    QLabel(parent)
{
	m_pEditData = pEditData ;
}


void CDataMarkerLabel::paintEvent(QPaintEvent */*event*/)
{
	int i, j ;
	QPainter painter(this) ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	CObjectModel::LayerGroupList *pLayerGroupList = pModel->getLayerGroupListFromID(m_pEditData->getSelectObject()) ;
	if ( !pLayerGroupList ) { return ; }

	if ( m_pEditData->getSelectLayerNum() == 0 ) {	// オブジェクト選択中
		painter.setPen(QPen(QColor(255, 0, 0)));
		for ( i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			drawLine(pLayerGroupList->at(i).second, painter) ;
		}
	}
	else {	// レイヤ選択中
		// 非選択のレイヤ
		painter.setPen(QPen(QColor(0, 0, 255, 128)));
		for ( i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			for( j = 0 ; j < m_pEditData->getSelectLayerNum() ; j ++ ) {
				if ( pLayerGroupList->at(i).first == m_pEditData->getSelectLayer(j) ) {
					break ;
				}
			}
			if ( j != m_pEditData->getSelectLayerNum() ) { continue ; }
			drawLine(pLayerGroupList->at(i).second, painter) ;
		}

		// 選択中のレイヤ
		painter.setPen(QPen(QColor(255, 0, 0)));
		for ( i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
			for( j = 0 ; j < m_pEditData->getSelectLayerNum() ; j ++ ) {
				if ( pLayerGroupList->at(i).first == m_pEditData->getSelectLayer(j) ) {
					break ;
				}
			}
			if ( j == m_pEditData->getSelectLayerNum() ) { continue ; }
			drawLine(pLayerGroupList->at(i).second, painter) ;
		}
	}
}

void CDataMarkerLabel::drawLine( const QModelIndex &indexLayer, QPainter &painter )
{
	for ( int i = 0 ; i < frameDataList.size() ; i ++ ) {
		const FrameData data = frameDataList.at(i) ;
		int x = (width()-1) * data.frame / CEditData::kMaxFrame ;
		painter.drawLine(x, 0, x, height());
	}
}

