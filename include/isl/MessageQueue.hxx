#ifndef ISL__MESSAGE_QUEUE__HXX
#define ISL__MESSAGE_QUEUE__HXX

#include <isl/Core.hxx>
#include <isl/WaitCondition.hxx>
#include <deque>
#include <memory>

namespace isl
{

template <typename Msg> class MessageQueue
{
public:
	enum Constants {
		DefaultMaxSize = 1024
	};

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
	{}

	bool push(const Msg& msg)
	{
		if (!isAccepting(msg)) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by queue's filter"));
			return false;
		}
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			// TODO Maybe to throw an exception?
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum size of queue has been exceeded"));
			return false;
		}
		_queue.push_front(msg);
		_queueCond.wakeAll();
		return true;
	}

	std::auto_ptr<Msg> pop(Timeout timeout = Timeout::defaultTimeout(), size_t * queueSize = 0)
	{
		if (queueSize) {
			*queueSize = 0;
		}
		MutexLocker locker(_queueCond.mutex());
		if (!_queue.empty()) {
			std::auto_ptr<Msg> msg(new Msg(_queue.back()));
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
		std::auto_ptr<Msg> msg(new Msg(_queue.back()));
		_queue.pop_back();
		if (queueSize) {
			*queueSize = _queue.size();
		}
		return msg;
	}

	size_t clear()
	{
		MutexLocker locker(_queueCond.mutex());
		size_t queueSize = _queue.size();
		_queue.clear();
		return queueSize;
	}

	void wakeRecipients()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeAll();
	}
protected:
	virtual bool isAccepting(const Msg& /*msg*/)
	{
		return true;
	}
private:
	MessageQueue(const MessageQueue&);			// No copy

	MessageQueue& operator=(const MessageQueue&);		// No copy

	typedef std::deque<Msg> Messages;

	size_t _maxSize;
	Messages _queue;
	WaitCondition _queueCond;
};

template <typename Msg> class ClonableMessageQueue
{
public:
	enum Constants {
		DefaultMaxSize = 1024
	};

	ClonableMessageQueue() :
		_maxSize(DefaultMaxSize),
		_queue(),
		_queueCond()
	{}
	ClonableMessageQueue(size_t maxSize) :
		_maxSize(maxSize),
		_queue(),
		_queueCond()
	{}
	virtual ~ClonableMessageQueue()
	{
		resetQueue();
	}

	bool push(Msg& msg)
	{
		if (!isAccepting(msg)) {
			Core::debugLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Message has been rejected by queue's filter"));
			return false;
		}
		MutexLocker locker(_queueCond.mutex());
		if (_queue.size() >= _maxSize) {
			// TODO Maybe to throw an exception?
			Core::errorLog.log(DebugLogMessage(SOURCE_LOCATION_ARGS, L"Maximum size of message queue has been exceeded"));
			return false;
		}
		_queue.push_front(msg.clone());
		_queueCond.wakeAll();
		return true;
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

	size_t clear()
	{
		MutexLocker locker(_queueCond.mutex());
		size_t queueSize = _queue.size();
		resetQueue();
		return queueSize;
	}

	void wakeRecipients()
	{
		MutexLocker locker(_queueCond.mutex());
		_queueCond.wakeAll();
	}
protected:
	virtual bool isAccepting(Msg& /*msg*/)
	{
		return true;
	}
private:
	ClonableMessageQueue(const ClonableMessageQueue&);			// No copy

	ClonableMessageQueue& operator=(const ClonableMessageQueue&);		// No copy

	typedef std::deque<Msg *> Messages;

	void resetQueue()
	{
		for (typename Messages::iterator i = _queue.begin(); i != _queue.end(); ++i) {
			delete (*i);
		}
	}

	size_t _maxSize;
	Messages _queue;
	WaitCondition _queueCond;
};

} // namespace isl

#endif
