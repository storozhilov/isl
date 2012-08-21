#ifndef ISL__ABSTRACT_LOG_TARGET__HXX
#define ISL__ABSTRACT_LOG_TARGET__HXX

namespace isl
{

class AbstractLogDevice;

//! Logging target abstract base class
/*!
  One loging device could serve multiple logging targets, e.g. one file and a symlink to this file are serving by
  the same logging device which represents it's file.

  \sa AbstractLogDevice
*/
class AbstractLogTarget
{
public:
	AbstractLogTarget();
	virtual ~AbstractLogTarget();
protected:
	//! Loging device creation abstract virtual factory method
	/*!
	  \return Pointer to the new logging device, which is serving the target
	*/
	virtual AbstractLogDevice * createDevice() const = 0;

	friend class LogDispatcher;
};

} // namespace isl

#endif

