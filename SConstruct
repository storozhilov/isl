# ISL scons script
#
# To build & install type 'scons & sudo scons install' in command line.
# To uninstall type 'sudo scons uninstall' in command line.
# Run 'scons -h' to list build options.
#
# TODO "prefix" as scons option support.

import os

AddOption('--build-scada',
		dest = 'build-scada',
		action = 'store_true',
		help = 'Build SCADA module (depends on libmodbus, http://libmodbus.org/) (the same as if \'ISL_BUILD_SCADA\' environment variable is set to \'yes\')')
AddOption('--build-examples',
		dest = 'build-examples',
		action = 'store_true',
		help = 'Build ISL examples of use (the same as if \'ISL_BUILD_EXAMPLES\' environment variable is set to \'yes\')')
AddOption('--build-tests',
		dest = 'build-tests',
		action = 'store_true',
		help = 'Build ISL tests (the same as if \'ISL_BUILD_TESTS\' environment variable is set to \'yes\')')
AddOption('--core-debugging',
		dest = 'core-debugging',
		action = 'store_true',
		help = 'Turn on ISL core debugging (the same as if \'ISL_CORE_DEBUGGING\' environment variable is set to \'yes\')')
AddOption('--log-debugging',
		dest = 'log-debugging',
		action = 'store_true',
		help = 'Turn on ISL logging subsystem debugging to stdouts (the same as if \'ISL_LOG_DEBUGGING\' environment variable is set to \'yes\')')
AddOption('--prefix',
		dest = 'prefix',
		nargs = 1,
		type = 'string',
		action = 'store',
		help = 'Prefix for installation. Usage: "scons --prefix=<instalation_path> install"')

sconscriptTargets = ['src/SConscript']
if GetOption('build-scada') or os.environ.get('ISL_BUILD_SCADA', '').upper() == 'YES':
	sconscriptTargets.append('modules/scada/SConscript')
if GetOption('build-examples') or os.environ.get('ISL_BUILD_EXAMPLES', '').upper() == 'YES':
	sconscriptTargets.append('examples/SConscript')
if GetOption('build-tests') or os.environ.get('ISL_BUILD_TESTS', '').upper() == 'YES':
	sconscriptTargets.append('tests/SConscript')
SConscript(sconscriptTargets)

# Uninstall section
env = Environment()
uninstaller = env.Command('uninstall', None, Delete(env.FindInstalledFiles()))
env.Alias('uninstall', uninstaller)
