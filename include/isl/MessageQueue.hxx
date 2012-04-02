#ifndef ISL__MESSAGE_QUEUE__HXX
#define ISL__MESSAGE_QUEUE__HXX

#include <isl/Core.hxx>
#include <isl/WaitCondition.hxx>
#include <deque>
#include <memory>

namespace isl
{

template <typename Msg> class CopyMessageCloner
{
public:
	static Msg * clone(const Msg& msg)
	{
		return new Msg(msg);
	}
};

template <typename Msg> class CloneMessageCloner
{
public:
	static Msg * clone(const Msg& msg)
	{
		return msg.clone();
	}
};

template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class MessageQueue
{
public:
	enum Constants {
		DefaultMaxSize = 1024
	};

	typedef std::list<Msg> MessageList;

	MessageQueue() :
		_maxSize(DefaultMaxSize),
		_queue(),
		_queueCond()
	{}
	MessageQueue(size_t maxSize) :
		_maxSize(maxSize),
		_queue(),
		_queueCond()
	{}
	virtual ~MessageQueue()
	{
		resetQueue();
	}

	inline size_t maxSize() const
	{
		return _maxSize;
	}
	bool push(const Msg& msg)
	{
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum size of queue has been exceeded"));
			return false;
		}
		if (!isAccepting(msg, _queue.size())) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by queue's filter"));
			return false;
		}
		_queue.push_front(Cloner::clone(msg));
		_queueCond.wakeOne();
		return true;
	}

	bool push(Msg * msg)
	{
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum size of queue has been exceeded"));
			return false;
		}
		if (!isAccepting(msg, _queue.size())) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by queue's filter"));
			return false;
		}
		_queue.push_front(msg);
		_queueCond.wakeOne();
		return true;
	}

	size_t push(MessageList& msgs, bool allAcceptedOrNothing = false)
	{
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum size of queue has been exceeded"));
			return 0;
		}
		std::list<typename MessageList::iterator> acceptingMsgs;
		for (typename MessageList::iterator i = msgs.begin(); i != msgs.end(); ++i) {
			if (isAccepting(**i, _queue.size() + acceptingMsgs.size())) {
				acceptingMsgs.push_back(i);
			}
		}
		if (acceptingMsgs.empty()) {
			return 0;
		}
		if (allAcceptedOrNothing && (_queue.size() + acceptingMsgs.size() >= _maxSize)) {
			return 0;
		}
		size_t acceptedAmount = 0;
		for (typename std::list<typename MessageList::iterator>::iterator i = acceptingMsgs.begin(); i != acceptingMsgs.end(); ++i) {
			if (_queue.size() >= _maxSize) {
				break;
			}
			_queue.push_front(**i);
			msgs.erase(*i);
			++acceptedAmount;
		}
		_queueCond.wakeAll();
		return acceptedAmount;
	}

	std::auto_ptr<Msg> pop(Timeout timeout = Timeout::defaultTimeout(), size_t * queueSize = 0)
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

	void clear()
	{
		MutexLocker locker(_queueCond.mutex());
		resetQueue();
	}

	void wakeRecipient()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeOne();
	}
	void wakeRecipients()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeAll();
	}
protected:
	virtual bool isAccepting(const Msg& /*msg*/, size_t /*queueSize*/)
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
