#ifndef __KeyboardManager_h__
#define __KeyboardManager_h__
#include <map>
#include <string>
#include "keyboard.h"
#include "keyboardfactory.h"
#include "handlemanager.h"
class KeyboardManager :
	public HandleManager
{
	public:
		KeyboardManager(const char *module_path) : mModulePath(module_path), mCounter(0) {}
		virtual Keyboard::Id loadKeyboard(const char *name, Postable* receiver, void *param = NULL);
		virtual void unloadKeyboard(Keyboard::Id id);
		virtual Keyboard* getKeyboard(Keyboard::Id id);
	private:
		typedef std::map<std::string, KeyboardFactory*> FactoryMap;
		FactoryMap  mFactories;
		std::string mModulePath;
		int mCounter;
};
#endif /* __KeyboardManager_h__ */
