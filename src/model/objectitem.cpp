#include "objectitem.h"
#include "../editdata.h"

ObjectItem *ObjectItem::child(int row)
{
    if (row < 0 || row >= m_children.size())
    {
        return NULL;
    }
    return m_children[row];
}
void ObjectItem::insertChild(int row, ObjectItem *p)
{
    m_children.insert(row, p);
}
void ObjectItem::removeChild(ObjectItem *p)
{
    int index = m_children.indexOf(p);
    if (index < 0)
    {
        return;
    }
    m_children.removeAt(index);
    delete p;
}

void ObjectItem::flat()
{
    QList<ObjectItem *> layers = getLayers(this);
    QList<ObjectItem *> children;

    for (int i = 0; i < layers.size(); i++)
    {
        ObjectItem *p = new ObjectItem(layers[i]->getName(), layers[i]->parent());
        p->copy(layers[i]);
        p->applyFrameDataFromParent();
        p->m_pParent = this;

        children.append(p);
    }
    for (int i = 0; i < children.size(); i++)
    {
        qDeleteAll(children[i]->m_children);
        children[i]->m_children.clear();
    }

    qDeleteAll(m_children);
    m_children = children;
}

QList<ObjectItem *> ObjectItem::getLayers(ObjectItem *pObj)
{
    QList<ObjectItem *> ret;
    for (int i = 0; i < pObj->childCount(); i++)
    {
        ret << pObj->child(i);
        ret << getLayers(pObj->child(i));
    }
    return ret;
}

void ObjectItem::addFrameData(const FrameData &data)
{
    FrameData *p = getFrameDataPtr(data.frame);
    if (p)
    {
        //		qDebug() << "addFrameData frame:" << data.frame << " modify" ;
        *p = data;
    }
    else
    {
        //		qDebug() << "addFrameData frame:" << data.frame << " size:" << m_frameDatas.size() << " add" ;
        m_frameDatas.append(data);
    }
}
void ObjectItem::removeFrameData(int frame)
{
    int index = getFrameDataIndex(frame);
    qDebug() << "removeFrameData frame:" << frame << " index:" << index;
    if (index < 0)
    {
        return;
    }
    m_frameDatas.removeAt(index);
}

int ObjectItem::getFrameDataIndex(int frame)
{
    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        if (frame == m_frameDatas.at(i).frame)
        {
            return i;
        }
    }
    return -1;
}

FrameData *ObjectItem::getFrameDataPtr(int frame, bool bRecv)
{
    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        if (frame == m_frameDatas[i].frame)
        {
            return &m_frameDatas[i];
        }
    }
    if (bRecv)
    {
        for (int i = 0; i < childCount(); i++)
        {
            FrameData *p = child(i)->getFrameDataPtr(frame, bRecv);
            if (p)
            {
                return p;
            }
        }
    }
    return NULL;
}

void ObjectItem::sortFrameData()
{
    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (m_frameDatas.at(i).frame < m_frameDatas.at(j).frame)
            {
                m_frameDatas.swap(i, j);
            }
        }
    }
}

void ObjectItem::copy(ObjectItem *p)
{
    this->m_nLoop = p->m_nLoop;
    this->m_nCurrLoop = p->m_nCurrLoop;
    this->m_nFps = p->m_nFps;
    this->m_frameDatas = p->m_frameDatas;
    this->m_checkStateData = p->m_checkStateData;

    for (int i = 0; i < p->m_children.size(); i++)
    {
        insertChild(i, new ObjectItem(p->m_children[i]->m_name, this));
        this->m_children[i]->copy(p->m_children[i]);
    }
}

int ObjectItem::getMaxFrameNum(bool bRecv)
{
    int ret = 0;

    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        if (m_frameDatas.at(i).frame > EditData::kMaxFrame)
        {
            continue;
        }
        if (ret < m_frameDatas.at(i).frame)
        {
            ret = m_frameDatas.at(i).frame;
        }
    }
    if (bRecv)
    {
        for (int i = 0; i < m_children.size(); i++)
        {
            int tmp = m_children[i]->getMaxFrameNum();
            if (ret < tmp)
            {
                ret = tmp;
            }
        }
    }
    return ret;
}

