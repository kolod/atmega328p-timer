// key.h

#ifndef _KEY_h
#define _KEY_h

#include <arduino.h>

typedef void(*callback)();

class Key
{
public:
	explicit Key(int timeout = 0, callback onPressed = nullptr, callback onRepeatPress = nullptr, callback mOnLongPressed = nullptr);
	void update(boolean state);

private:
	boolean mState;
	boolean mPressDone;
	boolean mLongPressDone;
	int mTimeout;
	int mCounter;
	callback mOnPressed;
	callback mOnRepeatPress;
	callback mOnLongPressed;
};

#endif

