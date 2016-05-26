#ifndef SETTING_H
#define SETTING_H

#include "include.h"
#include <QtGui>

class CSettings
{
public:
    void read();
    void write();

    kAccessor(QString, m_fileOpenDir, OpenDir);
    kAccessor(QString, m_fileSaveDir, SaveDir);
    kAccessor(QString, m_pngSaveDir, SavePngDir);
    kAccessor(QString, m_jsonSaveDir, SaveJsonDir);
    kAccessor(QString, m_asmSaveDir, SaveAsmDir);
    kAccessor(QString, m_backImagePath, BackImagePath);

    kAccessor(QColor, m_animeBGColor, AnimeBGColor);
    kAccessor(QColor, m_imageBGColor, ImageBGColor);

    kAccessor(bool, m_bSaveImage, SaveImage);
    kAccessor(bool, m_bUseBackImage, UseBackImage);
    kAccessor(bool, m_bDrawFrame, DrawFrame);
    kAccessor(bool, m_bDrawCenter, DrawCenter);
    kAccessor(bool, m_bFlat, Flat);
    kAccessor(bool, m_bLayerHierarchy, LayerHierarchy);
    kAccessor(bool, m_bUseDepthTest, UseDepthTest);
    kAccessor(bool, m_bUseZSort, UseZSort);
    kAccessor(bool, m_bBackup, Backup);
    kAccessor(bool, m_bCheckGrid, CheckGrid);
    kAccessor(bool, m_bCheckLinearFilter, CheckLinearFilter);

    kAccessor(int, m_anmWindowTreeWidth, AnmWindowTreeWidth);
    kAccessor(int, m_anmWindowTreeWidthIndex, AnmWindowTreeWidthIndex);
    kAccessor(int, m_anmWindowScreenW, AnmWindowScreenW);
    kAccessor(int, m_anmWindowScreenH, AnmWindowScreenH);
    kAccessor(int, m_anmWindowW, AnmWindowW);
    kAccessor(int, m_anmWindowH, AnmWindowH);
    kAccessor(int, m_curveSplitterWidth, CurveSplitterWidth);
    kAccessor(int, m_curveSplitterWidthIndex, CurveSplitterWidthIndex);
    kAccessor(int, m_frameStart, FrameStart);
    kAccessor(int, m_frameEnd, FrameEnd);
    kAccessor(int, m_backupNum, BackupNum);

    kAccessor(QByteArray, m_mainWindowGeometry, MainWindowGeometry);
    kAccessor(QByteArray, m_mainWindowState, MainWindowState);
    kAccessor(QByteArray, m_anmWindowGeometry, AnmWindowGeometry);
    kAccessor(QByteArray, m_imgWindowGeometry, ImgWindowGeometry);
    kAccessor(QByteArray, m_loupeWindowGeometry, LoupeWindowGeometry);
    kAccessor(QByteArray, m_curveWindowGeometry, CurveWindowGeometry);

    kAccessor(QKeySequence, m_scPosSelect, ShortcutPosSelect);
    kAccessor(QKeySequence, m_scRotSelect, ShortcutRotSelect);
    kAccessor(QKeySequence, m_scCenterSelect, ShortcutCenterSelect);
    kAccessor(QKeySequence, m_scScaleSelect, ShortcutScaleSelect);
    kAccessor(QKeySequence, m_scPathSelect, ShortcutPathSelect);
    kAccessor(QKeySequence, m_scCopyFrame, ShortcutCopyFrame);
    kAccessor(QKeySequence, m_scPasteFrame, ShortcutPasteFrame);
    kAccessor(QKeySequence, m_scPlayAnime, ShortcutPlayAnime);
    kAccessor(QKeySequence, m_scStopAnime, ShortcutStopAnime);
    kAccessor(QKeySequence, m_scJumpStartFrame, ShortcutJumpStartFrame);
    kAccessor(QKeySequence, m_scJumpEndFrame, ShortcutJumpEndFrame);
    kAccessor(QKeySequence, m_scAddFrameData, ShortcutAddFrameData);
    kAccessor(QKeySequence, m_scDelFrameData, ShortcutDelFrameData);
    kAccessor(QKeySequence, m_scDelItem, ShortcutDelItem);
    kAccessor(QKeySequence, m_scDispItem, ShortcutDispItem);
    kAccessor(QKeySequence, m_scLockItem, ShortcutLockItem);
    kAccessor(QKeySequence, m_scMoveAnimeWindow, ShortcutMoveAnimeWindow);
    kAccessor(QKeySequence, m_scLockLoupe, ShortcutLockLoupe);
    kAccessor(QKeySequence, m_scCopyAllFrame, ShortcutCopyAllFrame);
    kAccessor(QKeySequence, m_scPasteAllFrame, ShortcutPasteAllFrame);
    kAccessor(QKeySequence, m_scDeleteAllFrame, ShortcutDeleteAllFrame);

private:
};

#endif // SETTING_H
