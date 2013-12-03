#ifndef __Matirx_h__
#define __Matirx_h__
#include "moduleinfo.h"
#include "postable.h"
class Matrix : public Postable {
	public:
		Matrix(Postable* receiver, ModuleInfo *info) : mInfo(info), mReceiver(receiver) {}
		virtual int start(void *param) = 0;
		virtual void stop() = 0;
	protected:
		inline void notify(Message* msg) { if(mReceiver) mReceiver->post(msg);}
		ModuleInfo *mInfo;
		Postable   *mReceiver;
};
#endif /* __Matirx_h__ */
