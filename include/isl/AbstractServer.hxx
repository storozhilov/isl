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
	AbstractServer(int argc, char * argv[], const Timeout& timeout = Timeout(1));

	void run();
	inline unsigned int argc() const
	{
		return _argv.size();
	}
	inline std::string argv(unsigned int argNo) const
	{
		return _argv.at(argNo);
	}
	//! Restarting server abstract virtual method to override in descendants
	virtual void restart() = 0;
private:
	AbstractServer();
	AbstractServer(const AbstractServer&);						// No copy

	AbstractServer& operator=(const AbstractServer&);				// No copy

	std::vector<std::string> _argv;
	Timeout _timeout;
};

} // namespace isl

#endif

