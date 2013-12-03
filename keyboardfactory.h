#ifndef __KeyboardFactory_h__
#define __KeyboardFactory_h__
#include "keyboard.h"
#include "moduleinfo.h"
#include "postable.h"
class HandleManager;
#define DECL_KEYBOARD_FACTORY(keyboardname) \
	class keyboardname##Factory : public KeyboardFactory { \
		public: \
				virtual Keyboard* create(HandleManager& mgr, Postable *receiver) { \
					if(mInfo) { \
						return new keyboardname(mgr, receiver, mInfo); \
					} \
					return NULL; \
				} \
	} _factory;\

class KeyboardFactory {
	public:
		KeyboardFactory() : mInfo(NULL) {}
		virtual ~KeyboardFactory() { if(mInfo) delete mInfo;}
		virtual Keyboard* create(HandleManager& mgr, Postable *receiver) = 0;
		void setInfo(ModuleInfo *info) { mInfo = info;}
	protected:
		ModuleInfo *mInfo;
};
#endif /* __KeyboardFactory_h__ */