QVariant ObjectItem::data(int role)
{
    switch (role)
    {
        case Qt::DisplayRole:
            if (m_pParent && !m_pParent->getIndex().isValid())
            {
                return getName() + QString(" Layer=%1").arg(getAllChildNum(this));
            }
        case Qt::EditRole:
            return getName();
        case Qt::CheckStateRole:
            return m_checkStateData;
        case Qt::ForegroundRole:
        {
            QBrush brush;
            if (m_checkStateData.toInt() & kState_Lock)
            {
                brush.setColor(Qt::red);
            }
            else
            {
                brush.setColor(Qt::black);
            }
            return brush;
        }
    }
    return QVariant();
}

void ObjectItem::setData(const QVariant &value, int role)
{
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            setName(value.toString());
        case Qt::CheckStateRole:
            m_checkStateData = value;
            break;
    }
}

FrameData ObjectItem::getDisplayFrameData(int frame, bool *bValid)
{
    FrameData d;
    FrameData *pPrev = getFrameDataFromPrevFrame(frame + 1);
    if (pPrev)
    {
        FrameData *pNext = getFrameDataFromNextFrame(frame);
        d = pPrev->getInterpolation(pNext, frame);
        d.frame = frame;
        if (bValid)
        {
            *bValid = true;
        }
    }
    else
    {
        if (bValid)
        {
            *bValid = false;
        }
    }
    return d;
}

QMatrix4x4 ObjectItem::getDisplayMatrix(int frame, bool *bValid)
{
    QMatrix4x4 parent, mat;
    if (m_pParent)
    {
        parent = m_pParent->getDisplayMatrix(frame);
    }

    bool valid;
    FrameData d = getDisplayFrameData(frame, &valid);
    if (valid)
    {
        mat = parent * d.getMatrix();

        if (bValid)
        {
            *bValid = true;
        }
    }
    else
    {
        if (bValid)
        {
            *bValid = false;
        }
    }
    return mat;
}

QMatrix4x4 ObjectItem::getParentDispMatrix(int frame, bool *bValid)
{
    QMatrix4x4 m;
    if (m_pParent)
    {
        m = m_pParent->getDisplayMatrix(frame, bValid);
    }
    return m;
}

FrameData *ObjectItem::getFrameDataFromPrevFrame(int frame, bool bRepeat)
{
    int old = frame;
    frame--;
    while (old != frame)
    {
        if (frame < 0)
        {
            if (!bRepeat)
            {
                return NULL;
            }

            frame += EditData::kMaxFrame;
        }
        FrameData *pData = getFrameDataPtr(frame);
        if (pData)
        {
            return pData;
        }
        frame--;
    }
    return NULL;
}

FrameData *ObjectItem::getFrameDataFromNextFrame(int frame)
{
    int max = getMaxFrameNum(false);

    for (int i = frame + 1; i < max + 1; i++)
    {
        FrameData *pData = getFrameDataPtr(i);
        if (pData)
        {
            return pData;
        }
    }
    return NULL;
}

bool ObjectItem::isContain(ObjectItem **ppRet, QPoint &pos, int frame, bool bChild, bool bCheckFlag)
{
    if (bChild)
    {
        for (int i = childCount() - 1; i >= 0; i--)
        {
            if (child(i)->isContain(ppRet, pos, frame, true))
            {
                return true;
            }
        }
    }

    if (bCheckFlag)
    {
        int flag = data(Qt::CheckStateRole).toInt();
        if (!(flag & kState_Disp) || (flag & kState_Lock))
        { // 非表示
            return false;
        }
    }

    bool valid;
    FrameData d = getDisplayFrameData(frame, &valid);
    if (valid && isContain(d, pos, getDisplayMatrix(frame)))
    {
        *ppRet = this;
        return true;
    }
    return false;
}

bool ObjectItem::isUseImageRecv(int imageNo)
{
    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        if (m_frameDatas.at(i).nImage == imageNo)
        {
            return true;
        }
    }
    for (int i = 0; i < childCount(); i++)
    {
        if (child(i)->isUseImageRecv(imageNo))
        {
            return true;
        }
    }
    return false;
}

