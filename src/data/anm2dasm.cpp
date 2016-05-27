#include "anm2dasm.h"
#include <float.h>

/*-----------------------------------------------------------*/ /**
  @brief	.asm インターフェース
  @author	Kenji Nishida
  @date		2011/11/24
*/ /*------------------------------------------------------------*/
Anm2DAsm::Anm2DAsm(bool bFlat)
    : Anm2DBase()
{
    m_bFlat = bFlat;
    m_pModel = NULL;
    memset(m_bUnused, 0, sizeof(m_bUnused));
    m_nCnt = 0;
}

Anm2DAsm::~Anm2DAsm()
{
    if (m_bFlat && m_pModel)
    {
        delete m_pModel;
        m_pModel = NULL;
    }
}

void makeFromEditDataArea(ObjectItem *pObj, QVector4D *pqv4AreaMin, QVector4D *pqv4AreaMax, bool isRecursive)
{
    QMatrix4x4 mat = pObj->getDisplayMatrix(0);
    FrameData frameData = pObj->getDisplayFrameData(0);
    QVector3D v[4];
    frameData.getVertexApplyMatrix(v, mat);
    for (int i = 0; i < 4; i++)
    {
        if (pqv4AreaMin->x() > v[i].x())
            pqv4AreaMin->setX(v[i].x());
        if (pqv4AreaMin->y() > v[i].y())
            pqv4AreaMin->setY(v[i].y());
        if (pqv4AreaMin->z() > v[i].z())
            pqv4AreaMin->setZ(v[i].z());
        if (pqv4AreaMax->x() < v[i].x())
            pqv4AreaMax->setX(v[i].x());
        if (pqv4AreaMax->y() < v[i].y())
            pqv4AreaMax->setY(v[i].y());
        if (pqv4AreaMax->z() < v[i].z())
            pqv4AreaMax->setZ(v[i].z());
    }

    if (isRecursive == false)
        return;

    if (pObj->childCount())
    {
        for (int i = 0; i < pObj->childCount(); i++)
        {
            ObjectItem *pChild = pObj->child(i);
            makeFromEditDataArea(pChild, pqv4AreaMin, pqv4AreaMax, isRecursive);
        }
    }
}

bool Anm2DAsm::makeFromEditDataTip(QString qsLabel, ObjectItem *pObj)
{
    addString(";---------------------------------------------------------------- ANM_TIP\n");
    addString(qsLabel + ":\n");
    addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->getMaxFrameNum(false)) + "\t\t; nKeyFrame\n");
    addString("\t\t\tdd\t\t.key\t\t; pKey\n");
    addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
    if (pObj->childCount())
    {
        addString("\t\t\tdd\t\t.tips\t\t; papTip\n");
    }
    else
    {
        addString("\t\t\tdd\t\tNO_READ\t\t; papTip\n");
    }
    addString("\t\n");

    // キーフレーム
    addString("\t.key:\n");
    for (int i = 0; i <= pObj->getMaxFrameNum(false); i++)
    {
        bool valid;
        FrameData frameData = pObj->getDisplayFrameData(i, &valid);
        if (valid == false)
            continue;
        if (m_aqsVramID[frameData.nImage].isEmpty())
        {
            m_nError = kErrorNo_InvalidImageNo;
            return false;
        }
        if (pObj->getName().indexOf("null") >= 0)
        {
            frameData.rgba[3] = 0;
        }
        addString("\t\t\t; frame " + QString("%1").arg(i) + " --------------------------------\n");
        addString("\t\t\tdd\t\t1\t\t; [NORMAL]\n");
        addString("\t\t\tdw\t\t" + QString("%1").arg(frameData.frame) + "\t\t; uTime\n");
        addString("\t\t\tdw\t\t" + m_aqsVramID[frameData.nImage] + "\t\t; uVramID\n");
        addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2), F32(%3)").arg(frameData.pos_x, 0, 'f').arg(frameData.pos_y, 0, 'f').arg(-frameData.pos_z, 0, 'f') + "\t\t; fvPos\n");
        addString("\t\t\tdd\t\t" + QString("F32(%1)").arg(frameData.rot_z * M_PI / 180.0f, 0, 'f') + "\t\t; fRot\n");
        addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2)").arg(frameData.fScaleX, 0, 'f').arg(frameData.fScaleY, 0, 'f') + "\t\t; fvSca\n");
        addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2)").arg(frameData.center_x, 0, 'f').arg(frameData.center_y, 0, 'f') + "\t\t; fvCenter\n");
        addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2), F32(%3), F32(%4)").arg(frameData.left, 0, 'f').arg(frameData.top, 0, 'f').arg(frameData.right, 0, 'f').arg(frameData.bottom, 0, 'f') + "\t\t; fvUV\n");
        addString("\t\t\tdb\t\t" + QString("%1, %2, %3, %4").arg(frameData.rgba[0]).arg(frameData.rgba[1]).arg(frameData.rgba[2]).arg(frameData.rgba[3]) + "\t\t; bvRGBA\n");
        addString("\t\t\t\n");
    }
    addString("\t\t\tdd\t\t0\t\t; [TERM]\n");
    addString("\n");

    // 子供の処理
    if (pObj->childCount())
    {
        addString("\t.tips:\n");
        for (int i = 0; i < pObj->childCount(); i++)
        {
            addString("\t\t\tdd\t\t" + qsLabel + "_" + QString("%1").arg(i) + "\n");
        }
        addString("\n");
        for (int i = 0; i < pObj->childCount(); i++)
        {
            QString qsLabelChild = qsLabel + QString("_%1").arg(i);
            ObjectItem *pChild = pObj->child(i);
            if (!makeFromEditDataTip(qsLabelChild, pChild))
            {
                return false;
            }
        }
    }
    return true;
}

