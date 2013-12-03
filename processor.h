#ifndef __Processor_h__
#define __Processor_h__
class Processor {
	public:
		enum Result {
			Done,			// process has done
			EventTaken,		// process pending
			Skip			// skip reast processor
		};
		virtual Result process(Message *msg) = 0;
};
#endif /* __Processor_h__ */
