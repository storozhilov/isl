#ifndef ISL__WAIT_CONDITION__HXX
#define ISL__WAIT_CONDITION__HXX

#include "Mutex.hxx"

namespace isl
{

/*------------------------------------------------------------------------------
 * WaitCondition
------------------------------------------------------------------------------*/

class WaitCondition
{
public:
	WaitCondition();
	~WaitCondition();

	Mutex& mutex();
	void wait();
	bool wait(const Timeout& timeout);
	void wakeOne();
	void wakeAll();
private:
	WaitCondition(const WaitCondition&);					// No copy

	WaitCondition& operator=(const WaitCondition&);				// No copy

	pthread_cond_t _cond;
	Mutex _mutex;
};

} // namespace isl

#endif

