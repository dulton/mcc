#include "serialmatrix.h"
#include "keyboardmessagehelper.h"
#include "matrixfactory.h"
#include "log.h"
#include "string.h"
class MatrixCZ08 : public SerialMatrix {
	public:
		MatrixCZ08(Postable *receiver, ModuleInfo *info) : SerialMatrix(receiver, info) {}
		virtual int onReadable(SerialPort& sp)
		{
			return 0;
		}
		virtual int onExcept(SerialPort& sp) {
			return 0;
		}	
		virtual void post(Message* msg)
		{
			char buf[50];
			int len = toPacket(buf, msg);
			if(len)
			{
				mSerialPort.write(buf, len);
			}
		}
	protected:
		virtual int toPacket(char *packet, Message* msg)
		{
			KeyboardSwitchMessage *smsg = dynamic_cast<KeyboardSwitchMessage*>(msg);
			if(smsg)
			{
				sprintf(packet, "<swit,%d,0,%d>", smsg->monitor(), smsg->camera());
				char *p = strstr(packet, ">");
				return p - packet + 1;
			}
			return 0;
		}
};

DECL_MATRIX_FACTORY(MatrixCZ08);
