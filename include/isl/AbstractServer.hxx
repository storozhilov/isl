#ifndef ISL__ABSTRACT_SERVER__HXX
#define ISL__ABSTRACT_SERVER__HXX

#include <isl/Enum.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/AbstractSubsystem.hxx>
#include <vector>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractServer
------------------------------------------------------------------------------*/

class AbstractServer : public AbstractSubsystem
{
public:
	AbstractServer(int argc, char * argv[]);

	void run();
	inline unsigned int argc() const
	{
		return _argv.size();
	}
	inline std::string argv(unsigned int argNo) const
	{
		return _argv.at(argNo);
	}
protected:
	virtual void onStart() = 0;
	virtual void onStop() = 0;
	virtual void onRestart();
private:
	AbstractServer();
	AbstractServer(const AbstractServer&);						// No copy

	AbstractServer& operator=(const AbstractServer&);				// No copy

	void start();

	virtual void onStartCommand();
	virtual void onStopCommand();
	virtual void onRestartCommand();

	std::vector<std::string> _argv;
};

} // namespace isl

#endif

