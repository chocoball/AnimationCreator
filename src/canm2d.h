#ifndef CANM2D_H
#define CANM2D_H

#include "editdata.h"
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
		kErrorNo_NoError = 0,			///< [ 0]エラーなし
		kErrorNo_BlockNumNotSame,		///< [ 1]ブロック数が違う
		kErrorNo_InvalidFileSize,		///< [ 2]ファイルサイズが違う
		kErrorNo_InvalidVersion,		///< [ 3]バージョンが違う
		kErrorNo_InvalidID,				///< [ 4]不正なID
		kErrorNo_Cancel,				///< [ 5]キャンセルされた
		kErrorNo_InvalidNode,			///< [ 6]ノードが不正
		kErrorNo_InvalidObjNum,			///< [ 7]オブジェ数が不正
		kErrorNo_InvalidImageNum,		///< [ 8]イメージ数が不正
		kErrorNo_InvalidLayerNum,		///< [ 9]レイヤ数が不正
		kErrorNo_InvalidFrameDataNum,	///< [10]フレームデータ数が不正
		kErrorNo_InvalidImageData,		///< [11]イメージデータが不正
		kErrorNo_InvalidFilePath,		///< [12]イメージのファイルパスが不正
		kErrorNo_InvalidObjectName,		///< [13]オブジェクト名が不正
		kErrorNo_InvalidLayerName,		///< [14]レイヤ名が不正

		kErrorNo_Max
	} ;

	int getErrorNo() { return m_nError ; }

	QString getErrorString()
	{
		const char *str[] = {
			"エラーはありません",
			"ブロック数が違います",
			"ファイルサイズが違います",
			"バージョンが違います",
			"不正なIDです",
			"キャンセルされました",
			"ノードが不正です",
			"オブジェクト数が不正です",
			"イメージ数が不正です",
			"レイヤ数が不正です",
			"フレームデータ数が不正です",
			"イメージデータが不正です",
			"イメージのファイルパスが不正です",
			"オブジェクト名が不正です",
			"レイヤ名が不正です"
		} ;
		if ( m_nError >= 0 && m_nError < kErrorNo_Max ) {
			return QObject::trUtf8(str[m_nError]) ;
		}
		return QObject::trUtf8("エラー番号が不正です:%1").arg(m_nError) ;
	}

	CAnm2DBase()
	{
		m_nError = kErrorNo_NoError ;
	}

	void setFilePath( QString &str )
	{
		m_filePath = str ;
	}

	// srcからdestへの相対パスを取得
	QString getRelativePath(QString &src, QString &dest)
	{
		QString path = src ;
		if ( path.at(path.count()-1) != '/' ) {
			path.chop(path.count()-path.lastIndexOf("/")-1) ;
		}
		QDir dir(path) ;
		return dir.relativeFilePath(dest) ;
	}

	// srcからdestへの相対パスを絶対パスに変換
	QString getAbsolutePath(QString &src, QString &dest)
	{
		QString path = src ;
		if ( path.at(path.count()-1) != '/' ) {
			path.chop(path.count()-path.lastIndexOf("/")-1) ;
		}
		QDir dir(path) ;
		return dir.absoluteFilePath(dest) ;
	}

protected:
	int				m_nError ;
	QString			m_filePath ;
};

// binary
class CAnm2DBin : public CAnm2DBase
{
public:
	CAnm2DBin();

	bool makeFromEditData( CEditData &rEditData ) ;
	bool makeFromFile(QByteArray &data, CEditData &rEditData) ;

	QByteArray &getData() { return m_Data ; }

private:
	bool makeHeader( QByteArray &rData, CEditData &rEditData ) ;
	bool makeImageList( QList<QByteArray> &rData, CEditData &rEditData ) ;

	bool addObject(Anm2DHeader *pHeader, CEditData &rEditData) ;
	bool addLayer(Anm2DHeader *pHeader, CEditData &rEditData) ;
	bool addFrameData(Anm2DHeader *pHeader, CEditData &rEditData) ;
	bool addImageData(Anm2DHeader *pHeader, CEditData &rEditData) ;

	Anm2DObject	*search2DObjectFromName(Anm2DHeader *pHeader, QString name) ;
	Anm2DLayer	*search2DLayerFromName(Anm2DHeader *pHeader, QString name) ;

private:
	QByteArray		m_Data ;
};

// XML形式
class CAnm2DXml : public CAnm2DBase
{
public:
	CAnm2DXml(bool bSaveImage) ;

	bool makeFromEditData( CEditData &rEditData ) ;
	bool makeFromFile(QDomDocument &xml, CEditData &rEditData) ;

	QDomDocument &getData()					{ return m_Data ; }
	void setProgress( QProgressDialog *p )	{ m_pProgress = p ; }

private:
	bool makeHeader( QDomElement &element, QDomDocument &doc, CEditData &rEditData ) ;
	bool makeObject(QDomElement &element, QDomDocument &doc, CEditData &rEditData) ;
	bool makeImage( QDomElement &element, QDomDocument &doc, CEditData &rEditData ) ;

	void setProgMaximum( QProgressDialog *pProg, CEditData &rEditData ) ;

	bool addElement( QDomNode &node, CEditData &rEditData ) ;
	bool addLayer( QDomNode &node, CObjectModel::LayerGroupList &layerGroupList, QStandardItem *pParentItem, int maxLayerNum ) ;
	bool addFrameData( QDomNode &node, CObjectModel::FrameDataList &frameDataList, int maxFrameDataNum ) ;
	bool addImage( QDomNode &node, CEditData::ImageData &data ) ;

private:
	QDomDocument	m_Data ;
	QProgressDialog	*m_pProgress ;

	int				m_ObjNum, m_ImageNum ;
	bool			m_bSaveImage ;
};

#endif // CANM2D_H