void Anm2DAsm::subUnusedVramSkip(ObjectItem *pObj)
{
    // キーフレーム
    for (int i = 0; i <= pObj->getMaxFrameNum(false); i++)
    {
        bool valid;
        FrameData frameData = pObj->getDisplayFrameData(i, &valid);
        if (valid == false)
            continue;
        m_bUnused[frameData.nImage] = false;
    }

    // 子供の処理
    if (pObj->childCount())
    {
        for (int i = 0; i < pObj->childCount(); i++)
        {
            ObjectItem *pChild = pObj->child(i);
            subUnusedVramSkip(pChild);
        }
    }
    return;
}

bool Anm2DAsm::makeFromEditData(EditData &rEditData)
{
    m_pModel = rEditData.getObjectModel();
    if (m_bFlat)
    {
        ObjectModel *p = new ObjectModel();
        p->copy(m_pModel);
        p->flat();
        m_pModel = p;
    }

    ObjectItem *pRoot = m_pModel->getItemFromIndex(QModelIndex());

    // 画像をラベル化
    // 未使用画像をスキップするテーブル構築
    int nVram = 0;
    {
        for (int i = 0; i < rEditData.getImageDataListSize(); i++)
        {
            if (i >= KM_VRAM_MAX)
            {
                return false;
            }
            EditData::ImageData *p = rEditData.getImageData(i);
            if (!p)
                continue;
            QFileInfo fi(p->fileName);
            QString sImageLabel = fi.fileName();
            sImageLabel = QString("ID_") + sImageLabel.replace(".", "_").toUpper();
            sImageLabel = sImageLabel.toUtf8();
            m_aqsVramID[i] = sImageLabel;
            m_bUnused[i] = true;
        }
        for (int i = 0; i < pRoot->childCount(); i++)
        {
            ObjectItem *pObj = pRoot->child(i);
            for (int j = 0; j < pObj->childCount(); j++)
            {
                ObjectItem *pChild = pObj->child(j);
                subUnusedVramSkip(pChild);
            }
        }
        for (int i = 0; i < rEditData.getImageDataListSize(); i++)
        {
            if (m_bUnused[i] == false)
                nVram++;
        }
    }

    addString(";----------------------------------------------------------------\n");
    addString("; @kamefile\t" + pRoot->getName().toUtf8() + "\n");
    addString(";---------------------------------------------------------------- HEADER\n");
    addString("\t\t\%include\t\"../imageid.inc\"\n");
    addString("\n");
    addString(";---------------------------------------------------------------- DATA\n");
    addString("%define\t\tNO_READ\t\t0\n");
    addString("%define\t\tF32(f32)\t\t__float32__(f32)\n");
    addString("data:\n");
    addString(";---------------------------------------------------------------- ANM_HEAD\n");
    addString("\t\t\tdb\t\t'ANM0'\t\t; ANM0\n");
    addString("\t\t\tdd\t\t00000003h\t\t; uVersion\n");
    addString("\t\t\tdd\t\t" + QString("%1").arg(/*rEditData.getImageDataListSize()*/ nVram) + "\t\t; nVram\n");
    addString("\t\t\tdd\t\t.vram\t\t; pauVram\n");
    addString("\t\t\tdd\t\t" + QString("%1").arg(pRoot->childCount()) + "\t\t; nObject\n");
    addString("\t\t\tdd\t\t.object\t\t; paObj\n");
    addString("\t\n");
    addString("\t.vram:\n");
    for (int i = 0; i < rEditData.getImageDataListSize(); i++)
    {
        if (i >= KM_VRAM_MAX)
        {
            return false;
        }
        if (m_bUnused[i])
            continue;
        EditData::ImageData *p = rEditData.getImageData(i);
        if (!p)
            continue;
        QFileInfo fi(p->fileName);
        QString sImageLabel = fi.fileName();
        sImageLabel = QString("ID_") + sImageLabel.replace(".", "_").toUpper();
        sImageLabel = sImageLabel.toUtf8();
        m_aqsVramID[i] = sImageLabel;
        addString("\t\t\tdd\t\t" + m_aqsVramID[i] + "\n");
    }
    addString("\t\n");
    addString("\t.object:\n");
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        addString("\t\t\tdd\t\tanmobj" + QString("%1").arg(i) + "\n");
    }
    addString("\n");
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        ObjectItem *pObj = pRoot->child(i);
        QVector4D qv4AreaMin = QVector4D(FLT_MAX, FLT_MAX, FLT_MAX, 0);
        QVector4D qv4AreaMax = QVector4D(FLT_MIN, FLT_MIN, FLT_MIN, 0);
        addString(";---------------------------------------------------------------- ANM_OBJ\n");
        addString("; " + QString(pObj->getName().toUtf8()) + "\n");
        addString("anmobj" + QString("%1").arg(i) + ":\n");
        if (pObj->childCount())
        {
            if (pObj->getLoop())
            {
                addString("\t\t\tdd\t\t00000001h\t\t; bFlag\n");
            }
            else
            {
                addString("\t\t\tdd\t\t00000000h\t\t; bFlag\n");
            }
            // 最小矩形算出
            for (int j = 0; j < pObj->childCount(); j++)
            {
                ObjectItem *pChild = pObj->child(j);
                makeFromEditDataArea(pChild, &qv4AreaMin, &qv4AreaMax, true);
            }
            addString("\t\t\tdd\t\t" + QString("F32(%1), F32(%2), F32(%3), F32(%4)").arg(qv4AreaMin.x(), 0, 'f').arg(qv4AreaMin.y(), 0, 'f').arg(qv4AreaMax.x(), 0, 'f').arg(qv4AreaMax.y(), 0, 'f') + "\t\t; fvArea\n");
            addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
            addString("\t\t\tdd\t\t.tips\t\t; papTip\n");
            addString("\t\n");
            addString("\t.tips:\n");
            for (int j = 0; j < pObj->childCount(); j++)
            {
                addString("\t\t\tdd\t\tobj" + QString("%1").arg(i) + "tip" + QString("%1").arg(j) + "\n");
            }
            addString("\n");
            for (int j = 0; j < pObj->childCount(); j++)
            {
                QString qsLabel = "obj" + QString("%1").arg(i) + "tip" + QString("%1").arg(j);
                ObjectItem *pChild = pObj->child(j);
                if (!makeFromEditDataTip(qsLabel, pChild))
                {
                    return false;
                }
            }
        }
        else
        {
            addString("\t\t\tdd\t\t00000000h\t\t; bFlag\n");
            addString("\t\t\tdd\t\t0, 0, 0, 0\t\t; fvArea\n");
            addString("\t\t\tdd\t\t" + QString("%1").arg(pObj->childCount()) + "\t\t; nTip\n");
            addString("\t\t\tdd\t\tNO_READ\t\t; papTip\n");
        }
    }
    addString("\n");

    return true;
}

