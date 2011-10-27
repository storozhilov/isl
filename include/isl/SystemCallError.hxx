#ifndef ISL__SYSTEM_CALL_ERROR__HXX
#define ISL__SYSTEM_CALL_ERROR__HXX

#include <isl/AbstractError.hxx>
#include <sstream>
#include <string.h>

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
		StrFTime,
		StrPTime,
		GetTimeOfDay,
		// System calls
		Fork,
		GetPid,
		SetSid
	};

	SystemCallError(SOURCE_LOCATION_ARGS_DECLARATION, Function function, int errnum, const std::wstring& info = std::wstring()) :
		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
		_function(function),
		_errnum(errnum)
	{}
	
	virtual AbstractError * clone() const
	{
		return new SystemCallError(*this);
	}
	virtual std::wstring composeMessage() const
	{
		std::wostringstream msg;
		msg << functionName(_function) << L" system call error: (" << _errnum << L") " << strerror(_errnum);
		std::wstring result(msg.str());
		appendInfo(result);
		return result;
	}

	static std::wstring functionName(Function function)
	{
		switch (function) {
			// pthread functions
			case PThreadCreate:
				return L"pthread_create(3)";
			case PThreadJoin:
				return L"pthread_join(3)";
			case PThreadMutexInit:
				return L"pthread_mutex_init(3)";
			case PThreadMutexLock:
				return L"pthread_mutex_lock(3)";
			case PThreadMutexTryLock:
				return L"pthread_mutex_trylock(3)";
			case PThreadMutexTimedLock:
				return L"pthread_mutex_timedlock(3)";
			case PThreadMutexUnlock:
				return L"pthread_mutex_unlock(3)";
			case PThreadMutexDestroy:
				return L"pthread_mutex_destroy(3)";
			case PThreadCondInit:
				return L"pthread_cond_init(3)";
			case PThreadCondSignal:
				return L"pthread_cond_signal(3)";
			case PThreadCondBroadcast:
				return L"pthread_cond_broadcast(3)";
			case PThreadCondWait:
				return L"pthread_cond_wait(3)";
			case PThreadCondTimedWait:
				return L"pthread_cond_timedwait(3)";
			case PThreadCondDestroy:
				return L"pthread_cond_destroy(3)";
			case PThreadRWLockInit:
				return L"pthread_rwlock_init(3)";
			case PThreadRWLockDestroy:
				return L"pthread_rwlock_destroy(3)";
			case PThreadRWLockRdLock:
				return L"pthread_rwlock_rdlock(3)";
			case PThreadRWLockWrLock:
				return L"pthread_rwlock_wrlock(3)";
			case PThreadRWLockTryRdLock:
				return L"pthread_rwlock_tryrdlock(3)";
			case PThreadRWLockTryWrLock:
				return L"pthread_rwlock_trywrlock(3)";
			case PThreadRWLockUnlock:
				return L"pthread_rwlock_unlock(3)";
			case PThreadRWLockTimedRdLock:
				return L"pthread_rwlock_timedrdlock(3)";
			case PThreadRWLockTimedWrLock:
				return L"pthread_rwlock_timedwrlock(3)";
			case PThreadSelf:
				return L"pthread_rwlock_unlock(3)";
			case PThreadSigMask:
				return L"pthread_sigmask(3)";
			case PThreadAtFork:
				return L"pthread_atfork(3)";
			// Signal functions
			case SigEmptySet:
				return L"sigemptyset(3)";
			case SigAddSet:
				return L"sigaddset(3)";
			case SigDelSet:
				return L"sigdelset(3)";
			case SigPending:
				return L"sigpending(2)";
			case SigWait:
				return L"sigwait(2)";
			// I/O functions
			case Socket:
				return L"socket(2)";
			case Fcntl:
				return L"fcntl(2)";
			case SetSockOpt:
				return L"setsockopt(2)";
			case Bind:
				return L"bind(2)";
			case Listen:
				return L"listen(2)";
			case PSelect:
				return L"pselect(2)";
			case Accept:
				return L"accept(2)";
			case InetNToP:
				return L"inet_ntop(3)";
			case RecvFrom:
				return L"recvfrom(2)";
			case Recv:
				return L"recv(2)";
			case Send:
				return L"send(2)";
			case Open:
				return L"open(2)";
			case Close:
				return L"close(2)";
			case Read:
				return L"read(2)";
			case Write:
				return L"write(2)";
			case Stat:
				return L"stat(2)";
			case FStat:
				return L"stat(2)";
			case GetSockName:
				return L"getsockname(2)";
			case GetPeerName:
				return L"getpeername(2)";
			case Connect:
				return L"connect(2)";
			case ScanDir:
				return L"scandir(3)";
			// Date & time functions
			case Time:
				return L"time(3)";
			case GMTimeR:
				return L"gmtime(3)";
			case LocalTimeR:
				return L"localtime_r(3)";
			case StrFTime:
				return L"strftime(3)";
			case StrPTime:
				return L"strptime(3)";
			case GetTimeOfDay:
				return L"gettimeofday(2)";
			// System calls
			case Fork:
				return L"fork(2)";
			case GetPid:
				return L"getpid(2)";
			case SetSid:
				return L"setsid(2)";
			default:
				return L"[UNKNOWN FUNCTION]";
		}
	}
private:
	SystemCallError();

	Function _function;
	int _errnum;
};

} // namespace isl

#endif

