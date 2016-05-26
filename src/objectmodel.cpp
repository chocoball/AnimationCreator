#include "objectmodel.h"
#include "editdata.h"

// ---------------------------------------------------------------------------------
//
// CObjectModel
//
// ---------------------------------------------------------------------------------
ObjectModel::ObjectModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_pRoot = new ObjectItem("Root", NULL);
}

ObjectModel::~ObjectModel()
{
    delete m_pRoot;
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
    ObjectItem *p = getItemFromIndex(index);
    return p->data(role);
}

int ObjectModel::rowCount(const QModelIndex &parent) const
{
    ObjectItem *p = getItemFromIndex(parent);
    return p->childCount();
}

int ObjectModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool ObjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return false;
    }

    ObjectItem *p = getItemFromIndex(index);
    p->setName(value.toString());
    emit dataChanged(index, index);
    return true;
}

bool ObjectModel::insertRows(int row, int count, const QModelIndex &parent)
{
    //	qDebug() << "insertRows " << row << count << parent ;
    beginInsertRows(parent, row, row + count - 1);

    ObjectItem *p = getItemFromIndex(parent);
    p->insertChild(row, new ObjectItem(QString(), p));

    endInsertRows();
    return true;
}

bool ObjectModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    ObjectItem *p = getItemFromIndex(parent);
    p->removeChild(p->child(row));

    endRemoveRows();
    return true;
}

QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    ObjectItem *p = getItemFromIndex(parent);

    ObjectItem *pChild = p->child(row);
    if (pChild)
    {
        return createIndex(row, column, pChild);
    }
    return QModelIndex();
}

QModelIndex ObjectModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }
    ObjectItem *c = static_cast<ObjectItem *>(child.internalPointer());
    ObjectItem *p = c->parent();
    if (p == m_pRoot || c == m_pRoot)
    {
        return QModelIndex();
    }
    return createIndex(p->row(), 0, p);
}

Qt::DropActions ObjectModel::supportedDropActions() const
{
    return Qt::CopyAction; //| Qt::MoveAction ;
}

QStringList ObjectModel::mimeTypes() const
{
    QStringList types;
    types << "AnimationCreator/object.item.list";
    return types;
}

QMimeData *ObjectModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodeData;

    qDebug() << "mimeData";

    QDataStream stream(&encodeData, QIODevice::WriteOnly);
    foreach (const QModelIndex &index, indexes)
    {
        if (index.isValid())
        {
            stream << reinterpret_cast<quint64>(index.internalPointer());
            qDebug() << reinterpret_cast<quint64>(index.internalPointer());
        }
    }
    mimeData->setData("AnimationCreator/object.item.list", encodeData);
    return mimeData;
}

bool ObjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }
    if (!data->hasFormat("AnimationCreator/object.item.list"))
    {
        return false;
    }
    if (column > 0)
    {
        return false;
    }

    qDebug() << "dropMimeData row:" << row << " col:" << column;
    qDebug() << " parent:" << parent << " action:" << action;

    QByteArray encodeData = data->data("AnimationCreator/object.item.list");
    QDataStream stream(&encodeData, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
        quint64 val;
        ObjectItem *p;
        stream >> val;
        p = reinterpret_cast<ObjectItem *>(val);

        if (p->parent() == m_pRoot)
        { // オブジェクト
            if (!parent.isValid())
            {
                emit sig_copyIndex(row, p, parent, action);
            }
        }
        else
        { // レイヤ
            if (parent.isValid())
            {
                emit sig_copyIndex(row, p, parent, action);
            }
        }
    }

    return true;
}

QModelIndex ObjectModel::addItem(QString name, const QModelIndex &parent)
{
    ObjectItem *p = getItemFromIndex(parent);
    int row = p->childCount();

    return insertItem(row, name, parent);
}

QModelIndex ObjectModel::insertItem(int row, QString name, const QModelIndex &parent)
{
    insertRows(row, 1, parent);

    QModelIndex index = this->index(row, 0, parent);
    ObjectItem *p = static_cast<ObjectItem *>(index.internalPointer());
    p->setIndex(index);
    setData(index, name, Qt::EditRole);
    //	qDebug() << "insertItem row:" << row << " name:" << name << " p:" << p ;
    return index;
}

