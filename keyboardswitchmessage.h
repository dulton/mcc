#ifndef __KeyboardSwitchMessage_h__
#define __KeyboardSwitchMessage_h__
#include "keyboardmessage.h"
//#include "keyboardmessagehelper.h"
class KeyboardMessageHelper;
class KeyboardSwitchMessage : public KeyboardMessage {
	public:
		inline int monitor() const { return mMonitor;}
		virtual Message* clone() const { return new KeyboardSwitchMessage(*this);}
		virtual std::ostream& encode(std::ostream& os) const { return os;}
	protected:
		friend class KeyboardMessageHelper;
		int mMonitor;
};

#endif /* __KeyboardSwitchMessage_h__ */
