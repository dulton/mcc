#ifndef __MatrixFactory_h__
#define __MatrixFactory_h__
#include "matrix.h"
#include "moduleinfo.h"
#include "postable.h"
#define DECL_MATRIX_FACTORY(matrixname) \
	class matrixname##Factory : public MatrixFactory { \
		public: \
				virtual Matrix* create(Postable *receiver) { \
					if(mInfo) { \
						return new matrixname(receiver, mInfo); \
					}\
					return NULL; \
				}\
	} _factory;
class MatrixFactory {
	public:
		MatrixFactory() : mInfo(NULL) {}
		virtual ~MatrixFactory() { if(mInfo) delete mInfo;}
		virtual Matrix* create(Postable *receiver) = 0;
		void setInfo(ModuleInfo *info) { mInfo = info;}
	protected:
		ModuleInfo *mInfo;
};
#endif /* __MatrixFactory_h__ */
