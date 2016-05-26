#ifndef DROPABLEMDIAREA_H
#define DROPABLEMDIAREA_H

#include <QMdiArea>
#include <QtGui>

class DropableMdiArea : public QMdiArea
{
    Q_OBJECT
public:
    explicit DropableMdiArea(QWidget *parent = 0);

signals:
    void dropFiles(QString fileName);

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // DROPABLEMDIAREA_H
