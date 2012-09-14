#ifndef ISL__SYSTEM_CALL_ERROR__HXX
#define ISL__SYSTEM_CALL_ERROR__HXX

#include <isl/AbstractError.hxx>
#include <sstream>
#include <string.h>

namespace isl
{

//! System call error
class SystemCallError : public AbstractError
{
public:
	//! Recognized function ids
	enum Function {
		Undefined,					// Undefined function
		// pthread functions
		PThreadCreate,
		PThreadJoin,
		PThreadTimedJoinNp,
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
		GetSockOpt,
		SetSockOpt,
		Connect,
		ScanDir,
		// Date & time functions
		Time,
		GMTimeR,
		LocalTimeR,
		StrFTime,
		StrPTime,
		GetTimeOfDay,
		ClockGetTime,
		MkTime,
		// System calls
		Fork,
		GetPid,
		SetSid
	};
	//! Constructs an object from recognized function id
	/*
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter

	   \param func Function id
	   \param errnum Errno value
	   \param info User info
	*/
	SystemCallError(SOURCE_LOCATION_ARGS_DECLARATION, Function func, int errnum, const std::string& info = std::string()) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_function(func),
		_functionName(functionName(func)),
		_errnum(errnum)
	{}
	//! Constructs an object from arbitrary function name
	/*
	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter

	   \param func Function name
	   \param errnum Errno value
	   \param info User info
	*/
	SystemCallError(SOURCE_LOCATION_ARGS_DECLARATION, const  std::string& func, int errnum, const std::string& info = std::string()) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_function(Undefined),
		_functionName(func),
		_errnum(errnum)
	{}
	//! Returns function id
	inline Function function() const
	{
		return _function;
	}
	//! Returns function name
	inline const std::string& functionName() const
	{
		return _functionName;
	}
	//! Clones error
	virtual AbstractError * clone() const
	{
		return new SystemCallError(*this);
	}
protected:
	static std::string functionName(Function func)
	{
		switch (func) {
			// pthread functions
			case PThreadCreate:
				return "pthread_create(3)";
			case PThreadJoin:
				return "pthread_join(3)";
			case PThreadTimedJoinNp:
				return "pthread_timedjoin_np(3)";
			case PThreadMutexInit:
				return "pthread_mutex_init(3)";
			case PThreadMutexLock:
				return "pthread_mutex_lock(3)";
			case PThreadMutexTryLock:
				return "pthread_mutex_trylock(3)";
			case PThreadMutexTimedLock:
				return "pthread_mutex_timedlock(3)";
			case PThreadMutexUnlock:
				return "pthread_mutex_unlock(3)";
			case PThreadMutexDestroy:
				return "pthread_mutex_destroy(3)";
			case PThreadCondInit:
				return "pthread_cond_init(3)";
			case PThreadCondSignal:
				return "pthread_cond_signal(3)";
			case PThreadCondBroadcast:
				return "pthread_cond_broadcast(3)";
			case PThreadCondWait:
				return "pthread_cond_wait(3)";
			case PThreadCondTimedWait:
				return "pthread_cond_timedwait(3)";
			case PThreadCondDestroy:
				return "pthread_cond_destroy(3)";
			case PThreadRWLockInit:
				return "pthread_rwlock_init(3)";
			case PThreadRWLockDestroy:
				return "pthread_rwlock_destroy(3)";
			case PThreadRWLockRdLock:
				return "pthread_rwlock_rdlock(3)";
			case PThreadRWLockWrLock:
				return "pthread_rwlock_wrlock(3)";
			case PThreadRWLockTryRdLock:
				return "pthread_rwlock_tryrdlock(3)";
			case PThreadRWLockTryWrLock:
				return "pthread_rwlock_trywrlock(3)";
			case PThreadRWLockUnlock:
				return "pthread_rwlock_unlock(3)";
			case PThreadRWLockTimedRdLock:
				return "pthread_rwlock_timedrdlock(3)";
			case PThreadRWLockTimedWrLock:
				return "pthread_rwlock_timedwrlock(3)";
			case PThreadSelf:
				return "pthread_rwlock_unlock(3)";
			case PThreadSigMask:
				return "pthread_sigmask(3)";
			case PThreadAtFork:
				return "pthread_atfork(3)";
			// Signal functions
			case SigEmptySet:
				return "sigemptyset(3)";
			case SigAddSet:
				return "sigaddset(3)";
			case SigDelSet:
				return "sigdelset(3)";
			case SigPending:
				return "sigpending(2)";
			case SigWait:
				return "sigwait(2)";
			// I/O functions
			case Socket:
				return "socket(2)";
			case Fcntl:
				return "fcntl(2)";
			case Bind:
				return "bind(2)";
			case Listen:
				return "listen(2)";
			case PSelect:
				return "pselect(2)";
			case Accept:
				return "accept(2)";
			case InetNToP:
				return "inet_ntop(3)";
			case RecvFrom:
				return "recvfrom(2)";
			case Recv:
				return "recv(2)";
			case Send:
				return "send(2)";
			case Open:
				return "open(2)";
			case Close:
				return "close(2)";
			case Read:
				return "read(2)";
			case Write:
				return "write(2)";
			case Stat:
				return "stat(2)";
			case FStat:
				return "stat(2)";
			case GetSockName:
				return "getsockname(2)";
			case GetPeerName:
				return "getpeername(2)";
			case GetSockOpt:
				return "getsockopt(2)";
			case SetSockOpt:
				return "setsockopt(2)";
			case Connect:
				return "connect(2)";
			case ScanDir:
				return "scandir(3)";
			// Date & time functions
			case Time:
				return "time(3)";
			case GMTimeR:
				return "gmtime(3)";
			case LocalTimeR:
				return "localtime_r(3)";
			case StrFTime:
				return "strftime(3)";
			case StrPTime:
				return "strptime(3)";
			case GetTimeOfDay:
				return "gettimeofday(2)";
			case ClockGetTime:
				return "clock_gettime(2)";
			case MkTime:
				return "mktime(3)";
			// System calls
			case Fork:
				return "fork(2)";
			case GetPid:
				return "getpid(2)";
			case SetSid:
				return "setsid(2)";
			default:
				return "[UNKNOWN FUNCTION]";
		}
	}
private:
	SystemCallError();

	virtual std::string composeMessage() const
	{
		std::ostringstream oss;
		oss << _functionName << " system call error: (" << _errnum << ") " << strerror(_errnum);
		return oss.str();
	}

	Function _function;
	std::string _functionName;
	int _errnum;
};

} // namespace isl

#endif

