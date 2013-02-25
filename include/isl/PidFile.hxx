#ifndef ISL__PID_FILE__HXX
#define ISL__PID_FILE__HXX

#include <string>

namespace isl
{

//! Saves a process ID to file in constructor and deletes this file in destructor
class PidFile
{
public:
	//! Constructor
	/*!
	  \param fileName PID file name
	*/
	PidFile(const char * fileName);
	//! Constructor
	/*!
	  \param fileName PID file name
	*/
	PidFile(const std::string& fileName);
	//! Destructor
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
