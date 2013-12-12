#ifndef __User_h__
#define __User_h__
#include <string>
class User {
	public:
		enum {
			LowestPriority = -1
		};
		User() : mPriority(LowestPriority) {}
		int priority() const { return mPriority;}
		int groupId() const { return mGroupId;}
		int id() const { return mUserId;}
		const std::string& username() const { return mUserName;}
	protected:
		int mGroupId;
		int mUserId;
		int mPriority;
		std::string mUserName;
};
#endif /* __User_h__ */
