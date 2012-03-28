#include <isl/Thread.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Core.hxx>
#include <errno.h>
#include <stdexcept>

namespace isl
{

Thread::Thread() :
	_thread(),
	_isRunning(false),
	_isRunningRWLock(),
	_awaitStartup(false),
	_awaitStartupCond(0)
{}

Thread::Thread(bool awaitStartup) :
	_thread(),
	_isRunning(false),
	_isRunningRWLock(),
	_awaitStartup(awaitStartup),
	_awaitStartupCond(0)
{}

Thread::~Thread()
{}

void Thread::start()
{
	if (_awaitStartup) {
		WaitCondition awaitStartupCond;
		_awaitStartupCond = &awaitStartupCond;
		{
			MutexLocker locker(awaitStartupCond.mutex());
			if (int errorCode = pthread_create(&_thread, NULL, Thread_execute, this)) {
				throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
			}
			awaitStartupCond.wait();
		}
		_awaitStartupCond = 0;
	} else {
		if (int errorCode = pthread_create(&_thread, NULL, Thread_execute, this)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCreate, errorCode));
		}
	}
}

void Thread::join()
{
	if (pthread_equal(_thread, pthread_self())) {
		return;
	}
	if (int errorCode = pthread_join(_thread, NULL)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadJoin, errorCode));
	}
}

bool Thread::join(const Timeout& timeout)
{
	if (pthread_equal(_thread, pthread_self())) {
		return true;
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_timedjoin_np(_thread, NULL, &timeoutLimit);
	switch (errorCode) {
		case 0:
			return true;
		case ETIMEDOUT:
			return false;
		default:
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadTimedJoinNp, errorCode));
	}
}

bool Thread::isRunning() const
{
	ReadLocker locker(_isRunningRWLock);
	return _isRunning;
}

void Thread::execute()
{
	if (_awaitStartup) {
		MutexLocker locker(_awaitStartupCond->mutex());
		_awaitStartupCond->wakeAll();
	}
	{
		WriteLocker locker(_isRunningRWLock);
		if (_isRunning) {
			//throw Exception(new ThreadError(ThreadError::ThreadIsAlreadyRunning, this));
			throw std::runtime_error("Thread is already running");
		}
		_isRunning = true;
	}
	try {
		run();
	} catch (std::exception& e) {
		Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Thread execution error"));
	} catch (...) {
		Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Thread execution unknown error"));
	}
	WriteLocker locker(_isRunningRWLock);
	_isRunning = false;
}

extern "C" void * Thread_execute(void * arg)
{
	reinterpret_cast<Thread *>(arg)->execute();
	return 0;
}

} // namespace

