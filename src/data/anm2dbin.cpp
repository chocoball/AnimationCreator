#include "anm2dbin.h"

/************************************************************
*
* Anm2DBin
*
************************************************************/
Anm2DBin::Anm2DBin()
    : Anm2DBase()
{
}

// 作成中のデータをアニメデータに変換
bool Anm2DBin::makeFromEditData(EditData &rEditData)
{
    QByteArray header;
    QList<QByteArray> objectList;
    QList<QByteArray> layerList;
    QList<QByteArray> frameList;
    QList<QByteArray> imageList;

    ObjectItem *pRoot = rEditData.getObjectModel()->getItemFromIndex(QModelIndex());
    for (int i = 0; i < pRoot->childCount(); i++)
    {
        if (!makeObject(pRoot->child(i), objectList, layerList, frameList))
        {
            return false;
        }
    }

    if (!makeHeader(header, rEditData, objectList, layerList, frameList))
    {
        return false;
    }
    if (!makeImageList(imageList, rEditData))
    {
        return false;
    }

    // ヘッダのオフセットを設定
    Anm2DHeader *pHeader = (Anm2DHeader *)header.data();
    unsigned int offset = 0;
    unsigned int blockCnt = 0;
    offset += header.size();
    for (int i = 0; i < objectList.size(); i++)
    {
        pHeader->nBlockOffset[blockCnt++] = offset;
        offset += objectList[i].size();
    }
    for (int i = 0; i < layerList.size(); i++)
    {
        pHeader->nBlockOffset[blockCnt++] = offset;
        offset += layerList[i].size();
    }
    for (int i = 0; i < frameList.size(); i++)
    {
        pHeader->nBlockOffset[blockCnt++] = offset;
        offset += frameList[i].size();
    }
    for (int i = 0; i < imageList.size(); i++)
    {
        pHeader->nBlockOffset[blockCnt++] = offset;
        offset += imageList[i].size();
    }
    pHeader->nFileSize = offset;

    if (pHeader->nBlockNum != blockCnt)
    {
        m_nError = kErrorNo_BlockNumNotSame;
        return false;
    }

    // データセット
    m_Data.clear();
    m_Data += header;
    for (int i = 0; i < objectList.size(); i++)
    {
        m_Data += objectList[i];
    }
    for (int i = 0; i < layerList.size(); i++)
    {
        m_Data += layerList[i];
    }
    for (int i = 0; i < frameList.size(); i++)
    {
        m_Data += frameList[i];
    }
    for (int i = 0; i < imageList.size(); i++)
    {
        m_Data += imageList[i];
    }
    return true;
}

// アニメファイルから作成中データへ変換
bool Anm2DBin::makeFromFile(QByteArray &data, EditData &rEditData)
{
    m_Data = data;

    Anm2DHeader *pHeader = (Anm2DHeader *)m_Data.data();
    if (pHeader->header.nID != kANM2D_ID_HEADER)
    {
        m_nError = kErrorNo_InvalidID;
        return false;
    }
    if (pHeader->nFileSize != (unsigned int)m_Data.size())
    {
        m_nError = kErrorNo_InvalidFileSize;
        return false;
    }
    if (pHeader->nVersion != kANM2D_VERSION)
    {
        m_nError = kErrorNo_InvalidVersion;
        return false;
    }
    if (!addList(pHeader))
    {
        return false;
    }
    if (!addModel(rEditData))
    {
        return false;
    }
    if (!addImageData(pHeader, rEditData))
    {
        return false;
    }

    return true;
}

// ヘッダ作成
bool Anm2DBin::makeHeader(QByteArray &rData, EditData &rEditData, QList<QByteArray> &objectList, QList<QByteArray> &layerList, QList<QByteArray> &frameList)
{
    int blockNum = 0;
    blockNum += objectList.size();
    blockNum += layerList.size();
    blockNum += frameList.size();
    blockNum += rEditData.getImageDataListSize();

    rData.resize(sizeof(Anm2DHeader) + (blockNum - 1) * sizeof(unsigned int));
    Anm2DHeader *pHeader = (Anm2DHeader *)rData.data();
    memset(pHeader, 0, sizeof(Anm2DHeader) + (blockNum - 1) * sizeof(unsigned int));
    pHeader->header.nID = kANM2D_ID_HEADER;
    pHeader->header.nSize = sizeof(Anm2DHeader);
    pHeader->nVersion = kANM2D_VERSION;
    pHeader->nFileSize = 0; // 後で入れる
    pHeader->nBlockNum = blockNum;
    return true;
}

