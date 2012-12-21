#include <isl/common.hxx>
#include <isl/Server.hxx>
#include <isl/Timer.hxx>
#include <isl/LogMessage.hxx>
#include <isl/FileLogTarget.hxx>
#include <iostream>

class ScheduledTask : public isl::Timer::AbstractTask
{
public:
	ScheduledTask() :
		isl::Timer::AbstractTask()
	{}
private:
	virtual void execute(isl::Timer& timer, const struct timespec& lastExpiredTimestamp, size_t expiredTimestamps, const isl::Timeout& timeout)
	{
		std::ostringstream msg;
		msg << "Scheduled task execution has been fired. Last expired timestamp: {" << isl::DateTime(lastExpiredTimestamp).toString() <<
			"}, expired timestamps: " << expiredTimestamps << ", task execution timeout: {" << timeout.timeSpec().tv_sec << ", " <<
			timeout.timeSpec().tv_nsec << "}";
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	}
};

class PeriodicTask : public isl::Timer::AbstractTask
{
public:
	PeriodicTask(ScheduledTask& scheduledTask) :
		isl::Timer::AbstractTask(),
		_scheduledTask(scheduledTask)
	{}
private:
	PeriodicTask();

	virtual void execute(isl::Timer& timer, const struct timespec& lastExpiredTimestamp, size_t expiredTimestamps, const isl::Timeout& timeout)
	{
		timer.scheduleTask(_scheduledTask, isl::Timeout(1));
		std::ostringstream msg;
		msg << "Periodic task execution has been fired. Last expired timestamp: {" << isl::DateTime(lastExpiredTimestamp).toString() <<
			"}, expired timestamps: " << expiredTimestamps << ", task execution timeout: {" << timeout.timeSpec().tv_sec << ", " <<
			timeout.timeSpec().tv_nsec << "}";
		isl::debugLog().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
		// Sleep a little
		/*struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 500000000;
		nanosleep(&ts, 0);*/
	}

	ScheduledTask& _scheduledTask;
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

//class TimerServer : public isl::Server
class TimerServer : public isl::Server
{
public:
	TimerServer(int argc, char * argv[]) :
		isl::Server(argc, argv),
		_timer(this),
		_scheduledTask(),
		_periodicTask(_scheduledTask)
	{
		_timer.registerPeriodicTask(_periodicTask, isl::Timeout(5));
	}
private:
	TimerServer();
	TimerServer(const TimerServer&);

	Timer _timer;
	ScheduledTask _scheduledTask;
	PeriodicTask _periodicTask;
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
