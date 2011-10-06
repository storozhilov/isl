import os
#def_env = DefaultEnvironment()
#env = def_env.Clone(ENV = {'PATH' : os.environ['PATH']})

# Environment for the Sum Solaris
#env = Environment(
#CCFLAGS = ['-features=extensions', '-D_POSIX_PTHREAD_SEMANTICS', '-mt'],
#CPPPATH = './include'
#)

# Environment for Linux
env = Environment(
CCFLAGS = '-g',
CPPPATH = './include',
LIBS = 'dl'
)
env.Append(ENV = {'PATH' : os.environ['PATH']})

target_name = 'lib/isl'
include_path = './include'
#compiler_flags = ['-features=extensions', '-D_POSIX_PTHREAD_SEMANTICS', '-mt']

core_src = Glob('./src/Core/*.cxx')
expt_src = Glob('./src/Exception/*.cxx')
http_src = Glob('./src/HTTP/*.cxx')
io_src = Glob('./src/IO/*.cxx')
log_src = Glob('./src/Log/*.cxx')
srv_src = Glob('./src/Server/*.cxx')
thr_src = Glob('./src/Thread/*.cxx')
#xml_src = Glob('./src/XML/*.cxx')
contrib_source_browser_src = Glob('./src/contrib/SourceBrowser/*.cxx')
src_files = [core_src, expt_src, http_src, io_src, log_src, srv_src, thr_src, contrib_source_browser_src]
#src_files = [core_src, expt_src, http_src, io_src, log_src, srv_src, thr_src, xml_src]

#env.StaticLibrary(target_name, src_files, CPPPATH = include_path, CCFLAGS = compiler_flags)
#env.StaticLibrary(target_name, src_files)
staticLibraryBuilder = env.StaticLibrary(target_name, src_files)
sharedLibraryBuilder = env.SharedLibrary(target_name, src_files)
#libraryInstaller = env.Install('/usr/local/lib', staticLibraryBuilder)
libraryInstaller = env.Install('/usr/local/lib', [staticLibraryBuilder, sharedLibraryBuilder])
headersInstaller = env.Install('/usr/local/include/isl', Glob('./include/isl/*.hxx'))
#env.Alias('install', libraryInstaller)
env.Alias('install', [libraryInstaller, headersInstaller])

#env.Command('uninstall', None, Delete(FindInstalledFiles()))


#env.SharedLibrary(target_name, src_files, CPPPATH = include_path, CCFLAGS = compiler_flags)

# Builders for examples
env.Program('examples/Test/test', Glob('examples/Test/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
env.Program('examples/SourceBrowser/sbd', Glob('examples/SourceBrowser/*.cxx'), LIBS = ['isl', 'pthread', 'rt'], LIBPATH = 'lib')
