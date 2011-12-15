#ifndef CURVEEDITORFORM_H
#define CURVEEDITORFORM_H

#include <QWidget>
#include "include.h"
#include "editdata.h"
#include "setting.h"
#include "curvegraphlabel.h"

namespace Ui {
    class CurveEditorForm;
}

class CurveEditorForm : public QWidget
{
    Q_OBJECT

public:
	explicit CurveEditorForm(CEditData *pEditData, CSettings *pSetting, QWidget *parent = 0);
    ~CurveEditorForm();

protected:
	void paintEvent(QPaintEvent *event) ;

public slots:
	void slot_clickedListView(QModelIndex index) ;
	void slot_resizeFrame(QResizeEvent *) ;
	void slot_changeSelLayer(QModelIndex) ;

private:
	Ui::CurveEditorForm *ui ;
	CEditData			*m_pEditData ;
	CSettings			*m_pSetting ;

	CurveGraphLabel		*m_pGraphLabel ;
	float				m_fMag ;
};

#endif // CURVEEDITORFORM_H
