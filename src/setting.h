#ifndef SETTING_H
#define SETTING_H

#include <QtGui>
#include "include.h"


class CSettings
{
public:
	void setAnmWindowTreeWidth(int w, int idx) {
		m_anmWindowTreeWidth = w ;
		m_anmWindowTreeWidthIndex = idx ;
	}
	int getAnmWindowTreeWidth() {
		return m_anmWindowTreeWidth ;
	}
	int getAnmWindowTreeWidthIndex() {
		return m_anmWindowTreeWidthIndex ;
	}

	kAccessor(QString, m_fileOpenDir, OpenDir)
	kAccessor(QString, m_fileSaveDir, SaveDir)
	kAccessor(QString, m_pngSaveDir, SavePngDir)
	kAccessor(bool, m_bSaveImage, SaveImage)
	kAccessor(QColor, m_animeBGColor, AnimeBGColor)
	kAccessor(QColor, m_imageBGColor, ImageBGColor)
	kAccessor(QString, m_backImagePath, BackImagePath)
	kAccessor(bool, m_bUseBackImage, UseBackImage)
	kAccessor(bool, m_bDrawFrame, DrawFrame)
	kAccessor(bool, m_bDrawCenter, DrawCenter)
	kAccessor(QPoint, m_anmWindowPos, AnmWindowPos)
	kAccessor(QSize, m_anmWindowSize, AnmWindowSize)
	kAccessor(QPoint, m_imgWindowPos, ImgWindowPos)
	kAccessor(QSize, m_imgWindowSize, ImgWindowSize)
	kAccessor(QPoint, m_loupeWindowPos, LoupeWindowPos)
	kAccessor(QSize, m_loupeWindowSize, LoupeWindowSize)

private:
	int			m_anmWindowTreeWidth ;
	int			m_anmWindowTreeWidthIndex ;
};

#endif // SETTING_H
