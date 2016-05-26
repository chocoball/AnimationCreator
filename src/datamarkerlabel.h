#ifndef DATAMARKERLABEL_H
#define DATAMARKERLABEL_H

#include "editdata.h"
#include "include.h"
#include <QLabel>
#include <QScrollBar>

class DataMarkerLabel : public QLabel
{
    Q_OBJECT

public:
    enum
    {
        kMoveMode_None = 0,
        kMoveMode_Layer,
        kMoveMode_Object
    };

public:
    explicit DataMarkerLabel(QWidget *parent = 0);

    void setValue(int val);
    void setScrollBarSize();

    int value() { return m_value; }
    void setHorizontalBar(QScrollBar *pBar) { m_pHorizontalScrollBar = pBar; }

signals:
    void sig_changeValue(int val);
    void sig_moveFrameData(int prevFrame, int nextFrame);
    void sig_moveAllFrameData(int prevFrame, int nextFrame);

public slots:
    void slot_setFrameStart(int val);
    void slot_setFrameEnd(int val);
    void slot_moveScrollBar(int val);

    void slot_copyAllFrame();
    void slot_pasteAllFrame();
    void slot_deleteAllFrame();

protected:
    void paintEvent(QPaintEvent *event);
    void drawFrameBase(QPainter &painter);
    void drawLayer(const QModelIndex &index, QPainter &painter, const QModelIndex selIndex, QList<int> &frames);

    int getX(int frame, bool bAddOffset = true);

    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

    void fixOffset(void);

    void contextMenuEvent(QContextMenuEvent *ev);

    kAccessor(CEditData *, m_pEditData, EditData);

private:
    int m_value;
    int m_frameStart, m_frameEnd;
    int m_offset;
    QPoint m_oldMousePos;
    QScrollBar *m_pHorizontalScrollBar;

    QAction *m_pActCopyAllFrame;
    QAction *m_pActPasteAllFrame;
    QAction *m_pActDeleteAllFrame;

    bool m_bPressLeft;
    bool m_bMouseMove;

    int m_pressFrame, m_pressCurrentFrame;
    int m_moveMode;
};

#endif // DATAMARKERLABEL_H
