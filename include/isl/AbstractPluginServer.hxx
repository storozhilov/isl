#ifndef ISL__ABSTRACT_PLUGIN_SERVER__HXX
#define ISL__ABSTRACT_PLUGIN_SERVER__HXX

#include <isl/AbstractServer.hxx>
#include <isl/AbstractSubsystem.hxx>
#include <isl/ReadWriteLock.hxx>
#include <vector>
#include <string>
#include <dirent.h>

namespace isl
{

class AbstractPluginServer : public AbstractServer
{
public:
	AbstractPluginServer(int argc, char * argv[], const std::string& pluginsPath = "modules/", bool autoLoadPlugins = true);
	~AbstractPluginServer();

	std::string pluginsPath() const;
	void setPluginsPath(const std::string& newValue);
protected:
	typedef AbstractSubsystem * (* CreatePluginSubsystemFunction)(AbstractPluginServer&);

	void loadPlugins();							// NOTE: Non thread-safe
	void unloadPlugins();							// NOTE: Non thread-safe

	virtual void beforeStartPlugins();
	virtual void afterStartPlugins();
	virtual void beforeStopPlugins();
	virtual void afterStopPlugins();
private:
	AbstractPluginServer();
	AbstractPluginServer(const AbstractPluginServer&);

	AbstractPluginServer& operator==(const AbstractPluginServer&);

	struct Plugin
	{
		std::string name;
		std::string fileName;
		void * handle;
		CreatePluginSubsystemFunction createFunc;
		AbstractSubsystem * subsystem;
	};
	typedef std::vector<Plugin> Plugins;

	virtual void onStart();
	virtual void onStop();

	std::string _pluginsPath;
	mutable ReadWriteLock _pluginsPathRwLock;
	Plugins _plugins;
	bool _pluginsLoaded;

	static const char * CreatePluginSubsystemFunctionName;
};

extern "C" int AbstractPluginServer_filterDirEntry(const struct ::dirent * entry);

} // namespace isl

#endif
