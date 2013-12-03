#ifndef __MatrixManager_h__
#define __MatrixManager_h__
#include <map>
#include "matrix.h"
#include "matrixfactory.h"
class MatrixManager {
	public:
		MatrixManager(const char *module_path) : mModulePath(module_path), mMatrix(NULL) {}
		virtual int loadMatrix(const char *name, Postable* receiver, void *param);
		virtual void unloadMatrix();
		virtual Matrix* getMatrix();
	private:
		std::string mModulePath;
		Matrix *mMatrix;
		std::map<std::string, MatrixFactory *> mFactories;
};
#endif /* __MatrixManager_h__ */
