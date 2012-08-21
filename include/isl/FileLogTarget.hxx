#ifndef ISL__FILE_LOG_TARGET__HXX
#define ISL__FILE_LOG_TARGET__HXX

#include <isl/AbstractLogTarget.hxx>
#include <string>

namespace isl
{

class AbstractLogDevice;

//! File logging target
class FileLogTarget : public AbstractLogTarget
{
public:
	//! Constructor
	/*!
	  \param fileName Log file name
	*/
	FileLogTarget(const std::string& fileName);
	//! Returns file logging target filename
	std::string fileName() const;
private:
	FileLogTarget();

	virtual AbstractLogDevice * createDevice() const;

	std::string _fileName;

	friend class LogDispatcher;
};

}

#endif

