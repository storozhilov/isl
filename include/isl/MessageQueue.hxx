#ifndef ISL__MESSAGE_QUEUE__HXX
#define ISL__MESSAGE_QUEUE__HXX

#include <isl/common.hxx>
#include <isl/LogMessage.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/AbstractMessageConsumer.hxx>
#include <list>
#include <deque>
#include <memory>

namespace isl
{

//! Thread-safe message queue templated class
/*!
  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class MessageQueue : public AbstractMessageConsumer<Msg>
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
	  \param consumer Message consumer to store messages to
	  \param timeout Timeout to wait for the messages
	  \return Fetched messages amount
	  \note If the consumer's filter rejects a message it will be discarded!
	*/
	size_t popAll(AbstractMessageConsumerType& consumer, const Timeout& timeout = Timeout::defaultTimeout())
	{
		MutexLocker locker(_queueCond.mutex());
		if (!_queue.empty()) {
			size_t providedMessages = 0;
			for (typename Messages::const_reverse_iterator i = _queue.rbegin(); i != _queue.rend(); ++i) {
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
		for (typename Messages::const_reverse_iterator i = _queue.rbegin(); i != _queue.rend(); ++i) {
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
	  \return True if messages appeared in the queue
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
		if (!isAccepting(msg, _queue.size())) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by queue's filter"));
			return false;
		}
		if (_queue.size() >= _maxSize) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Maximum size of queue has been exceeded"));
			return false;
		}
		//std::auto_ptr<Msg> clonedMsgAutoPtr(msg.clone());
		std::auto_ptr<Msg> clonedMsgAutoPtr(Cloner::clone(msg));
		if (!clonedMsgAutoPtr.get()) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message cloner returns null pointer"));
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
