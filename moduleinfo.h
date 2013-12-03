#ifndef _ModuleInfo_h_
#define _ModuleInfo_h_
#include <string>
#include <iostream>
#include <stdexcept>
class TiXmlDocument;
class ModuleInfo 
{
	public:
		ModuleInfo() : mState(CREATED), mDoc(NULL) {}
		enum Type { KeyboardModule, MatrixModule, MessageProcessorModule, Other};
		Type type() const;
		const std::string& name() const;
		int version() const;
		const std::string& date() const;
		const std::string& author() const;
		std::string attribute(const std::string& name, const std::string& default_value) const;
		long attribute(const std::string& name, const long default_value) const;
		double attribute(const std::string& name, const double default_value) const;
		bool attribute(const std::string& name, const bool default_value) const;
		int open(const char *path); 
		int save(); 
		void close(); 
		void setAttribute(const std::string& name, const std::string& value);
		void setAttribute(const std::string& name, const long value);
		void setAttribute(const std::string& name, const double value);
		void setAttribute(const std::string& name, const bool value);
		void encode(std::string& buf) const;

	protected:
		enum State {
			CREATED,
			LOADED,
			CLOSED};
		const char *getItem(const std::string& name) const;
		long toInteger(const std::string& value) const;
		double toFloat(const std::string& value) const;
		bool toBoolean(const std::string& value) const;

		State			mState;
		std::string		mPath;
		Type			mType;
		std::string		mName;
		int				mVersion;
		std::string		mAuthor;
		std::string		mDate;
		TiXmlDocument  *mDoc;
};

std::ostream& operator<<(std::ostream& os, const ModuleInfo& mi);
#endif /* _ModuleInfo_h_ */
