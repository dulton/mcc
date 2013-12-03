#ifndef __KeyboardMessageHelper_h__
#define __KeyboardMessageHelper_h__
#include "keyboardswitchmessage.h"
#include "keyboardptzmessage.h"
class KeyboardMessageHelper {
	public:
		static Message *buildSwitchMessage(int keyboard_handle, int matrix, int camera, int monitor)
		{
			KeyboardSwitchMessage *msg = new KeyboardSwitchMessage;
			msg->mHandle = keyboard_handle;
			msg->mMatrix = matrix;
			msg->mCamera = camera;
			msg->mInPort= camera;
			msg->mMonitor = monitor;
			msg->mOutPort= monitor;
			return msg;
		}
		static Message *buildPtzMessage(int keyboard_handle, int matrix, int camera, int zi_speed, int zo_speed, int vspeed, int hspeed)
		{
			KeyboardPtzMessage *msg = new KeyboardPtzMessage;
			msg->mHandle = keyboard_handle;
			msg->mMatrix = matrix;
			msg->mCamera = camera;
			msg->mZoomInSpeed = zi_speed;
			msg->mZoomOutSpeed = zo_speed;
			msg->mvSpeed = vspeed;
			msg->mhSpeed = hspeed;
		}
};
#endif /* __KeyboardMessageHelper_h__ */
