#include "postable.h"
#include "serialmatrix.h"
#include "matrixfactory.h"
#include "moduleinfo.h"
#include "log.h"
#include "keyboardmessagehelper.h"
class MatrixFLMix : public SerialMatrix {
	public:
		MatrixFLMix(Postable *receiver, ModuleInfo *info) : SerialMatrix(receiver, info) {
			mAddr = info->attribute("Address", 1l);
		}
		virtual int onReadable(SerialPort& sp)
		{
			char buf[10];
			char c;
			int len = sp.read(&c, 1);
			if(c != '#')
				return 0;
			len = sp.nRead(buf, 3);
			int addr = buf[0] - '0';
			int cmdlen = (buf[1] - '0')*10 + (buf[2] - '0');
			char *cmd = new char[cmdlen+1];
			sp.nRead(cmd, cmdlen);
			cmd[cmdlen] = 0;
			if(cmd[cmdlen - 1] != 'K')
				ErrLog("MatrixFLMix: Command failed "<<cmd);
			delete cmd;
			/*
			KeyboardMessage *msg;
			msg = parse(cmd, cmdlen);
			if(!msg){
				ErrLog("Failed to parse command");
			}
			delete cmd;
			mReceiver->post(msg);
			*/
		}
		virtual int onExcept(SerialPort& sp)
		{
			ErrLog("Serialport of MatrixFLMix has encounter a problem, will be shutdown");
		}
		virtual void post(Message* msg)
		{
			char packet[105];
			KeyboardSwitchMessage* smsg = dynamic_cast<KeyboardSwitchMessage*>(msg);
			if(smsg)
			{
				int packet_len = toPacket(packet, *smsg);
				if(mSerialPort.write(packet, packet_len) != packet_len)
					ErrLog("MatrixFLMix failed to send "<<packet_len<<" bytes!");
			}
		}
	protected:
		/*
		KeyboardMessage *parse(const char *buf, int buflen) const {
			return NULL;
		}
		*/
		virtual int toPacket(char *packet, KeyboardSwitchMessage& msg)
		{
			sprintf(packet, "#%1d05V%02d%02d", mAddr, msg.camera(), msg.monitor());
			return 9;
		}
		int mAddr;
};

DECL_MATRIX_FACTORY(MatrixFLMix);
