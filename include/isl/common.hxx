#ifndef ISL__COMMON__HXX
#define ISL__COMMON__HXX

#include <isl/Log.hxx>
#include <isl/DebugLog.hxx>

namespace isl
{

//! Daemonizes current process
void daemonize();
//! Writes current process's pid to file
/*!
  \param fileName Name of the file
*/
void writePid(const char * pidFileName);
//! Returns a reference to ISL error log
Log& errorLog();
//! Returns a reference to ISL warning log
Log& warningLog();
//! Returns a reference to ISL debug log
DebugLog& debugLog();

} // namespace isl

#endif
