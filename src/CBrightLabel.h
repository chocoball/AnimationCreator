#ifndef CBRIGHTLABEL_H
#define CBRIGHTLABEL_H

#include <QtGui>
#include <QWidget>
#include <QLabel>

class CBrightLabel : public QLabel
{
public:
	explicit CBrightLabel(QWidget *parent = 0) ;
	virtual ~CBrightLabel() ;

	void setSelectPos(QPoint p)
	{
		m_SelectPos = p ;
	}

protected:
	void paintEvent(QPaintEvent *) ;

private:
	QPoint		m_SelectPos ;
} ;

#endif // CBRIGHTLABEL_H
