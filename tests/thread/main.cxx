#include <isl/FunctorThread.hxx>
#include <isl/MemFunThread.hxx>
#include <isl/TaskDispatcher.hxx>
#include <isl/MultiTaskDispatcher.hxx>
#include <isl/FileLogTarget.hxx>
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
	void execute(isl::NewTaskDispatcher<Task>& dispatcher)
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

int main(int argc, char *argv[])
{
	isl::debugLog().connectTarget(isl::FileLogTarget("thread.log"));
	isl::warningLog().connectTarget(isl::FileLogTarget("thread.log"));
	isl::errorLog().connectTarget(isl::FileLogTarget("thread.log"));

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

	isl::NewTaskDispatcher<Task> td(0, 20);
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
