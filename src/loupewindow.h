#ifndef LOUPEWINDOW_H
#define LOUPEWINDOW_H

#include "editdata.h"
#include "setting.h"
#include <QWidget>
#include <QtGui>

class MainWindow;

class LoupeWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoupeWindow(CEditData *pEditData, CSettings *pSetting, QWidget *parent = 0);

    bool keyPress(QKeyEvent *event);

signals:

public slots:
    void slot_cursorScreenShort();
    void slot_changeScale(QString str);

protected:
    void resizeEvent(QResizeEvent *event);

    void fixImage(QSize &size);
    void toggleLock(void);

private:
    CEditData *m_pEditData;
    CSettings *m_pSetting;
    QLabel *m_pLabel;
    MainWindow *m_pMainWindow;
    QCheckBox *m_pCheckBox_Cursor, *m_pCheckBox_Center;

    QTimer *m_pTimer;
    int m_Scale;

    QPoint m_CenterPos;
};

#endif // LOUPEWINDOW_H
