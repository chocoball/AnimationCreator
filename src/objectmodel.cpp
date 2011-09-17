#include "objectmodel.h"
#include "editdata.h"

// ---------------------------------------------------------------------------------
//
// CObjectModel
//
// ---------------------------------------------------------------------------------
CObjectModel::CObjectModel(QObject *parent) :
	QAbstractItemModel(parent)
{
	m_pRoot = new ObjectItem("Root", NULL) ;
}

CObjectModel::~CObjectModel()
{
	delete m_pRoot ;
}

bool CObjectModel::isFrameDataInPos( const FrameData &data, QPoint pos )
{
	QVector3D v[4] ;
	data.getVertexApplyMatrix(v);
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
		}
	} ;
	for ( int i = 0 ; i < 3 ; i ++ ) {
		tri[0][i].setZ(0);
		tri[1][i].setZ(0);
	}

	QVector3D p = QVector3D(pos.x(), pos.y(), 0) ;
	for ( int i = 0 ; i < 2 ; i ++ ) {
		QVector3D p0 = tri[i][0] - p ;
		QVector3D p1 = tri[i][1] - p ;
		QVector3D p2 = tri[i][2] - p ;
		QVector3D c0 = QVector3D::crossProduct(p0, p1) ;
		QVector3D c1 = QVector3D::crossProduct(p1, p2) ;
		QVector3D c2 = QVector3D::crossProduct(p2, p0) ;
		if ( QVector3D::dotProduct(c0, c1) >= 0
		  && QVector3D::dotProduct(c1, c2) >= 0
		  && QVector3D::dotProduct(c2, c0) >= 0 ) {
			return true ;
		}
	}

	return false ;
}

bool CObjectModel::isFrameDataInRect( const FrameData &data, QRect rect )
{
	int i ;
	QVector3D v[4] ;
	data.getVertexApplyMatrix(v);

	for ( i = 0 ; i < 4 ; i ++ ) {
		if ( rect.contains(v[i].x(), v[i].y(), false) ) {
			return true ;
		}
	}
	QVector3D tri[2][3] = {
		{ v[0], v[2], v[1] },
		{ v[2], v[1], v[3] }
	} ;
	for ( i = 0 ; i < 3 ; i ++ ) {
		tri[0][i].setZ(0);
		tri[1][i].setZ(0);
	}
	QPoint pos[4] = {
		rect.topLeft(),
		rect.topRight(),
		rect.bottomLeft(),
		rect.bottomRight()
	} ;
	for ( i = 0 ; i < 4 ; i ++ ) {
		QVector3D vPos = QVector3D(pos[i]) ;
		vPos.setZ(0);

		for ( int j = 0 ; j < 2 ; j ++ ) {
			QVector3D p0 = tri[j][0] - vPos ;
			QVector3D p1 = tri[j][1] - vPos ;
			QVector3D p2 = tri[j][2] - vPos ;
			QVector3D c0 = QVector3D::crossProduct(p0, p1) ;
			QVector3D c1 = QVector3D::crossProduct(p1, p2) ;
			QVector3D c2 = QVector3D::crossProduct(p2, p0) ;
			if ( QVector3D::dotProduct(c0, c1) >= 0
			  && QVector3D::dotProduct(c1, c2) >= 0
			  && QVector3D::dotProduct(c2, c0) >= 0 ) {
				return true ;
			}
		}
	}

	return false ;
}

QVariant CObjectModel::data(const QModelIndex &index, int role) const
{
	ObjectItem *p = getItemFromIndex(index) ;
	return p->data(role) ;
}

int CObjectModel::rowCount(const QModelIndex &parent) const
{
	ObjectItem *p = getItemFromIndex(parent) ;
	return p->childCount() ;
}

int CObjectModel::columnCount(const QModelIndex &parent) const
{
	return 1 ;
}

Qt::ItemFlags CObjectModel::flags(const QModelIndex &index) const
{
	if ( !index.isValid() ) {
		return Qt::ItemIsEnabled ;
	}
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled ;
}

bool CObjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ( role != Qt::DisplayRole && role != Qt::EditRole ) {
		return false ;
	}

	ObjectItem *p = getItemFromIndex(index) ;
	p->setName(value.toString()) ;
	emit dataChanged(index, index);
	return true ;
}

bool CObjectModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row+count-1) ;

	ObjectItem *p = getItemFromIndex(parent) ;
	p->insertChild(row, new ObjectItem(QString(), p)) ;

	endInsertRows();
	return true ;
}

bool CObjectModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row+count-1) ;

	ObjectItem *p = getItemFromIndex(parent) ;
	p->removeChild(p->child(row)) ;

	endRemoveRows();
	return true ;
}

