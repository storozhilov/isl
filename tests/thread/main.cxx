#include <isl/FunctorThread.hxx>
#include <isl/MemFunThread.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/MultiTaskDispatcher.hxx>
#include <isl/FileLogTarget.hxx>
#include <isl/InterThreadRequester.hxx>
#include <iostream>

isl::Mutex consoleMutex;

void thrfun()
{
	isl::MutexLocker locker(consoleMutex);
	std::cout << "Hello from thrfun() function!" << std::endl;
}

class ThreadFunctor
{
public:
	ThreadFunctor(const std::string& name) :
		_name(name)
	{}

	void operator()()
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from \"" << _name << "\" functor!" << std::endl;
	}
private:
	std::string _name;
};

class ThreadMemFun
{
public:
	ThreadMemFun(const std::string& name) :
		_name(name)
	{}

	void threadFunction0()
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from \"" << _name << "\"'s ThreadMemFun::threadFunction0() member function!" << std::endl;
	}
	void threadFunction1(isl::MemFunThread<ThreadMemFun>& thread)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from \"" << _name << "\"'s ThreadMemFun::threadFunction1() member function!" << std::endl;
	}
private:
	std::string _name;
};

class Task
{
public:
	void execute(isl::TaskDispatcher<Task>& dispatcher)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from Task::execute() member function!" << std::endl;
	}
	void execute1(isl::MultiTaskDispatcher<Task>& dispatcher)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from Task::execute1() member function!" << std::endl;
	}
	void execute2(isl::MultiTaskDispatcher<Task>& dispatcher)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from Task::execute2() member function!" << std::endl;
	}
	void execute3(isl::MultiTaskDispatcher<Task>& dispatcher)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from Task::execute3() member function!" << std::endl;
	}
	void execute4(isl::MultiTaskDispatcher<Task>& dispatcher)
	{
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Hello from Task::execute4() member function!" << std::endl;
	}
private:
};

class RespondentThread : public isl::AbstractThread
{
public:
	enum Message {
		OkResponse,
		StopRequest,
		PingRequest,
		PongResponse
	};
	typedef isl::InterThreadRequester<Message> InterThreadRequesterType;

	RespondentThread() :
		isl::AbstractThread(),
		_requester()
	{}
	InterThreadRequesterType& requester()
	{
		return _requester;
	}

	static const char * messageName(const Message& msg)
	{
		static const char * okResponseName = "OK response";
		static const char * stopRequestName = "Stop request";
		static const char * pingRequestName = "Ping request";
		static const char * pongResponseName = "Pong response";
		static const char * unknownName = "[Unknown]";
		if (msg == OkResponse) {
			return okResponseName;
		} else if (msg == StopRequest) {
			return stopRequestName;
		} else if (msg == PingRequest) {
			return pingRequestName;
		} else if (msg == PongResponse) {
			return pongResponseName;
		} else {
			return unknownName;
		}
	}

private:
	virtual void run()
	{
		while (true) {
			const InterThreadRequesterType::PendingRequest * pendingRequestPtr = _requester.awaitRequest(isl::Timeout::defaultTimeout());
			if (!pendingRequestPtr) {
				continue;
			}
			if (pendingRequestPtr->request() == StopRequest) {
				std::cout << pthread_self() << ": RespondentThread::run(): \"" << messageName(pendingRequestPtr->request()) << "\" message has been received" << std::endl;
				_requester.sendResponse(OkResponse);
				return;
			} else if (pendingRequestPtr->request() == PingRequest) {
				std::cout << pthread_self() << ": RespondentThread::run(): \"" << messageName(pendingRequestPtr->request()) << "\" message has been received" << std::endl;
				_requester.sendResponse(PongResponse);
			} else {
				std::cout << pthread_self() << ": RespondentThread::run(): Invalid command (" << pendingRequestPtr->request() << ") has been received" << std::endl;
			}
		}
	}

	InterThreadRequesterType _requester;
};