void Anm2DAsm::makeFromEditData2IncTip(QString qsLabel, ObjectItem *pObj)
{
    QString qsTmp = qsLabel + pObj->getName().replace(" ", "_").toUpper().toUtf8();
    bool isAscii = true;
    for (QChar *pszData = qsTmp.data(); *pszData != '\0'; pszData++)
    {
        if (*pszData >= 0x80)
        {
            isAscii = false;
            break;
        }
    }
    if (isAscii)
    {
        QVector4D qv4AreaMin = QVector4D(FLT_MAX, FLT_MAX, FLT_MAX, 0);
        QVector4D qv4AreaMax = QVector4D(FLT_MIN, FLT_MIN, FLT_MIN, 0);
        addString("%define\t\t" + qsTmp + QString("\t\t%1\n").arg(m_nCnt));
        makeFromEditDataArea(pObj, &qv4AreaMin, &qv4AreaMax, false);
        addString("\t\t; Layer:" + QString("(%1, %2, %3)-(%4, %5, %6) size:(%7, %8, %9)").arg(qv4AreaMin.x(), 0, 'f').arg(qv4AreaMin.y(), 0, 'f').arg(qv4AreaMin.z(), 0, 'f').arg(qv4AreaMax.x(), 0, 'f').arg(qv4AreaMax.y(), 0, 'f').arg(qv4AreaMax.z(), 0, 'f').arg(qv4AreaMax.x() - qv4AreaMin.x(), 0, 'f').arg(qv4AreaMax.y() - qv4AreaMin.y(), 0, 'f').arg(qv4AreaMax.z() - qv4AreaMin.z(), 0, 'f') + "\n");
    }
    m_nCnt++;

    // 子供の処理
    if (pObj->childCount())
    {
        for (int i = 0; i < pObj->childCount(); i++)
        {
            ObjectItem *pChild = pObj->child(i);
            makeFromEditData2IncTip(qsLabel, pChild);
        }
    }
}

