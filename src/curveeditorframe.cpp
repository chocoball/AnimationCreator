#include <QDebug>
#include "curveeditorframe.h"

CurveEditorFrame::CurveEditorFrame(QWidget *parent) :
    QFrame(parent)
{
}

void CurveEditorFrame::resizeEvent(QResizeEvent *event)
{
	emit sig_resize(event) ;
}
