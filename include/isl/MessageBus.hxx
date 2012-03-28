#ifndef ISL__MESSAGE_BUS__HXX
#define ISL__MESSAGE_BUS__HXX

#include <isl/Core.hxx>
#include <isl/ReadWriteLock.hxx>
#include <isl/MessageQueue.hxx>
#include <isl/Error.hxx>
#include <set>

namespace isl
{

template <typename Msg> class MessageBus
{
public:
	class Subscriber
	{
	public:
		Subscriber(MessageBus<Msg>& bus, MessageQueue<Msg>& queue) :
			_bus(bus),
			_queue(queue)
		{
			_bus.subscribe(_queue);
		}
		~Subscriber()
		{
			_bus.unsubscribe(_queue);
		}
	private:
		Subscriber();
		Subscriber(const Subscriber&);				// No copy

		Subscriber& operator=(const Subscriber&);		// No copy

		MessageBus<Msg>& _bus;
		MessageQueue<Msg>& _queue;
	};

	enum Constants {
		DefaultMaxSubscriptionsAmount = 1024
	};
	
	MessageBus(size_t maxSubscriptionsAmount = DefaultMaxSubscriptionsAmount) :
		_maxSubscriptionsAmount(maxSubscriptionsAmount),
		_subscriptions(),
		_subscriptionsRwLock()
	{}

	void subscribe(MessageQueue<Msg>& queue)
	{
		WriteLocker locker(_subscriptionsRwLock);
		if (_subscriptions.find(&queue) != _subscriptions.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been already subscribed to message bus"));
			return;
		}
		if (_subscriptions.size() >= _maxSubscriptionsAmount) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum subscriptions amount has been exceeded"));
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Maximum subscriptions amount has been exceeded"));
		}
		_subscriptions.insert(&queue);
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been subscribed to the message bus"));
	}

	void unsubscribe(MessageQueue<Msg>& queue)
	{
		WriteLocker locker(_subscriptionsRwLock);
		typename Subscriptions::iterator pos = _subscriptions.find(&queue);
		if (pos == _subscriptions.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue have not been subscribed to message bus"));
			return;
		}
		_subscriptions.erase(pos);
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been unsubscribed from the message bus"));
	}

	size_t push(const Msg& msg)
	{
		size_t recipientsCount = 0;
		ReadLocker locker(_subscriptionsRwLock);
		for (typename Subscriptions::iterator i = _subscriptions.begin(); i != _subscriptions.end(); ++i) {
			if ((*i)->push(msg)) {
				++recipientsCount;
			}
		}
		return recipientsCount;
	}
private:
	MessageBus(const MessageBus&);					// No copy

	MessageBus& operator=(const MessageBus&);			// No copy

	typedef std::set<MessageQueue<Msg> *> Subscriptions;

	size_t _maxSubscriptionsAmount;
	Subscriptions _subscriptions;
	ReadWriteLock _subscriptionsRwLock;
};

template <typename Msg> class ClonableMessageBus
{
public:
	class Subscriber
	{
	public:
		Subscriber(ClonableMessageBus<Msg>& bus, ClonableMessageQueue<Msg>& queue) :
			_bus(bus),
			_queue(queue)
		{
			_bus.subscribe(_queue);
		}
		~Subscriber()
		{
			_bus.unsubscribe(_queue);
		}
	private:
		Subscriber();
		Subscriber(const Subscriber&);				// No copy

		Subscriber& operator=(const Subscriber&);		// No copy

		ClonableMessageBus<Msg>& _bus;
		ClonableMessageQueue<Msg>& _queue;
	};

	enum Constants {
		DefaultMaxSubscriptionsAmount = 1024
	};
	
	ClonableMessageBus(size_t maxSubscriptionsAmount = DefaultMaxSubscriptionsAmount) :
		_maxSubscriptionsAmount(maxSubscriptionsAmount),
		_subscriptions(),
		_subscriptionsRwLock()
	{}

	void subscribe(ClonableMessageQueue<Msg>& queue)
	{
		WriteLocker locker(_subscriptionsRwLock);
		if (_subscriptions.find(&queue) != _subscriptions.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been already subscribed to message bus"));
			return;
		}
		if (_subscriptions.size() >= _maxSubscriptionsAmount) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum subscriptions amount has been exceeded"));
			throw Exception(Error(SOURCE_LOCATION_ARGS, L"Maximum subscriptions amount has been exceeded"));
		}
		_subscriptions.insert(&queue);
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been subscribed to the message bus"));
	}

	void unsubscribe(ClonableMessageQueue<Msg>& queue)
	{
		WriteLocker locker(_subscriptionsRwLock);
		typename Subscriptions::iterator pos = _subscriptions.find(&queue);
		if (pos == _subscriptions.end()) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue have not been subscribed to message bus"));
			return;
		}
		_subscriptions.erase(pos);
		Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Queue has been unsubscribed from the message bus"));
	}

	size_t push(const Msg& msg)
	{
		size_t recipientsCount = 0;
		ReadLocker locker(_subscriptionsRwLock);
		for (typename Subscriptions::iterator i = _subscriptions.begin(); i != _subscriptions.end(); ++i) {
			if ((*i)->push(msg)) {
				++recipientsCount;
			}
		}
		return recipientsCount;
	}
private:
	ClonableMessageBus(const ClonableMessageBus&);					// No copy

	ClonableMessageBus& operator=(const ClonableMessageBus&);			// No copy

	typedef std::set<ClonableMessageQueue<Msg> *> Subscriptions;

	size_t _maxSubscriptionsAmount;
	Subscriptions _subscriptions;
	ReadWriteLock _subscriptionsRwLock;
};

} // namespace isl

#endif
