#ifndef KEYBOARDMODEL_H
#define KEYBOARDMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

class KeyboardModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit KeyboardModel(QList<QStringList> &datas, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

signals:

public slots:

private:
    QList<QStringList> m_items;
};

#endif // KEYBOARDMODEL_H
