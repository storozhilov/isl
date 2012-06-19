#include <isl/common.hxx>
#include <isl/AbstractPluginServer.hxx>
#include <isl/LogMessage.hxx>
#include <isl/NameListReleaser.hxx>
#include <isl/Exception.hxx>
#include <isl/SystemCallError.hxx>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <sstream>

namespace isl
{

const char * AbstractPluginServer::CreatePluginSubsystemFunctionName = "islCreatePluginSubsystem";

AbstractPluginServer::AbstractPluginServer(int argc, char * argv[], const std::string& pluginsPath, bool autoLoadPlugins) :
	AbstractServer(argc, argv),
	_plugins(),
	_pluginsPath(pluginsPath),
	_pluginsPathRwLock(),
	_pluginsLoaded(false)
{
	if (autoLoadPlugins) {
		loadPlugins();
	}
}

AbstractPluginServer::~AbstractPluginServer()
{
	if (_pluginsLoaded) {
		unloadPlugins();
	}
}

std::string AbstractPluginServer::pluginsPath() const
{
	ReadLocker locker(_pluginsPathRwLock);
	return _pluginsPath;
}

void AbstractPluginServer::setPluginsPath(const std::string& newValue)
{
	WriteLocker locker(_pluginsPathRwLock);
	_pluginsPath = newValue;
}

void AbstractPluginServer::loadPlugins()
{
	if (_pluginsLoaded) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Plugins have been already loaded"));
	}
	//if (_pluginsPath.empty()) {
	//	// TODO
	//	throw std::runtime_error("Plugins path has not been set");
	//}
	struct stat fileInfo;
	if (stat(_pluginsPath.c_str(), &fileInfo) != 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::Stat, errno));
	}
	if (!S_ISDIR(fileInfo.st_mode)) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Path to pluging should be directory"));
	}
	struct ::dirent **nameList;
	int pluginsAmount = scandir(_pluginsPath.c_str(), &nameList, AbstractPluginServer_filterDirEntry, alphasort);
	if (pluginsAmount < 0) {
		throw Exception(SystemCallError(SOURCE_LOCATION_ARGS, SystemCallError::ScanDir, errno));
	}
	NameListReleaser releaser(nameList, pluginsAmount);
	for (int i = 0; i < pluginsAmount; ++i) {
		Plugin newPlugin;
		newPlugin.name = nameList[i]->d_name;
		newPlugin.fileName = _pluginsPath;
		if (!newPlugin.fileName.empty() && newPlugin.fileName.at(newPlugin.fileName.length() - 1) != '/') {
			newPlugin.fileName += '/';
		}
		newPlugin.fileName += newPlugin.name;
		newPlugin.handle = dlopen(newPlugin.fileName.c_str(), RTLD_LAZY);
		if (!newPlugin.handle) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Error load plugin library '" + newPlugin.fileName + "\': " + dlerror()));
			continue;
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Plugin library '" + newPlugin.fileName + "' loaded successfully"));
		newPlugin.createFunc = reinterpret_cast<CreatePluginSubsystemFunction>(dlsym(newPlugin.handle, CreatePluginSubsystemFunctionName));
		if (!newPlugin.createFunc) {
			errorLog().log(LogMessage(
				SOURCE_LOCATION_ARGS,
				std::string("Error create plugin subsystem function lookup '") + CreatePluginSubsystemFunctionName + "\' in plugin library '" + newPlugin.fileName +
					"': " + dlerror()
			));
			continue;
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, std::string("Create subsystem function '") + CreatePluginSubsystemFunctionName + "\' found in '" + newPlugin.fileName + '\''));
		newPlugin.subsystem = (*newPlugin.createFunc)(*this);
		_plugins.push_back(newPlugin);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Plugin '" + newPlugin.fileName + "' loaded successfully"));
	}
	_pluginsLoaded = true;
}

void AbstractPluginServer::unloadPlugins()
{
	if (!_pluginsLoaded) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Plugins already unloaded"));
	}
	for (Plugins::iterator i = _plugins.begin(); i != _plugins.end(); ++i) {
		delete (*i).subsystem;
		if (dlclose((*i).handle)) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Error unload plugin '" + (*i).name + "' library: " + dlerror()));
		}
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Plugin '" + (*i).name + "' unloaded successfully"));
	}
	_plugins.clear();
	_pluginsLoaded = false;
}

void AbstractPluginServer::beforeStartPlugins()
{}

void AbstractPluginServer::afterStartPlugins()
{}

void AbstractPluginServer::beforeStopPlugins()
{}

void AbstractPluginServer::afterStopPlugins()
{}

void AbstractPluginServer::onStart()
{
	beforeStartPlugins();
	for (Plugins::iterator i = _plugins.begin(); i != _plugins.end(); ++i) {
		(*i).subsystem->start();
	}
	afterStartPlugins();
}

void AbstractPluginServer::onStop()
{
	beforeStopPlugins();
	for (Plugins::iterator i = _plugins.begin(); i != _plugins.end(); ++i) {
		(*i).subsystem->stop();
	}
	afterStopPlugins();
}

extern "C" int AbstractPluginServer_filterDirEntry(const struct ::dirent * entry)
{
	const char * fn = entry->d_name;
	size_t len = strlen(fn);
	return (fn[len - 3] == '.' && fn[len - 2] == 's' && fn[len - 1] == 'o') ? 1 : 0;
}

} // namespace isl