bool Anm2DAsm::makeFromEditData2Inc(EditData &rEditData, QString qsFname)
{
    qsFname = QFileInfo(qsFname).baseName().toUpper();
    m_pModel = rEditData.getObjectModel();
    if (m_bFlat)
    {
        ObjectModel *p = new ObjectModel();
        p->copy(m_pModel);
        p->flat();
        m_pModel = p;
    }

    ObjectItem *pRoot = m_pModel->getItemFromIndex(QModelIndex());
    //	ObjectItem	*pObj = pRoot->child(0);

    addString("; このファイルはAnimationCreatorにより生成されました。\n");
    addString("\n");
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        ObjectItem *pObj = pRoot->child(i);
        addString("%define\t\tACO_" + qsFname + "__" + pObj->getName().replace(" ", "_").toUpper().toUtf8() + QString("\t\t%1").arg(i) + "\n");
    }
    addString("\n");
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        ObjectItem *pObj = pRoot->child(i);
        QVector4D qv4AreaMin = QVector4D(FLT_MAX, FLT_MAX, FLT_MAX, 0);
        QVector4D qv4AreaMax = QVector4D(FLT_MIN, FLT_MIN, FLT_MIN, 0);
        QString qsLabel = "ACL_" + qsFname + "__" + pObj->getName().replace(" ", "_").toUpper().toUtf8() + "__";
        addString("%define\t\t" + qsLabel + "ROOT\t\t0\n");
        makeFromEditDataArea(pObj, &qv4AreaMin, &qv4AreaMax, true);
        addString("\t\t; Area:" + QString("(%1, %2, %3)-(%4, %5, %6) size:(%7, %8, %9)").arg(qv4AreaMin.x(), 0, 'f').arg(qv4AreaMin.y(), 0, 'f').arg(qv4AreaMin.z(), 0, 'f').arg(qv4AreaMax.x(), 0, 'f').arg(qv4AreaMax.y(), 0, 'f').arg(qv4AreaMax.z(), 0, 'f').arg(qv4AreaMax.x() - qv4AreaMin.x(), 0, 'f').arg(qv4AreaMax.y() - qv4AreaMin.y(), 0, 'f').arg(qv4AreaMax.z() - qv4AreaMin.z(), 0, 'f') + "\n");
        m_nCnt = 1;
        for (int j = 0; j < pObj->childCount(); j++)
        {
            ObjectItem *pChild = pObj->child(j);
            makeFromEditData2IncTip(qsLabel, pChild);
        }
        addString("\n");
    }

    return true;
}

void Anm2DAsm::addString(QString str, int tab)
{
    QString t;
    for (int i = 0; i < tab; i++)
    {
        t += "  ";
    }
    m_Data += t + str;
}
