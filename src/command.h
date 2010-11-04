#ifndef COMMAND_H
#define COMMAND_H

#include <QtGui>
#include <QUndoCommand>
#include <QStandardItemModel>
#include "editimagedata.h"

class CEditImageData ;

// オブジェクト追加コマンド
class Command_AddObject : public QUndoCommand
{
public:
	Command_AddObject(CEditImageData *pEditImageData, QString &str);

	void undo();
	void redo();

	QStandardItem *getNewItem() { return m_pItem ; }

private:
	QString						m_strObjName ;
	CObjectModel				*m_pObjModel ;
	QStandardItemModel			*m_pTreeModel ;
	QStandardItem				*m_pItem ;

	CObjectModel::ObjectGroup	m_ObjGroup ;
	int							m_ItemIndex ;
	int							m_objListIndex ;
};

// オブジェクト/レイヤ削除コマンド
class Command_DelObject : public QUndoCommand
{
public:
	Command_DelObject(CEditImageData *pEditImageData, QModelIndex index, QLabel *pDataMakerLabel) ;

	void undo();
	void redo();

private:
	CObjectModel				*m_pObjModel ;
	QStandardItemModel			*m_pTreeModel ;
	QModelIndex					m_ItemIndex ;
	QLabel						*m_pDataMarkerLabel ;

	QList<QStandardItem *>		m_ItemList ;
	CObjectModel::ObjectGroup	m_ObjGroup ;
	CObjectModel::LayerGroup	m_LayerGroup ;
	int							m_Index ;
};

// レイヤ追加コマンド
class Command_AddLayer : public QUndoCommand
{
public:
	Command_AddLayer(CEditImageData *pEditImageData,
					 QModelIndex parentIndex,
					 QStandardItem *pAddItem,
					 CObjectModel::FrameData frameData,
					 QList<QWidget *> &updateWidget) ;

	void undo() ;
	void redo() ;

private:
	CObjectModel				*m_pObjModel ;
	QStandardItemModel			*m_pTreeModel ;
	QModelIndex					m_ObjIndex ;
	QStandardItem				*m_pAddItem ;

	int							m_ItemIndex ;
	CObjectModel::LayerGroup	m_LayerGroup ;

	QList<QWidget *>			m_UpdateWidgetList ;
};

// フレームデータ追加コマンド
class Command_AddFrameData : public QUndoCommand
{
public:
	Command_AddFrameData(CEditImageData *pEditImageData,
						 CObjectModel::typeID objID,
						 CObjectModel::typeID layerID,
						 CObjectModel::FrameData &data,
						 QList<QWidget *> &updateWidget) ;

	void undo() ;
	void redo() ;

private:
	CObjectModel				*m_pObjModel ;
	CObjectModel::typeID		m_objID ;
	CObjectModel::typeID		m_layerID ;
	CObjectModel::FrameData		m_FrameData ;
	int							m_Index ;

	QList<QWidget *>			m_UpdateWidgetList ;
};

// フレームデータ削除コマンド
class Command_DelFrameData : public QUndoCommand
{
public:
	Command_DelFrameData(CEditImageData *pEditImageData,
						 CObjectModel::typeID objID,
						 CObjectModel::typeID layerID,
						 int frame,
						 QList<QWidget *> &updateWidget) ;
	void undo() ;
	void redo() ;

private:
	CObjectModel				*m_pObjModel ;
	CObjectModel::typeID		m_objID ;
	CObjectModel::typeID		m_layerID ;
	CObjectModel::FrameData		m_FrameData ;
	int							m_Index ;

	QList<QWidget *>			m_UpdateWidgetList ;
};

// フレームデータ編集コマンド
class Command_EditFrameData : public QUndoCommand
{
public:
	Command_EditFrameData(CEditImageData *pEditImageData,
						  CObjectModel::typeID objID,
						  CObjectModel::typeID layerID,
						  int frame,
						  CObjectModel::FrameData &data,
						  QList<QWidget *> &updateWidget) ;
	void undo() ;
	void redo() ;

private:
	CObjectModel				*m_pObjModel ;
	CObjectModel::typeID		m_objID ;
	CObjectModel::typeID		m_layerID ;
	CObjectModel::FrameData		m_FrameData, m_OldFrameData ;
	int							m_Frame ;

	QList<QWidget *>			m_UpdateWidgetList ;
};


#endif // COMMAND_H
