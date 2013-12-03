#ifndef __KeyboardMessage_h__
#define __KeyboardMessage_h__
#include "message.h"
class KeyboardMessage : public Message {
	public:
		int handle() const { return mHandle;}
		inline int in() const { return mPortIn;}
		inline int out() const { return mPortOut;}
		inline int matrix() const { return mMatrix;}
		inline int camera() const { return mCamera;}
	protected:
		int mHandle;
		int mPortIn;
		int mPortOut;
		int mCamera;
		int mMatrix;
};
#endif /* __KeyboardMessage_h__ */
