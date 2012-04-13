# ISL scons script
#
# To build & install type 'scons & sudo scons install' in command line.
# To uninstall type 'sudo scons uninstall' in command line.
# Run 'scons -c' to clear all the staff.
# 
# Supported environment variables (yes/YES/on/ON values are recognized):
# * ISL_CORE_DEBUGGING - turn on core debugging;
# * ISL_LOG_DEBUGGING - turn on log subsystem debugging to std::wclog;
# * ISL_BUILD_EXAPMLES - build examples.
#
# Suppose following scenario for the developer:
#
# $ ISL_LOG_DEBUGGING=yes ISL_BUILD_EXAPMLES=yes ISL_CORE_DEBUGGING=yes scons
# $ sudo ISL_LOG_DEBUGGING=yes ISL_BUILD_EXAPMLES=yes ISL_CORE_DEBUGGING=yes scons install
# $ ISL_LOG_DEBUGGING=yes ISL_BUILD_EXAPMLES=yes ISL_CORE_DEBUGGING=yes scons -c

import os

if os.environ.get('ISL_BUILD_EXAPMLES', '').upper() == 'YES' or os.environ.get('ISL_BUILD_EXAPMLES', '').upper() == 'ON':
	SConscript(['src/SConscript', 'examples/Test/SConscript', 'examples/HttpServer/SConscript', 'examples/HttpCopy/SConscript'])
else:
	SConscript(['src/SConscript'])