bool Anm2DBin::makeObject(ObjectItem *pObj, QList<QByteArray> &objList, QList<QByteArray> &layerList, QList<QByteArray> &frameList)
{
    int layerNum = pObj->childCount();

    QByteArray objArray;
    objArray.resize(sizeof(Anm2DObject) + (layerNum - 1) * sizeof(unsigned int));
    Anm2DObject *pObjData = (Anm2DObject *)objArray.data();
    memset(pObjData, 0, sizeof(Anm2DObject) + (layerNum - 1) * sizeof(unsigned int));
    pObjData->header.nID = kANM2D_ID_OBJECT;
    pObjData->header.nSize = sizeof(Anm2DObject) + (layerNum - 1) * sizeof(unsigned int);
    strncpy(pObjData->objName.name, pObj->getName().toUtf8().data(), sizeof(Anm2DName)); // オブジェクト名
    pObjData->nLayerNum = layerNum;
    pObjData->nLoopNum = pObj->getLoop(); // ループ回数(after ver 0.1.0)

    int layerNo = 0;
    for (int i = 0; i < pObj->childCount(); i++)
    {
        if (!makeLayer(pObj->child(i), &layerNo, pObjData, layerList, frameList, -1))
        {
            return false;
        }
    }

    objList << objArray;
    return true;
}

bool Anm2DBin::makeLayer(ObjectItem *pLayer,
                         int *pLayerNo,
                         Anm2DObject *pObjData,
                         QList<QByteArray> &layerList,
                         QList<QByteArray> &frameList,
                         int parentNo)
{
    const QList<FrameData> &frameDatas = pLayer->getFrameData();
    QByteArray layerArray;
    layerArray.resize(sizeof(Anm2DLayer) + (frameDatas.size() - 1) * sizeof(unsigned int));
    Anm2DLayer *pLayerData = (Anm2DLayer *)layerArray.data();
    memset(pLayerData, 0, sizeof(Anm2DLayer) + (frameDatas.size() - 1) * sizeof(unsigned int));
    pLayerData->header.nID = kANM2D_ID_LAYER;
    pLayerData->header.nSize = sizeof(Anm2DLayer) + (frameDatas.size() - 1) * sizeof(unsigned int);
    strncpy(pLayerData->layerName.name, pLayer->getName().toUtf8().data(), sizeof(Anm2DName)); // レイヤ名
    pLayerData->nLayerNo = layerList.size();
    pLayerData->nFrameDataNum = frameDatas.size();
    pLayerData->nParentNo = parentNo;

    if (pObjData)
    {
        pObjData->nLayerNo[*pLayerNo] = pLayerData->nLayerNo; // オブジェクトが参照するレイヤ番号セット
        *pLayerNo += 1;
    }

    for (int i = 0; i < frameDatas.size(); i++)
    {
        const FrameData &data = frameDatas.at(i);

        QByteArray frameDataArray;
        frameDataArray.resize(sizeof(Anm2DFrameData));
        Anm2DFrameData *pFrameData = (Anm2DFrameData *)frameDataArray.data();
        memset(pFrameData, 0, sizeof(Anm2DFrameData));
        pFrameData->header.nID = kANM2D_ID_FRAMEDATA;
        pFrameData->header.nSize = sizeof(Anm2DFrameData);
        pFrameData->nFrameDataNo = frameList.size();
        pFrameData->nFrame = data.frame;
        pFrameData->pos_x = data.pos_x;
        pFrameData->pos_y = data.pos_y;
        pFrameData->pos_z = data.pos_z;
        pFrameData->rot_x = data.rot_x;
        pFrameData->rot_y = data.rot_y;
        pFrameData->rot_z = data.rot_z;
        pFrameData->cx = data.center_x;
        pFrameData->cy = data.center_y;
        pFrameData->nImageNo = data.nImage;
        pFrameData->uv[0] = data.left;
        pFrameData->uv[1] = data.right;
        pFrameData->uv[2] = data.top;
        pFrameData->uv[3] = data.bottom;
        pFrameData->fScaleX = data.fScaleX;
        pFrameData->fScaleY = data.fScaleY;
        pFrameData->bFlag = (data.bUVAnime) ? 1 : 0;
        // 頂点色(after ver 0.1.0)
        pFrameData->rgba[0] = data.rgba[0];
        pFrameData->rgba[1] = data.rgba[1];
        pFrameData->rgba[2] = data.rgba[2];
        pFrameData->rgba[3] = data.rgba[3];

        pLayerData->nFrameDataNo[i] = pFrameData->nFrameDataNo; // レイヤが参照するフレームデータ番号セット

        frameList << frameDataArray;
    }

    layerList << layerArray;

    for (int i = 0; i < pLayer->childCount(); i++)
    {
        if (!makeLayer(pLayer->child(i), pLayerNo, NULL, layerList, frameList, pLayerData->nLayerNo))
        {
            return false;
        }
    }
    return true;
}

