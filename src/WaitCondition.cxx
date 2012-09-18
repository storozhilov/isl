#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <iostream>
#include <errno.h>

namespace isl
{

WaitCondition::WaitCondition() :
	_cond(),
	_providedMutexPtr(),
	_internalMutexAutoPtr(new Mutex())
{
	if (int errorCode = pthread_cond_init(&_cond, NULL)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondInit, errorCode));
	}
}

WaitCondition::WaitCondition(Mutex& mutex) :
	_cond(),
	_providedMutexPtr(&mutex),
	_internalMutexAutoPtr()
{
	if (int errorCode = pthread_cond_init(&_cond, NULL)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondInit, errorCode));
	}
}

WaitCondition::~WaitCondition()
{
	if (int errorCode = pthread_cond_destroy(&_cond)) {
		std::cerr << SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondDestroy, errorCode).message() << std::endl;
	}
}

void WaitCondition::wait()
{
	if (int errorCode = pthread_cond_wait(&_cond, &(mutex()._mutex))) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondWait, errorCode));
	}
}

bool WaitCondition::wait(const Timeout& timeout, Timeout * timeoutLeft)
{
	if (timeout.isZero()) {
		return false;
	}
	timespec timeoutLimit = timeout.limit();
	int errorCode = pthread_cond_timedwait(&_cond, &(mutex()._mutex), &timeoutLimit);
	switch (errorCode) {
		case 0:
			if (timeoutLeft) {
				*timeoutLeft = Timeout::leftToLimit(timeoutLimit);
			}
			return true;
		case ETIMEDOUT:
			if (timeoutLeft) {
				*timeoutLeft = Timeout();
			}
			return false;
		default:
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondTimedWait, errorCode));
	}
}

void WaitCondition::wakeOne()
{
	if (int errorCode = pthread_cond_signal(&_cond)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondSignal, errorCode));
	}
}

void WaitCondition::wakeAll()
{
	if (int errorCode = pthread_cond_broadcast(&_cond)) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::PThreadCondBroadcast, errorCode));
	}
}

} // namespace isl
