#ifndef CANM2D_H
#define CANM2D_H

#include "editimagedata.h"
#include "Anm2dTypes.h"

#ifdef Q_OS_MAC
#include <stdint.h>
#define LP_ADD(p, n) (uint64_t)((uint64_t)((uint64_t *)(p)) + (uint64_t)((uint64_t *)(n)))
#else
#define LP_ADD(p, n) (unsigned int)((unsigned int)(p) + (unsigned int)(n))
#endif

class CAnm2D
{
public:
	enum {
		kErrorNo_NoError = 0,		///< エラーなし
		kErrorNo_BlockNumNotSame,	///< ブロック数が違う
		kErrorNo_InvalidFileSize,	///< ファイルサイズが違う
		kErrorNo_InvalidVersion,	///< バージョンが違う
		kErrorNo_InvalidID,			///< 不正なID
	} ;
public:
	CAnm2D();

	bool makeFromEditImageData( CEditImageData &rEditImageData ) ;
	bool makeFromFile(QByteArray &data, QImage &imageData, CEditImageData &rEditImageData) ;

	QByteArray &getData() { return m_Data ; }
	int getErrorNo() { return m_nError ; }

private:
	bool makeHeader( QByteArray &rData, CEditImageData &rEditImageData ) ;
	bool makeImageList( QList<QByteArray> &rData, CEditImageData &rEditImageData ) ;

	bool addObject(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addLayer(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addFrameData(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addImageData(Anm2DHeader *pHeader, QImage &image) ;

	Anm2DObject	*search2DObjectFromName(Anm2DHeader *pHeader, QString name) ;
	Anm2DLayer	*search2DLayerFromName(Anm2DHeader *pHeader, QString name) ;

private:
	CEditImageData	*m_pEditImageData ;
	QByteArray		m_Data ;
	int				m_nError ;
};

#endif // CANM2D_H
