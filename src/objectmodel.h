#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include "objectitem.h"
#include <QAbstractItemModel>
#include <QMatrix4x4>
#include <QPair>
#include <QtGui>

class CObjectModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CObjectModel(QObject *parent = 0);
    virtual ~CObjectModel();

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    QModelIndex addItem(QString name, const QModelIndex &parent);
    QModelIndex insertItem(int row, QString name, const QModelIndex &parent);
    void removeItem(QModelIndex &index);

    ObjectItem *getItemFromIndex(const QModelIndex &index) const;
    ObjectItem *getObject(const QModelIndex &index);

    bool isObject(const QModelIndex &index) const;
    bool isLayer(const QModelIndex &index) const;

    FrameData *getFrameDataFromPrevFrame(QModelIndex index, int frame, bool bRepeat = false);
    FrameData *getFrameDataFromPrevFrame(ObjectItem *pItem, int frame, bool bRepeat = false);

    FrameData *getFrameDataFromNextFrame(QModelIndex index, int frame);
    FrameData *getFrameDataFromNextFrame(ObjectItem *pItem, int frame);

    int getRow(QModelIndex index);
    int getRow(ObjectItem *root, ObjectItem *p, int *row);
    QModelIndex getIndex(int row);
    QModelIndex getIndex(ObjectItem *root, int row, int *currRow);

    void updateIndex();

    void flat();

    bool isUseImage(int imageNo);

    void copy(CObjectModel *p)
    {
        this->m_pRoot->copy(p->m_pRoot);
        updateIndex();
    }

private:
    void updateIndex(ObjectItem *pItem, const QModelIndex &parent, int row);

signals:
    void sig_copyIndex(int row, ObjectItem *pItem, QModelIndex parent, Qt::DropAction action);

public slots:

private:
    ObjectItem *m_pRoot;
};

#endif // OBJECTMODEL_H
