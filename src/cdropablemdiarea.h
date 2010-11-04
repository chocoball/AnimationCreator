#ifndef CDROPABLEMDIAREA_H
#define CDROPABLEMDIAREA_H

#include <QtGui>
#include <QMdiArea>

class CDropableMdiArea : public QMdiArea
{
    Q_OBJECT
public:
    explicit CDropableMdiArea(QWidget *parent = 0);

signals:
	void dropFiles(QString fileName) ;

public slots:

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event) ;
	void dropEvent(QDropEvent *event) ;

};

#endif // CDROPABLEMDIAREA_H
