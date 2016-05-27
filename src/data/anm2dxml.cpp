#include "anm2dxml.h"

/************************************************************
*
* Anm2DXml
*
************************************************************/
Anm2DXml::Anm2DXml(bool bSaveImage)
    : Anm2DBase()
{
    m_pProgress = NULL;
    m_bSaveImage = bSaveImage;
    m_bAdd = false;
}

// 作成中のデータをアニメデータに変換
bool Anm2DXml::makeFromEditData(EditData &rEditData)
{
    QDomDocument doc(kAnmXML_ID_Anm2D);
    QDomElement root = doc.createElement(kAnmXML_ID_Root);

    setProgMaximum(m_pProgress, rEditData);

    if (!makeHeader(root, doc, rEditData))
    {
        return false;
    }
    if (!makeObject(root, doc, rEditData))
    {
        return false;
    }
    if (!makeImage(root, doc, rEditData))
    {
        return false;
    }

    doc.appendChild(root);

    m_Data = doc;
    return true;
}

// アニメファイルから作成中データへ変換
bool Anm2DXml::makeFromFile(QDomDocument &xml, EditData &rEditData, bool bAdd)
{
    m_bAdd = bAdd;
    m_Data = xml;
    if (m_Data.doctype().name() != kAnmXML_ID_Anm2D)
    {
        m_nError = kErrorNo_InvalidID;
        return false;
    }

    QDomElement root = m_Data.documentElement();
    if (root.isNull())
    {
        m_nError = kErrorNo_InvalidNode;
        return false;
    }
    if (root.nodeName() != kAnmXML_ID_Root)
    {
        m_nError = kErrorNo_InvalidID;
        return false;
    }

    QDomNamedNodeMap nodeMap = root.attributes();
    if (nodeMap.isEmpty())
    {
        m_nError = kErrorNo_InvalidNode;
        return false;
    }
    if (nodeMap.namedItem(kAnmXML_Attr_Version).isNull())
    {
        m_nError = kErrorNo_InvalidVersion;
        return false;
    }

    int version = nodeMap.namedItem(kAnmXML_Attr_Version).toAttr().value().toInt();
    if (version == 0x00001000)
    {
        m_bAdd = false;
        if (nodeMap.namedItem(kAnmXML_Attr_ObjNum).isNull())
        {
            m_nError = kErrorNo_InvalidObjNum;
            return false;
        }
        if (nodeMap.namedItem(kAnmXML_Attr_ImageNum).isNull())
        {
            m_nError = kErrorNo_InvalidImageNum;
            return false;
        }
        m_ObjNum = nodeMap.namedItem(kAnmXML_Attr_ObjNum).toAttr().value().toInt();
        m_ImageNum = nodeMap.namedItem(kAnmXML_Attr_ImageNum).toAttr().value().toInt();

        //		qDebug("objNum:%d imageNum:%d", m_ObjNum, m_ImageNum) ;

        QDomNode n = root.firstChild();
        if (!addElement_00001000(n, rEditData))
        {
            return false;
        }
    }
    else if (version >= 0x01000000)
    {
        if (m_bAdd)
        {
            int num = rEditData.getImageDataListSize();
            for (int i = 0; i < num; i++)
            {
                m_oldImageDatas << *rEditData.getImageData(i);
            }
        }
        if (nodeMap.namedItem(kAnmXML_Attr_ObjNum).isNull())
        {
            m_nError = kErrorNo_InvalidObjNum;
            return false;
        }
        if (nodeMap.namedItem(kAnmXML_Attr_ImageNum).isNull())
        {
            m_nError = kErrorNo_InvalidImageNum;
            return false;
        }
        m_ObjNum = nodeMap.namedItem(kAnmXML_Attr_ObjNum).toAttr().value().toInt();
        m_ImageNum = nodeMap.namedItem(kAnmXML_Attr_ImageNum).toAttr().value().toInt();

        //		qDebug("objNum:%d imageNum:%d", m_ObjNum, m_ImageNum) ;

        QDomNode n = root.firstChild();
        if (!addElement_01000000(n, rEditData))
        {
            return false;
        }
        if (m_bAdd)
        {
            for (int i = 0; i < m_oldImageDatas.size(); i++)
            {
                rEditData.addImageData(m_oldImageDatas[i]);
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

// ヘッダエレメント作成
bool Anm2DXml::makeHeader(QDomElement &element, QDomDocument &doc, EditData &rEditData)
{
    Q_UNUSED(doc);

    ObjectModel *pModel = rEditData.getObjectModel();
    ObjectItem *pRoot = pModel->getItemFromIndex(QModelIndex());

    element.setAttribute(kAnmXML_Attr_Version, kAnmXML_Version);
    element.setAttribute(kAnmXML_Attr_ObjNum, pRoot->childCount());
    element.setAttribute(kAnmXML_Attr_ImageNum, rEditData.getImageDataListSize());

    return true;
}

// オブジェクトエレメント作成
bool Anm2DXml::makeObject(QDomElement &element, QDomDocument &doc, EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();
    ObjectItem *pRoot = pModel->getItemFromIndex(QModelIndex());
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        ObjectItem *obj = pRoot->child(i);

        QDomElement elmObj = doc.createElement(kAnmXML_ID_Object);
        elmObj.setAttribute(kAnmXML_Attr_Name, QString(obj->getName().toUtf8()));
        elmObj.setAttribute(kAnmXML_Attr_No, i);
        elmObj.setAttribute(kAnmXML_Attr_LayerNum, obj->childCount());
        elmObj.setAttribute(kAnmXML_Attr_LoopNum, obj->getLoop()); // ループ回数(after ver 0.1.0)
        elmObj.setAttribute(kAnmXML_Attr_FpsNum, obj->getFps());   // FPS(after ver 1.0.1)
        element.appendChild(elmObj);

        for (int j = 0; j < obj->childCount(); j++)
        {
            if (!makeLayer(obj->child(j), elmObj, doc, rEditData))
            {
                return false;
            }
        }
    }
    return true;
}

bool Anm2DXml::makeLayer(ObjectItem *root, QDomElement &element, QDomDocument &doc, EditData &rEditData)
{
    QDomElement elmLayer = doc.createElement(kAnmXML_ID_Layer);
    elmLayer.setAttribute(kAnmXML_Attr_Name, QString(root->getName().toUtf8()));
    elmLayer.setAttribute(kAnmXML_Attr_FrameNum, root->getFrameData().size());
    elmLayer.setAttribute(kAnmXML_Attr_ChildNum, root->childCount());
    elmLayer.setAttribute(kAnmXML_Attr_State, root->data(Qt::CheckStateRole).toInt()); // レイヤ状態(after ver 1.0.3)
    element.appendChild(elmLayer);

    const QList<FrameData> &datas = root->getFrameData();
    for (int i = 0; i < datas.size(); i++)
    {
        const FrameData &data = datas.at(i);

        QDomElement elmFrameData = doc.createElement(kAnmXML_ID_FrameData);
        QDomElement elmTmp;
        QDomText text;

        elmTmp = doc.createElement("frame");
        text = doc.createTextNode(QString("%1").arg(data.frame));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("pos");
        text = doc.createTextNode(QString("%1 %2 %3").arg(data.pos_x).arg(data.pos_y).arg(data.pos_z));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("rot");
        text = doc.createTextNode(QString("%1 %2 %3").arg(data.rot_x).arg(data.rot_y).arg(data.rot_z));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("center");
        text = doc.createTextNode(QString("%1 %2").arg(data.center_x).arg(data.center_y));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("UV");
        text = doc.createTextNode(QString("%1 %2 %3 %4").arg(data.left).arg(data.top).arg(data.right).arg(data.bottom));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("ImageNo");
        text = doc.createTextNode(QString("%1").arg(data.nImage));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("scale");
        text = doc.createTextNode(QString("%1 %2").arg(data.fScaleX).arg(data.fScaleY));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        elmTmp = doc.createElement("UVAnime");
        text = doc.createTextNode(QString("%1").arg(data.bUVAnime));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        // 頂点色(after ver 0.1.0)
        elmTmp = doc.createElement("Color");
        text = doc.createTextNode(QString("%1 %2 %3 %4").arg(data.rgba[0]).arg(data.rgba[1]).arg(data.rgba[2]).arg(data.rgba[3]));
        elmTmp.appendChild(text);
        elmFrameData.appendChild(elmTmp);

        // パス(after ver 1.0.2)
        for (int j = 0; j < 2; j++)
        {
            if (data.path[j].bValid)
            {
                elmTmp = doc.createElement(QString("Path%1").arg(j));
                text = doc.createTextNode(QString("%1 %2").arg(data.path[j].v.x()).arg(data.path[j].v.y()));
                elmTmp.appendChild(text);
                elmFrameData.appendChild(elmTmp);
            }
        }

        elmLayer.appendChild(elmFrameData);

        if (m_pProgress)
        {
            m_pProgress->setValue(m_pProgress->value() + 1);

            if (m_pProgress->wasCanceled())
            {
                m_nError = kErrorNo_Cancel;
                return false;
            }
        }
    }

    for (int i = 0; i < root->childCount(); i++)
    {
        makeLayer(root->child(i), elmLayer, doc, rEditData);
    }
    return true;
}

// イメージエレメント作成
bool Anm2DXml::makeImage(QDomElement &element, QDomDocument &doc, EditData &rEditData)
{
    for (int i = 0; i < rEditData.getImageDataListSize(); i++)
    {
#if 1
        EditData::ImageData *p = rEditData.getImageData(i);
        if (!p)
        {
            continue;
        }
        QString imgFilePath = p->fileName;
        QImage image = p->Image;
#else
        QString imgFilePath = rEditData.getImageFileName(i);
        QImage image = rEditData.getImage(i);
#endif
        QDomElement elmImage = doc.createElement(kAnmXML_ID_Image);
        elmImage.setAttribute(kAnmXML_Attr_No, p->nNo);

        QDomElement elmTmp;
        QDomText text;

        elmTmp = doc.createElement("FilePath");
        QString relPath = getRelativePath(m_filePath, imgFilePath);
        text = doc.createTextNode(relPath.toUtf8());
        elmTmp.appendChild(text);
        elmImage.appendChild(elmTmp);

        int w, h;
        if (m_bSaveImage)
        {
            w = image.width();
            h = image.height();
        }
        else
        {
#if 1
            w = p->origImageW;
            h = p->origImageH;
#else
            rEditData.getOriginalImageSize(i, w, h);
#endif
        }

        elmTmp = doc.createElement("Size");
        text = doc.createTextNode(QString("%1 %2").arg(w).arg(h));
        elmTmp.appendChild(text);
        elmImage.appendChild(elmTmp);

        if (m_bSaveImage)
        {
            elmTmp = doc.createElement("Data");
            QString str;
            str.reserve(image.height() * image.width() * 9 + image.height() * 15);
            for (int y = 0; y < image.height(); y++)
            {
                for (int x = 0; x < image.width(); x++)
                {
                    if (!(x % 16))
                    {
                        str.append(QString("\n            "));
                    }
                    str.append(QString::number(image.pixel(x, y), 16));
                    str.append(" ");

                    if (m_pProgress)
                    {
                        m_pProgress->setValue(m_pProgress->value() + 1);
                        if (m_pProgress->wasCanceled())
                        {
                            m_nError = kErrorNo_Cancel;
                            return false;
                        }
                    }
                }
            }
            str.append(QString("\n        "));
            text = doc.createTextNode(str);
            elmTmp.appendChild(text);
            elmImage.appendChild(elmTmp);
        }

        element.appendChild(elmImage);
    }

    return true;
}

// プログレスバーの最大値セット
void Anm2DXml::setProgMaximum(QProgressDialog *pProg, EditData & /*rEditData*/)
{
    if (!pProg)
    {
        return;
    }
#if 0
	TODO
	int max = 0 ;
	CObjectModel *pModel = rEditData.getObjectModel() ;
	const CObjectModel::ObjectList &objList = pModel->getObjectList() ;
	for ( int i = 0 ; i < objList.size() ; i ++ ) {
		const CObjectModel::LayerGroupList &layerGroupList = objList.at(i).layerGroupList ;
		for ( int j = 0 ; j < layerGroupList.size() ; j ++ ) {
			max += layerGroupList.at(j).second.size() ;
		}
	}

	if ( m_bSaveImage ) {
		for ( int i = 0 ; i < rEditData.getImageDataListSize() ; i ++ ) {
#if 1
			CEditData::ImageData *p = rEditData.getImageData(i) ;
			if ( !p ) { continue ; }
			max += p->Image.height() * p->Image.width() ;
#else
			max += rEditData.getImage(i).height() * rEditData.getImage(i).width() ;
#endif
		}
	}
	qDebug() << "max:" << max ;
	pProg->setMaximum(max);
#endif
}

// エレメント追加
bool Anm2DXml::addElement_00001000(QDomNode &node, EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();

    QList<EditData::ImageData> ImageData;

    while (!node.isNull())
    {
        if (node.nodeName() == kAnmXML_ID_Object)
        { // オブジェクト
            QString name;
            int layerNum = 0;
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_Name).isNull() || nodeMap.namedItem(kAnmXML_Attr_LayerNum).isNull() || nodeMap.namedItem(kAnmXML_Attr_No).isNull() || nodeMap.namedItem(kAnmXML_Attr_LoopNum).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value();
            layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt();
            //            int no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt() ;
            int loopNum = nodeMap.namedItem(kAnmXML_Attr_LoopNum).toAttr().value().toInt();

            QModelIndex index = pModel->addItem(name, QModelIndex());
            ObjectItem *pObj = pModel->getItemFromIndex(index);
            pObj->setLoop(loopNum);
            pObj->setFps(60);

            QDomNode child = node.firstChild();
            if (!addLayer_00001000(child, pObj, layerNum, rEditData))
            {
                return false;
            }
        }
        else if (node.nodeName() == kAnmXML_ID_Image)
        { // イメージ
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_No).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            int no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt();

            EditData::ImageData data;
            QDomNode child = node.firstChild();
            if (!addImage(child, data))
            {
                return false;
            }
            data.lastModified = QDateTime::currentDateTimeUtc();
            data.nTexObj = 0;
            data.nNo = no;
            ImageData.insert(no, data);
        }
        node = node.nextSibling();
    }

    if (pModel->getItemFromIndex(QModelIndex())->childCount() != m_ObjNum || ImageData.size() != m_ImageNum)
    {
        m_nError = kErrorNo_InvalidObjNum;
        return false;
    }

    rEditData.setImageData(ImageData);
    return true;
}

// レイヤデータを追加
bool Anm2DXml::addLayer_00001000(QDomNode &node, ObjectItem *pRoot, int maxLayerNum, EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();

    while (!node.isNull())
    {
        if (node.nodeName() == kAnmXML_ID_Layer)
        {
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_Name).isNull() || nodeMap.namedItem(kAnmXML_Attr_FrameNum).isNull() || nodeMap.namedItem(kAnmXML_Attr_No).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            QString name;
            int frameDataNum = 0;

            name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value();
            frameDataNum = nodeMap.namedItem(kAnmXML_Attr_FrameNum).toAttr().value().toInt();

            QModelIndex index = pModel->addItem(name, pRoot->getIndex());
            ObjectItem *pItem = pModel->getItemFromIndex(index);
            pItem->setData(ObjectItem::kState_Disp, Qt::CheckStateRole);

            QDomNode child = node.firstChild();
            if (!addFrameData_00001000(child, pItem, frameDataNum))
            {
                return false;
            }
        }
        node = node.nextSibling();
    }

    if (pRoot->childCount() != maxLayerNum)
    {
        m_nError = kErrorNo_InvalidLayerNum;
        return false;
    }
    return true;
}

