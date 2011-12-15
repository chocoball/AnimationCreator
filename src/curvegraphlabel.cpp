#include "curvegraphlabel.h"

#define kFrameNumWidth		20
#define kDataSubNumWidth	20

CurveGraphLabel::CurveGraphLabel(CEditData *pEditData, QWidget *parent) :
    QLabel(parent)
{
	m_pEditData = pEditData ;
	m_currDispType = kDispType_None ;
}

// ラベルサイズ調整
void CurveGraphLabel::adjustSize()
{
	if ( m_currDispType == kDispType_None ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( !pModel->isLayer(m_currIndex) ) { return ; }
	ObjectItem *pItem = pModel->getItemFromIndex(m_currIndex) ;
	if ( !pItem ) { return ; }

	QList< QPair<int, float> > datas = getDatasFromCurrentType(pItem) ;
	if ( datas.size() < 1 ) { return ; }

	// フレーム数の最小、最大
	QPair<int, int> frameMaxMin = qMakePair(datas.first().first, datas.last().first) ;
	// データ差分の最小、最大
	QPair<float, float> dataSubMaxMin = getDataSubMaxMin(datas) ;

	float dataAbs = fabs(dataSubMaxMin.first)>fabs(dataSubMaxMin.second) ? dataSubMaxMin.first : dataSubMaxMin.second ;
	dataAbs = fabs(dataAbs) ;
	float dataSingleStep ;
	if ( dataAbs < 1 ) { dataSingleStep = 0.5 ; }
	else if ( dataAbs < 10 ) { dataSingleStep = 1 ; }
	else { dataSingleStep = 5 ; }

	QSize size ;
	size.setWidth((frameMaxMin.second+20)*kFrameNumWidth + 20);
	size.setHeight(dataAbs/dataSingleStep*2*kDataSubNumWidth + 20);
	resize(size) ;

	qDebug() << "CurveGraphLabel size" << size ;
}

// ペイントイベント
void CurveGraphLabel::paintEvent(QPaintEvent *event)
{
	if ( m_currDispType == kDispType_None ) { return ; }

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( !pModel->isLayer(m_currIndex) ) { return ; }
	ObjectItem *pItem = pModel->getItemFromIndex(m_currIndex) ;
	if ( !pItem ) { return ; }

	QList< QPair<int, float> > datas = getDatasFromCurrentType(pItem) ;
	if ( datas.size() < 1 ) { return ; }

	// フレーム数の最小、最大
	QPair<int, int> frameMaxMin = qMakePair(datas.first().first, datas.last().first) ;
	// データ差分の最小、最大
	QPair<float, float> dataSubMaxMin = getDataSubMaxMin(datas) ;

	float dataAbs = fabs(dataSubMaxMin.first)>fabs(dataSubMaxMin.second) ? dataSubMaxMin.first : dataSubMaxMin.second ;
	dataAbs = fabs(dataAbs) ;
	float dataSingleStep ;
	if ( dataAbs < 1 ) { dataSingleStep = 0.5 ; }
	else if ( dataAbs < 10 ) { dataSingleStep = 1 ; }
	else { dataSingleStep = 5 ; }

	QPainter painter(this) ;
	drawFrameNum(painter, frameMaxMin.second) ;
}

// 現在の表示タイプからデータ取得
QList< QPair<int, float> > CurveGraphLabel::getDatasFromCurrentType(ObjectItem *pLayer)
{
	QList< QPair<int, float> > ret ;
	const QList<FrameData> data = pLayer->getFrameData() ;

	for ( int i = 0 ; i < data.size() ; i ++ ) {
		const FrameData frame = data.at(i) ;
		switch ( m_currDispType ) {
			case kDispType_PosX:	ret.append(qMakePair((int)frame.frame, (float)frame.pos_x)) ;		break ;
			case kDispType_PosY:	ret.append(qMakePair((int)frame.frame, (float)frame.pos_y)) ;		break ;
			case kDispType_PosZ:	ret.append(qMakePair((int)frame.frame, (float)frame.pos_z)) ;		break ;
			case kDispType_RotX:	ret.append(qMakePair((int)frame.frame, (float)frame.rot_x)) ;		break ;
			case kDispType_RotY:	ret.append(qMakePair((int)frame.frame, (float)frame.rot_y)) ;		break ;
			case kDispType_RotZ:	ret.append(qMakePair((int)frame.frame, (float)frame.rot_z)) ;		break ;
			case kDispType_CenterX:	ret.append(qMakePair((int)frame.frame, (float)frame.center_x)) ;	break ;
			case kDispType_CenterY:	ret.append(qMakePair((int)frame.frame, (float)frame.center_y)) ;	break ;
			case kDispType_ScaleX:	ret.append(qMakePair((int)frame.frame, (float)frame.fScaleX)) ;		break ;
			case kDispType_ScaleY:	ret.append(qMakePair((int)frame.frame, (float)frame.fScaleY)) ;		break ;
			case kDispType_UvTop:	ret.append(qMakePair((int)frame.frame, (float)frame.top)) ;			break ;
			case kDispType_UvLeft:	ret.append(qMakePair((int)frame.frame, (float)frame.left)) ;		break ;
			case kDispType_UvRight:	ret.append(qMakePair((int)frame.frame, (float)frame.right)) ;		break ;
			case kDispType_UvBottom:ret.append(qMakePair((int)frame.frame, (float)frame.bottom)) ;		break ;
			case kDispType_ColorR:	ret.append(qMakePair((int)frame.frame, (float)frame.rgba[0])) ;		break ;
			case kDispType_ColorG:	ret.append(qMakePair((int)frame.frame, (float)frame.rgba[1])) ;		break ;
			case kDispType_ColorB:	ret.append(qMakePair((int)frame.frame, (float)frame.rgba[2])) ;		break ;
			case kDispType_ColorA:	ret.append(qMakePair((int)frame.frame, (float)frame.rgba[3])) ;		break ;
			default:	return ret ;
		}
	}

	for ( int i = 0 ; i < ret.size() ; i ++ ) {
		for ( int j = 0 ; j < i ; j ++ ) {
			if ( ret.at(i).first < ret.at(j).first ) {
				ret.swap(i, j) ;
			}
		}
	}
	return ret ;
}

// データ差分の最小最大取得
QPair<float, float> CurveGraphLabel::getDataSubMaxMin(const QList< QPair<int, float> > &datas)
{
	float zeroData = datas.first().second ;
	float min = 0 ;
	float max = 0 ;

	for ( int i = 1 ; i < datas.size() ; i ++ ) {
		float data = datas.at(i).second - zeroData ;
		if ( data > min ) { min = data ; }
		if ( data < max ) { max = data ; }
	}
	return qMakePair(min, max) ;
}

void CurveGraphLabel::drawFrameNum(QPainter &painter, int max)
{
	for ( int i = 0 ; i < max+20 ; i ++ ) {
		painter.drawLine(20 + i*kFrameNumWidth, height()-20, 20 + i*kFrameNumWidth, height()) ;
	}
}


