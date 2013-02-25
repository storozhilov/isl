#include <isl/PidFile.hxx>
#include <isl/Server.hxx>
#include <isl/Timer.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/DirectLogger.hxx>
#include <isl/StreamLogTarget.hxx>
#include <iostream>

class ScheduledTask : public isl::Timer::AbstractScheduledTask
{
public:
	ScheduledTask() :
		isl::Timer::AbstractScheduledTask()
	{}
private:
	virtual void execute(isl::Timer& timer, const isl::Timestamp& timestamp)
	{
		std::ostringstream msg;
		msg << "Scheduled task execution has been fired. Task timestamp: " << isl::DateTime(timestamp).toString();
		isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
	}
};

class PeriodicTask : public isl::Timer::AbstractPeriodicTask
{
public:
	PeriodicTask(ScheduledTask& scheduledTask) :
		isl::Timer::AbstractPeriodicTask(),
		_scheduledTask(scheduledTask)
	{}
private:
	PeriodicTask();

	virtual void execute(isl::Timer& timer, const isl::Timestamp& lastExpiredTimestamp, size_t expiredTimestamps, const isl::Timeout& timeout)
	{
		timer.scheduleTask(_scheduledTask, isl::Timeout(1));
		std::ostringstream msg;
		msg << "Periodic task execution has been fired. Last expired timestamp: {" << isl::DateTime(lastExpiredTimestamp).toString() <<
			"}, expired timestamps: " << expiredTimestamps << ", task execution timeout: {" << timeout.timeSpec().tv_sec << ", " <<
			timeout.timeSpec().tv_nsec << "}";
		isl::Log::debug().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
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
		isl::Log::warning().log(isl::LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
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
	isl::PidFile pidFile("timer.pid");					// Writing PID of the server to file
	isl::DirectLogger logger;						// Logging setup
	isl::StreamLogTarget coutTarget(logger, std::cout);
	isl::Log::debug().connect(coutTarget);
	isl::Log::warning().connect(coutTarget);
	isl::Log::error().connect(coutTarget);
	TimerServer server(argc, argv);						// Creating server object
	server.run();								// Running server
}
