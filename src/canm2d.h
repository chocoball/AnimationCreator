#ifndef CANM2D_H
#define CANM2D_H

#include "editimagedata.h"
#include "Anm2dTypes.h"

#include <QDomDocument>

#define kAnmXML_Version			0x00000001

#define kAnmXML_Attr_Version	"Version"
#define kAnmXML_Attr_ObjNum		"ObjNum"
#define kAnmXML_Attr_LayerNum	"LayerNum"
#define kAnmXML_Attr_FrameNum	"FrameDataNum"
#define kAnmXML_Attr_ImageNum	"ImageNum"
#define kAnmXML_Attr_No			"No"
#define kAnmXML_Attr_Name		"Name"

#define kAnmXML_ID_Anm2D		"ANM2D"
#define kAnmXML_ID_Root			"ROOT"
#define kAnmXML_ID_Object		"OBJECT"
#define kAnmXML_ID_Layer		"LAYER"
#define kAnmXML_ID_FrameData	"FRAMEDATA"
#define kAnmXML_ID_Image		"IMAGE"

#ifdef Q_OS_MAC
#include <stdint.h>
#define LP_ADD(p, n) (uint64_t)((uint64_t)((uint64_t *)(p)) + (uint64_t)((uint64_t *)(n)))
#else
#define LP_ADD(p, n) (unsigned int)((unsigned int)(p) + (unsigned int)(n))
#endif

class CAnm2DBase
{
public:
	enum {
		kErrorNo_NoError = 0,		///< エラーなし
		kErrorNo_BlockNumNotSame,	///< ブロック数が違う
		kErrorNo_InvalidFileSize,	///< ファイルサイズが違う
		kErrorNo_InvalidVersion,	///< バージョンが違う
		kErrorNo_InvalidID,			///< 不正なID
		kErrorNo_Cancel,			///< キャンセルされた
	} ;

	int getErrorNo() { return m_nError ; }

	CAnm2DBase()
	{
		m_nError = kErrorNo_NoError ;
	}

protected:
	int				m_nError ;
};

// binary
class CAnm2DBin : public CAnm2DBase
{
public:
	CAnm2DBin();

	bool makeFromEditImageData( CEditImageData &rEditImageData ) ;
	bool makeFromFile(QByteArray &data, CEditImageData &rEditImageData) ;
	QByteArray &getData() { return m_Data ; }

private:
	bool makeHeader( QByteArray &rData, CEditImageData &rEditImageData ) ;
	bool makeImageList( QList<QByteArray> &rData, CEditImageData &rEditImageData ) ;

	bool addObject(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addLayer(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addFrameData(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;
	bool addImageData(Anm2DHeader *pHeader, CEditImageData &rEditImageData) ;

	Anm2DObject	*search2DObjectFromName(Anm2DHeader *pHeader, QString name) ;
	Anm2DLayer	*search2DLayerFromName(Anm2DHeader *pHeader, QString name) ;

private:
	CEditImageData	*m_pEditImageData ;
	QByteArray		m_Data ;
};

// XML形式
class CAnm2DXml : public CAnm2DBase
{
public:
	CAnm2DXml() ;

	bool makeFromEditImageData( CEditImageData &rEditImageData ) ;
	bool makeFromFile(QDomDocument &xml, CEditImageData &rEditImageData) ;
	QDomDocument &getData() { return m_Data ; }
	void setProgress( QProgressDialog *p ) { m_pProgress = p ; }

private:
	bool makeHeader( QDomElement &element, QDomDocument &doc, CEditImageData &rEditImageData ) ;
	bool makeObject(QDomElement &element, QDomDocument &doc, CEditImageData &rEditImageData) ;
	bool makeImage( QDomElement &element, QDomDocument &doc, CEditImageData &rEditImageData ) ;

	void setProgMaximum( QProgressDialog *pProg, CEditImageData &rEditImageData ) ;

private:
	CEditImageData	*m_pEditImageData ;
	QDomDocument	m_Data ;

	QProgressDialog	*m_pProgress ;
};

#endif // CANM2D_H
