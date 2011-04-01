#ifndef ISL__SYSTEM_CALL_ERROR__HXX
#define ISL__SYSTEM_CALL_ERROR__HXX

#include <isl/AbstractError.hxx>

namespace isl
{

class SystemCallError : public AbstractError
{
public:
	// TODO Make them Token's or Enum's children
	enum Function {
		// pthread functions
		PThreadCreate,
		PThreadJoin,
		PThreadMutexInit,
		PThreadMutexLock,
		PThreadMutexTryLock,
		PThreadMutexTimedLock,
		PThreadMutexUnlock,
		PThreadMutexDestroy,
		PThreadCondInit,
		PThreadCondSignal,
		PThreadCondBroadcast,
		PThreadCondWait,
		PThreadCondTimedWait,
		PThreadCondDestroy,
		PThreadRWLockInit,
		PThreadRWLockDestroy,
		PThreadRWLockRdLock,
		PThreadRWLockWrLock,
		PThreadRWLockTryRdLock,
		PThreadRWLockTryWrLock,
		PThreadRWLockTimedRdLock,
		PThreadRWLockTimedWrLock,
		PThreadRWLockUnlock,
		PThreadSelf,
		PThreadSigMask,
		PThreadAtFork,
		// Signal functions
		SigEmptySet,
		SigAddSet,
		SigDelSet,
		SigPending,
		SigWait,
		// I/O functions
		Socket,
		Fcntl,
		SetSockOpt,
		Bind,
		Listen,
		PSelect,
		Accept,
		InetNToP,
		RecvFrom,
		Recv,
		Send,
		Open,
		Close,
		Read,
		Write,
		Stat,
		FStat,
		GetSockName,
		GetPeerName,
		Connect,
		ScanDir,
		// Date & time functions
		Time,
		GMTimeR,
		LocalTimeR,
		// System calls
		Fork,
		GetPid,
		SetSid
	};

	SystemCallError(Function function, int errnum, SOURCE_LOCATION_ARGS_DECLARATION);
	
	virtual AbstractError * clone() const;

	static std::wstring getFunctionName(Function function);
	static std::wstring composeMessage(Function function, int errnum);
private:
	SystemCallError();

	Function _function;
	int _errnum;
};

} // namespace isl

#endif

