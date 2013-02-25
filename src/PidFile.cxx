#include <isl/PidFile.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <isl/Log.hxx>
#include <isl/ErrorLogMessage.hxx>
#include <fstream>
#include <cstdio>
#include <cerrno>

namespace isl
{

PidFile::PidFile(const char * fileName) :
	_fileName(fileName)
{
	init();
}

PidFile::PidFile(const std::string& fileName) :
	_fileName(fileName)
{
	init();
}

PidFile::~PidFile()
{
	if (::remove(_fileName.c_str())) {
		Log::error().log(ErrorLogMessage(SOURCE_LOCATION_ARGS, SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Remove, errno)));
	}
}

void PidFile::init()
{
	std::ofstream ofs(_fileName.c_str(), std::ios_base::out | std::ios_base::trunc);
	ofs << ::getpid() << std::flush;
}

} // namespace isl
