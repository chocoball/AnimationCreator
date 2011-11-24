#ifndef SETTING_H
#define SETTING_H

#include <QtGui>
#include "include.h"


class CSettings
{
public:
	void read() ;
	void write() ;

	kAccessor(QString,		m_fileOpenDir,				OpenDir)
	kAccessor(QString,		m_fileSaveDir,				SaveDir)
	kAccessor(QString,		m_pngSaveDir,				SavePngDir)
	kAccessor(QString,		m_jsonSaveDir,				SaveJsonDir)
	kAccessor(QString,		m_asmSaveDir,				SaveAsmDir)
	kAccessor(bool,			m_bSaveImage,				SaveImage)
	kAccessor(QColor,		m_animeBGColor,				AnimeBGColor)
	kAccessor(QColor,		m_imageBGColor,				ImageBGColor)
	kAccessor(QString,		m_backImagePath,			BackImagePath)
	kAccessor(bool,			m_bUseBackImage,			UseBackImage)
	kAccessor(bool,			m_bDrawFrame,				DrawFrame)
	kAccessor(bool,			m_bDrawCenter,				DrawCenter)
	kAccessor(bool,			m_bFlat,					Flat)
	kAccessor(bool,			m_bLayerHierarchy,			LayerHierarchy)
	kAccessor(QByteArray,	m_mainWindowGeometry,		MainWindowGeometry)
	kAccessor(QByteArray,	m_mainWindowState,			MainWindowState)
	kAccessor(QByteArray,	m_anmWindowGeometry,		AnmWindowGeometry)
	kAccessor(QByteArray,	m_imgWindowGeometry,		ImgWindowGeometry)
	kAccessor(QByteArray,	m_loupeWindowGeometry,		LoupeWindowGeometry)
	kAccessor(QByteArray,	m_curveWindowGeometry,		CurveWindowGeometry)
	kAccessor(int,			m_anmWindowTreeWidth,		AnmWindowTreeWidth)
	kAccessor(int,			m_anmWindowTreeWidthIndex,	AnmWindowTreeWidthIndex)
	kAccessor(int,			m_anmWindowScreenW,			AnmWindowScreenW)
	kAccessor(int,			m_anmWindowScreenH,			AnmWindowScreenH)
	kAccessor(int,			m_curveSplitterWidth,		CurveSplitterWidth)
	kAccessor(int,			m_curveSplitterWidthIndex,	CurveSplitterWidthIndex)

	kAccessor(QKeySequence,	m_scPosSelect,				ShortcutPosSelect)
	kAccessor(QKeySequence,	m_scRotSelect,				ShortcutRotSelect)
	kAccessor(QKeySequence,	m_scCenterSelect,			ShortcutCenterSelect)
	kAccessor(QKeySequence,	m_scScaleSelect,			ShortcutScaleSelect)
	kAccessor(QKeySequence,	m_scPathSelect,				ShortcutPathSelect)
	kAccessor(QKeySequence,	m_scCopyFrame,				ShortcutCopyFrame)
	kAccessor(QKeySequence,	m_scPasteFrame,				ShortcutPasteFrame)

private:
};

#endif // SETTING_H
