#ifndef ISL__MESSAGE_BUFFER__HXX
#define ISL__MESSAGE_BUFFER__HXX

#include <isl/AbstractMessageConsumer.hxx>
#include <list>
#include <deque>
#include <memory>

namespace isl
{

//! Thread-unsafe message buffer templated class
/*!
  This class should be used in the same thread only, cause it's not thread safe. Use it as target consumer to fetch all messages
  from the message queue - see MessageQueue::popAll() method.

  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class MessageBuffer : public AbstractMessageConsumer<Msg>
{
public:
	typedef Msg MessageType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;

	enum Constants {
		DefaultMaxSize = 1024			//!< Default message buffer maximum size
	};

	//! Constructor
	MessageBuffer() :
		AbstractMessageConsumerType(),
		_maxSize(DefaultMaxSize),
		_buffer()
	{}
	//! Constructor
	/*!
	  \param maxSize Queue maximum size
	*/
	MessageBuffer(size_t maxSize) :
		AbstractMessageConsumerType(),
		_maxSize(maxSize),
		_buffer()
	{}
	//! Destructor
	virtual ~MessageBuffer()
	{
		clear();
	}
	//! Returns buffer maximum size
	inline size_t maxSize() const
	{
		return _maxSize;
	}
	//! Pushes message to the buffer
	/*!
	  This method does not clone the message.
	  \param msgAutoPtr Reference to the auto-pointer to the message to push
	  \return True if the message has been accepted by the buffer and no buffer overlow has been detected
	*/
	bool push(std::auto_ptr<Msg>& msgAutoPtr)
	{
		if (!msgAutoPtr.get()) {
			return false;
		}
		if (_buffer.size() >= _maxSize) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Maximum size of buffer has been exceeded"));
			return false;
		}
		if (!isAccepting(*msgAutoPtr.get(), _buffer.size())) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by buffer's filter"));
			return false;
		}
		_buffer.push_front(msgAutoPtr.get());
		msgAutoPtr.release();
		return true;
	}
	//! Pops message from the buffer
	/*!
	  \param bufferSize Pointer to value where buffer size after message fetching should be saved or NULL pointer if not
	  \return Auto-pointer to the fetched message
	*/
	std::auto_ptr<Msg> pop(size_t * bufferSize = 0)
	{
		if (bufferSize) {
			*bufferSize = 0;
		}
		if (_buffer.empty()) {
			return std::auto_ptr<Msg>();
		}
		std::auto_ptr<Msg> msg(_buffer.back());
		_buffer.pop_back();
		if (bufferSize) {
			*bufferSize = _buffer.size();
		}
		return msg;
	}
	//! Fetches all messages to the supplied consumer
	/*!
	  NOTE: If the consumer's filter rejects a message it will be discarded!

	  \param consumer Message consumer to store messages to
	  \return Fetched messages amount
	*/
	size_t popAll(AbstractMessageConsumerType& consumer)
	{
		if (_buffer.empty()) {
			return false;
		}
		size_t providedMessages = 0;
		for (typename Messages::const_iterator i = _buffer.begin(); i != _buffer.end(); ++i) {
			if (consumer.push(**i)) {
				++providedMessages;
			} else {
				errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been discarded cause it has been rejected by the target consumer"));
			}
		}
		clear();
		return providedMessages;
	}
	//! Returns message buffer size
	inline size_t size() const
	{
		return _buffer.size();
	}
	//! Inspects if the message buffer is empty
	inline bool empty() const
	{
		return _buffer.empty();
	}
	//! Clears message buffer
	void clear()
	{
		for (typename Messages::iterator i = _buffer.begin(); i != _buffer.end(); ++i) {
			delete (*i);
		}
		_buffer.clear();
	}
	//! Pushes message to the buffer
	/*!
	  \param msg Message to push
	  \return True if the message has been accepted by the buffer and no buffer overlow has been detected
	*/
	virtual bool push(const Msg& msg)
	{
		if (_buffer.size() >= _maxSize) {
			errorLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Maximum size of buffer has been exceeded"));
			return false;
		}
		if (!isAccepting(msg, _buffer.size())) {
			debugLog().log(LogMessage(SOURCE_LOCATION_ARGS, "Message has been rejected by buffer's filter"));
			return false;
		}
		_buffer.push_front(msg.clone());
		return true;
	}
protected:
	//! Incoming message filter virtual method
	/*!
	  \param msg Constant reference to message to apply a filter on
	  \param bufferSize Current buffer size
	  \return True if the message is to be accepted
	*/
	virtual bool isAccepting(const Msg& msg, size_t bufferSize)
	{
		return true;
	}
private:
	MessageBuffer(const MessageBuffer&);			// No copy

	MessageBuffer& operator=(const MessageBuffer&);		// No copy

	typedef std::deque<Msg *> Messages;

	size_t _maxSize;
	Messages _buffer;
};


} // namespace isl

#endif
