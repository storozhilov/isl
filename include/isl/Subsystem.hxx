#ifndef ISL__SUBSYSTEM__HXX
#define ISL__SUBSYSTEM__HXX

#include <isl/Timeout.hxx>
#include <list>

namespace isl
{

//! Server subsystem base class
/*!
  Basic component of any server according to <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite Design Pattern</a>.
*/
class Subsystem
{
public:
	//! Constructs a new subsystem
	/*!
	  \param owner The owner subsystem of the new subsystem
	  \param clockTimeout Subsystem's clock timeout
	 */
	Subsystem(Subsystem * owner, const Timeout& clockTimeout = Timeout::defaultTimeout());
	//! Destructor
	virtual ~Subsystem();
	//! Returns an owner of the subsystem
	/*!
	  \note Thread-safe
	*/
	inline Subsystem * owner() const
	{
		return _owner;
	}
	//! Returns clock timeout
	inline const Timeout& clockTimeout() const
	{
		return _clockTimeout;
	}
	//! Sets new clock timeout
	/*!
	  \param newValue New clock timeout value

	  \note Thread-unsafe: call it when subsystem is idling only
	*/
	inline void setClockTimeout(const Timeout& newValue)
	{
		_clockTimeout = newValue;
	}
	//! Starting subsystem virtual method
	/*!
	  Default implementation starts all children subsystems and subsystem's threads
	  \note Thread-unsafe
	*/
	virtual void start();
	//! Stopping subsystem and awaiting for it's termination virtual method
	/*!
	  Default implementation stops all subsystem's threads and children subsystems
	  \note Thread-unsafe
	*/
	virtual void stop();
protected:
	//! Starts children subsystems
	void startChildren();
	//! Stops children subsystems
	void stopChildren();
private:
	Subsystem();
	Subsystem(const Subsystem&);							// No copy

	Subsystem& operator=(const Subsystem&);						// No copy

	typedef std::list<Subsystem *> Children;

	void registerChild(Subsystem * child);
	void unregisterChild(Subsystem * child);

	Subsystem * _owner;
	Timeout _clockTimeout;
	Children _children;
};

} // namespace isl

#endif
