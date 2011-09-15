#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QtGui>
#include <QPair>
#include <QMatrix4x4>
#include "objectitem.h"


class CObjectModel : public QAbstractItemModel
{
public:
	explicit CObjectModel(QObject *parent = 0) ;
	virtual ~CObjectModel() ;

	bool isFrameDataInPos( const FrameData &data, QPoint pos ) ;
	bool isFrameDataInRect( const FrameData &data, QRect rect ) ;

	QVariant data(const QModelIndex &index, int role) const ;
	int rowCount(const QModelIndex &parent) const ;
	int columnCount(const QModelIndex &parent) const ;
	Qt::ItemFlags flags(const QModelIndex &index) const ;
	bool setData(const QModelIndex &index, const QVariant &value, int role) ;
	bool insertRows(int row, int count, const QModelIndex &parent) ;
	bool removeRows(int row, int count, const QModelIndex &parent) ;
	QModelIndex index(int row, int column, const QModelIndex &parent) const ;
	QModelIndex parent(const QModelIndex &child) const ;

	Qt::DropActions supportedDropActions() const ;
	QStringList mimeTypes() const ;
	QMimeData *mimeData(const QModelIndexList &indexes) const ;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) ;

	QModelIndex addItem(QString name, const QModelIndex &parent) ;
	void removeItem(QModelIndex &index) ;

	ObjectItem *getItemFromIndex(const QModelIndex &index) ;
	ObjectItem *getObject(const QModelIndex &index) ;

private:
	ObjectItem		*m_pRoot ;
};


#endif // OBJECTMODEL_H
