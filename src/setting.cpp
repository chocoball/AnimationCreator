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
	m_fileSaveDir	= settings.value("save_dir", m_fileOpenDir).toString() ;
	m_pngSaveDir	= settings.value("png_dir", m_fileOpenDir).toString() ;
	m_jsonSaveDir	= settings.value("json_dir", m_fileOpenDir).toString() ;
	m_asmSaveDir	= settings.value("asm_dir", m_fileOpenDir).toString() ;

	QRgb col_anm = settings.value("anime_color", 0).toUInt() ;
	QRgb col_img = settings.value("image_color", 0).toUInt() ;
	m_animeBGColor		= QColor(qRed(col_anm), qGreen(col_anm), qBlue(col_anm), qAlpha(col_anm)) ;
	m_imageBGColor		= QColor(qRed(col_img), qGreen(col_img), qBlue(col_img), qAlpha(col_img)) ;
	m_bSaveImage		= settings.value("save_image", false).toBool() ;
	m_bFlat				= settings.value("save_flat_json", false).toBool() ;
	m_bLayerHierarchy	= settings.value("layer_hierarchy", false).toBool() ;
	m_frameStart		= settings.value("frame_start", 0).toInt() ;
	m_frameEnd			= settings.value("frame_end", 30).toInt() ;
	m_bBackup			= settings.value("backup", false).toBool() ;
	m_backupNum			= settings.value("backup_num", 1).toInt() ;
	settings.endGroup();

	settings.beginGroup("MainWindow");
	m_mainWindowGeometry	= settings.value("geometry").toByteArray() ;
	m_mainWindowState		= settings.value("state").toByteArray() ;
	settings.endGroup();

	settings.beginGroup("AnimationWindow");
	m_anmWindowGeometry			= settings.value("geometry").toByteArray() ;
	m_bUseBackImage				= settings.value("use_back_image", false).toBool() ;
	m_backImagePath				= settings.value("back_image", "").toString() ;
	m_bDrawFrame				= settings.value("disp_frame", true).toBool() ;
	m_bDrawCenter				= settings.value("disp_center", false).toBool() ;
	m_anmWindowTreeWidth		= settings.value("tree_width", -1).toInt() ;
	m_anmWindowTreeWidthIndex	= settings.value("tree_width_idx", -1).toInt() ;
	m_anmWindowScreenW			= settings.value("scr_w", 0).toInt() ;
	m_anmWindowScreenH			= settings.value("scr_h", 0).toInt() ;
	m_anmWindowW				= settings.value("win_w", 2048).toInt() ;
	m_anmWindowH				= settings.value("win_h", 2048).toInt() ;
	m_bUseDepthTest				= settings.value("use_depth_test", true).toBool() ;
	m_bUseZSort					= settings.value("use_zsort", true).toBool() ;
	settings.endGroup();

	settings.beginGroup("ImageWindow");
	m_imgWindowGeometry = settings.value("geometry").toByteArray() ;
	settings.endGroup();

	settings.beginGroup("LoupeWindow");
	m_loupeWindowGeometry = settings.value("geometry").toByteArray() ;
	settings.endGroup();

	settings.beginGroup("CurveEditorWindow") ;
	m_curveWindowGeometry		= settings.value("geometry").toByteArray() ;
	m_curveSplitterWidth		= settings.value("splitter_width", -1).toInt() ;
	m_curveSplitterWidthIndex	= settings.value("splitter_width_idx", -1).toInt() ;
	settings.endGroup() ;

	settings.beginGroup("Shortcut");
	m_scPosSelect		= QKeySequence(settings.value("pos", "Z").toString()) ;
	m_scRotSelect		= QKeySequence(settings.value("rot", "X").toString()) ;
	m_scCenterSelect	= QKeySequence(settings.value("center", "C").toString()) ;
	m_scScaleSelect		= QKeySequence(settings.value("scale", "V").toString()) ;
	m_scPathSelect		= QKeySequence(settings.value("path", "B").toString()) ;
	m_scCopyFrame		= QKeySequence(settings.value("copy_frame", "Ctrl+C").toString()) ;
	m_scPasteFrame		= QKeySequence(settings.value("paste_frame", "Ctrl+V").toString()) ;
	m_scPlayAnime		= QKeySequence(settings.value("play_anime", "").toString()) ;
	m_scStopAnime		= QKeySequence(settings.value("stop_anime", "").toString()) ;
	m_scJumpStartFrame	= QKeySequence(settings.value("jump_start", "").toString()) ;
	m_scJumpEndFrame	= QKeySequence(settings.value("jump_end", "").toString()) ;
	m_scAddFrameData	= QKeySequence(settings.value("add_frame", "").toString()) ;
	m_scDelFrameData	= QKeySequence(settings.value("del_frame", "").toString()) ;
	m_scDelItem			= QKeySequence(settings.value("del_item", "").toString()) ;
	m_scDispItem		= QKeySequence(settings.value("disp_item", "").toString()) ;
	m_scLockItem		= QKeySequence(settings.value("lock_item", "").toString()) ;
	m_scMoveAnimeWindow	= QKeySequence(settings.value("move_anm_win", "").toString()) ;
	m_scLockLoupe		= QKeySequence(settings.value("lock_loupe", "").toString()) ;
	m_scCopyAllFrame	= QKeySequence(settings.value("copy_allframe", "").toString()) ;
	m_scPasteAllFrame	= QKeySequence(settings.value("paste_allframe", "").toString()) ;
	m_scDeleteAllFrame	= QKeySequence(settings.value("delete_allframe", "").toString()) ;
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
	settings.setValue("asm_dir",		m_asmSaveDir) ;
	settings.setValue("anime_color",	m_animeBGColor.rgba()) ;
	settings.setValue("image_color",	m_imageBGColor.rgba()) ;
	settings.setValue("save_image",		m_bSaveImage) ;
	settings.setValue("save_flat_json",	m_bFlat) ;
	settings.setValue("layer_hierarchy",m_bLayerHierarchy) ;
	settings.setValue("frame_start",	m_frameStart) ;
	settings.setValue("frame_end",		m_frameEnd) ;

	settings.setValue("backup",			m_bBackup) ;
	settings.setValue("backup_num",		m_backupNum) ;
	settings.endGroup();

	settings.beginGroup("MainWindow");
	settings.setValue("geometry",		m_mainWindowGeometry) ;
	settings.setValue("state",			m_mainWindowState) ;
	settings.endGroup();

	settings.beginGroup("AnimationWindow");
	settings.setValue("geometry",		m_anmWindowGeometry) ;
	settings.setValue("use_back_image",	m_bUseBackImage);
	settings.setValue("back_image",		m_backImagePath);
	settings.setValue("disp_frame",		m_bDrawFrame);
	settings.setValue("disp_center",	m_bDrawCenter) ;
	settings.setValue("tree_width",		m_anmWindowTreeWidth);
	settings.setValue("tree_width_idx",	m_anmWindowTreeWidthIndex);
	settings.setValue("scr_w",			m_anmWindowScreenW) ;
	settings.setValue("scr_h",			m_anmWindowScreenH) ;
	settings.setValue("win_w",			m_anmWindowW) ;
	settings.setValue("win_h",			m_anmWindowH) ;
	settings.setValue("use_depth_test",	m_bUseDepthTest);
	settings.setValue("use_zsort",		m_bUseZSort);
	settings.endGroup();

	settings.beginGroup("ImageWindow");
	settings.setValue("geometry", m_imgWindowGeometry) ;
	settings.endGroup();

	settings.beginGroup("LoupeWindow");
	settings.setValue("geometry", m_loupeWindowGeometry) ;
	settings.endGroup();

	settings.beginGroup("CurveEditorWindow") ;
	settings.setValue("geometry",			m_curveWindowGeometry) ;
	settings.setValue("splitter_width",		m_curveSplitterWidth) ;
	settings.setValue("splitter_width_idx",	m_curveSplitterWidthIndex) ;
	settings.endGroup() ;

	settings.beginGroup("Shortcut");
	settings.setValue("pos",				m_scPosSelect.toString()) ;
	settings.setValue("rot",				m_scRotSelect.toString()) ;
	settings.setValue("center",				m_scCenterSelect.toString()) ;
	settings.setValue("scale",				m_scScaleSelect.toString()) ;
	settings.setValue("path",				m_scPathSelect.toString()) ;
	settings.setValue("copy_frame",			m_scCopyFrame.toString()) ;
	settings.setValue("paste_frame",		m_scPasteFrame.toString()) ;
	settings.setValue("play_anime",			m_scPlayAnime) ;
	settings.setValue("stop_anime",			m_scStopAnime) ;
	settings.setValue("jump_start",			m_scJumpStartFrame) ;
	settings.setValue("jump_end",			m_scJumpEndFrame) ;
	settings.setValue("add_frame",			m_scAddFrameData) ;
	settings.setValue("del_frame",			m_scDelFrameData) ;
	settings.setValue("del_item",			m_scDelItem) ;
	settings.setValue("disp_item",			m_scDispItem) ;
	settings.setValue("lock_item",			m_scLockItem) ;
	settings.setValue("move_anm_win",		m_scMoveAnimeWindow) ;
	settings.setValue("lock_loupe",			m_scLockLoupe) ;
	settings.setValue("copy_allframe",		m_scCopyAllFrame) ;
	settings.setValue("paste_allframe",		m_scPasteAllFrame) ;
	settings.setValue("delete_allframe",	m_scDeleteAllFrame) ;

	settings.endGroup();
}
