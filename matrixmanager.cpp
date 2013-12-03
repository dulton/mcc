#include "matrixmanager.h"
#include "log.h"
#include <dlfcn.h>
#include <cassert>
using namespace std;
int MatrixManager::loadMatrix(const char *name, Postable* receiver, void *param)
{
	Matrix *matrix;
	MatrixFactory *factory;
	if(mMatrix)
		return -1;
	string path = name;
	map<string, MatrixFactory*>::iterator i = mFactories.find(path);
	if(i!=mFactories.end())
	{
		factory = i->second;
		assert(factory);
	}
	else
	{
		ModuleInfo *info = new ModuleInfo;
		path = mModulePath + name;
		path += ".xml";
		try {
			info->open(path.c_str());
		}
		catch(runtime_error e)
		{
			ErrLog("Failed to parse matrix module infomation file "<<path<<" error message:"<<e.what());
			delete info;
			return -1;
		}
		if(info->type() != ModuleInfo::MatrixModule)
		{
			ErrLog("Module "<<path<<" is not a matrix");
			delete info;
			return -1;
		}
		InfoLog("Matrix Module Infomation:"<<endl<<*info);
		path = mModulePath + name;
		path += ".so";
		void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_NODELETE);
		if(!handle)
		{
			delete info;
			ErrLog("Failed to open matrix module "<<path<<" error message:"<<dlerror());
			return -1;
		}
		factory = (MatrixFactory *)dlsym(handle, "_factory");
		dlclose(handle);
		if(!factory)
		{
			delete info;
			ErrLog("Matrix ["<<path<<"] factory not found");
			return -1;
		}
		factory->setInfo(info);
	}
	matrix = factory->create(receiver);
	if(!matrix)
	{
		return -1;
	}
	mMatrix = matrix;
	return matrix->start(param);
}
void MatrixManager::unloadMatrix()
{
	if(mMatrix)
	{
		mMatrix->stop();
		mMatrix = NULL;
	}
}
Matrix* MatrixManager::getMatrix()
{
	return mMatrix;
}