// フレームデータを追加
bool Anm2DXml::addFrameData_00001000(QDomNode &node, ObjectItem *pItem, int maxFrameDataNum)
{
    while (!node.isNull())
    {
        if (node.nodeName() == kAnmXML_ID_FrameData)
        {
            FrameData data;

            QDomNode dataNode = node.firstChild();
            while (!dataNode.isNull())
            {
                if (dataNode.nodeName() == "frame")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QString frame = dataNode.firstChild().toText().nodeValue();
                    if (frame.isEmpty())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.frame = frame.toInt();
                }
                else if (dataNode.nodeName() == "pos")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QStringList pos = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (pos.size() != 3)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.pos_x = pos[0].toFloat();
                    data.pos_y = pos[1].toFloat();
                    data.pos_z = pos[2].toFloat();
                }
                else if (dataNode.nodeName() == "rot")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QStringList rot = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (rot.size() != 3)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.rot_x = rot[0].toFloat();
                    data.rot_y = rot[1].toFloat();
                    data.rot_z = rot[2].toFloat();
                }
                else if (dataNode.nodeName() == "center")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QStringList center = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (center.size() != 2)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.center_x = center[0].toFloat();
                    data.center_y = center[1].toFloat();
                }
                else if (dataNode.nodeName() == "UV")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QStringList uv = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (uv.size() != 4)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.left = uv[0].toFloat();
                    data.top = uv[1].toFloat();
                    data.right = uv[2].toFloat();
                    data.bottom = uv[3].toFloat();
                }
                else if (dataNode.nodeName() == "ImageNo")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QString image = dataNode.firstChild().toText().nodeValue();
                    if (image.isEmpty())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.nImage = image.toInt() + getNewImageStartNo();
                }
                else if (dataNode.nodeName() == "scale")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QStringList scale = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (scale.size() != 2)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.fScaleX = scale[0].toFloat();
                    data.fScaleY = scale[1].toFloat();
                }
                else if (dataNode.nodeName() == "UVAnime")
                {
                    if (!dataNode.hasChildNodes())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    QString anime = dataNode.firstChild().toText().nodeValue();
                    if (anime.isEmpty())
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.bUVAnime = anime.toInt() ? true : false;
                }
                else if (dataNode.nodeName() == "Color")
                { // 頂点色(after ver 0.1.0)
                    QStringList color = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (color.size() != 4)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.rgba[0] = color[0].toInt();
                    data.rgba[1] = color[1].toInt();
                    data.rgba[2] = color[2].toInt();
                    data.rgba[3] = color[3].toInt();
                }
                else if (dataNode.nodeName() == "Path0" || dataNode.nodeName() == "Path1")
                { // パス(after ver 1.0.2)
                    int index = (dataNode.nodeName() == "Path0") ? 0 : 1;
                    QStringList vec = dataNode.firstChild().toText().nodeValue().split(" ");
                    if (vec.size() != 2)
                    {
                        m_nError = kErrorNo_InvalidNode;
                        return false;
                    }
                    data.path[index].bValid = true;
                    data.path[index].v.setX(vec[0].toFloat());
                    data.path[index].v.setY(vec[1].toFloat());
                }

                dataNode = dataNode.nextSibling();
            }

            if (data.frame == 0xffff)
            {
                maxFrameDataNum--;
            }
            else
            {
                pItem->addFrameData(data);
            }
        }
        node = node.nextSibling();
    }

    if (pItem->getFrameData().size() != maxFrameDataNum)
    {
        qDebug("%d %d", pItem->getFrameData().size(), maxFrameDataNum);
        m_nError = kErrorNo_InvalidFrameDataNum;
        return false;
    }
    return true;
}

