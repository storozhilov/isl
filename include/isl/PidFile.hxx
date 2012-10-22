#ifndef ISL__PID_FILE__HXX
#define ISL__PID_FILE__HXX

#include <string>

namespace isl
{

//! Class for savid PID file of the process
class PidFile
{
public:
	PidFile(const char * fileName);
	PidFile(const std::string& fileName);
	~PidFile();
private:
	PidFile();
	PidFile(const PidFile&);				// No copy

	PidFile& operator=(const PidFile&);			// No copy

	void init();

	const std::string _fileName;
};

} // namespace isl

#endif
