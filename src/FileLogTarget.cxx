#include <isl/FileLogTarget.hxx>
#include <isl/FileLogDevice.hxx>

namespace isl
{

/*------------------------------------------------------------------------------
 * FileLogTarget
 *------------------------------------------------------------------------------*/

FileLogTarget::FileLogTarget(const std::string& fileName) :
	AbstractLogTarget(),
	_fileName(fileName)
{}

std::string FileLogTarget::fileName() const
{
	return _fileName;
}

AbstractLogDevice *FileLogTarget::createDevice() const
{
	return new FileLogDevice(_fileName);
}

} // namespace isl

