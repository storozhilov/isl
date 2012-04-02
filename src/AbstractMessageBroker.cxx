#include <isl/AbstractMessageBroker.hxx>
#include <isl/Core.hxx>
#include <memory>
#include <list>

namespace isl
{

///*------------------------------------------------------------------------------
// * AbstractMessageBroker
//------------------------------------------------------------------------------*/
//
//AbstractMessageBroker::AbstractMessageBroker(AbstractSubsystem * owner, unsigned int port, size_t maxClients,
//		size_t sendQueueSize, const Timeout& timeout, const std::list<std::string>& interfaces,
//		unsigned int backLog) :
//	AbstractSubsystem(owner),
//	_taskDispatcher(this, maxClients * 2),
//	_listenerThread(*this),
//	_port(port),
//	_portRwLock(),
//	_sendQueueSize(sendQueueSize),
//	_sendQueueSizeRwLock(),
//	_timeout(timeout),
//	_timeoutRwLock(),
//	_interfaces(interfaces),
//	_interfacesRwLock(),
//	_backLog(backLog),
//	_backLogRwLock()
//{}
//
//void AbstractMessageBroker::start()
//{
//	setState(IdlingState, StartingState);
//	_taskDispatcher.start();
//	_listenerThread.start();
//}
//
//void AbstractMessageBroker::stop()
//{
//	setState(StoppingState);
//	_listenerThread.join();
//	_taskDispatcher.stop();
//	setState(IdlingState);
//}
//
//AbstractMessageBroker::SenderTask * AbstractMessageBroker::createSenderTask(TcpSocket * socket)
//{
//	return new SenderTask(*this, socket);
//}
//
//AbstractMessageBroker::ReceiverTask * AbstractMessageBroker::createReceiverTask(SenderTask& senderTask)
//{
//	return new ReceiverTask(senderTask);
//}
//
///*------------------------------------------------------------------------------
// * AbstractMessageBroker::SenderTask
//------------------------------------------------------------------------------*/
//
//AbstractMessageBroker::SenderTask::SenderTask(AbstractMessageBroker& broker, TcpSocket * socket) :
//	TaskDispatcher::AbstractTask(),
//	_broker(broker),
//	_socketAutoPtr(socket),
//	_sendCond(),
//	_terminateFlag(false),
//	_messageQueue()
//{}
//
//AbstractMessageBroker::SenderTask::~SenderTask()
//{
//	for (MessageQueue::iterator i = _messageQueue.begin(); i != _messageQueue.end(); ++i) {
//		delete (*i);
//	}
//}
//
//bool AbstractMessageBroker::SenderTask::sendMessage(AbstractMessage * msg)
//{
//	// TODO Handle message queue overflow
//	MutexLocker locker(_sendCond.mutex());
//	if (_messageQueue.size() >= _broker.sendQueueSize()) {
//		Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Sending message queue overflow has been detected"));
//		return false;
//	}
//	_messageQueue.push_front(msg);
//	_sendCond.wakeOne();
//	return true;
//}
//
//void AbstractMessageBroker::SenderTask::executeImpl(TaskDispatcher::Worker& worker)
//{
//	while (true) {
//		std::auto_ptr<AbstractMessage> msg;
//		try {
//			MutexLocker locker(_sendCond.mutex());
//			// Checking self termination
//			if (_terminateFlag) {
//				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
//							"Task termination state has been detected before waiting for message - exiting from the task execution"));
//				return;
//			}
//			if (_messageQueue.empty()) {
//				// Wait for the next message if the message queue is empty
//				_sendCond.wait();
//				// Checking self termination
//				if (_terminateFlag) {
//					Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
//								"Task termination state has been detected after waiting for message - exiting from the task execution"));
//					return;
//				}
//				if (!_messageQueue.empty()) {
//					// Pick up the next message from the message queue if the message queue is not empty
//					msg.reset(_messageQueue.back());
//					_messageQueue.pop_back();
//				}
//			} else {
//				// Pick up the next message from the message queue if the message queue is not empty
//				msg.reset(_messageQueue.back());
//				_messageQueue.pop_back();
//			}
//		} catch (std::exception& e) {
//			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Fetching message error - exiting from the task execution"));
//		} catch (...) {
//			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Fetching message unknown error - exiting from the task execution"));
//		}
//		if (msg.get()) {
//			try {
//				_broker.sendMessage(*_socketAutoPtr.get(), *msg.get(), *this);
//			} catch (std::exception& e) {
//				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Sending message error - exiting from the task execution"));
//			} catch (...) {
//				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Sending message unknown error - exiting from the task execution"));
//			}
//		} else {
//			Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"No message to send"));
//		}
//	}
//}
//
///*------------------------------------------------------------------------------
// * AbstractMessageBroker::ReceiverTask
//------------------------------------------------------------------------------*/
//
//AbstractMessageBroker::ReceiverTask::ReceiverTask(SenderTask& senderTask) :
//	_senderTask(senderTask),
//	_broker(senderTask._broker),
//	_socket(*senderTask._socketAutoPtr.get()),
//	_terminateFlag(false),
//	_terminateFlagRWLock()
//{}
//
//void AbstractMessageBroker::ReceiverTask::executeImpl(TaskDispatcher::Worker& worker)
//{
//	// Finally to terminate sender task:
//	SenderTaskTerminator senderTaskTerminator(_senderTask);
//	while (true) {
//		// Checking broker termination
//		if (_broker.shouldTerminate()) {
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Broker termination has been detected before receiving message - exiting from the task execution"));
//			break;
//		}
//		// Checking self termination
//		if (shouldTerminate()) {
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Task termination has been detected before receiving message - exiting from the task execution"));
//			break;
//		}
//		std::auto_ptr<AbstractMessage> msgAutoPtr;
//		try {
//			// Receiving the message
//			msgAutoPtr.reset(_broker.recieveMessage(_socket, *this));
//		} catch (std::exception& e) {
//			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Receiving message error - exiting from the task execution"));
//			return;
//		} catch (...) {
//			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Receiving message unknown error - exiting from the task execution"));
//			return;
//		}
//		// Checking broker termination
//		if (_broker.shouldTerminate()) {
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Broker termination has been detected after receiving message - exiting from the task execution"));
//			break;
//		}
//		// Checking self termination
//		if (shouldTerminate()) {
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Task termination has been detected after receiving message - exiting from the task execution"));
//			break;
//		}
//		// Processing message if received
//		if (msgAutoPtr.get()) {
//			try {
//				_broker.processMessage(*msgAutoPtr.get(), *this, _senderTask);
//			} catch (std::exception& e) {
//				Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Processing message error - exiting from the task execution"));
//				return;
//			} catch (...) {
//				Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Processing message unknown error - exiting from the task execution"));
//				return;
//			}
//		}
//	}
//}
//
///*------------------------------------------------------------------------------
// * AbstractMessageBroker::ListenerThread
//------------------------------------------------------------------------------*/
//
//AbstractMessageBroker::ListenerThread::ListenerThread(AbstractMessageBroker& broker) :
//	Thread(true),
//	_broker(broker),
//	_sleepCond()
//{}
//
//void AbstractMessageBroker::ListenerThread::run()
//{
//	// Starting section
//	TcpSocket serverSocket;
//	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been created"));
//	while (true) {
//		try {
//			if (_broker.state() != AbstractSubsystem::StartingState) {
//				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS,
//						L"Unexpected state detected while starting up. Exiting from subsystem's listemer thread."));
//				return;
//			}
//			if (!serverSocket.isOpen()) {
//				serverSocket.open();
//				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been opened"));
//			}
//			serverSocket.bind(_broker.port(), _broker.interfaces());
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been binded"));
//			serverSocket.listen(_broker.backLog());
//			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Server socket has been switched to the listening state"));
//			break;
//		} catch (std::exception& e) {
//			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Starting subsystem error."));
//			sleep();
//		} catch (...) {
//			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Starting subsystem unknown error."));
//			sleep();
//		}
//	}
//	_broker.setState(StartingState, RunningState);
//	Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Subsystem has been successfully started"));
//	// Running section
//	while (true) {
//		try {
//			// Checking broker termination
//			if (_broker.state() != AbstractSubsystem::RunningState) {
//				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Broker termination has been detected before accepting TCP-connection - exiting from the listener thread"));
//				break;
//			}
//			// Accepting TCP-connection
//			std::auto_ptr<TcpSocket> socketAutoPtr(serverSocket.accept(_broker.timeout()));
//			// Checking broker termination
//			if (_broker.state() != AbstractSubsystem::RunningState) {
//				Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, "Broker termination has been detected after accepting TCP-connection - exiting from the listener thread"));
//				break;
//			}
//			if (!socketAutoPtr.get()) {
//				// Accepting TCP-connection timeout expired
//				continue;
//			}
//			// Creating sender and receiver tasks
//			std::auto_ptr<SenderTask> senderTaskAutoPtr(_broker.createSenderTask(socketAutoPtr.get()));
//			socketAutoPtr.release();
//			std::auto_ptr<ReceiverTask> receiverTaskAutoPtr(_broker.createReceiverTask(*senderTaskAutoPtr.get()));
//			// Making a tasks list and perfoming it in the task dispatcher
//			std::list<TaskDispatcher::AbstractTask *> tasks;
//			tasks.push_back(receiverTaskAutoPtr.get());
//			tasks.push_back(senderTaskAutoPtr.get());
//			if (_broker._taskDispatcher.perform(tasks)) {
//				receiverTaskAutoPtr.release();
//				senderTaskAutoPtr.release();
//			} else {
//				Core::warningLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Too many TCP-connection requests"));
//			}
//		} catch (std::exception& e) {
//			Core::errorLog.log(ExceptionLogMessage(SOURCE_LOCATION_ARGS, e, L"Running subsystem error. Stopping subsystem."));
//			break;
//		} catch (...) {
//			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Running subsystem unknown error. Stopping subsystem."));
//			break;
//		}
//	}
//}

} // namespace isl
