#include <isl/common.hxx>
#include <isl/Server.hxx>
#include <isl/SignalHandler.hxx>
#include <isl/Timer.hxx>
#include <isl/LogMessage.hxx>
#include <isl/FileLogTarget.hxx>
#include <iostream>

class TimerTask : public isl::Timer::AbstractTask
{
public:
	TimerTask() :
		isl::Timer::AbstractTask()
	{}
private:
	//virtual void execute(isl::Timer& timer, size_t expirationsAmount)
	virtual void execute(isl::Timer& timer, const isl::Timer::TimestampContainer& expiredTimestamps)
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Timer task has been executed"));
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 500000000;
		nanosleep(&ts, 0);
	}
};

class TimerServer : public isl::Server
{
public:
	TimerServer(int argc, char * argv[]) :
		isl::Server(argc, argv),
		_signalHandler(this),
		_timer(this),
		_timerTask()
	{
		_timer.registerTask(_timerTask, isl::Timeout(5));
	}
private:
	TimerServer();
	TimerServer(const TimerServer&);
	// Some event handlers re-definition
	void beforeStart()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Starting server"));
	}
	void afterStart()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Server has been started"));
	}
	void beforeStop()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Stopping server"));
	}
	void afterStop()
	{
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, "Server has been stopped"));
	}

	isl::SignalHandler _signalHandler;					// We need a signal handler to stop/restart server
	isl::Timer _timer;
	TimerTask _timerTask;
};

int main(int argc, char *argv[])
{
	isl::writePid("timer.pid");						// Writing PID of the server to file
	isl::debugLog().connectTarget(isl::FileLogTarget("timer.log"));		// Connecting basic logs to one file target
	isl::warningLog().connectTarget(isl::FileLogTarget("timer.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("timer.log"));
	TimerServer server(argc, argv);						// Creating server object
	server.run();								// Running server
	isl::debugLog().disconnectTargets();					// Disconnecting basic logs from the targets
	isl::warningLog().disconnectTargets();
	isl::errorLog().disconnectTargets();
	/*struct timespec ts1 = isl::now();
	struct timespec ts2 = isl::operator+(ts1, isl::Timeout(0, 999999999).timeSpec());
	struct timespec ts3 = isl::operator-(ts1, isl::Timeout(1, 999999999).timeSpec());
	std::clog << "ts1 = {" << ts1.tv_sec << ", " << ts1.tv_nsec << "}" << std::endl;
	std::clog << "ts2 = {" << ts2.tv_sec << ", " << ts2.tv_nsec << "}" << std::endl;
	std::clog << "ts3 = {" << ts3.tv_sec << ", " << ts3.tv_nsec << "}" << std::endl;

	isl::DateTime dt(2012, 12, 21, 10, 30, 45, 800000000);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt = dt + isl::Timeout(1, 200000001);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt += isl::Timeout(0, 999999999);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt -= isl::Timeout(0, 999999999);
	std::clog << "dt = " << dt.toString() << std::endl;
	dt = dt - isl::Timeout(1, 200000001);
	std::clog << "dt = " << dt.toString() << std::endl;

	isl::DateTime sf(2012, 12, 21, 23, 59, 59);
	isl::DateTime fb(2012, 12, 22, 0, 0, 1);
	size_t expirationsAmount = isl::DateTime::expirationsInFrame(sf, fb, isl::Timeout(0, 200000000));
	std::clog << "expirationsAmount = " << expirationsAmount << std::endl;*/
}
