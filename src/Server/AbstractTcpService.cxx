#include <isl/AbstractTcpService.hxx>
#include <isl/Core.hxx>
#include <isl/TcpSocket.hxx>
#include <memory>

namespace isl
{

/*------------------------------------------------------------------------------
 * AbstractTcpService
------------------------------------------------------------------------------*/

AbstractTcpService::AbstractTcpService(AbstractSubsystem * owner, unsigned int port, unsigned int maxClients,
		const Timeout& timeout, const std::list<std::string>& interfaces, unsigned int backLog,
		unsigned int maxTaskQueueOverflowSize) :
	AbstractSubsystem(owner),
	_taskDispatcher(this, maxClients, maxTaskQueueOverflowSize),
	_listenerThread(*this),
	_port(port),
	_portRwLock(),
	_timeout(timeout),
	_timeoutRwLock(),
	_interfaces(interfaces),
	_interfacesRwLock(),
	_backLog(backLog),
	_backLogRwLock()
{}

void AbstractTcpService::start()
{
	setState(IdlingState, StartingState);
	_taskDispatcher.start();
	_listenerThread.start();
}

void AbstractTcpService::stop()
{
	setState(StoppingState);
	_listenerThread.join();
	_taskDispatcher.stop();
	setState(IdlingState);
}

/*------------------------------------------------------------------------------
 * AbstractTcpService::ListenerThread
------------------------------------------------------------------------------*/

AbstractTcpService::ListenerThread::ListenerThread(AbstractTcpService& service) :
	Thread(true),
	_service(service),
	_sleepCond()
{}

void AbstractTcpService::ListenerThread::run()
{
	// Starting section
	TcpSocket serverSocket;
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
	while (true) {
		try {
			if (_service.state() != AbstractSubsystem::StartingState) {
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
						L"Unexpected state detected while starting up. Exiting from subsystem's listemer thread."));
				return;
			}
			if (!serverSocket.isOpen()) {
				serverSocket.open();
				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
			}
			serverSocket.bind(_service.port(), _service.interfaces());
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been binded"));
			serverSocket.listen(_service.backLog());
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been switched to the listening state"));
			break;
		} catch (std::exception& e) {
			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Starting subsystem error."));
			sleep();
		} catch (...) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem unknown error."));
			sleep();
		}
	}
	_service.setState(StartingState, RunningState);
	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been successfully started"));
	// Running section
	while (true) {
		try {
			if (_service.state() != AbstractSubsystem::RunningState) {
				break;
			}
			std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(_service.timeout()));
			if (_service.state() != AbstractSubsystem::RunningState) {
				break;
			}
			if (!socketAutoPtr.get()) {
				// Accepting TCP-connection timeout expired
				continue;
			}
			std::auto_ptr<AbstractTask> taskAutoPtr(_service.createTask(socketAutoPtr.get()));
			socketAutoPtr.release();
			if (_service._taskDispatcher.perform(taskAutoPtr.get())) {
				taskAutoPtr.release();
			} else {
				Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
			}
		} catch (std::exception& e) {
			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Running subsystem error. Stopping subsystem."));
			break;
		} catch (...) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Running subsystem unknown error. Stopping subsystem."));
			break;
		}
	}
}

} // namespace isl

