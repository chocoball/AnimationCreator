#ifndef CURVEEDITORFORM_H
#define CURVEEDITORFORM_H

#include "animationwindowsplitter.h"
#include "curvegraphlabel.h"
#include "editdata.h"
#include "include.h"
#include "setting.h"
#include <QWidget>

namespace Ui
{
class CurveEditorForm;
}

class CurveEditorForm : public QWidget
{
    Q_OBJECT

public:
    explicit CurveEditorForm(CEditData *pEditData, CSettings *pSetting, QWidget *parent = 0);
    ~CurveEditorForm();

    void setSplitterPos();

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void slot_clickedListView(QModelIndex index);
    void slot_changeSelLayer(QModelIndex);
    void slot_movedSlider(int val);
    void slot_movedSplitter(int pos, int index);

private:
    Ui::CurveEditorForm *ui;
    CEditData *m_pEditData;
    CSettings *m_pSetting;

    CurveGraphLabel *m_pGraphLabel;
    float m_fMag;

    AnimationWindowSplitter *m_pSplitter;
};

#endif // CURVEEDITORFORM_H
