#include "anm2djson.h"

/************************************************************
*
* Anm2DJson
*
************************************************************/
Anm2DJson::Anm2DJson(bool bFlat)
    : Anm2DBase()
{
    m_bFlat = bFlat;
    m_pModel = NULL;
}

Anm2DJson::~Anm2DJson()
{
    if (m_bFlat && m_pModel)
    {
        delete m_pModel;
        m_pModel = NULL;
    }
}

bool Anm2DJson::makeFromEditData(EditData &rEditData)
{
    m_pModel = rEditData.getObjectModel();
    if (m_bFlat)
    {
        ObjectModel *p = new ObjectModel();
        p->copy(m_pModel);
        p->flat();
        m_pModel = p;
    }

    addString("{\n");
    if (!makeObject(rEditData))
    {
        return false;
    }
    addString("}");

    m_Data.replace(QRegExp("( |\n)"), "");
    return true;
}

void Anm2DJson::addString(QString str, int tab)
{
    QString t;
    for (int i = 0; i < tab; i++)
    {
        t += "  ";
    }
    m_Data += t + str;
}

bool Anm2DJson::makeObject(EditData &rEditData)
{
    ObjectItem *pRoot = m_pModel->getItemFromIndex(QModelIndex());
    int tab = 1;
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        ObjectItem *pObj = pRoot->child(i);

        addString("\"" + QString(pObj->getName().toUtf8()) + "\": {\n", tab);
        tab++;
        addString("\"animeTime\": " + QVariant((int)(pObj->getMaxFrameNum() * (100.0 / 6.0))).toString() + ",\n", tab);
        addString("\"loopNum\": " + QVariant(pObj->getLoop()).toString() + ",\n", tab);
        addString("\"speed\": " + QVariant((double)pObj->getFps() / 60.0).toString() + ",\n", tab);
        addString("\"layer\": [\n", tab);
        for (int j = 0; j < pObj->childCount(); j++)
        {
            makeLayer(pObj->child(j), rEditData, tab + 1);
            if (j < pObj->childCount() - 1)
            {
                addString(",");
            }
            addString("\n");
        }
        addString("]\n", tab);
        tab--;
        addString("}", tab);
        if (i < pRoot->childCount() - 1)
        {
            addString(",");
        }
        addString("\n");
    }

    return true;
}

bool Anm2DJson::makeLayer(ObjectItem *pItem, EditData &rEditData, int tab)
{
    const QList<FrameData> &datas = pItem->getFrameData();

    addString("{\n", tab);
    tab++;
    addString(QString("\"name\": \"%1\",\n").arg(pItem->getName()), tab);
    addString("\"frame\": [\n", tab);
    tab++;
    for (int i = 0; i < datas.size(); i++)
    {
        const FrameData &data = datas.at(i);

        EditData::ImageData *pImageData = rEditData.getImageDataFromNo(data.nImage);
        if (!pImageData)
        {
            continue;
        }

        int extPos = pImageData->fileName.lastIndexOf("/");
        QString path;
        path = pImageData->fileName.right(pImageData->fileName.size() - extPos - 1);
        double anchor[2], uv[4];
        int w, h;
        w = data.right - data.left;
        h = data.bottom - data.top;
        anchor[0] = w == 0 ? 0 : (double)data.center_x / (double)w;
        anchor[1] = h == 0 ? 0 : (double)data.center_y / (double)h;
        uv[0] = (double)data.left / (double)pImageData->origImageW;
        uv[1] = (double)data.top / (double)pImageData->origImageH;
        uv[2] = (double)w / (double)pImageData->origImageW;
        uv[3] = (double)h / (double)pImageData->origImageH;

        addString("{\n", tab);
        tab++;
        addString("\"frame\": " + QVariant((int)(data.frame * (100.0 / 6.0))).toString() + ",\n", tab);
        addString("\"pos\": [" + QVariant(data.pos_x).toString() + ", " + QVariant(data.pos_y).toString() + ", " + QVariant(data.pos_z).toString() + "],\n", tab);
        addString("\"rot\": [" + QVariant(data.rot_x).toString() + ", " + QVariant(data.rot_y).toString() + ", " + QVariant(data.rot_z).toString() + "],\n", tab);
        addString("\"sca\": [" + QVariant(data.fScaleX).toString() + ", " + QVariant(data.fScaleY).toString() + "],\n", tab);
        addString("\"color\": [" + QVariant((double)data.rgba[0] / 255.0).toString() + ", " + QVariant((double)data.rgba[1] / 255.0).toString() + ", " + QVariant((double)data.rgba[2] / 255.0).toString() + ", " + QVariant((double)data.rgba[3] / 255.0).toString() + "],\n", tab);
        addString("\"uvAnime\": " + QVariant((int)data.bUVAnime).toString() + ",\n", tab);
        addString("\"image\": {\n", tab);
        tab++;
        addString("\"path\": \"" + path + "\",\n", tab);
        addString("\"size\": [" + QVariant(w).toString() + ", " + QVariant(h).toString() + "],\n", tab);
        addString("\"center\": [" + QVariant(anchor[0]).toString() + ", " + QVariant(anchor[1]).toString() + "],\n", tab);
        addString("\"uvrect\": [" + QVariant(uv[0]).toString() + ", " + QVariant(uv[1]).toString() + ", " + QVariant(uv[2]).toString() + ", " + QVariant(uv[3]).toString() + "]\n", tab);
        tab--;
        addString("}\n", tab);
        tab--;
        addString("}", tab);
        if (i < datas.size() - 1)
        {
            addString(",");
        }
        addString("\n");
    }
    tab--;
    addString("]", tab);
    if (pItem->childCount())
    {
        addString(",\n");
        addString("\"layer\": [\n", tab);
        for (int i = 0; i < pItem->childCount(); i++)
        {
            makeLayer(pItem->child(i), rEditData, tab + 1);
            if (i < pItem->childCount() - 1)
            {
                addString(",");
            }
            addString("\n");
        }
        addString("]", tab);
    }
    addString("\n");
    tab--;
    addString("}", tab);
    return true;
}
