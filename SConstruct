# ISL scons script
#
# To build & install type 'scons & sudo scons install' in command line.
# To uninstall type 'sudo scons uninstall' in command line.
# Run 'scons -h' to list build options.
#
# TODO "prefix" as scons option support.

import os

AddOption('--build-examples',
		dest = 'build-examples',
		action = 'store_true',
		help = 'Build ISL examples of use (the same as if \'ISL_BUILD_EXAMPLES\' environment variable is set to \'yes\')')
AddOption('--core-debugging',
		dest = 'core-debugging',
		action = 'store_true',
		help = 'Turn on ISL core debugging (the same as if \'ISL_CORE_DEBUGGING\' environment variable is set to \'yes\')')
AddOption('--log-debugging',
		dest = 'log-debugging',
		action = 'store_true',
		help = 'Turn on ISL logging subsystem debugging to stdouts (the same as if \'ISL_LOG_DEBUGGING\' environment variable is set to \'yes\')')


if GetOption('build-examples') or os.environ.get('ISL_BUILD_EXAMPLES', '').upper() == 'YES':
	SConscript(['src/SConscript', 'examples/Test/SConscript', 'examples/HttpServer/SConscript', 'examples/HttpCopy/SConscript'])
else:
	SConscript(['src/SConscript'])

# Uninstall section
env = Environment()
uninstaller = env.Command('uninstall', None, Delete(env.FindInstalledFiles()))
env.Alias('uninstall', uninstaller)
