#ifndef ISL__SIGNAL_SET__HXX
#define ISL__SIGNAL_SET__HXX

#include <isl/Core.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <set>

namespace isl
{

class SignalSet
{
public:
	SignalSet() :
		_set(),
		_signals()
	{
		clear();
	}
	SignalSet(sigset_t set) :
		_set(set),
		_signals()
	{
		for (int signo = 1; signo <= SIGRTMAX; ++signo) {
			if (sigismember(&set, signo)) {
				_signals.insert(signo);
			}
		}
	}
	SignalSet(unsigned int signalCount, int signo, ...) :
		_set(),
		_signals()
	{
		clear();
		add(signo);
		va_list args;
		va_start(args, signo);
		for (unsigned int i = 1; i < signalCount; ++i) {
			add(va_arg(args, int));
		}
		va_end(args);
	}

	inline void clear()
	{
		if (sigemptyset(&_set)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigEmptySet, errno));
		}
		_signals.clear();
	}
	inline void add(int signo)
	{
		if (sigaddset(&_set, signo)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigAddSet, errno));
		}
		_signals.insert(signo);
	}
	inline void remove(int signo)
	{
		if (sigdelset(&_set, signo)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigDelSet, errno));
		}
	}
	inline bool contains(int signo) const
	{
		return sigismember(&_set, signo);
	}
	inline sigset_t sigset() const
	{
		return _set;
	}
	inline const std::set<int>& signals() const
	{
		return _signals;
	}
private:
	sigset_t _set;
	std::set<int> _signals;
};



} // namespace isl

#endif