// イメージデータ作成
bool Anm2DBin::makeImageList(QList<QByteArray> &rData, EditData &rEditData)
{
    int imageNum = rEditData.getImageDataListSize();

    for (int i = 0; i < imageNum; i++)
    {
        EditData::ImageData *p = rEditData.getImageData(i);
        if (!p)
        {
            continue;
        }

        QImage img = p->Image;
        unsigned int size = sizeof(Anm2DImage) + img.width() * img.height() * 4 * sizeof(unsigned char) - 1;
        size = (size + 0x03) & ~0x03;

        QByteArray imgArray;
        imgArray.resize(size);
        Anm2DImage *pImage = (Anm2DImage *)imgArray.data();
        memset(pImage, 0, size);
        pImage->header.nID = kANM2D_ID_IMAGE;
        pImage->header.nSize = size;
        pImage->nWidth = img.width();
        pImage->nHeight = img.height();
        pImage->nImageNo = p->nNo;
        QString relPath = getRelativePath(m_filePath, p->fileName);
        strncpy(pImage->fileName, relPath.toUtf8().data(), 255);
        memcpy(pImage->data, img.bits(), img.width() * img.height() * 4);

        rData << imgArray;
    }
    return true;
}

bool Anm2DBin::addList(Anm2DHeader *pHeader)
{
    for (int i = 0; i < pHeader->nBlockNum; i++)
    {
        Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]);
        switch (p->nID)
        {
            case kANM2D_ID_OBJECT:
                m_ObjPtrList.append((Anm2DObject *)p);
                break;
            case kANM2D_ID_LAYER:
                m_LayerPtrList.append((Anm2DLayer *)p);
                break;
            case kANM2D_ID_FRAMEDATA:
                m_FrameDataPtrList.append((Anm2DFrameData *)p);
                break;
            case kANM2D_ID_IMAGE:
                continue;

            default:
                m_nError = kErrorNo_InvalidID;
                return false;
        }
    }
    return true;
}

bool Anm2DBin::addModel(EditData &rEditData)
{
    ObjectModel *pModel = rEditData.getObjectModel();

    for (int i = 0; i < m_ObjPtrList.size(); i++)
    {
        const Anm2DObject *pObj = m_ObjPtrList.at(i);
        QModelIndex index = pModel->addItem(QString(pObj->objName.name), QModelIndex());
        ObjectItem *pItem = pModel->getItemFromIndex(index);
        pItem->setLoop(pObj->nLoopNum);

        for (int j = 0; j < pObj->nLayerNum; j++)
        {
            if (!addModel_Layer(index, pObj->nLayerNo[j], pModel))
            {
                return false;
            }
        }
    }
    return true;
}

