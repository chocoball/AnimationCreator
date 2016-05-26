#ifndef EXPORTPNGFORM_H
#define EXPORTPNGFORM_H

#include "editdata.h"
#include "setting.h"
#include <QWidget>
#include <QtGui>

namespace Ui
{
class ExportPNGForm;
}

class ExportPNGForm : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPNGForm(EditData *pEditData, CSettings *pSetting, QWidget *parent = 0);
    ~ExportPNGForm();

signals:
    void sig_changeRect(void);
    void sig_startExport(void);
    void sig_cancel(void);

public slots:
    void resizeEvent(QResizeEvent *event);

    void slot_changeRect(void);
    void slot_openSaveDir(void);

    void slot_changeLeft(int val);
    void slot_changeRight(int val);
    void slot_changeTop(int val);
    void slot_changeBottom(int val);

    void slot_startExport(void);
    void slot_changeSaveDir(void);

    void slot_cancel(void);

private:
    Ui::ExportPNGForm *ui;
    EditData *m_pEditData;
    CSettings *m_pSetting;
};

#endif // EXPORTPNGFORM_H
