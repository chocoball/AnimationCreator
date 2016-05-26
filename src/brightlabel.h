#ifndef BRIGHTLABEL_H
#define BRIGHTLABEL_H

#include <QLabel>
#include <QWidget>
#include <QtGui>

class BrightLabel : public QLabel
{
public:
    explicit BrightLabel(QWidget *parent = 0);
    virtual ~BrightLabel();

    void setSelectPos(QPoint p)
    {
        m_SelectPos = p;
    }

protected:
    void paintEvent(QPaintEvent *);

private:
    QPoint m_SelectPos;
};

#endif // BRIGHTLABEL_H
