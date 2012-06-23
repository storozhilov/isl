#ifndef ISL__MESSAGE_PROVIDER__HXX
#define ISL__MESSAGE_PROVIDER__HXX

#include <isl/common.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/LogMessage.hxx>
#include <isl/AbstractMessageConsumer.hxx>
#include <isl/ReadWriteLock.hxx>
#include <set>

namespace isl
{

//! Thread-safe message provider templated class
/*!
  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class MessageProvider
{
public:
	typedef Msg MessageType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;

	enum Constants {
		DefaultMaxConsumersAmount = 1024			// Max subscribed consumers amount
	};

	//! Subscribes message consumer to the message provider in constructor and unsubscribes in destructor
	class Subscriber
	{
	public:
		//! Constructor
		/*!
		  \param provider Reference to message provider
		  \param consumer Reference to message consumer
		*/
		Subscriber(MessageProvider& provider, AbstractMessageConsumerType& consumer) :
			_provider(provider),
			_consumer(consumer)
		{
			_provider.subscribe(_consumer);
		}
		//! Destructor
		~Subscriber()
		{
			_provider.unsubscribe(_consumer);
		}
	private:
		Subscriber();
		Subscriber(const Subscriber&);				// No copy

		Subscriber& operator=(const Subscriber&);			// No copy

		MessageProvider& _provider;
		AbstractMessageConsumerType& _consumer;
	};
	//! Releases subscribers in destructor (TODO Remove it)
	class SubscriberListReleaser
	{
	public:
		SubscriberListReleaser() :
			_subscribers()
		{}
		~SubscriberListReleaser()
		{
			for (typename SubcriberList::iterator i = _subscribers.begin(); i != _subscribers.end(); ++i) {
				delete (*i);
			}
		}

		void addSubscriber(Subscriber * subscriber)
		{
			_subscribers.push_back(subscriber);
		}
	private:
		typedef std::list<Subscriber *> SubcriberList;
		
		SubcriberList _subscribers;
	};

	//! Constructor
	MessageProvider() :
		_maxConsumersAmount(DefaultMaxConsumersAmount),
		_consumers(),
		_subscribersRwLock()
	{}
	//! Constructor
	/*!
	  \param maxConsumersAmount Maximum subscribed message consumers amount
	*/
	MessageProvider(size_t maxConsumersAmount) :
		_maxConsumersAmount(maxConsumersAmount),
		_consumers(),
		_subscribersRwLock()
	{}
	virtual ~MessageProvider()
	{}
	//! Subscribes consumer to the provider
	void subscribe(AbstractMessageConsumerType& consumer)
	{
		WriteLocker locker(_subscribersRwLock);
		if (_consumers.find(&consumer) != _consumers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer has been already subscribed to message provider"));
			return;
		}
		if (_consumers.size() >= _maxConsumersAmount) {
			Error err(SOURCE_LOCATION_ARGS, "Maximum subscriptions amount has been exceeded");
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, err.message()));
			throw Exception(err);
		}
		_consumers.insert(&consumer);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer has been subscribed to the message provider"));
	}
	//! Unsubscribes consumer from the provider
	void unsubscribe(AbstractMessageConsumerType& consumer)
	{
		WriteLocker locker(_subscribersRwLock);
		typename ConsumersContainer::iterator pos = _consumers.find(&consumer);
		if (pos == _consumers.end()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer have not been subscribed to message provider"));
			return;
		}
		_consumers.erase(pos);
		debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message consumer has been unsubscribed from the message provider"));
	}
protected:
	//! Provide message to all subscribed consumers
	/*!
	  \param msg Constant reference to a message to provide
	*/
	void provideToAll(const Msg& msg)
	{
		ReadLocker locker(_subscribersRwLock);
		for (typename ConsumersContainer::iterator i = _consumers.begin(); i != _consumers.end(); ++i) {
			(*i)->push(msg);
		}
	}
	//! Provide message to one subscribed consumers which accepted a message
	/*!
	  \param msg Constant reference to a message to provide
	*/
	void provideToOne(const Msg& msg)
	{
		ReadLocker locker(_subscribersRwLock);
		for (typename ConsumersContainer::iterator i = _consumers.begin(); i != _consumers.end(); ++i) {
			if ((*i)->push(msg)) {
				break;
			}
		}
	}
private:
	typedef std::set<AbstractMessageConsumerType *> ConsumersContainer;

	size_t _maxConsumersAmount;
	ConsumersContainer _consumers;
	ReadWriteLock _subscribersRwLock;
};


} // namespace isl

#endif
