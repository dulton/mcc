#include "moduleinfo.h"
#include <tinyxml.h>
#include <sstream>
using namespace std;

int ModuleInfo::open(const char *path)
{
	if(mDoc)
		return -1;
	mDoc = new TiXmlDocument(path);
	if(!mDoc)
		return -1;
	if(!mDoc->LoadFile())
		return -1;
	TiXmlHandle h(mDoc);
	// get module type;
	const TiXmlElement *e = h.FirstChild("Module").FirstChild("Infomation").FirstChild("Type").ToElement();
	try {
	if(e)
	{
		string strtype = e->GetText();
		if(strtype == "Keyboard")
			mType = KeyboardModule;
		else if(strtype == "Matrix")
			mType = MatrixModule;
		else if(strtype == "MessageProcessor")
			mType = MessageProcessorModule;
		else
			mType = Other;
	}
	else
		throw std::runtime_error("Module type not specified");

	// get module name
	e = h.FirstChild("Module").FirstChild("Infomation").FirstChild("Name").ToElement();
	if(e)
		mName = e->GetText();
	else
		throw std::runtime_error("Module name not specified");
	// get module version
	e = h.FirstChild("Module").FirstChild("Infomation").FirstChild("Version").ToElement();
	if(e)
	{
		mVersion = 0;
	}
	else
		throw std::runtime_error("Module name not specified");
	}
	catch(...)
	{
		delete mDoc;
		mDoc = NULL;
		mState = CLOSED;
		throw;
	}
	// get module author
	e = h.FirstChild("Module").FirstChild("Infomation").FirstChild("Author").ToElement();
	if(e)
		mAuthor= e->GetText();
	else
		mAuthor = "";
	// get module date
	e = h.FirstChild("Module").FirstChild("Infomation").FirstChild("Date").ToElement();
	if(e)
		mDate = e->GetText();
	else
		mDate = "";
	mState = LOADED;
	return 0;
}

void ModuleInfo::close()
{
	if(mState == LOADED)
	{
		mDoc->SaveFile();
		delete mDoc;
		mDoc = NULL;
		mState = CLOSED;
	}
}
ModuleInfo::Type ModuleInfo::type() const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	return mType;
}
const string& ModuleInfo::name() const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	return mName;
}
int ModuleInfo::version() const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	return mVersion;
}
const string& ModuleInfo::author() const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	return mAuthor;
}
const string& ModuleInfo::date() const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	return mDate;
}


string ModuleInfo::attribute(const string& name, const string& default_value) const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	const char *value = getItem(name);
	if(value)
		return string(value);
	else
		return default_value;
}
long  ModuleInfo::attribute(const string& name, const long default_value) const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	const char *value = getItem(name);
	if(value)
	{
		string str(value);
		return toInteger(value);
	}
	else
		return default_value;
}

double  ModuleInfo::attribute(const string& name, const double default_value) const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	const char *value = getItem(name);
	if(value)
	{
		string str(value);
		return toFloat(value);
	}
	else
		return default_value;
}

bool  ModuleInfo::attribute(const string& name, const bool default_value) const
{
	if(mState != LOADED)
		throw runtime_error("File not loaded!");
	const char *value = getItem(name);
	if(value)
	{
		string str(value);
		return toBoolean(value);
	}
	else
		return default_value;
}

const char *ModuleInfo::getItem(const string& name) const {
	TiXmlHandle h(mDoc);
	TiXmlElement *item= h.FirstChild("Module").FirstChild("Configuration").FirstChild("Item").ToElement();
	while(item && name != item->Attribute("name"))
	{
		item = item->NextSiblingElement();
	}
	if(item)
	{
		return item->GetText();
	}
	else
		return NULL;
}
long ModuleInfo::toInteger(const string& value) const
{
	long v = 0;
	const char *buf = value.c_str();
	const char *p = buf;
	for(p = buf;*p;++p)
	{
		if(*p >= '0' && *p <= '9')
		{
			v *= 10;
			v += (*p - '0');
		}
		else
			throw std::runtime_error("Invalid value for value type float");
	}
	return v;
}

double ModuleInfo::toFloat(const string& value) const
{
	bool point_part = false;
	const char *buf = value.c_str();
	const char *p = buf;
	double v = 0.0;
	double k = 1.0;
	for(p = buf;*p;++p)
	{
		if(*p >= '0' && *p <= '9')
		{
			v *= 10;
			v += ((*p - '0') * 1.0);
			if(point_part)
			{
				k *= 10;
			}
		}
		else if(*p == '.')
		{
			point_part = true;
			continue;
		}
		else
			throw std::runtime_error("Invalid value for value type float");
	}
	return v/k;
}

bool ModuleInfo::toBoolean(const string& value) const
{
	if(value == "1" || value == "true" || value == "yes" || value == "enable")
		return true;
	else if(value == "0" || value == "false" || value == "no" || value == "disable")
		return false;
	else
		throw std::runtime_error("Invalid value for value type boolean");
}


void ModuleInfo::encode(string& buf) const
{
	ostringstream oss;
	oss<<"---------------  Module Info ----------------"<<endl;
	oss<<"Type    :"<<mType<<endl;
	oss<<"Name    :"<<mName<<endl;
	oss<<"Version :"<<mVersion<<endl;
	oss<<"Author  :"<<mAuthor<<endl;
	oss<<"Date    :"<<mDate<<endl;
	oss<<"---------------  Module AVPs ----------------"<<endl;
	TiXmlHandle h(mDoc);
	TiXmlElement *item= h.FirstChild("Module").FirstChild("Configuration").FirstChild("Item").ToElement();
	while(item)
	{
		oss<<item->Attribute("name")<<" = "<<item->GetText()<<endl;
		item = item->NextSiblingElement();
	}
	oss<<endl;
	oss.flush();
	buf = oss.str();
}

std::ostream& operator<<(std::ostream& os, const ModuleInfo& mi)
{
	string buf;
	mi.encode(buf);
	os<<buf;
	return os;
}
