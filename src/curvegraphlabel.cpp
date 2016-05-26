#include "curvegraphlabel.h"

#define kFrameNumWidth 20
#define kDataSubNumWidth 20

CurveGraphLabel::CurveGraphLabel(EditData *pEditData, QWidget *parent)
    : QLabel(parent)
{
    m_pEditData = pEditData;
    m_currDispType = kDispType_None;
}

// ラベルサイズ調整
void CurveGraphLabel::adjustSize()
{
    if (m_currDispType == kDispType_None)
    {
        return;
    }

    ObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(m_currIndex))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(m_currIndex);
    if (!pItem)
    {
        return;
    }

    QList<QPair<int, float> > datas = getDatasFromCurrentType(pItem);
    if (datas.size() < 1)
    {
        return;
    }

    // フレーム数の最小、最大
    QPair<int, int> frameMaxMin = qMakePair(datas.first().first, datas.last().first);
    // データ差分の最小、最大
    QPair<float, float> dataSubMaxMin = getDataSubMaxMin(datas);

    float dataAbs = fabs(dataSubMaxMin.first) > fabs(dataSubMaxMin.second) ? dataSubMaxMin.first : dataSubMaxMin.second;
    dataAbs = fabs(dataAbs);
    float dataSingleStep;
    if (dataAbs < 1)
    {
        dataSingleStep = 0.5;
        dataSubMaxMin.first = 1;
    }
    else if (dataAbs < 10)
    {
        dataSingleStep = 1;
        dataSubMaxMin.first = 10;
    }
    else if (dataAbs < 100)
    {
        dataSingleStep = 10;
        dataSubMaxMin.first = (int)(dataAbs / 10) + 10;
    }
    else
    {
        dataSingleStep = 100;
        dataSubMaxMin.first = (int)(dataAbs / 100) + 100;
    }
    dataSubMaxMin.second = -dataSubMaxMin.first;

    int w = (frameMaxMin.second + 20) * kFrameNumWidth + 20;
    if (w < this->parentWidget()->height())
    {
        w = this->parentWidget()->height();
    }
    int h = dataAbs / dataSingleStep * 2 * kDataSubNumWidth + 20;
    if (h < this->parentWidget()->height())
    {
        h = this->parentWidget()->height();
    }
    resize(w, h);

    qDebug() << "CurveGraphLabel size" << QSize(w, h);
}

// ペイントイベント
void CurveGraphLabel::paintEvent(QPaintEvent * /*event*/)
{
    if (m_currDispType == kDispType_None)
    {
        return;
    }

    ObjectModel *pModel = m_pEditData->getObjectModel();
    if (!pModel->isLayer(m_currIndex))
    {
        return;
    }
    ObjectItem *pItem = pModel->getItemFromIndex(m_currIndex);
    if (!pItem)
    {
        return;
    }

    m_currDispDatas = getDatasFromCurrentType(pItem);
    if (m_currDispDatas.size() < 1)
    {
        return;
    }

    // フレーム数の最小、最大
    QPair<int, int> frameMaxMin = qMakePair(m_currDispDatas.first().first, m_currDispDatas.last().first);
    // データ差分の最小、最大
    QPair<float, float> dataSubMaxMin = getDataSubMaxMin(m_currDispDatas);

    float dataAbs = fabs(dataSubMaxMin.first) > fabs(dataSubMaxMin.second) ? dataSubMaxMin.first : dataSubMaxMin.second;
    dataAbs = fabs(dataAbs);
    m_dataSingleStep = 0;
    if (dataAbs < 1)
    {
        m_dataSingleStep = 0.5;
        dataAbs = 1;
    }
    else if (dataAbs < 10)
    {
        m_dataSingleStep = 1;
        dataAbs = 10;
    }
    else if (dataAbs < 100)
    {
        m_dataSingleStep = 10;
        dataAbs = ((int)(dataAbs / 10) + 1) * 10;
    }
    else
    {
        m_dataSingleStep = 100;
        dataAbs = ((int)(dataAbs / 100) + 1) * 100;
    }
    int cnt = 0;
    float curr = 1;
    while (curr < dataAbs)
    {
        cnt++;
        curr += m_dataSingleStep;
    }
    m_dispStepH = (float)height() * 0.5f / (dataAbs / m_dataSingleStep);
    qDebug() << "dispStepH" << m_dispStepH << "singleStep" << m_dataSingleStep << "dataAbs" << dataAbs;

    QPainter painter(this);
    drawFrameNum(painter, frameMaxMin.second);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(255, 0, 0));
    drawDatas(painter);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QColor(0, 0, 0));
    drawDataSub(painter, dataAbs, m_dataSingleStep);
}

