#ifndef ANM2DJSON_H
#define ANM2DJSON_H

#include "../editdata.h"
#include "anm2dbase.h"

// JSON形式
class Anm2DJson : public Anm2DBase
{
public:
    Anm2DJson(bool bFlat);
    ~Anm2DJson();

    bool makeFromEditData(EditData &rEditData);
    bool makeFromFile(QString &, EditData &) { return false; }

    QString getData() { return m_Data; }

private:
    void addString(QString str, int tab = 0);
    bool makeObject(EditData &rEditData);
    bool makeLayer(ObjectItem *pItem, EditData &rEditData, int tab);

private:
    bool m_bFlat;
    ObjectModel *m_pModel;
    QString m_Data;
    int m_ObjNum, m_ImageNum;
};

#endif // ANM2DJSON_H
