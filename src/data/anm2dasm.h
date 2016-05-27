#ifndef ANM2DASM_H
#define ANM2DASM_H

#include "../editdata.h"
#include "anm2dbase.h"

#define KM_VRAM_MAX 32

// asm形式
class Anm2DAsm : public Anm2DBase
{
public:
    Anm2DAsm(bool bFlat);
    ~Anm2DAsm();

    bool makeFromEditDataTip(QString qsLabel, ObjectItem *pObj);
    void subUnusedVramSkip(ObjectItem *pObj);
    bool makeFromEditData(EditData &rEditData);
    void makeFromEditData2IncTip(QString qsLabel, ObjectItem *pObj);
    bool makeFromEditData2Inc(EditData &rEditData, QString qsFname);

    QString getData() { return m_Data; }

private:
    void addString(QString str, int tab = 0);

private:
    bool m_bFlat;
    ObjectModel *m_pModel;
    QString m_Data;
    QString m_aqsVramID[KM_VRAM_MAX];
    bool m_bUnused[KM_VRAM_MAX];
    int m_nCnt;
};

#endif // ANM2DASM_H
