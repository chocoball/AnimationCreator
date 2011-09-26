#include "setting.h"

void CSettings::read()
{
	QSettings settings(qApp->applicationDirPath() + "/settnig.ini", QSettings::IniFormat) ;
	qDebug() << "readRootSetting\n" << settings.allKeys() ;
	qDebug() << "file:" << qApp->applicationDirPath() + "/settnig.ini" ;

	settings.beginGroup("Global");
#if defined(Q_OS_WIN32)
	m_fileOpenDir = settings.value("cur_dir", QString(".\\")).toString() ;
#elif defined(Q_OS_MAC)
	m_fileOpenDir = settings.value("cur_dir", QString("/Users/")).toString() ;
#elif defined(Q_OS_LINUX)
	m_fileOpenDir = settings.value("cur_dir", QString("/home/")).toString() ;
#else
	#error OSが定義されてないよ
#endif
	m_fileSaveDir = settings.value("save_dir", m_fileOpenDir).toString() ;
	m_pngSaveDir = settings.value("png_dir", m_fileOpenDir).toString() ;
	m_jsonSaveDir = settings.value("json_dir", m_fileOpenDir).toString() ;

	QRgb col ;
	col = settings.value("anime_color", 0).toUInt() ;
	m_animeBGColor = QColor(qRed(col), qGreen(col), qBlue(col), qAlpha(col)) ;
	col = settings.value("image_color", 0).toUInt() ;
	m_imageBGColor = QColor(qRed(col), qGreen(col), qBlue(col), qAlpha(col)) ;
	m_bSaveImage = settings.value("save_image", false).toBool() ;
	m_bFlat = settings.value("save_flat_json", false).toBool() ;
	settings.endGroup();

	settings.beginGroup("MainWindow");
	m_mainWindowPos = settings.value("pos", QPoint(200, 200)).toPoint() ;
	m_mainWindowSize = settings.value("size", QSize(400, 400)).toSize() ;
	settings.endGroup();

	settings.beginGroup("AnimationWindow");
	m_anmWindowPos = settings.value("pos", QPoint(-1, -1)).toPoint() ;
	m_anmWindowSize = settings.value("size", QSize(-1, -1)).toSize() ;
	m_bUseBackImage = settings.value("use_back_image", false).toBool() ;
	m_backImagePath = settings.value("back_image", "").toString() ;
	m_bDrawFrame = settings.value("disp_frame", true).toBool() ;
	m_bDrawCenter = settings.value("disp_center", false).toBool() ;
	m_anmWindowTreeWidth = settings.value("tree_width", -1).toInt() ;
	m_anmWindowTreeWidthIndex = settings.value("tree_width_idx", -1).toInt() ;
	settings.endGroup();

	settings.beginGroup("ImageWindow");
	m_imgWindowPos = settings.value("pos", QPoint(-1, -1)).toPoint() ;
	m_imgWindowSize = settings.value("size", QSize(-1, -1)).toSize() ;
	settings.endGroup();

	settings.beginGroup("LoupeWindow");
	m_loupeWindowPos = settings.value("pos", QPoint(-1, -1)).toPoint() ;
	m_loupeWindowSize = settings.value("size", QSize(-1, -1)).toSize() ;
	settings.endGroup();
}

void CSettings::write()
{
	QSettings settings(qApp->applicationDirPath() + "/settnig.ini", QSettings::IniFormat) ;
	qDebug() << "writeRootSetting writable:" << settings.isWritable() ;
	qDebug() << "file:" << qApp->applicationDirPath() + "/settnig.ini" ;

	settings.beginGroup("Global");
	settings.setValue("cur_dir",		m_fileOpenDir) ;
	settings.setValue("save_dir",		m_fileSaveDir) ;
	settings.setValue("png_dir",		m_pngSaveDir) ;
	settings.setValue("json_dir",		m_jsonSaveDir) ;
	settings.setValue("anime_color",	m_animeBGColor.rgba()) ;
	settings.setValue("image_color",	m_imageBGColor.rgba()) ;
	settings.setValue("save_image",		m_bSaveImage) ;
	settings.setValue("save_flat_json",	m_bFlat) ;
	settings.endGroup();

	settings.beginGroup("MainWindow");
	settings.setValue("pos",	m_mainWindowPos) ;
	settings.setValue("size",	m_mainWindowSize) ;
	settings.endGroup();

	settings.beginGroup("AnimationWindow");
	settings.setValue("pos",			m_anmWindowPos) ;
	settings.setValue("size",			m_anmWindowSize) ;
	settings.setValue("use_back_image",	m_bUseBackImage);
	settings.setValue("back_image",		m_backImagePath);
	settings.setValue("disp_frame",		m_bDrawFrame);
	settings.setValue("disp_center",	m_bDrawCenter) ;
	settings.setValue("tree_width",		m_anmWindowTreeWidth);
	settings.setValue("tree_width_idx",	m_anmWindowTreeWidthIndex);
	settings.endGroup();

	settings.beginGroup("ImageWindow");
	settings.setValue("pos",			m_imgWindowPos) ;
	settings.setValue("size",			m_imgWindowSize) ;
	settings.endGroup();

	settings.beginGroup("LoupeWindow");
	settings.setValue("pos",			m_loupeWindowPos) ;
	settings.setValue("size",			m_loupeWindowSize) ;
	settings.endGroup();
}
