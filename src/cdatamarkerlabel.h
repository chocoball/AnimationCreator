#ifndef CDATAMARKERLABEL_H
#define CDATAMARKERLABEL_H

#include <QLabel>
#include "editimagedata.h"

class CDataMarkerLabel : public QLabel
{
    Q_OBJECT
public:
	explicit CDataMarkerLabel(CEditImageData *pEditImageData, QWidget *parent = 0);

signals:

public slots:

protected:
	void paintEvent(QPaintEvent *event);

private:
	CEditImageData		*m_pEditImageData ;
};

#endif // CDATAMARKERLABEL_H
