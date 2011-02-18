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

private:
	QString		mCurrDir ;
	QString		mCurrSaveDir ;
	QString		mCurrPngDir ;

	bool		m_bSaveImage ;		///< XML保存時、画像データも保存するならtrue
	QColor		m_AnimeBGColor ;	///< アニメーションウィンドウのBG色
	QColor		m_ImageBGColor ;	///< イメージウィンドウのBG色
};

#endif // SETTING_H
