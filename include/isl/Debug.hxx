#ifndef ISL__DEBUG__HXX
#define ISL__DEBUG__HXX

// ISL debugging macros

#ifdef __GNUC__
#define SOURCE_LOCATION_ARGS __FILE__, __LINE__, __PRETTY_FUNCTION__
#else
#define SOURCE_LOCATION_ARGS __FILE__, __LINE__, __func__
#endif
#define SOURCE_LOCATION_ARGS_FILE __file
#define SOURCE_LOCATION_ARGS_LINE __line
#define SOURCE_LOCATION_ARGS_FUNCTION __function
#define SOURCE_LOCATION_ARGS_DECLARATION const char * SOURCE_LOCATION_ARGS_FILE, unsigned int SOURCE_LOCATION_ARGS_LINE, const char * SOURCE_LOCATION_ARGS_FUNCTION
#define SOURCE_LOCATION_ARGS_PASSTHRU SOURCE_LOCATION_ARGS_FILE, SOURCE_LOCATION_ARGS_LINE, SOURCE_LOCATION_ARGS_FUNCTION

#include <string>

namespace isl
{

std::string composeSourceLocation(SOURCE_LOCATION_ARGS_DECLARATION);

} // namespace isl

#endif
