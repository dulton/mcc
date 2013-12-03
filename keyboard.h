#ifndef __Keyboard_h__
#define __Keyboard_h__
#include "moduleinfo.h"
#include "postable.h"
#include "user.h"
#include "handled.h"
class KeyboardManager;
class Keyboard : 
	public Postable,
	public Handled
{
	public:
		Keyboard(HandleManager &mgr, Postable *receiver, ModuleInfo *info) : 
			Handled(mgr), 
			mReceiver(receiver),
			mInfo(info) {}
		virtual int start(void *param) = 0;
		virtual void stop() = 0;
		const User& user() const { return mUser;}
		void bindUser(const User& userToBind) { mUser = userToBind;}
		Keyboard::Id handle() const { return mId;}
	protected:
		friend class KeyboardManager;
		void notify(Message* msg) { if(mReceiver) mReceiver->post(msg);}
		Postable		   *mReceiver;
		User				mUser;
		ModuleInfo		   *mInfo;
};
#endif /* __Keyboard_h__ */