bool Anm2DBin::addModel_Layer(QModelIndex &parent, int layerNo, ObjectModel *pModel)
{
    if (layerNo < 0 || layerNo >= m_LayerPtrList.size())
    {
        m_nError = kErrorNo_InvalidLayerNum;
        return false;
    }

    const Anm2DLayer *pLayer = m_LayerPtrList.at(layerNo);
    QModelIndex index = pModel->addItem(QString(pLayer->layerName.name), parent);
    ObjectItem *pItem = pModel->getItemFromIndex(index);
    pItem->setData(ObjectItem::kState_Disp, Qt::CheckStateRole);

    for (int i = 0; i < pLayer->nFrameDataNum; i++)
    {
        const Anm2DFrameData *pFrame = m_FrameDataPtrList.at(pLayer->nFrameDataNo[i]);
        FrameData data;
        data.frame = pFrame->nFrame;
        data.pos_x = pFrame->pos_x;
        data.pos_y = pFrame->pos_y;
        data.pos_z = pFrame->pos_z;
        data.rot_x = pFrame->rot_x;
        data.rot_y = pFrame->rot_y;
        data.rot_z = pFrame->rot_z;
        data.center_x = pFrame->cx;
        data.center_y = pFrame->cy;
        data.nImage = pFrame->nImageNo;
        data.left = pFrame->uv[0];
        data.right = pFrame->uv[1];
        data.top = pFrame->uv[2];
        data.bottom = pFrame->uv[3];
        data.fScaleX = pFrame->fScaleX;
        data.fScaleY = pFrame->fScaleY;
        data.bUVAnime = pFrame->bFlag ? true : false;
        data.rgba[0] = pFrame->rgba[0];
        data.rgba[1] = pFrame->rgba[1];
        data.rgba[2] = pFrame->rgba[2];
        data.rgba[3] = pFrame->rgba[3];

        pItem->addFrameData(data);
    }

    QList<int> childList;
    for (int i = 0; i < m_LayerPtrList.size(); i++)
    {
        if (m_LayerPtrList.at(i)->nParentNo == layerNo)
        {
            childList << i;
        }
    }
    for (int i = 0; i < childList.size(); i++)
    {
        if (!addModel_Layer(index, childList[i], pModel))
        {
            return false;
        }
    }
    return true;
}

// 編集データにイメージを追加
bool Anm2DBin::addImageData(Anm2DHeader *pHeader, EditData &rEditData)
{
    QList<EditData::ImageData> data;
    for (int i = 0; i < pHeader->nBlockNum; i++)
    {
        Anm2DBlockHeader *p = (Anm2DBlockHeader *)LP_ADD(pHeader, pHeader->nBlockOffset[i]);
        switch (p->nID)
        {
            case kANM2D_ID_IMAGE:
            {
                Anm2DImage *pImage = (Anm2DImage *)p;

                QImage image;
                QString fileName = QString::fromUtf8(pImage->fileName);
                QString path = getAbsolutePath(m_filePath, fileName);
                if (!image.load(path))
                {
                    qDebug() << "not load:" << path << fileName;
                    image = QImage(pImage->nWidth, pImage->nHeight, QImage::Format_ARGB32);
                    unsigned int *pCol = (unsigned int *)pImage->data;
                    for (int y = 0; y < pImage->nHeight; y++)
                    {
                        for (int x = 0; x < pImage->nWidth; x++)
                        {
                            image.setPixel(x, y, *pCol);
                            pCol++;
                        }
                    }
                }

                qDebug("w:%d h:%d, %d %d", image.width(), image.height(), pImage->nWidth, pImage->nHeight);

                EditData::ImageData ImageData;
                ImageData.Image = image;
                ImageData.nTexObj = 0;
                ImageData.fileName = path;
                ImageData.lastModified = QDateTime::currentDateTimeUtc();
                ImageData.nNo = pImage->nImageNo;
                data.insert(pImage->nImageNo, ImageData);
            }
            break;
            case kANM2D_ID_OBJECT:
            case kANM2D_ID_LAYER:
            case kANM2D_ID_FRAMEDATA:
                continue;

            default:
                m_nError = kErrorNo_InvalidID;
                return false;
        }
    }
    rEditData.setImageData(data);
    return true;
}

// 名前からオブジェクトを探す
Anm2DObject *Anm2DBin::search2DObjectFromName(Anm2DHeader *pHeader, QString name)
{
    for (int i = 0; i < pHeader->nBlockNum; i++)
    {
        Anm2DObject *p = (Anm2DObject *)LP_ADD(pHeader, pHeader->nBlockOffset[i]);
        if (p->header.nID != kANM2D_ID_OBJECT)
        {
            continue;
        }
        if (name != QString::fromUtf8(p->objName.name))
        {
            continue;
        }

        return p;
    }
    return NULL;
}

// 名前からレイヤを探す
Anm2DLayer *Anm2DBin::search2DLayerFromName(Anm2DHeader *pHeader, QString name)
{
    for (int i = 0; i < pHeader->nBlockNum; i++)
    {
        Anm2DLayer *p = (Anm2DLayer *)LP_ADD(pHeader, pHeader->nBlockOffset[i]);
        if (p->header.nID != kANM2D_ID_LAYER)
        {
            continue;
        }
        if (name != QString::fromUtf8(p->layerName.name))
        {
            continue;
        }

        return p;
    }
    return NULL;
}
