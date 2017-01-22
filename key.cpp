#include "key.h"

Key::Key(int timeout, callback onPressed, callback onRepeatPress, callback mOnLongPressed)
	: mState(false)
	, mPressDone(false)
	, mLongPressDone(false)
	, mTimeout(timeout)
	, mCounter(0)
	, mOnPressed(onPressed)
	, mOnRepeatPress(onRepeatPress)
	, mOnLongPressed(mOnLongPressed)
{}

void Key::update(boolean state) {

	// Если состояние поменялось сбрасываем счётчик
	if (state != mState) {
		mCounter = 0;
		mState = state;
		return;
	}

	mCounter++;

	// Кнопка не нажата
	if (!state) {
		if (mCounter > 10) {
			mPressDone = false;
			mLongPressDone = false;
		}
		return;
	}

	// Первое событие по нажатию клавиши
	if (!mPressDone && (mCounter > 10)) {
		mPressDone = true;
		if (mOnPressed != nullptr) mOnPressed();
		return;
	}
	
	if (mCounter > mTimeout) {
		mCounter = 0;

		// Повторное событие нажатия клавиши
		if (mOnRepeatPress != nullptr) {
			mOnRepeatPress();
			return;
		}

		// Длинное нажатие клавиши
		if (mOnLongPressed != nullptr) {
			if (!mLongPressDone) {
				mLongPressDone = true;
				mOnLongPressed();
			}
		}
	}
}