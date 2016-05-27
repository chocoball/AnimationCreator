#ifndef OBJECTSCALEFORM_H
#define OBJECTSCALEFORM_H

#include "editdata.h"
#include "defines.h"
#include <QWidget>

namespace Ui
{
class ObjectScaleForm;
}

class ObjectScaleForm : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectScaleForm(EditData *pEditData, QWidget *parent = 0);
    ~ObjectScaleForm();

public slots:
    void slot_clickedExec();

private:
    Ui::ObjectScaleForm *ui;
    EditData *m_pEditData;
};

#endif // OBJECTSCALEFORM_H