bool Anm2DXml::addElement_01000000(QDomNode &node, EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();

    QList<EditData::ImageData> ImageData;
    int objNum = 0;

    while (!node.isNull())
    {
        if (node.nodeName() == kAnmXML_ID_Object)
        { // オブジェクト
            QString name;
            int layerNum = 0;
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_Name).isNull() || nodeMap.namedItem(kAnmXML_Attr_LayerNum).isNull() || nodeMap.namedItem(kAnmXML_Attr_No).isNull() || nodeMap.namedItem(kAnmXML_Attr_LoopNum).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value();
            layerNum = nodeMap.namedItem(kAnmXML_Attr_LayerNum).toAttr().value().toInt();
            int loopNum = nodeMap.namedItem(kAnmXML_Attr_LoopNum).toAttr().value().toInt();
            int fps = 60;
            if (!nodeMap.namedItem(kAnmXML_Attr_FpsNum).isNull())
            {
                fps = nodeMap.namedItem(kAnmXML_Attr_FpsNum).toAttr().value().toInt();
            }

            QModelIndex index = pModel->addItem(name, QModelIndex());
            ObjectItem *pObj = pModel->getItemFromIndex(index);
            pObj->setLoop(loopNum);
            pObj->setFps(fps);

            QDomNode child = node.firstChild();
            if (!addLayer_01000000(child, pObj, layerNum, rEditData))
            {
                return false;
            }
            objNum++;
        }
        else if (node.nodeName() == kAnmXML_ID_Image)
        { // イメージ
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_No).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            int no = nodeMap.namedItem(kAnmXML_Attr_No).toAttr().value().toInt();

            EditData::ImageData data;
            QDomNode child = node.firstChild();
            if (!addImage(child, data))
            {
                return false;
            }
            data.lastModified = QDateTime::currentDateTimeUtc();
            data.nTexObj = 0;
            data.nNo = no + getNewImageStartNo();
            ImageData.insert(no, data);
        }
        node = node.nextSibling();
    }

    if (objNum != m_ObjNum || ImageData.size() != m_ImageNum)
    {
        m_nError = kErrorNo_InvalidObjNum;
        return false;
    }

    rEditData.setImageData(ImageData);
    return true;
}

