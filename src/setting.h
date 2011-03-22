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
};

#endif // SETTING_H
