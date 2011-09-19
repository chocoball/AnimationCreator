#ifndef SETTING_H
#define SETTING_H

#include <QtGui>

class CSettings
{
public:
	void setCurrentDir( QString &str )		{ mCurrDir = str ; }
	QString &getCurrentDir()				{ return mCurrDir ; }

	void setCurrentSaveDir( QString &str )	{ mCurrSaveDir = str ; }
	QString &getCurrentSaveDir()			{ return mCurrSaveDir ; }

	void setCurrentPNGDir( QString &str )	{ mCurrPngDir = str ; }
	QString &getCurrentPNGDir()				{ return mCurrPngDir ; }

	void setSaveImage(bool flag)			{ m_bSaveImage = flag ; }
	bool getSaveImage()						{ return m_bSaveImage ; }

	void setAnimeBGColor(QColor col)		{ m_AnimeBGColor = col ; }
	QColor getAnimeBGColor()				{ return m_AnimeBGColor ; }

	void setImageBGColor(QColor col)		{ m_ImageBGColor = col ; }
	QColor getImageBGColor()				{ return m_ImageBGColor ; }

	void setBackImagePath(QString &str)		{ m_BackImagePath = str ; }
	QString getBackImagePath()				{ return m_BackImagePath ; }

	void setUseBackImage(bool flag)			{ m_bUseBackImage = flag ; }
	bool getUseBackImage()					{ return m_bUseBackImage ; }

	void setDrawFrame(bool flag)			{ m_bDrawFrame = flag ; }
	bool getDrawFrame()						{ return m_bDrawFrame ; }

	void setDrawCenter(bool flag)			{ m_bDrawCenter = flag ; }
	bool getDrawCenter()					{ return m_bDrawCenter ; }

	void setAnmWindowPos(QPoint pos)		{ m_anmWindowPos = pos ; }
	QPoint getAnmWindowPos()				{ return m_anmWindowPos ; }
	void setAnmWindowSize(QSize size)		{ m_anmWindowSize = size ; }
	QSize getAnmWindowSize()				{ return m_anmWindowSize ; }

	void setImgWindowPos(QPoint pos)		{ m_imgWindowPos = pos ; }
	QPoint getImgWindowPos()				{ return m_imgWindowPos ; }
	void setImgWindowSize(QSize size)		{ m_imgWindowSize = size ; }
	QSize getImgWindowSize()				{ return m_imgWindowSize ; }

	void setLoupeWindowPos(QPoint pos)		{ m_loupeWindowPos = pos ; }
	QPoint getLoupeWindowPos()				{ return m_loupeWindowPos ; }
	void setLoupeWindowSize(QSize size)		{ m_loupeWindowSize = size ; }
	QSize getLoupeWindowSize()				{ return m_loupeWindowSize ; }

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

private:
	QString		mCurrDir ;
	QString		mCurrSaveDir ;
	QString		mCurrPngDir ;

	bool		m_bSaveImage ;		///< XML保存時、画像データも保存するならtrue
	QColor		m_AnimeBGColor ;	///< アニメーションウィンドウのBG色
	QColor		m_ImageBGColor ;	///< イメージウィンドウのBG色
	QString		m_BackImagePath ;
	bool		m_bUseBackImage ;
	bool		m_bDrawFrame ;
	bool		m_bDrawCenter ;

	QPoint		m_anmWindowPos ;
	QSize		m_anmWindowSize ;

	QPoint		m_imgWindowPos ;
	QSize		m_imgWindowSize ;

	QPoint		m_loupeWindowPos ;
	QSize		m_loupeWindowSize ;

	int			m_anmWindowTreeWidth ;
	int			m_anmWindowTreeWidthIndex ;
};

#endif // SETTING_H
