#ifndef __User_h__
#define __User_h__
class User {
	public:
		enum {
			LowestPriority = -1
		};
		User() : mPriority(LowestPriority) {}
		int priority() const { return mPriority;}
	protected:
		int mPriority;
};
#endif /* __User_h__ */
