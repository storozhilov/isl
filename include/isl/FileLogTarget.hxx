#ifndef ISL__FILE_LOG_TARGET__HXX
#define ISL__FILE_LOG_TARGET__HXX

#include <isl/AbstractLogTarget.hxx>
#include <string>

namespace isl
{

class AbstractLogDevice;

class FileLogTarget : public AbstractLogTarget
{
public:
	FileLogTarget(const std::string& fileName);

	std::string fileName() const;
private:
	FileLogTarget();

	AbstractLogDevice *createDevice() const;

	std::string _fileName;

	friend class LogDispatcher;
};

}

#endif

