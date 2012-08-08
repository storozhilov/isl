#ifndef ISL__MESSAGE_QUEUE__HXX
#define ISL__MESSAGE_QUEUE__HXX

#include <isl/WaitCondition.hxx>
#include <isl/AbstractMessageConsumer.hxx>
#include <list>
#include <deque>
#include <memory>

namespace isl
{

//! Thread-safe message queue templated class
/*!
  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class MessageQueue : public AbstractMessageConsumer<Msg>
{
public:
	typedef Msg MessageType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;

	enum Constants {
		DefaultMaxSize = 1024			//!< Default message queue maximum size
	};

	//! Constructor
	MessageQueue() :
		AbstractMessageConsumerType(),
		_maxSize(DefaultMaxSize),
		_queue(),
		_queueCond()
	{}
	//! Constructor
	/*!
	  \param maxSize Queue maximum size
	*/
	MessageQueue(size_t maxSize) :
		AbstractMessageConsumerType(),
		_maxSize(maxSize),
		_queue(),
		_queueCond()
	{}
	//! Destructor
	virtual ~MessageQueue()
	{
		resetQueue();
	}
	//! Returns queue maximum size
	inline size_t maxSize() const
	{
		return _maxSize;
	}
	//! Pushes message to the queue
	/*!
	  This method does not clone the message.
	  \param msgAutoPtr Reference to the auto-pointer to the message to push
	  \return True if the message has been accepted by the queue and no queue overlow has been detected
	*/
	bool push(std::auto_ptr<Msg>& msgAutoPtr)
	{
		if (!msgAutoPtr.get()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Attempting to push a zero pointer to message into the message queue"));
			return false;
		}
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Maximum size of queue has been exceeded"));
			return false;
		}
		if (!isAccepting(*msgAutoPtr.get(), _queue.size())) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by queue's filter"));
			return false;
		}
		_queue.push_front(msgAutoPtr.get());
		msgAutoPtr.release();
		_queueCond.wakeOne();
		return true;
	}
	//! Pops message from the queue
	/*!
	  \param timeout Timeout to wait for the message
	  \param queueSize Pointer to value where queue size after message fetching should be saved or NULL pointer if not
	  \return Auto-pointer to the fetched message
	*/
	std::auto_ptr<Msg> pop(const Timeout& timeout = Timeout::defaultTimeout(), size_t * queueSize = 0)
	{
		if (queueSize) {
			*queueSize = 0;
		}
		MutexLocker locker(_queueCond.mutex());
		if (!_queue.empty()) {
			std::auto_ptr<Msg> msg(_queue.back());
			_queue.pop_back();
			if (queueSize) {
				*queueSize = _queue.size();
			}
			return msg;
		}
		if (timeout.isZero()) {
			return std::auto_ptr<Msg>();
		}
		_queueCond.wait(timeout);
		if (_queue.empty()) {
			return std::auto_ptr<Msg>();
		}
		std::auto_ptr<Msg> msg(_queue.back());
		_queue.pop_back();
		if (queueSize) {
			*queueSize = _queue.size();
		}
		return msg;
	}
	//! Fetches all messages into the supplied consumer
	/*!
	  NOTE: If the consumer's filter rejects a message it will be discarded!

	  \param consumer Message consumer to store messages to
	  \param timeout Timeout to wait for the messages
	  \return Fetched messages amount
	*/
	size_t popAll(AbstractMessageConsumerType& consumer, const Timeout& timeout = Timeout::defaultTimeout())
	{
		MutexLocker locker(_queueCond.mutex());
		if (!_queue.empty()) {
			size_t providedMessages = 0;
			for (typename Messages::const_iterator i = _queue.begin(); i != _queue.end(); ++i) {
				if (consumer.push(**i)) {
					++providedMessages;
				} else {
					errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been discarded cause it has been rejected by the target consumer"));
				}
			}
			resetQueue();
			return providedMessages;
		}
		if (timeout.isZero()) {
			return 0;
		}
		_queueCond.wait(timeout);
		if (_queue.empty()) {
			return 0;
		}
		size_t providedMessages = 0;
		for (typename Messages::const_iterator i = _queue.begin(); i != _queue.end(); ++i) {
			if (consumer.push(**i)) {
				++providedMessages;
			} else {
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been discarded cause it has been rejected by the target consumer"));
			}
		}
		resetQueue();
		return providedMessages;
	}
	//! Awaits for messages
	/*!
	  \param timeout Timeout to wait for the messages
	  \param queueSize Pointer to value where queue size should be saved or NULL pointer if not
	  \return True ia messages appeared in the queue
	*/
	bool await(const Timeout& timeout = Timeout::defaultTimeout(), size_t * queueSize = 0)
	{
		if (queueSize) {
			*queueSize = 0;
		}
		MutexLocker locker(_queueCond.mutex());
		if (!_queue.empty()) {
			if (queueSize) {
				*queueSize = _queue.size();
			}
			return true;
		}
		if (timeout.isZero()) {
			return false;
		}
		_queueCond.wait(timeout);
		return !_queue.empty();
	}
	//! Clears message queue
	void clear()
	{
		MutexLocker locker(_queueCond.mutex());
		resetQueue();
	}
	//! Wakes up one awaiting message recipient
	void wakeRecipient()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeOne();
	}
	//! Wakes up all awaiting messages recipient
	void wakeRecipients()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeAll();
	}

	//! Pushes message to the queue
	/*!
	  \param msg Message to push
	  \return True if the message has been accepted by the queue and no queue overlow has been detected
	*/
	virtual bool push(const Msg& msg)
	{
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Maximum size of queue has been exceeded"));
			return false;
		}
		if (!isAccepting(msg, _queue.size())) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by queue's filter"));
			return false;
		}
		std::auto_ptr<Msg> clonedMsgAutoPtr(msg.clone());
		if (!clonedMsgAutoPtr.get()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message cloning method returns null pointer"));
			return false;
		}
		_queue.push_front(clonedMsgAutoPtr.get());
		clonedMsgAutoPtr.release();
		_queueCond.wakeOne();
		return true;
	}
protected:
	//! Incoming message filter virtual method
	/*!
	  \param msg Constant reference to message to apply a filter on
	  \param queueSize Current queue size
	  \return True if the message is to be accepted
	*/
	virtual bool isAccepting(const Msg& msg, size_t queueSize)
	{
		return true;
	}
private:
	MessageQueue(const MessageQueue&);			// No copy

	MessageQueue& operator=(const MessageQueue&);		// No copy

	void resetQueue()
	{
		for (typename Messages::iterator i = _queue.begin(); i != _queue.end(); ++i) {
			delete (*i);
		}
		_queue.clear();
	}

	typedef std::deque<Msg *> Messages;

	size_t _maxSize;
	Messages _queue;
	WaitCondition _queueCond;
};

} // namespace isl

#endif
