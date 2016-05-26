#include "keyboardmodel.h"

KeyboardModel::KeyboardModel(QList<QStringList> &datas, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_items = datas;
}

QVariant KeyboardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (index.row() >= m_items.size())
    {
        return QVariant();
    }
    if (index.column() >= m_items.at(index.row()).size())
    {
        return QVariant();
    }

    return QVariant(m_items.at(index.row()).at(index.column()));
}

bool KeyboardModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole && role != Qt::DisplayRole)
    {
        return false;
    }
    QStringList &list = m_items[index.row()];
    list.replace(index.column(), value.toString());
    return true;
}

Qt::ItemFlags KeyboardModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant KeyboardModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QVariant();
    }
    switch (section)
    {
        case 0:
            return trUtf8("コマンド");
        case 1:
            return trUtf8("ショートカット");
    }

    return QVariant();
}

QModelIndex KeyboardModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if (m_items.size() <= row)
    {
        return QModelIndex();
    }
    if (parent.isValid())
    {
        return QModelIndex();
    }

    return createIndex(row, column, (QString *)&m_items[row][column]);
}

QModelIndex KeyboardModel::parent(const QModelIndex & /*index*/) const
{
    return QModelIndex();
}

int KeyboardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }
    if (!parent.isValid())
    {
        return m_items.size();
    }
    return 0;
}

int KeyboardModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 2;
}
