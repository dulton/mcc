#include "keyboardmanager.h"
#include "moduleinfo.h"
#include "keyboardfactory.h"
#include "keyboard.h"
#include <dlfcn.h>
#include <cassert>
#include "log.h"
using namespace std;
Keyboard::Id KeyboardManager::loadKeyboard(const char *name, Postable *receiver, void *param)
{
	KeyboardFactory *f = NULL;
	Keyboard* keyboard = NULL;
	string path = name;
	FactoryMap::iterator i = mFactories.find(path);
	if(i != mFactories.end())
	{
		f = i->second;
		assert(f);
	}
	else
	{
		ModuleInfo *info = new ModuleInfo();
		if(!info)
		{
			ErrLog("Module Infomation File["<<name<<".xml"<<"] not found");
			return -1;
		}
		path = mModulePath;
		path += name;
		path += ".xml";
		try {
			info->open(path.c_str());
		}
		catch(runtime_error e)
		{
			ErrLog("Failed to parse keyboard module infomation file "<<path<<" error message:"<<e.what());
			return -1;
		}
		if(info->type() != ModuleInfo::KeyboardModule)
		{
			ErrLog("Module ["<<path<<"] type is not Keyboard");
			delete info;
			return -1;
		}
		InfoLog("Keyboard Module Infomation:"<<endl<<*info);
		path = mModulePath;
		path += name;
		path += ".so";
		void *h = dlopen(path.c_str(), RTLD_LAZY | RTLD_NODELETE);
		//void *h = dlopen("./testkeyboard.so", RTLD_LAZY | RTLD_NODELETE);
		if(!h)
		{
			ErrLog("Failed to load module "<<path.c_str()<<" error:"<<dlerror());
			dlclose(h);
			return -1;
		}
		f = (KeyboardFactory*)dlsym(h, "_factory");
		if(!f)
		{
			ErrLog("KeyboardFactory not found in module file");
			dlclose(h);
			return -1;
		}
		dlclose(h);
		f->setInfo(info);
		path = name;
		mFactories[path] = f;
		InfoLog("Load new keyboard factory");
	}
	keyboard = f->create(*this, receiver);	
	if(!keyboard)
	{
		ErrLog("KeyboardFactory failed to create keyboard module instance");
		return 0;
	}
	if(keyboard->start(param) != 0)
	{
		delete keyboard;
		ErrLog("failed to start keyboard module instance");
		return 0;
	}
	return keyboard->handle();
	/*
	if(keyboard)
	{
		if(keyboard->start(param) != 0)
		{
			delete keyboard;
			ErrLog("failed to start keyboard module instance");
			return -1;
		}
		mKeyboards[mCounter++] = keyboard;
		InfoLog("Keyboard Module Instance created successfully");
		keyboard->setHandle(mCounter - 1);
		return mCounter - 1;
	}
	else
	{
		ErrLog("KeyboardFactory failed to create keyboard module instance");
		return -1;
	}
	*/
}

void KeyboardManager::unloadKeyboard(Keyboard::Id id)
{
	Keyboard *kb = getKeyboard(id);
	if(kb)
	{
		kb->stop();
		delete kb;
	}
}

Keyboard * KeyboardManager::getKeyboard(Keyboard::Id id)
{
	return static_cast<Keyboard*>(getHandled(id));
}
