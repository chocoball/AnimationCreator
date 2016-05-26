#ifndef ANIMATIONWINDOWSPLITTER_H
#define ANIMATIONWINDOWSPLITTER_H

#include <QSplitter>

class AnimationWindowSplitter : public QSplitter
{
public:
	AnimationWindowSplitter(QWidget *parent = 0) ;
	~AnimationWindowSplitter() ;

	void MoveSplitter(int pos, int index) ;
} ;

#endif // ANIMATIONWINDOWSPLITTER_H
