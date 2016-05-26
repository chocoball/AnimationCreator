#include "dropablemdiarea.h"
#include <QDebug>
#include <QMdiSubWindow>

DropableMdiArea::DropableMdiArea(QWidget *parent)
    : QMdiArea(parent)
{
    setAcceptDrops(true);
}

void DropableMdiArea::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QMdiSubWindow *> list = subWindowList();
    for (int i = 0; i < list.size(); i++)
    {
        QMdiSubWindow *p = list.at(i);
        if (p->rect().contains(event->pos() - p->pos()))
        {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void DropableMdiArea::dragMoveEvent(QDragMoveEvent *event)
{
    QList<QMdiSubWindow *> list = subWindowList();
    for (int i = 0; i < list.size(); i++)
    {
        QMdiSubWindow *p = list.at(i);
        if (p->rect().contains(event->pos() - p->pos()))
        {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void DropableMdiArea::dropEvent(QDropEvent *event)
{
    qDebug() << "dropEvent" << event->mimeData()->urls();
    QString file;
    QList<QUrl> urls = event->mimeData()->urls();
    for (int i = 0; i < urls.size(); i++)
    {
        file = urls.at(i).toLocalFile();
        break;
    }

    emit dropFiles(file);
}