bool Anm2DXml::addLayer_01000000(QDomNode &node, ObjectItem *pRoot, int maxLayerNum, EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();

    while (!node.isNull())
    {
        if (node.nodeName() == kAnmXML_ID_Layer)
        {
            QDomNamedNodeMap nodeMap = node.attributes();
            if (nodeMap.namedItem(kAnmXML_Attr_Name).isNull() || nodeMap.namedItem(kAnmXML_Attr_FrameNum).isNull() || nodeMap.namedItem(kAnmXML_Attr_ChildNum).isNull())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            QString name;
            int frameDataNum = 0;
            int childNum = 0;
            int state = ObjectItem::kState_Disp;

            name = nodeMap.namedItem(kAnmXML_Attr_Name).toAttr().value();
            frameDataNum = nodeMap.namedItem(kAnmXML_Attr_FrameNum).toAttr().value().toInt();
            childNum = nodeMap.namedItem(kAnmXML_Attr_ChildNum).toAttr().value().toInt();
            if (!nodeMap.namedItem(kAnmXML_Attr_State).isNull())
            {
                state = nodeMap.namedItem(kAnmXML_Attr_State).toAttr().value().toInt();
            }

            QModelIndex index = pModel->addItem(name, pRoot->getIndex());
            ObjectItem *pItem = pModel->getItemFromIndex(index);
            pItem->setData(state, Qt::CheckStateRole);

            QDomNode child = node.firstChild();
            if (!addFrameData_00001000(child, pItem, frameDataNum))
            {
                return false;
            }
            QDomNode layers = node.firstChild();
            if (!addLayer_01000000(layers, pItem, childNum, rEditData))
            {
                return false;
            }
        }
        node = node.nextSibling();
    }

    if (pRoot->childCount() != maxLayerNum)
    {
        m_nError = kErrorNo_InvalidLayerNum;
        return false;
    }
    return true;
}

