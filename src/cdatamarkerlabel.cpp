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
	CObjectModel::LayerGroupList *pLayerGroupList = pModel->getLayerGroupListFromID(m_pEditData->getSelectObject()) ;
	if ( !pLayerGroupList ) { return ; }

	painter.setPen(QPen(QColor(255, 0, 0)));
	for ( int i = 0 ; i < pLayerGroupList->size() ; i ++ ) {
		const CObjectModel::FrameDataList &frameDataList = pLayerGroupList->at(i).second ;
		for ( int j = 0 ; j < frameDataList.size() ; j ++ ) {
			const CObjectModel::FrameData data = frameDataList.at(j) ;
			int x = (width()-1) * data.frame / 180 ;
			painter.drawLine(x, 0, x, height());
		}
	}
}
