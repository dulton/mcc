#include "serialmatrix.h"
#include "keyboardswitchmessage.h"
#include "keyboardptzmessage.h"
#include "log.h"


static int toPacket(char *packet, KeyboardSwitchMessage &msg)
{
	packet[0] = 0xff;
	packet[1] = 0x00;
	packet[2] = 0x11;
	packet[3] = 0x00;
	packet[4] = (char)msg.camera();
	packet[5] = (char)msg.monitor();
	packet[6] = 0x55;
	packet[7] = packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6];
	return 8;
}

static int toPacket(char *packet, KeyboardPtzMessage &msg)
{
	packet[0] = 0xff;
	packet[1] = 0x00;
	packet[2] = 0x21;
	packet[3] = 0x00;
	packet[5] = 0x00;
	packet[6] = 0x00;
	// zoom param
	if(msg.zoomInSpeed())
		packet[3] = 0x40;
	else if(msg.zoomOutSpeed())
		packet[3] = 0x80;
	// camera id
	packet[4] = (char)msg.camera();
		// v operation
	if(msg.hSpeed() > 0)
		packet[5] = msg.hSpeed();
	else if(msg.hSpeed() < 0)
		packet[5] = 0x80 - msg.hSpeed();
	// h operation
	if(msg.vSpeed() > 0)
		packet[6] = msg.vSpeed();
	else if(msg.vSpeed() < 0)
		packet[6] = 0x80 - msg.vSpeed();
	packet[7] = packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6];
	return 8;
}

class MatrixJXJ : public SerialMatrix {
	public:
		MatrixJXJ(Postable *receiver, ModuleInfo *info) : SerialMatrix(receiver, info) {}
		virtual int onReadable(SerialPort& sp)
		{
			char queries = 0;
			static char packet[8] = {0xff, 0x00, 0x11, 0x00, 0x01, 0x01, 0x55, 0x68};
			char reg[] = {0xff, 0x00, 0x41, 0x11, 0x55, 0x00, 0x00, 0xa7};
			static int rcv_cnt = 0;
			Message *msg = NULL;
			int len = sp.read(&queries, 1);
			assert(len == 1);
			if(rcv_cnt == 0)
			{
				if((queries & 0xff) != 0x50)
					return 0;
				assert(sp.write(reg, 8) == 8);

				rcv_cnt++;
			}
			else if((0xff & queries) == 0x50)
			{
				mFifo.get(msg);
				//if(mFifo.get(msg) == true)
				if(msg)
				{
					KeyboardSwitchMessage *smsg = dynamic_cast<KeyboardSwitchMessage*>(msg);
					if(smsg)
					{
						InfoLog("matrix jxj receive a switch message");
						toPacket(mPacket, *smsg);
					}
					KeyboardPtzMessage *pmsg = dynamic_cast<KeyboardPtzMessage*>(msg);
					if(pmsg)
					{
						InfoLog("matrix jxj receive a ptz control message");
						toPacket(mPacket, *pmsg);
					}
					delete msg;
				}
				if(validatePacket(mPacket))
					sp.write(mPacket, 8);
			}
			return 0;
		}
		virtual int onExcept(SerialPort& sp)
		{
			ErrLog("serial port has encounter a problem!");
		}
		bool validatePacket(const char *packet)
		{
			return true;
		}
	protected:
		char mPacket[8];
};

DECL_MATRIX_FACTORY(MatrixJXJ);