int main(int argc, char *argv[])
{
	//isl::InterThreadRequester<int> r;
	isl::debugLog().connectTarget(isl::FileLogTarget("thread.log"));
	isl::warningLog().connectTarget(isl::FileLogTarget("thread.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("thread.log"));

	RespondentThread rt;
	rt.start();
	size_t requestId = rt.requester().sendRequest(RespondentThread::PingRequest);
	std::cout << pthread_self() << ": main(): Request id is: " << requestId << std::endl;
	std::auto_ptr<RespondentThread::Message> responseAutoPtr = rt.requester().awaitResponse(requestId, isl::Timeout::defaultTimeout());
	if (responseAutoPtr.get()) {
		std::cout << pthread_self() << ": main(): \"" << RespondentThread::messageName(*responseAutoPtr.get()) << "\" message has been received from respondent thread" << std::endl;
	} else {
		std::cout << pthread_self() << ": main(): No response from respondent thread" << std::endl;
	}
	requestId = rt.requester().sendRequest(RespondentThread::PingRequest);
	std::cout << pthread_self() << ": main(): Request id is: " << requestId << std::endl;
	responseAutoPtr = rt.requester().awaitResponse(requestId, isl::Timeout::defaultTimeout());
	if (responseAutoPtr.get()) {
		std::cout << pthread_self() << ": main(): \"" << RespondentThread::messageName(*responseAutoPtr.get()) << "\" message has been received from respondent thread" << std::endl;
	} else {
		std::cout << pthread_self() << ": main(): No response from respondent thread" << std::endl;
	}
	requestId = rt.requester().sendRequest(RespondentThread::StopRequest);
	std::cout << pthread_self() << ": main(): Request id is: " << requestId << std::endl;
	responseAutoPtr = rt.requester().awaitResponse(requestId, isl::Timeout::defaultTimeout());
	if (responseAutoPtr.get()) {
		std::cout << pthread_self() << ": main(): \"" << RespondentThread::messageName(*responseAutoPtr.get()) << "\" message has been received from respondent thread" << std::endl;
	} else {
		std::cout << pthread_self() << ": main(): No response from respondent thread" << std::endl;
	}
	rt.join();

	std::auto_ptr<Task> taskAutoPtr;
	isl::MultiTaskDispatcher<Task> mtd(0, 20);
	mtd.start();
	taskAutoPtr.reset(new Task());
	mtd.perform(taskAutoPtr, &Task::execute1, &Task::execute2, &Task::execute3, &Task::execute4);
	if (taskAutoPtr.get()) {
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Task auto-pointer has not been released" << std::endl;
	} else {
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Task auto-pointer has been released" << std::endl;
	}
	mtd.stop();
	mtd.start();
	taskAutoPtr.reset(new Task());
	mtd.perform(taskAutoPtr, &Task::execute1, &Task::execute2, &Task::execute3, &Task::execute4);
	if (taskAutoPtr.get()) {
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Task auto-pointer has not been released" << std::endl;
	} else {
		isl::MutexLocker locker(consoleMutex);
		std::cout << "Task auto-pointer has been released" << std::endl;
	}
	mtd.stop();

	isl::TaskDispatcher<Task> td(0, 20);
	td.start();
	taskAutoPtr.reset(new Task());
	td.perform(taskAutoPtr, &Task::execute);
	td.stop();

	isl::FunctionThread thr1;
	isl::FunctorThread<ThreadFunctor> thr2;
	isl::FunctorThread<ThreadFunctor> thr3(true);
	isl::FunctorThread<ThreadFunctor> thr4(true, true);
	isl::FunctorThread<ThreadFunctor> thr5(true, true, true);
	isl::MemFunThread<ThreadMemFun> thr6;
	isl::MemFunThread<ThreadMemFun> thr7;
	ThreadFunctor tf1("01");
	ThreadFunctor tf2("02");
	ThreadFunctor tf3("03");
	ThreadFunctor tf4("04");
	ThreadMemFun tmf1("05");
	thr1.start(thrfun);
	thr2.start(tf1);
	thr3.start(tf2);
	thr4.start(tf3);
	thr5.start(tf4);
	thr6.start(tmf1, &ThreadMemFun::threadFunction0);
	thr7.start(tmf1, &ThreadMemFun::threadFunction1);
	thr1.join();
	thr2.join();
	thr3.join();
	thr4.join();
	thr5.join();
	thr6.join();
	thr7.join();

}
