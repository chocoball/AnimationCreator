#include "brightlabel.h"

BrightLabel::BrightLabel(QWidget *parent)
    : QLabel(parent)
{
}

BrightLabel::~BrightLabel()
{
}

void BrightLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (this->pixmap())
    {
        painter.drawImage(QPoint(0, 0), this->pixmap()->toImage());
    }
    painter.setPen(QPen(QColor(255, 255, 255, 255)));
    painter.drawArc(m_SelectPos.x() - 5, m_SelectPos.y() - 5, 10, 10, 0, 360 * 16);
}