bool ObjectItem::validate(bool bRecv)
{
    int i;

    QList<unsigned short> frames;
    for (i = 0; i < m_frameDatas.size(); i++)
    {
        const FrameData &d = m_frameDatas.at(i);
        if (frames.contains(d.frame))
        {
            return false;
        }
        frames.append(d.frame);
    }

    if (bRecv)
    {
        for (i = 0; i < childCount(); i++)
        {
            if (!child(i)->validate(bRecv))
            {
                return false;
            }
        }
    }
    return true;
}

bool ObjectItem::isContain(FrameData &displayData, QPoint &pos, const QMatrix4x4 &matDisp)
{
    QVector3D v[4];
    displayData.getVertexApplyMatrix(v, matDisp);
    QVector3D tri[2][3] = {
        {
            v[0],
            v[2],
            v[1],
        },
        {
            v[2],
            v[1],
            v[3],
        }};
    for (int i = 0; i < 3; i++)
    {
        tri[0][i].setZ(0);
        tri[1][i].setZ(0);
    }

    QVector3D p = QVector3D(pos.x(), pos.y(), 0);
    for (int i = 0; i < 2; i++)
    {
        QVector3D p0 = tri[i][0] - p;
        QVector3D p1 = tri[i][1] - p;
        QVector3D p2 = tri[i][2] - p;
        QVector3D c0 = QVector3D::crossProduct(p0, p1);
        QVector3D c1 = QVector3D::crossProduct(p1, p2);
        QVector3D c2 = QVector3D::crossProduct(p2, p0);
        if (QVector3D::dotProduct(c0, c1) > 0 && QVector3D::dotProduct(c1, c2) > 0 && QVector3D::dotProduct(c2, c0) > 0)
        {
            return true;
        }
    }

    return false;
}

void ObjectItem::applyFrameDataFromParent()
{
    QList<FrameData> datas;
    for (int i = 0; i < m_frameDatas.size(); i++)
    {
        FrameData data = m_frameDatas[i];

        bool valid;
        QMatrix4x4 mat = getDisplayMatrix(data.frame, &valid);
        if (!valid)
        {
            continue;
        }

        data.applyMatrix(mat);
        datas.append(data);
    }

    FrameData old;
    int frame = getParentFrameMax();
    for (int i = 0; i < frame; i++)
    {
        if (getFrameDataPtr(i))
        {
            old = *getFrameDataPtr(i);
            continue;
        }

        FrameData *parent = getParentFrameDataPtr(i);
        if (!parent)
        {
            continue;
        }

        bool valid;
        FrameData data;
        data = getDisplayFrameData(i, &valid);
        if (!valid)
        {
            continue;
        }

        QMatrix4x4 mat = getDisplayMatrix(i, &valid);
        if (!valid)
        {
            continue;
        }

        data.applyMatrix(mat);
        if (!(old.frame & 0x8000))
        {
            if (old.rot_z != data.rot_z)
            {
                for (int j = old.frame + 1; j < data.frame; j++)
                {
                    bool b;
                    FrameData tmp = getDisplayFrameData(j, &b);
                    if (!b)
                    {
                        continue;
                    }
                    QMatrix4x4 mat = getDisplayMatrix(j, &b);
                    if (!b)
                    {
                        continue;
                    }
                    tmp.applyMatrix(mat);
                    datas.append(tmp);
                }
            }
        }
        old = data;
        datas.append(data);
    }

    m_frameDatas = datas;
}

int ObjectItem::getParentFrameMax()
{
    int ret = 0;
    ObjectItem *parent = m_pParent;
    while (parent)
    {
        for (int i = 0; i < parent->m_frameDatas.size(); i++)
        {
            if (ret < parent->m_frameDatas.at(i).frame + 1)
            {
                ret = parent->m_frameDatas.at(i).frame + 1;
            }
        }
        parent = parent->m_pParent;
    }
    return ret;
}

FrameData *ObjectItem::getParentFrameDataPtr(int frame)
{
    ObjectItem *parent = m_pParent;
    while (parent)
    {
        FrameData *p = parent->getFrameDataPtr(frame);
        if (p)
        {
            return p;
        }
        parent = parent->m_pParent;
    }
    return NULL;
}

int ObjectItem::getAllChildNum(ObjectItem *root)
{
    int num = 0;
    for (int i = 0; i < root->childCount(); i++)
    {
        num += getAllChildNum(root->child(i));
    }
    return root->childCount() + num;
}
