#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <math.h>

typedef struct {
	float		x0, y0 ;
	float		x1, y1 ;
} Vertex ;

typedef struct _tagFrameData {
	unsigned short	frame ;
	short			pos_x, pos_y, pos_z ;		///< global position
	short			rot_x, rot_y, rot_z ;		///< rotation
	short			center_x, center_y ;		///< local center position
	short			left, right, top, bottom ;	///< UV
	short			nImage ;					///< Image No.
	float			fScaleX, fScaleY ;			///< scale
	bool			bUVAnime ;					///< UVアニメするならtrue
	unsigned char	rgba[4] ;					///< RGBA

	_tagFrameData()
	{
		frame = 0xffff ;
		pos_x = pos_y = pos_z = 0 ;
		rot_x = rot_y = rot_z = 0 ;
		center_x = center_y = 0 ;
		fScaleX = fScaleY = 1 ;
		rgba[0] = rgba[1] = rgba[2] = rgba[3] = 255 ;
	}

	bool operator == (const struct _tagFrameData &r) const
	{
		if ( frame != r.frame )			{ return false ; }
		if ( pos_x != r.pos_x )			{ return false ; }
		if ( pos_y != r.pos_y )			{ return false ; }
		if ( pos_z != r.pos_z )			{ return false ; }
		if ( rot_x != r.rot_x )			{ return false ; }
		if ( rot_y != r.rot_y )			{ return false ; }
		if ( rot_z != r.rot_z )			{ return false ; }
		if ( center_x != r.center_x )	{ return false ; }
		if ( center_y != r.center_y )	{ return false ; }
		if ( left != r.left )			{ return false ; }
		if ( right != r.right )			{ return false ; }
		if ( top != r.top )				{ return false ; }
		if ( bottom != r.bottom )		{ return false ; }
		if ( nImage != r.nImage )		{ return false ; }
		if ( fScaleX != r.fScaleX )		{ return false ; }
		if ( fScaleY != r.fScaleY )		{ return false ; }
		if ( bUVAnime != r.bUVAnime )	{ return false ; }
		if ( rgba[0] != r.rgba[0] )		{ return false ; }
		if ( rgba[1] != r.rgba[1] )		{ return false ; }
		if ( rgba[2] != r.rgba[2] )		{ return false ; }
		if ( rgba[3] != r.rgba[3] )		{ return false ; }

		return true ;
	}
	bool operator != (const struct _tagFrameData &r) const
	{
		if ( *this == r ) {
			return false ;
		}
		return true ;
	}

	void setRect( QRect &rect )
	{
		left	= rect.left() ;
		right	= rect.right() ;
		top		= rect.top() ;
		bottom	= rect.bottom() ;
	}
	QRect getRect() const
	{
		return QRect(left, top, right-left+1, bottom-top+1) ;
	}
	int width() const { return right - left ; }
	int height() const { return bottom - top ; }
	Vertex getVertex() const
	{
		Vertex v ;
		v.x0 = -center_x ;
		v.y0 = -center_y ;
		v.x1 = v.x0 + width() ;
		v.y1 = v.y0 + height() ;
		return v ;
	}

	void getVertexApplyMatrix(QVector3D ret[4], const QMatrix4x4 &m) const
	{
		Vertex v = getVertex() ;
		ret[0] = QVector3D(v.x0, v.y0, 0) ;		// left-up
		ret[1] = QVector3D(v.x1, v.y0, 0) ;		// right-up
		ret[2] = QVector3D(v.x0, v.y1, 0) ;		// left-down
		ret[3] = QVector3D(v.x1, v.y1, 0) ;		// right-down

		for ( int i = 0 ; i < 4 ; i ++ ) {
			ret[i] = m * ret[i] ;
		}
	}

	struct _tagFrameData getInterpolation(const struct _tagFrameData *pNext, int nowFrame) const
	{
		struct _tagFrameData data = *this ;
		if ( pNext ) {
			int frameNow = nowFrame - frame ;
			int frameAll = pNext->frame - frame ;
			data.pos_x		+= (pNext->pos_x - pos_x)*frameNow/frameAll ;
			data.pos_y		+= (pNext->pos_y - pos_y)*frameNow/frameAll ;
			data.pos_z		+= (pNext->pos_z - pos_z)*frameNow/frameAll ;
			data.rot_x		+= (pNext->rot_x - rot_x)*frameNow/frameAll ;
			data.rot_y		+= (pNext->rot_y - rot_y)*frameNow/frameAll ;
			data.rot_z		+= (pNext->rot_z - rot_z)*frameNow/frameAll ;
			data.center_x	+= (pNext->center_x - center_x)*frameNow/frameAll ;
			data.center_y	+= (pNext->center_y - center_y)*frameNow/frameAll ;

			if ( pNext->bUVAnime ) {
				data.left		+= (pNext->left - left)*frameNow/frameAll ;
				data.right		+= (pNext->right - right)*frameNow/frameAll ;
				data.top		+= (pNext->top - top)*frameNow/frameAll ;
				data.bottom		+= (pNext->bottom - bottom)*frameNow/frameAll ;
			}

			data.fScaleX	+= (pNext->fScaleX - fScaleX)*frameNow/frameAll ;
			data.fScaleY	+= (pNext->fScaleY - fScaleY)*frameNow/frameAll ;

			for ( int i = 0 ; i < 4 ; i ++ ) {
				data.rgba[i] += (pNext->rgba[i] - rgba[i])*frameNow/frameAll ;
			}
		}
		return data ;
	}

	QMatrix4x4 getMatrix()
	{
		QMatrix4x4 mat ;
		mat.translate(pos_x, pos_y, pos_z) ;
		mat.rotate(rot_x, 1, 0, 0) ;
		mat.rotate(rot_y, 0, 1, 0) ;
		mat.rotate(rot_z, 0, 0, 1) ;
		mat.scale(fScaleX, fScaleY) ;
		return mat ;
	}

	void applyMatrix(QMatrix4x4 &mat)
	{
		pos_x = mat.column(3).x() ;
		pos_y = mat.column(3).y() ;
		pos_z = mat.column(3).z() ;

		QVector3D vx, vy, vz ;
		vx = QVector3D(mat.row(0)) ;
		vy = QVector3D(mat.row(1)) ;
		vz = QVector3D(mat.row(2)) ;
		fScaleX = vx.length() ;
		fScaleY = vy.length() ;

		vx.normalize();
		vy.normalize();
		vz.normalize();

		qreal rx, ry, rz ;
		rz = -atan2(vx.y(), vx.x()) ;
		ry = -asin(vx.z()) ;
		qreal cosy = cos(ry) ;
		if ( fabs(cosy) <= 0.0001 ) {
			rx = 0 ;
			rz = atan2(-vy.x(), vy.y()) ;
		}
		else {
			rx = asin(vy.z() / cosy) ;
			if ( vz.z() < 0 ) {
				rx = M_PI - rx ;
			}
		}
		rot_x = rx * 180 / M_PI ;
		rot_y = ry * 180 / M_PI ;
		rot_z = rz * 180 / M_PI ;

		if ( rot_x < 0 ) { rot_x += 360 ; }
		if ( rot_y < 0 ) { rot_y += 360 ; }
		if ( rot_z < 0 ) { rot_z += 360 ; }
	}

} FrameData ;


#endif // FRAMEDATA_H
