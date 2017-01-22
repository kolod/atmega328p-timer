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

	// ���� ��������� ���������� ���������� �������
	if (state != mState) {
		mCounter = 0;
		mState = state;
		return;
	}

	mCounter++;

	// ������ �� ������
	if (!state) {
		if (mCounter > 10) {
			mPressDone = false;
			mLongPressDone = false;
		}
		return;
	}

	// ������ ������� �� ������� �������
	if (!mPressDone && (mCounter > 10)) {
		mPressDone = true;
		if (mOnPressed != nullptr) mOnPressed();
		return;
	}
	
	if (mCounter > mTimeout) {
		mCounter = 0;

		// ��������� ������� ������� �������
		if (mOnRepeatPress != nullptr) {
			mOnRepeatPress();
			return;
		}

		// ������� ������� �������
		if (mOnLongPressed != nullptr) {
			if (!mLongPressDone) {
				mLongPressDone = true;
				mOnLongPressed();
			}
		}
	}
}