# ISL scons script
#
# To build & install type 'scons & sudo scons install' in command line
# To uninstall type 'sudo scons uninstall' in command line
# To build examples type 'ISL_BUILD_EXAPMLES=yes scons' in command line

import os

# Environment for the Sum Solaris
#env = Environment(
#CCFLAGS = ['-features=extensions', '-D_POSIX_PTHREAD_SEMANTICS', '-mt'],
#CPPPATH = './include'
#)
#compiler_flags = ['-features=extensions', '-D_POSIX_PTHREAD_SEMANTICS', '-mt']

# Environment for Linux
env = Environment(
CCFLAGS = '-g',
CPPPATH = './include',
LIBS = 'dl'
)
env.Append(ENV = {'PATH' : os.environ['PATH']})

# Build variables
prefix = os.environ['PREFIX'] if ('PREFIX' in os.environ) else '/usr/local'
targetName = 'lib/isl'
sourceFiles = [
Glob('./src/Core/*.cxx'),
Glob('./src/Exception/*.cxx'),
Glob('./src/HTTP/*.cxx'),
Glob('./src/IO/*.cxx'),
Glob('./src/Log/*.cxx'),
Glob('./src/Server/*.cxx'),
Glob('./src/Thread/*.cxx'),
Glob('./src/MQ/*.cxx')
]

# Library builders
staticLibraryBuilder = env.StaticLibrary(targetName, sourceFiles)
sharedLibraryBuilder = env.SharedLibrary(targetName, sourceFiles)

# Examples builers
testExampleBuilder = env.Program('examples/Test/test', Glob('examples/Test/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
httpServerExampleBuilder = env.Program('examples/HttpServer/hsd', Glob('examples/HttpServer/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
httpCopyServerExampleBuilder = env.Program('examples/HttpCopy/htcpd', Glob('examples/HttpCopy/server/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
httpCopyClientExampleBuilder = env.Program('examples/HttpCopy/htcp', Glob('examples/HttpCopy/client/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
sourceBrouserExampleBuilder = env.Program('examples/SourceBrowser/sbd', Glob('examples/SourceBrowser/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
echoMessageBrokerExampleBuilder = env.Program('examples/EchoMessageBroker/embd', Glob('examples/EchoMessageBroker/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')

# Installer
libraryInstaller = env.Install(os.path.join(prefix, 'lib'), [staticLibraryBuilder, sharedLibraryBuilder])
headersInstaller = env.Install(os.path.join(prefix, 'include/isl'), Glob('./include/isl/*.hxx'))
env.Alias('install', [libraryInstaller, headersInstaller])

# Uninstaller
env.Command('uninstall', None, Delete(FindInstalledFiles()))

# Setting default targets
if os.environ.get('ISL_BUILD_EXAPMLES','').upper() == 'YES':
	#Default(staticLibraryBuilder, sharedLibraryBuilder, testExampleBuilder, sourceBrouserExampleBuilder, echoMessageBrokerExampleBuilder)
	#Default(staticLibraryBuilder, sharedLibraryBuilder, testExampleBuilder, httpCopyServerExampleBuilder, httpCopyClientExampleBuilder, sourceBrouserExampleBuilder, echoMessageBrokerExampleBuilder)
	Default(staticLibraryBuilder, sharedLibraryBuilder, httpServerExampleBuilder, httpCopyServerExampleBuilder, httpCopyClientExampleBuilder, sourceBrouserExampleBuilder, echoMessageBrokerExampleBuilder)
else:
	Default(staticLibraryBuilder, sharedLibraryBuilder)
