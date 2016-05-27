#ifndef ANM2DBIN_H
#define ANM2DBIN_H

#include "../editdata.h"
#include "anm2dbase.h"
#include "anm2dtypes.h"

// binary
class Anm2DBin : public Anm2DBase
{
public:
    Anm2DBin();

    bool makeFromEditData(EditData &rEditData);
    bool makeFromFile(QByteArray &data, EditData &rEditData);

    QByteArray &getData() { return m_Data; }

private:
    static bool makeHeader(QByteArray &rData, EditData &rEditData, QList<QByteArray> &objectList, QList<QByteArray> &layerList, QList<QByteArray> &frameList);
    bool makeObject(ObjectItem *pObj, QList<QByteArray> &objList, QList<QByteArray> &layerList, QList<QByteArray> &frameList);
    bool makeLayer(ObjectItem *pLayer, int *pLayerNo, Anm2DObject *pObjData, QList<QByteArray> &layerList, QList<QByteArray> &frameList, int parentNo);
    bool makeImageList(QList<QByteArray> &rData, EditData &rEditData);

    bool addList(Anm2DHeader *pHeader);
    bool addModel(EditData &rEditData);
    bool addModel_Layer(QModelIndex &parent, int layerNo, ObjectModel *pModel);
    bool addImageData(Anm2DHeader *pHeader, EditData &rEditData);

    Anm2DObject *search2DObjectFromName(Anm2DHeader *pHeader, QString name);
    Anm2DLayer *search2DLayerFromName(Anm2DHeader *pHeader, QString name);

private:
    QByteArray m_Data;
    QList<Anm2DObject *> m_ObjPtrList;
    QList<Anm2DLayer *> m_LayerPtrList;
    QList<Anm2DFrameData *> m_FrameDataPtrList;
};

#endif // ANM2DBIN_H