QModelIndex CObjectModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( !hasIndex(row, column, parent) ) { return QModelIndex() ; }

	ObjectItem *p = getItemFromIndex(parent) ;

	ObjectItem *pChild = p->child(row) ;
	if ( pChild ) {
		return createIndex(row, column, pChild) ;
	}
	return QModelIndex() ;
}

QModelIndex CObjectModel::parent(const QModelIndex &child) const
{
	if ( !child.isValid() ) { return QModelIndex() ; }
	ObjectItem *c = static_cast<ObjectItem *>(child.internalPointer()) ;
	ObjectItem *p = c->parent() ;
	if ( p == m_pRoot ) { return QModelIndex() ; }
	return createIndex(p->row(), 0, p) ;
}

Qt::DropActions CObjectModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction ;
}

QStringList CObjectModel::mimeTypes() const
{
	QStringList types ;
	types << "application/object.item.list" ;
	return types ;
}

QMimeData *CObjectModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData() ;
	QByteArray encodeData ;

	QDataStream stream(&encodeData, QIODevice::WriteOnly) ;
	foreach ( const QModelIndex &index, indexes ) {
		if ( index.isValid() ) {
			stream << reinterpret_cast<quint64>(index.internalPointer()) ;
		}
	}
	mimeData->setData("application/object.item.list", encodeData) ;
	return mimeData ;
}

bool CObjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if ( action == Qt::IgnoreAction ) { return true ; }
	if ( !data->hasFormat("application/object.item.list") ) { return false ; }
	if ( column > 0 ) { return false ; }

	QByteArray encodeData = data->data("application/object.item.list") ;
	QDataStream stream(&encodeData, QIODevice::ReadOnly) ;

	while ( !stream.atEnd() ) {
		quint64 val ;
		ObjectItem *p ;
		stream >> val ;
		p = reinterpret_cast<ObjectItem *>(val) ;
		QModelIndex index = addItem(p->getName(), parent) ;
		ObjectItem *newItem = getItemFromIndex(index) ;
		newItem->copy(p) ;
	}
	return true ;
}

QModelIndex CObjectModel::addItem(QString name, const QModelIndex &parent)
{
	ObjectItem *p = getItemFromIndex(parent) ;
	int row = p->childCount() ;

	insertRows(row, 1, parent) ;
	QModelIndex index = this->index(row, 0, parent) ;
	p = static_cast<ObjectItem *>(index.internalPointer()) ;
	p->setIndex(index) ;
	setData(index, name, Qt::EditRole) ;
	return index ;
}

void CObjectModel::removeItem(QModelIndex &index)
{
	if ( !index.isValid() ) { return ; }

	removeRows(index.row(), 1, index.parent()) ;
}


// QModelIndex から ObjectItem 取得
ObjectItem *CObjectModel::getItemFromIndex(const QModelIndex &index) const
{
	ObjectItem *p = m_pRoot ;
	if ( index.isValid() ) {
		p = static_cast<ObjectItem *>(index.internalPointer()) ;
	}
	return p ;
}

ObjectItem *CObjectModel::getObject(const QModelIndex &index)
{
	if ( !index.isValid() ) { return NULL ; }
	ObjectItem *p = static_cast<ObjectItem *>(index.internalPointer()) ;
	while ( p->parent() != m_pRoot ) {
		p = p->parent() ;
	}
	return p ;
}

bool CObjectModel::isObject(const QModelIndex &index)
{
	if ( !index.isValid() ) { return false ; }
	return index.parent().internalPointer() == m_pRoot ? true : false ;
}

bool CObjectModel::isLayer(const QModelIndex &index)
{
	if ( !index.isValid() ) { return false ; }
	return index.parent().internalPointer() != m_pRoot ? true : false ;
}

FrameData *CObjectModel::getFrameDataFromPrevFrame(QModelIndex index, int frame, bool bRepeat)
{
	if ( !isLayer(index) ) { return NULL ; }
	ObjectItem *p = getItemFromIndex(index) ;

	return getFrameDataFromPrevFrame(p, frame, bRepeat) ;
}

FrameData *CObjectModel::getFrameDataFromPrevFrame(ObjectItem *p, int frame, bool bRepeat)
{
	if ( !p ) { return NULL ; }
	return p->getFrameDataFromPrevFrame(frame, bRepeat) ;
}

FrameData *CObjectModel::getFrameDataFromNextFrame(QModelIndex index, int frame)
{
	if ( !isLayer(index) ) { return NULL ; }
	ObjectItem *p = getItemFromIndex(index) ;
	return getFrameDataFromNextFrame(p, frame) ;
}

FrameData *CObjectModel::getFrameDataFromNextFrame(ObjectItem *p, int frame)
{
	if ( !p ) { return NULL ; }
	return p->getFrameDataFromNextFrame(frame) ;
}
