#ifndef __Module_h__
#define __Module_h__
class Module {
	public:
		virtual int start(ModuleInfo* info, void *param) = 0;
		virtual void stop() = 0;
};
#endif /* __Module_h__ */
