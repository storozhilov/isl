#ifndef ISL__SIGNAL_SET__HXX
#define ISL__SIGNAL_SET__HXX

#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <set>

namespace isl
{

//! UNIX signal set helper class
class SignalSet
{
public:
	//! Creates an empty UNIX signal set
	SignalSet() :
		_set(),
		_signals()
	{
		clear();
	}
	//! Creates a UNIX signal set which is initialized from libc UNIX signal set structure
	/*!
	  \param set libc UNIX signal set
	*/
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
	//! Creates a UNIX signal set which is initialized from variable arguments sequence. Use with care!
	/*!
	  \param signalCount Signal count in set (must be > 0)
	  \param signo First UNIX signal in set
	*/
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
	//! Clears UNIX signal set
	inline void clear()
	{
		if (sigemptyset(&_set)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigEmptySet, errno));
		}
		_signals.clear();
	}
	//! Adds UNIX signal to the set
	/*!
	  \param signo UNIX signal to add
	*/
	inline void add(int signo)
	{
		if (sigaddset(&_set, signo)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigAddSet, errno));
		}
		_signals.insert(signo);
	}
	//! Removes UNIX signal from the set
	/*!
	  \param signo UNIX signal to remove
	*/
	inline void remove(int signo)
	{
		if (sigdelset(&_set, signo)) {
			throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::SigDelSet, errno));
		}
	}
	//! Inspects set for UNIX signal
	/*!
	  \param signo UNIX signal to inspect for
	  \return TRUE if the set contains UNIX signal or FALSE otherwise
	*/
	inline bool contains(int signo) const
	{
		return sigismember(&_set, signo);
	}
	//! Returns libc UNIX signal set
	inline sigset_t sigset() const
	{
		return _set;
	}
	//! Returns constant reference to the STL set container with UNIX signals
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
