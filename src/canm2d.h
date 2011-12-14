#ifndef CANM2D_H
#define CANM2D_H

#include "editdata.h"
#include "Anm2dTypes.h"

#include <QDomDocument>

/*!
 @brief XMLフォーマット更新履歴
 2010/11/17	ver 0.0.1	フォーマット作成
 2011/01/19	ver 0.1.0	ループ回数、フレームデータRGBA追加
 2011/09/20	ver 1.0.0	レイヤを階層持つように修正
 2011/09/28	ver 1.0.1	FPS情報追加
 2011/12/07 ver 1.0.2	パス情報追加
 */

#define kAnmXML_Version			0x01000002		///< バージョン 0x[00][000][000]

#define kAnmXML_Attr_Version	"Version"
#define kAnmXML_Attr_ObjNum		"ObjNum"
#define kAnmXML_Attr_LoopNum	"LoopNum"
#define kAnmXML_Attr_LayerNum	"LayerNum"
#define kAnmXML_Attr_FrameNum	"FrameDataNum"
#define kAnmXML_Attr_ImageNum	"ImageNum"
#define kAnmXML_Attr_No			"No"
#define kAnmXML_Attr_Name		"Name"
#define kAnmXML_Attr_ChildNum	"ChildNum"
#define kAnmXML_Attr_FpsNum		"Fps"

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
	virtual ~CAnm2DBase() {}

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
	bool makeHeader( QByteArray &rData, CEditData &rEditData, QList<QByteArray> &objectList, QList<QByteArray> &layerList, QList<QByteArray> &frameList ) ;
	bool makeObject(ObjectItem *pObj, QList<QByteArray> &objList, QList<QByteArray> &layerList, QList<QByteArray> &frameList) ;
	bool makeLayer(ObjectItem *pLayer, int *pLayerNo, Anm2DObject *pObjData, QList<QByteArray> &layerList, QList<QByteArray> &frameList, int parentNo) ;
	bool makeImageList( QList<QByteArray> &rData, CEditData &rEditData ) ;

	bool addList(Anm2DHeader *pHeader) ;
	bool addModel(CEditData &rEditData) ;
	bool addModel_Layer(QModelIndex &parent, int layerNo, CObjectModel *pModel) ;
	bool addImageData(Anm2DHeader *pHeader, CEditData &rEditData) ;

	Anm2DObject	*search2DObjectFromName(Anm2DHeader *pHeader, QString name) ;
	Anm2DLayer	*search2DLayerFromName(Anm2DHeader *pHeader, QString name) ;

private:
	QByteArray				m_Data ;
	QList<Anm2DObject *>	m_ObjPtrList ;
	QList<Anm2DLayer *>		m_LayerPtrList ;
	QList<Anm2DFrameData *>	m_FrameDataPtrList ;
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
	bool makeLayer(ObjectItem *root, QDomElement &element, QDomDocument &doc, CEditData &rEditData) ;
	bool makeImage( QDomElement &element, QDomDocument &doc, CEditData &rEditData ) ;

	void setProgMaximum( QProgressDialog *pProg, CEditData &rEditData ) ;

	bool addElement_00001000(QDomNode &node, CEditData &rEditData) ;
	bool addLayer_00001000( QDomNode &node, ObjectItem *pRoot, int maxLayerNum, CEditData &rEditData ) ;
	bool addFrameData_00001000( QDomNode &node, ObjectItem *pItem, int maxFrameDataNum ) ;

	bool addElement_01000000(QDomNode &node, CEditData &rEditData) ;
	bool addLayer_01000000( QDomNode &node, ObjectItem *pRoot, int maxLayerNum, CEditData &rEditData ) ;
	bool addFrameData_01000000( QDomNode &node, ObjectItem *pItem, int maxFrameDataNum ) ;

	bool addImage( QDomNode &node, CEditData::ImageData &data ) ;

private:
	QDomDocument	m_Data ;
	QProgressDialog	*m_pProgress ;

	int				m_ObjNum, m_ImageNum ;
	bool			m_bSaveImage ;
};

// JSON形式
class CAnm2DJson : public CAnm2DBase
{
public:
	CAnm2DJson(bool bFlat) ;
	~CAnm2DJson() ;

	bool makeFromEditData( CEditData &rEditData ) ;
	bool makeFromFile(QString &, CEditData &) { return false ; }

	QString getData() { return m_Data ; }

private:
	void addString(QString str, int tab = 0) ;
	bool makeObject( CEditData &rEditData ) ;
	bool makeLayer(ObjectItem *pItem, CEditData &rEditData, int tab) ;

private:
	bool			m_bFlat ;
	CObjectModel	*m_pModel ;
	QString			m_Data ;
	int				m_ObjNum, m_ImageNum ;
};

#endif // CANM2D_H
