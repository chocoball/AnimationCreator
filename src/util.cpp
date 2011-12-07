#include "util.h"

namespace util {

void resizeImage( QImage &imageData )
{
	int origW = imageData.width() ;
	int origH = imageData.height() ;
	int fixW = 0, fixH = 0 ;

	int i = 1 ;

	while ( i < 1024 ) {
		if ( origW > i ) {
			i <<= 1 ;
		}
		else {
			fixW = i ;
			break ;
		}
	}
	if ( fixW == 0 ) { fixW = 1024 ; }

	i = 1 ;
	while ( i < 1024 ) {
		if ( origH > i ) {
			i <<= 1 ;
		}
		else {
			fixH = i ;
			break ;
		}
	}
	if ( fixH == 0 ) { fixH = 1024 ; }

	qDebug("orig %d/%d fix %d/%d", origW, origH, fixW, fixH) ;

	if ( fixW == origW && fixH == origH ) { return ; }

	if ( origW > 1024 ) { origW = fixW = 1024 ; }
	if ( origH > 1024 ) { origH = fixH = 1024 ; }

	QImage tmp = QImage(fixW, fixH, QImage::Format_ARGB32) ;
	for ( int y = 0 ; y < fixH ; y ++ ) {
		for ( int x = 0 ; x < fixW ; x ++ ) {
			tmp.setPixel(x, y, qRgba(0, 0, 0, 0));
		}
	}
	for ( int y = 0 ; y < origH ; y ++ ) {
		for ( int x = 0 ; x < origW ; x ++ ) {
			tmp.setPixel(x, y, imageData.pixel(x, y));
		}
	}
	imageData = tmp ;
}

QPointF getBezierPoint(QList<QPointF> &pointList, float t)
{
	if ( pointList.size() == 1 ) {
		return pointList[0] ;
	}
	QList<QPointF> tmp ;
	for ( int i = 0 ; i < pointList.size()-1 ; i ++ ) {
		const QPointF p0 = pointList.at(i) ;
		const QPointF p1 = pointList.at(i+1) ;
		tmp << QPointF((p1.x()-p0.x())*t+p0.x(), (p1.y()-p0.y())*t+p0.y()) ;
	}
	return getBezierPoint(tmp, t) ;
}

}	// namespace util
