#ifndef UTIL_H
#define UTIL_H

#include <QtGui>

namespace util {

	void resizeImage( QImage &imageData ) ;
	QPointF getBezierPoint(QList<QPointF> &pointList, float t) ;

}

#endif // UTIL_H
