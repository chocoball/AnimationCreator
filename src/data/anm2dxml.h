#ifndef ANM2DXML_H
#define ANM2DXML_H

#include "../editdata.h"
#include "anm2dbase.h"
#include <QDomDocument>

/*!
 @brief XMLフォーマット更新履歴
 2010/11/17	ver 0.0.1	フォーマット作成
 2011/01/19	ver 0.1.0	ループ回数、フレームデータRGBA追加
 2011/09/20	ver 1.0.0	レイヤを階層持つように修正
 2011/09/28	ver 1.0.1	FPS情報追加
 2011/12/07 ver 1.0.2	パス情報追加
 2011/12/14 ver 1.0.3	レイヤ状態情報 追加
 2012/01/06 ver 1.0.4	フレームデータの色以外をfloatにした
 */

#define kAnmXML_Version 0x01000004 ///< バージョン 0x[00][000][000]

#define kAnmXML_Attr_Version "Version"
#define kAnmXML_Attr_ObjNum "ObjNum"
#define kAnmXML_Attr_LoopNum "LoopNum"
#define kAnmXML_Attr_LayerNum "LayerNum"
#define kAnmXML_Attr_FrameNum "FrameDataNum"
#define kAnmXML_Attr_ImageNum "ImageNum"
#define kAnmXML_Attr_No "No"
#define kAnmXML_Attr_Name "Name"
#define kAnmXML_Attr_ChildNum "ChildNum"
#define kAnmXML_Attr_FpsNum "Fps"
#define kAnmXML_Attr_State "State"

#define kAnmXML_ID_Anm2D "ANM2D"
#define kAnmXML_ID_Root "ROOT"
#define kAnmXML_ID_Object "OBJECT"
#define kAnmXML_ID_Layer "LAYER"
#define kAnmXML_ID_FrameData "FRAMEDATA"
#define kAnmXML_ID_Image "IMAGE"

// XML形式
class Anm2DXml : public Anm2DBase
{
public:
    explicit Anm2DXml(bool bSaveImage);

    bool makeFromEditData(EditData &rEditData);
    bool makeFromFile(QDomDocument &xml, EditData &rEditData, bool bAdd = false);

    QDomDocument &getData() { return m_Data; }
    void setProgress(QProgressDialog *p) { m_pProgress = p; }

private:
    bool makeHeader(QDomElement &element, QDomDocument &doc, EditData &rEditData);
    bool makeObject(QDomElement &element, QDomDocument &doc, EditData &rEditData);
    bool makeLayer(ObjectItem *root, QDomElement &element, QDomDocument &doc, EditData &rEditData);
    bool makeImage(QDomElement &element, QDomDocument &doc, EditData &rEditData);

    void setProgMaximum(QProgressDialog *pProg, EditData &rEditData);

    bool addElement_00001000(QDomNode &node, EditData &rEditData);
    bool addLayer_00001000(QDomNode &node, ObjectItem *pRoot, int maxLayerNum, EditData &rEditData);
    bool addFrameData_00001000(QDomNode &node, ObjectItem *pItem, int maxFrameDataNum);

    bool addElement_01000000(QDomNode &node, EditData &rEditData);
    bool addLayer_01000000(QDomNode &node, ObjectItem *pRoot, int maxLayerNum, EditData &rEditData);
    bool addFrameData_01000000(QDomNode &node, ObjectItem *pItem, int maxFrameDataNum);

    bool addImage(QDomNode &node, EditData::ImageData &data);

    int getNewImageStartNo();

private:
    QDomDocument m_Data;
    QProgressDialog *m_pProgress;

    int m_ObjNum, m_ImageNum;
    bool m_bSaveImage;
    bool m_bAdd;
    QList<EditData::ImageData> m_oldImageDatas; // add 用
};

#endif // ANM2DXML_H
