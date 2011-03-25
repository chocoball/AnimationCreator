#ifndef COLORPICKERFORM_H
#define COLORPICKERFORM_H

#include <QtGui>
#include <QWidget>
#include "editdata.h"

namespace Ui {
    class ColorPickerForm;
}

class ColorPickerForm : public QWidget
{
    Q_OBJECT

public:
	explicit ColorPickerForm(CEditData *pEditData, QWidget *parent = 0);
    ~ColorPickerForm();

signals:
	void sig_setColorToFrameData(QRgb rgba) ;

protected:
	void mousePressEvent(QMouseEvent *event) ;
	void mouseMoveEvent(QMouseEvent *event) ;
	void mouseReleaseEvent(QMouseEvent *event) ;
	void paintEvent(QPaintEvent *event);

private:
	void makeColor() ;
	void makeBright() ;
	void setArrowPos() ;
	void setColorToFrameData() ;

private:
    Ui::ColorPickerForm *ui;
	CEditData			*m_pEditData ;

	int					m_selectColor ;
	QPoint				m_selectBright ;
	int					m_editMode ;
};

#endif // COLORPICKERFORM_H
