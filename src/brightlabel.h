#ifndef BRIGHTLABEL_H
#define BRIGHTLABEL_H

#include <QtGui>
#include <QWidget>
#include <QLabel>

class BrightLabel : public QLabel
{
public:
    explicit BrightLabel(QWidget *parent = 0) ;
    virtual ~BrightLabel() ;

	void setSelectPos(QPoint p)
	{
		m_SelectPos = p ;
	}

protected:
	void paintEvent(QPaintEvent *) ;

private:
	QPoint		m_SelectPos ;
} ;

#endif // BRIGHTLABEL_H
