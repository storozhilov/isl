#include <isl/DirectLogger.hxx>
#include <isl/StreamLogTarget.hxx>
#include <isl/LogMessage.hxx>
#include <isl/Log.hxx>
#include <isl/AbstractThread.hxx>
#include <iostream>
#include <fstream>
#include <list>

#define THREADS_AMOUNT 10

isl::Log log("NOTIFY");

class LogThread : public isl::AbstractThread
{
public:
	LogThread(int id) :
		AbstractThread(),
		_id(id)
	{}
private:
	virtual void run()
	{
		for (int i = 0; i < 1000; ++i) {
			log.log(isl::LogMessage(SOURCE_LOCATION_ARGS, "") << i << "-th log message from " << _id << " thread");
		}
	}

	int _id;
};

int main(int argc, char **argv)
{
	isl::DirectLogger logger;
	isl::StreamLogTarget coutTarget(logger, std::cout);
	log.connect(coutTarget);
	std::ofstream ofs("log.log", std::ios_base::out | std::ios_base::app);
	isl::StreamLogTarget fileTarget(logger, ofs);
	log.connect(fileTarget);
	std::list<LogThread *> threads;
	for (int i = 0; i < THREADS_AMOUNT; ++i) {
		threads.push_back(new LogThread(i));
	}
	for (std::list<LogThread *>::iterator i = threads.begin(); i != threads.end(); ++i) {
		(*i)->start();
	}
	for (std::list<LogThread *>::iterator i = threads.begin(); i != threads.end(); ++i) {
		(*i)->join();
		delete *i;
	}
}
