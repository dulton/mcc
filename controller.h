#ifndef __Controller_h__
#define __Controller_h__
class Controller :
	public Postable,
	public KeyboardManager,
	public MatrixManager,
	public ThreadIf
{
	public:
		void addMessageHandler(MessageHandler *handler) {
			mHandlers.push_back(handler);
		}
		int loadKeyboard(const char *name) { return loadKeyboard(name, this);}
		int loadMatrix(const char* name) { return loadMatrix(name, this);}
	protected:
		virtual void thread() {
			Message *msg;
			while(!isStop())
			{
				if(mFifo.get(msg, 1000))
				{
					assert(msg);
					std::vector<MessageHandler*>::iterator i;
					for(i=mHandlers.begin(); i!= mHandlers.end() ;++i)
					{
						if(*i && !i->onMessage(*msg))
							break;
					}
					delete msg;
				}
			}
		}
		std::vector<MessageHandler*> mHandlers;
};
#endif /* __Controller_h__ */
