#ifndef CDATAMARKERLABEL_H
#define CDATAMARKERLABEL_H

#include <QLabel>
#include "editdata.h"

class CDataMarkerLabel : public QLabel
{
    Q_OBJECT
public:
	explicit CDataMarkerLabel(CEditData *pEditData, QWidget *parent = 0);

signals:

public slots:

protected:
	void paintEvent(QPaintEvent *event);

    void drawLine( const QModelIndex &indexLayer, QPainter &painter ) ;

private:
	CEditData		*m_pEditData ;
};

#endif // CDATAMARKERLABEL_H
