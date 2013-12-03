#ifndef __ProcessorChain_h__
#define __ProcessorChain_h__
class ProcessorChain {
	public:
		enum Result {
			ChainDone,
			ChainSkiped,
			ChainEventTaken
		};
		ProcessorChain( const std::vector<Processor*>& prototype) : mChain(prototype) {}
		virtual Result process(Message* msg);
	protected:
		class ProcessContext {
			public:
				Message				mLastEvent;
				MessageProcessor   *mLastProcessor;
				int					mProcessorIndex;
		} mContext;
		std::vector<Processor*> mChain;
};
#endif /* __ProcessorChain_h__ */
