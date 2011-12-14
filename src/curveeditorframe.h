#ifndef CURVEEDITORFRAME_H
#define CURVEEDITORFRAME_H

#include <QFrame>

class CurveEditorFrame : public QFrame
{
    Q_OBJECT
public:
	explicit CurveEditorFrame(QWidget *parent = 0);

signals:
	void sig_resize(QResizeEvent *) ;

public slots:

protected:
	void resizeEvent(QResizeEvent *event) ;
};

#endif // CURVEEDITORFRAME_H
