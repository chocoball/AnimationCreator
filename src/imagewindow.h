#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include "editdata.h"
#include "setting.h"
#include "ui_imagewindow.h"
#include <QScrollArea>
#include <QtGui>

class CGridLabel;
class AnimationForm;
class MainWindow;

namespace Ui
{
class ImageWindow;
}

class ImageWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWindow(CSettings *p, EditData *pEditImage, AnimationForm *pAnimForm, MainWindow *pMainWindow, QWidget *parent = 0);
    ~ImageWindow();

    void setAnimationForm(AnimationForm *p)
    {
        m_pAnimationForm = p;
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void addTab(int imageIndex);

    void contextMenuEvent(QContextMenuEvent *event);

    void updateGridLabel(void);

    void resizeEvent(QResizeEvent *event);

    int getFreeTabIndex(void);

signals:
    void sig_addImage(int imageNo);
    void sig_delImage(int imageNo);

public slots:
    void slot_delImage(void);
    void slot_modifiedImage(int index);

    void slot_changeUVBottom(double val);
    void slot_changeUVTop(double val);
    void slot_changeUVLeft(double val);
    void slot_changeUVRight(double val);
    void slot_setUI(RectF rect);

    void slot_endedOption(void);
    void slot_changeDrawCenter(bool flag);
    void slot_dragedImage(FrameData data);

    void slot_changeTab(int nImage);

    void slot_clickedScaleButton();

private:
    Ui::ImageWindow *ui;

    CSettings *m_pSetting;
    EditData *m_pEditData;

    AnimationForm *m_pAnimationForm;

    QAction *m_pActDelImage;

    MainWindow *m_pMainWindow;

    QSize m_oldWinSize;
};

#endif // IMAGEWINDOW_H
