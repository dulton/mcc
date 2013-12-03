#ifndef __KeyboardPtzMessage_h__
#define __KeyboardPtzMessage_h__
#include "keyboardmessage.h"
//#include "keyboardmessagehelper.h"
class KeyboardMessageHelper;
class KeyboardPtzMessage : public KeyboardMessage {
	public:
		inline int zoomInSpeed() const { return mZoomInSpeed;}
		inline int zoomOutSpeed() const { return mZoomOutSpeed;}
		inline int hSpeed() const { return mhSpeed;}
		inline int vSpeed() const { return mvSpeed;}
		virtual Message* clone() const { return new KeyboardPtzMessage(*this);}
		virtual std::ostream& encode(std::ostream& os) const { return os;}
	protected:
		friend class KeyboardMessageHelper;
		int mZoomInSpeed;
		int mZoomOutSpeed;
		int mhSpeed;
		int mvSpeed;
};
#endif /* __KeyboardPtzMessage_h__ */
