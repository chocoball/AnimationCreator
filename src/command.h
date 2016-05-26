#ifndef COMMAND_H
#define COMMAND_H

#include "editdata.h"
#include <QStandardItemModel>
#include <QUndoCommand>
#include <QtGui>

class EditData;

class CommandBase : public QUndoCommand
{
public:
    CommandBase(const QString &text)
        : QUndoCommand(text)
    {
    }
    virtual ~CommandBase() {}

protected:
    void updateAllWidget();
    void error(QString title, QString text);
};

// アイテム追加 コマンド
class Command_AddItem : public CommandBase
{
public:
    Command_AddItem(EditData *pEditData, QString &str, QModelIndex &parent);

    void redo();
    void undo();

    QModelIndex getIndex() { return m_index; }

private:
    EditData *m_pEditData;
    QString m_str;
    QModelIndex m_index;
    int m_parentRow;
    int m_row;
};

// アイテム削除 コマンド
class Command_DelItem : public CommandBase
{
public:
    Command_DelItem(EditData *pEditData, QModelIndex &index);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    int m_row;
    int m_parentRow;
    int m_relRow;
    ObjectItem *m_pItem;
};

// フレームデータ追加コマンド
class Command_AddFrameData : public CommandBase
{
public:
    Command_AddFrameData(EditData *pEditData,
                         QModelIndex &index,
                         FrameData &data);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    CObjectModel *m_pObjModel;
    FrameData m_frameData;
    int m_row;
    int m_flag;
};

// フレームデータ削除コマンド
class Command_DelFrameData : public CommandBase
{
public:
    Command_DelFrameData(EditData *pEditData,
                         QModelIndex &index,
                         int frame);
    void redo();
    void undo();

private:
    EditData *m_pEditData;
    CObjectModel *m_pObjModel;
    FrameData m_FrameData;
    int m_row;
};

// フレームデータ編集コマンド
class Command_EditFrameData : public CommandBase
{
public:
    Command_EditFrameData(EditData *pEditData,
                          QModelIndex &index,
                          int frame,
                          FrameData &data,
                          FrameData *pOld,
                          QWidget *pAnimeWidget);
    void redo();
    void undo();

private:
    EditData *m_pEditData;
    CObjectModel *m_pObjModel;
    FrameData m_FrameData, m_OldFrameData;
    QWidget *m_pAnimeWidget;
    int m_row;
    int m_frame;

    bool m_bSetOld;
};

// オブジェクトコピーコマンド
class Command_CopyObject : public CommandBase
{
public:
    Command_CopyObject(EditData *pEditData, QModelIndex &index);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    ObjectItem *m_pObject;
    int m_row;
};

// レイヤコピー
class Command_CopyIndex : public CommandBase
{
public:
    Command_CopyIndex(EditData *pEditData, int row, ObjectItem *pLayer, QModelIndex parent);
    ~Command_CopyIndex();

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    int m_row, m_relRow, m_parentRow;
    ObjectItem *m_pLayer;
};

// フレームデータ移動
class Command_MoveFrameData : public CommandBase
{
public:
    Command_MoveFrameData(EditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    int m_row;

    int m_srcFrame, m_dstFrame;
    FrameData m_srcData, m_dstData;
};

// 全フレームデータ移動
class Command_MoveAllFrameData : public CommandBase
{
public:
    Command_MoveAllFrameData(EditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame);

    void redo();
    void undo();

private:
    void save_frameData(ObjectItem *pItem, int srcFrame, int dstFrame);
    void restore_frameData(ObjectItem *pItem, int srcFrame, int dstFrame);

private:
    EditData *m_pEditData;
    int m_row;

    int m_srcFrame, m_dstFrame;
    QList<QPair<int, FrameData> > m_dstDatas;
};

// ツリーアイテム上に移動
class Command_MoveItemUp : public CommandBase
{
public:
    Command_MoveItemUp(EditData *pEditData, const QModelIndex &index);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    QModelIndex m_index;
    ObjectItem *m_pItem;
};

// ツリーアイテム下に移動
class Command_MoveItemDown : public CommandBase
{
public:
    Command_MoveItemDown(EditData *pEditData, const QModelIndex &index);

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    QModelIndex m_index;
    ObjectItem *m_pItem;
};

// UVスケール
class Command_ScaleUv : public CommandBase
{
public:
    Command_ScaleUv(EditData *pEditData, double scale);

    void redo();
    void undo();

private:
    void save_framedata(ObjectItem *pItem);

private:
    EditData *m_pEditData;
    QList<QPair<int, FrameData> > m_changeFrameDatas;
    double m_scale;
};

// フレームスケール
class Command_ScaleFrame : public CommandBase
{
public:
    Command_ScaleFrame(EditData *pEditData, double scale);

    void redo();
    void undo();

private:
    void save_framedata(ObjectItem *pItem);

private:
    EditData *m_pEditData;
    QList<QPair<int, QList<FrameData> > > m_changeFrameDatas;
    double m_scale;
};

// 現在フレームの全レイヤのフレームデータペースト
class Command_PasteAllFrame : public CommandBase
{
public:
    Command_PasteAllFrame(EditData *pEditData, QModelIndex index, int frame);

    void redo();
    void undo();

private:
private:
    EditData *m_pEditData;
    int m_objRow;
    int m_frame;
    int m_copyObjRow;

    QList<QPair<int, FrameData> > m_copyDatas;
    QList<QPair<int, FrameData> > m_changeFrameDatas;
};

// 現在フレームの全レイヤのフレームデータ削除
class Command_DeleteAllFrame : public CommandBase
{
public:
    Command_DeleteAllFrame(EditData *pEditData, QModelIndex index, int frame);

    void redo();
    void undo();

private:
    void save_framedata(ObjectItem *pItem);

private:
    EditData *m_pEditData;
    int m_objRow;
    int m_frame;

    QList<QPair<int, FrameData> > m_datas;
};

// レイヤペースト
class Command_PasteLayer : public CommandBase
{
public:
    Command_PasteLayer(EditData *pEditData, QModelIndex parentIndex, ObjectItem *pItem);
    ~Command_PasteLayer();

    void redo();
    void undo();

private:
    EditData *m_pEditData;
    int m_parentRow, m_row;
    ObjectItem *m_pItem;
};

#endif // COMMAND_H
