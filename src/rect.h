#ifndef RECT_H
#define RECT_H

#include <QDataStream>
#include <QPoint>
#include <QRect>

class RectF
{
public:
    RectF()
    {
        m_left = m_right = m_top = m_bottom = 0;
    }
    RectF(float left, float top, float right, float bottom)
    {
        m_left = left;
        m_top = top;
        m_right = right;
        m_bottom = bottom;
    }
    float left() const { return m_left; }
    float right() const { return m_right; }
    float top() const { return m_top; }
    float bottom() const { return m_bottom; }
    float width() const { return m_right - m_left; }
    float height() const { return m_bottom - m_top; }

    void setLeft(float val) { m_left = val; }
    void setRight(float val) { m_right = val; }
    void setTop(float val) { m_top = val; }
    void setBottom(float val) { m_bottom = val; }

    void setRect(float x, float y, float w, float h)
    {
        m_left = x;
        m_top = y;
        m_right = x + w;
        m_bottom = y + h;
    }

    bool operator==(const RectF &r) const
    {
        if (m_left == r.m_left && m_right == r.m_right && m_top == r.m_top && m_bottom == r.m_bottom)
        {
            return true;
        }
        return false;
    }
    bool operator!=(const RectF &r) const
    {
        if (*this == r)
        {
            return false;
        }
        return true;
    }

    bool contains(const QPoint &pos) const
    {
        float l, r, t, b;
        if (m_left > m_right)
        {
            l = m_right;
            r = m_left;
        }
        else
        {
            l = m_left;
            r = m_right;
        }
        if (m_top > m_bottom)
        {
            t = m_bottom;
            b = m_top;
        }
        else
        {
            t = m_top;
            b = m_bottom;
        }
        if (pos.x() > l && pos.x() < r && pos.y() > t && pos.y() < b)
        {
            return true;
        }
        return false;
    }
    QRect toRect()
    {
        return QRect(m_left, m_top, m_right - m_left, m_bottom - m_top);
    }

private:
    float m_left, m_right, m_top, m_bottom;
};

#endif // RECT_H