bool Anm2DXml::addFrameData_01000000(QDomNode & /*node*/, ObjectItem * /*pItem*/, int /*maxFrameDataNum*/)
{
    return false;
}

// イメージ追加
bool Anm2DXml::addImage(QDomNode &node, EditData::ImageData &data)
{
    bool bFindData = false;

    while (!node.isNull())
    {
        if (node.nodeName() == "FilePath")
        {
            if (!node.hasChildNodes())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            data.fileName = node.firstChild().toText().nodeValue();
        }
        else if (node.nodeName() == "Size")
        {
            if (!node.hasChildNodes())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            QStringList size = node.firstChild().toText().nodeValue().split(" ");
            if (size.size() != 2)
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            data.Image = QImage(size[0].toInt(), size[1].toInt(), QImage::Format_ARGB32);
        }
        else if (node.nodeName() == "Data")
        {
            bFindData = true;
            if (!node.hasChildNodes())
            {
                m_nError = kErrorNo_InvalidNode;
                return false;
            }
            QStringList image = node.firstChild().toText().nodeValue().split(QRegExp("( |\n|\r)"), QString::SkipEmptyParts);
            qDebug("size:%d w:%d h:%d = %d", image.size(), data.Image.width(), data.Image.height(), data.Image.width() * data.Image.height());
            if (image.size() != data.Image.width() * data.Image.height())
            {
                for (int i = 0; i < 32; i++)
                {
                    if (i >= image.size())
                    {
                        break;
                    }
                    qDebug() << image[i];
                }
                m_nError = kErrorNo_InvalidImageData;
                return false;
            }
            int cnt = 0;
            for (int y = 0; y < data.Image.height(); y++)
            {
                for (int x = 0; x < data.Image.width(); x++)
                {
                    data.Image.setPixel(x, y, image[cnt].toUInt(NULL, 16));
                    cnt++;
                }
            }
        }
        node = node.nextSibling();
    }

    if (!bFindData)
    { // Data要素が無い場合はファイルパスから読み込む
        if (data.fileName.isEmpty())
        { // ファイルパスもない
            m_nError = kErrorNo_InvalidFilePath;
            return false;
        }
        QString path = getAbsolutePath(m_filePath, data.fileName);
        data.Image = QImage(path);
        data.fileName = path;

        if (data.Image.isNull())
        {
            m_nError = kErrorNo_InvalidFilePath;
            return false;
        }
    }
    return true;
}

int Anm2DXml::getNewImageStartNo()
{
    if (!m_bAdd)
    {
        return 0;
    }

    int no = 0;
    for (int i = 0; i < m_oldImageDatas.size(); i++)
    {
        if (no < m_oldImageDatas.at(i).nNo + 1)
        {
            no = m_oldImageDatas.at(i).nNo + 1;
        }
    }
    return no;
}