// 現在の表示タイプからデータ取得
QList<QPair<int, float> > CurveGraphLabel::getDatasFromCurrentType(ObjectItem *pLayer)
{
    QList<QPair<int, float> > ret;
    const QList<FrameData> data = pLayer->getFrameData();

    for (int i = 0; i < data.size(); i++)
    {
        const FrameData frame = data.at(i);
        switch (m_currDispType)
        {
            case kDispType_PosX:
                ret.append(qMakePair((int)frame.frame, (float)frame.pos_x));
                break;
            case kDispType_PosY:
                ret.append(qMakePair((int)frame.frame, (float)frame.pos_y));
                break;
            case kDispType_PosZ:
                ret.append(qMakePair((int)frame.frame, (float)frame.pos_z));
                break;
            case kDispType_RotX:
                ret.append(qMakePair((int)frame.frame, (float)frame.rot_x));
                break;
            case kDispType_RotY:
                ret.append(qMakePair((int)frame.frame, (float)frame.rot_y));
                break;
            case kDispType_RotZ:
                ret.append(qMakePair((int)frame.frame, (float)frame.rot_z));
                break;
            case kDispType_CenterX:
                ret.append(qMakePair((int)frame.frame, (float)frame.center_x));
                break;
            case kDispType_CenterY:
                ret.append(qMakePair((int)frame.frame, (float)frame.center_y));
                break;
            case kDispType_ScaleX:
                ret.append(qMakePair((int)frame.frame, (float)frame.fScaleX));
                break;
            case kDispType_ScaleY:
                ret.append(qMakePair((int)frame.frame, (float)frame.fScaleY));
                break;
            case kDispType_UvTop:
                ret.append(qMakePair((int)frame.frame, (float)frame.top));
                break;
            case kDispType_UvLeft:
                ret.append(qMakePair((int)frame.frame, (float)frame.left));
                break;
            case kDispType_UvRight:
                ret.append(qMakePair((int)frame.frame, (float)frame.right));
                break;
            case kDispType_UvBottom:
                ret.append(qMakePair((int)frame.frame, (float)frame.bottom));
                break;
            case kDispType_ColorR:
                ret.append(qMakePair((int)frame.frame, (float)frame.rgba[0]));
                break;
            case kDispType_ColorG:
                ret.append(qMakePair((int)frame.frame, (float)frame.rgba[1]));
                break;
            case kDispType_ColorB:
                ret.append(qMakePair((int)frame.frame, (float)frame.rgba[2]));
                break;
            case kDispType_ColorA:
                ret.append(qMakePair((int)frame.frame, (float)frame.rgba[3]));
                break;
            default:
                return ret;
        }
    }

    for (int i = 0; i < ret.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (ret.at(i).first < ret.at(j).first)
            {
                ret.swap(i, j);
            }
        }
    }
    return ret;
}

// データ差分の最小最大取得
QPair<float, float> CurveGraphLabel::getDataSubMaxMin(const QList<QPair<int, float> > &datas)
{
    float zeroData = datas.first().second;
    float min = 0;
    float max = 0;

    for (int i = 1; i < datas.size(); i++)
    {
        float data = datas.at(i).second - zeroData;
        if (data > min)
        {
            min = data;
        }
        if (data < max)
        {
            max = data;
        }
    }
    return qMakePair(min, max);
}

void CurveGraphLabel::drawFrameNum(QPainter &painter, int max)
{
    for (int i = 0; i < max + 20; i++)
    {
        if (mapToParent(QPoint(25 + i * kFrameNumWidth, 0)).x() < 25)
        {
            continue;
        }

        if (!(i % 10))
        {
            painter.drawText(QRect(25 + i * kFrameNumWidth + 2, height() - 10, 20, 10), QString("%1").arg(i));
            painter.setPen(QColor(128, 128, 0));
        }
        else
        {
            painter.setPen(QColor(0, 0, 0));
        }
        painter.drawLine(25 + i * kFrameNumWidth, 0, 25 + i * kFrameNumWidth, height());
    }
}

void CurveGraphLabel::drawDataSub(QPainter &painter, float dataSubAbs, float step)
{
    int ofs_x = mapFromParent(QPoint(0, 0)).x();

    painter.eraseRect(ofs_x, 0, 25, height());

    painter.setPen(QColor(128, 128, 0));

    int cnt = 0;
    float curr = 0;
    while (curr < dataSubAbs)
    {
        painter.drawLine(20 + ofs_x, height() / 2 - cnt * m_dispStepH, width(), height() / 2 - cnt * m_dispStepH);
        if (!cnt)
        {
            painter.setPen(QColor(0, 0, 0));
        }

        painter.drawText(QRect(1 + ofs_x, height() / 2 - cnt * m_dispStepH - 5, 20, 10), QString("%1").arg(curr));
        if (cnt)
        {
            painter.drawLine(20 + ofs_x, height() / 2 + cnt * m_dispStepH, width(), height() / 2 + cnt * m_dispStepH);
            painter.drawText(QRect(1 + ofs_x, height() / 2 + cnt * m_dispStepH - 5, 20, 10), QString("%1").arg(-curr));
        }
        curr += step;
        cnt++;
    }
}

void CurveGraphLabel::drawDatas(QPainter &painter)
{
    const QPair<int, float> first = m_currDispDatas.at(0);
    QPoint oldPos = QPoint(25 + first.first * kFrameNumWidth, height() / 2);

    painter.fillRect(oldPos.x() - 2, oldPos.y() - 2, 4, 4, painter.pen().color());
    for (int i = 1; i < m_currDispDatas.size(); i++)
    {
        const QPair<int, float> d = m_currDispDatas.at(i);
        QPoint pos = QPoint(25 + d.first * kFrameNumWidth, height() / 2 + (first.second - d.second) / m_dataSingleStep * m_dispStepH);
        painter.drawLine(oldPos, pos);
        painter.fillRect(pos.x() - 2, pos.y() - 2, 4, 4, painter.pen().color());
        oldPos = pos;
    }
}
