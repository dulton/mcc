#include <vector>
class TrunkInfo {
	public:
		inline int localPort() const { mLocalPort;}
		inline int remotePort() const { mRemotePort;}
		inline int remoteMatrix() const { return mRemoteMatrix;}
	protected:
		int mLocalPort;
		int mRemotePort;
		int mRemoteMatrix;
};
class CameraInfo {
	public:
		inline int port() const { mPort;}
	protected:
		int mPort;
};
class MonitorInfo {
	public:
		inline int port() const { mPort;}
	protected:
		int mPort;
};
class AbstractDB {
	public:
		virtual std::vector<TrunkInfo>& getTrunks() = 0;
		virtual std::vector<CameraInfo>& getCameras() = 0;
		virtual std::vector<MonitorInfo>& getMonitors() = 0;
};