void ObjectModel::removeItem(QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    qDebug() << "removeItem row:" << index.row() << "p:" << index.internalPointer();
    removeRows(index.row(), 1, index.parent());
}

// QModelIndex から ObjectItem 取得
ObjectItem *ObjectModel::getItemFromIndex(const QModelIndex &index) const
{
    ObjectItem *p = m_pRoot;
    if (index.isValid())
    {
        p = static_cast<ObjectItem *>(index.internalPointer());
    }
    return p;
}

ObjectItem *ObjectModel::getObject(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return NULL;
    }
    ObjectItem *p = static_cast<ObjectItem *>(index.internalPointer());
    while (p->parent() != m_pRoot)
    {
        p = p->parent();
    }
    return p;
}

bool ObjectModel::isObject(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }
    ObjectItem *p = getItemFromIndex(index);
    return p->parent() == m_pRoot ? true : false;
    //	return index.parent().internalPointer() == m_pRoot ? true : false ;
}

bool ObjectModel::isLayer(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }
    ObjectItem *p = getItemFromIndex(index);
    return p->parent() == m_pRoot ? false : true;
    //	return index.parent().internalPointer() != m_pRoot ? true : false ;
}

FrameData *ObjectModel::getFrameDataFromPrevFrame(QModelIndex index, int frame, bool bRepeat)
{
    if (!isLayer(index))
    {
        return NULL;
    }
    ObjectItem *p = getItemFromIndex(index);

    return getFrameDataFromPrevFrame(p, frame, bRepeat);
}

FrameData *ObjectModel::getFrameDataFromPrevFrame(ObjectItem *p, int frame, bool bRepeat)
{
    if (!p)
    {
        return NULL;
    }
    return p->getFrameDataFromPrevFrame(frame, bRepeat);
}

FrameData *ObjectModel::getFrameDataFromNextFrame(QModelIndex index, int frame)
{
    if (!isLayer(index))
    {
        return NULL;
    }
    ObjectItem *p = getItemFromIndex(index);
    return getFrameDataFromNextFrame(p, frame);
}

FrameData *ObjectModel::getFrameDataFromNextFrame(ObjectItem *p, int frame)
{
    if (!p)
    {
        return NULL;
    }
    return p->getFrameDataFromNextFrame(frame);
}

int ObjectModel::getRow(QModelIndex index)
{
    ObjectItem *p = getItemFromIndex(index);
    int row = 0;
    return getRow(m_pRoot, p, &row);
}

int ObjectModel::getRow(ObjectItem *root, ObjectItem *p, int *row)
{
    if (root == p)
    {
        return *row;
    }
    for (int i = 0; i < root->childCount(); i++)
    {
        *row += 1;
        int ret = getRow(root->child(i), p, row);
        if (ret >= 0)
        {
            return ret;
        }
    }
    return -1;
}

QModelIndex ObjectModel::getIndex(int row)
{
    int curr = 0;
    return getIndex(m_pRoot, row, &curr);
}

QModelIndex ObjectModel::getIndex(ObjectItem *root, int row, int *currRow)
{
    if (row == *currRow)
    {
        return root->getIndex();
    }

    for (int i = 0; i < root->childCount(); i++)
    {
        *currRow += 1;
        QModelIndex ret = getIndex(root->child(i), row, currRow);
        if (ret.isValid())
        {
            return ret;
        }
    }
    return QModelIndex();
}

void ObjectModel::updateIndex()
{
    for (int i = 0; i < m_pRoot->childCount(); i++)
    {
        updateIndex(m_pRoot->child(i), QModelIndex(), i);
    }
}

void ObjectModel::updateIndex(ObjectItem *pItem, const QModelIndex &parent, int row)
{
    pItem->setIndex(this->index(row, 0, parent));
    pItem->setParent(this->getItemFromIndex(parent));

    for (int i = 0; i < pItem->childCount(); i++)
    {
        updateIndex(pItem->child(i), pItem->getIndex(), i);
    }
}

void ObjectModel::flat()
{
    for (int i = 0; i < m_pRoot->childCount(); i++)
    {
        ObjectItem *pObj = m_pRoot->child(i);
        pObj->flat();
    }
    updateIndex();
}

bool ObjectModel::isUseImage(int imageNo)
{
    return m_pRoot->isUseImageRecv(imageNo);
}
