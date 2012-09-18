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
	virtual void execute(isl::Timer& timer, const struct timespec& lastExpiredTimestamp, size_t expiredTimestamps, const isl::Timeout& timeout)
	{
		std::ostringstream msg;
		msg << "Timer task execution has been fired. Last expired timestamp: {" << isl::DateTime(lastExpiredTimestamp).toString() <<
			"}, expired timestamps: " << expiredTimestamps << ", task execution timeout: {" << timeout.timeSpec().tv_sec << ", " <<
			timeout.timeSpec().tv_nsec << "}";
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		// Sleep alittle
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 500000000;
		nanosleep(&ts, 0);
	}
};

class Timer : public isl::Timer
{
public:

	Timer(Subsystem * owner) :
		isl::Timer(owner)
	{}
private:
	virtual void onOverload(size_t ticksExpired)
	{
		std::ostringstream msg;
		msg << "Timer overload has been detected: " << ticksExpired << " ticks expired";
		isl::warningLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
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
	Timer _timer;
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
}
