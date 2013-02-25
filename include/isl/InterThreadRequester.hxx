#ifndef ISL__INTER_THREAD_REQUESTER__HXX
#define ISL__INTER_THREAD_REQUESTER__HXX

#include <isl/AbstractMessageConsumer.hxx>
#include <isl/WaitCondition.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <isl/Log.hxx>
#include <isl/LogMessage.hxx>
#include <isl/InterThreadMessage.hxx>
#include <list>
#include <deque>
#include <map>
#include <memory>

#ifndef ISL__INTER_THREAD_REQUESTER_MAX_CONTAINER_SIZE
#define ISL__INTER_THREAD_REQUESTER_MAX_CONTAINER_SIZE 16
#endif

namespace isl
{

//! Inter-thread communication facility
/*!
  Implements inter-thread request-response mechanism for the price of mutex and condition variable.

  \tparam Msg Message class
  \tparam Cloner Message cloner class with static <tt>Msg * Cloner::clone(const Msg& msg)</tt> method for cloning the message
*/
template <typename Msg, typename Cloner = CopyMessageCloner<Msg> > class BasicInterThreadRequester
{
public:
	typedef Msg MessageType;							//!< Message type
	typedef Cloner ClonerType;							//!< Message cloner type
	//typedef size_t RequestIdType;
private:
	enum PrivateConstants {
		MaxContainerSize = ISL__INTER_THREAD_REQUESTER_MAX_CONTAINER_SIZE
	};

	struct RequestsQueueItem
	{
	public:
		RequestsQueueItem(size_t requestId, const MessageType& request, bool responseRequired) :
			requestId(requestId),
			requestPtr(ClonerType::clone(request)),
			responseRequired(responseRequired)
		{}

		const size_t requestId;
		const MessageType * requestPtr;
		const bool responseRequired;
	};
	typedef std::deque<RequestsQueueItem> RequestsQueue;
	typedef std::map<size_t, MessageType *> ResponsesMap;
public:
	//! Pending request class
	class PendingRequest
	{
	public:
		//! Returns request ID
		size_t id() const
		{
			return _id;
		}
		//! Returns a constant reference to the request object
		const MessageType& request() const
		{
			return *_requestAutoPtr.get();
		}
		//! Returns TRUE if the response is required for the request
		bool responseRequired() const
		{
			return _responseRequired;
		}
		//! Returns TRUE if the response has been sent to the requesting thread
		bool responseSent() const
		{
			return _responseSent;
		}
	private:
		PendingRequest();
		PendingRequest(const RequestsQueueItem& request) :
			_id(request.requestId),
			_requestAutoPtr(request.requestPtr),
			_responseRequired(request.responseRequired),
			_responseSent(false)
		{}

		PendingRequest(const PendingRequest&);

		PendingRequest& operator=(const PendingRequest&);

		const size_t _id;
		const std::auto_ptr<const MessageType> _requestAutoPtr;
		const bool _responseRequired;
		bool _responseSent;

		friend class BasicInterThreadRequester<Msg, Cloner>;
	};
	//! Constructor
	BasicInterThreadRequester() :
		_maxContainerSize(MaxContainerSize),
		_cond(),
		_lastRequestId(0),
		_requestsQueue(),
		_responsesMap(),
		_pendingRequestAutoPtr()
	{}
	//! Constructor
	BasicInterThreadRequester(size_t maxContainerSize) :
		_maxContainerSize(maxContainerSize),
		_cond(),
		_lastRequestId(0),
		_requestsQueue(),
		_responsesMap(),
		_pendingRequestAutoPtr()
	{}
	//! Destructor
	~BasicInterThreadRequester()
	{
		reset();
	}

	//! Sends a message to the respondent thread (no response)
	/*!
	  \param request Message to send
	  \return TRUE if the request has been accepted or FALSE if the requests container has been overflowed
	  \note Call it from requesting thread only!
	*/
	bool sendMessage(const MessageType& msg)
	{
		MutexLocker locker(_cond.mutex());
		if (_requestsQueue.size() >= _maxContainerSize) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Requests container overflow has been detected"));
			return false;
		}
		size_t newRequestId = ++_lastRequestId;
		if (newRequestId <= 0) {
			newRequestId = 1;
		}
		_requestsQueue.push_front(RequestsQueueItem(newRequestId, msg, false));
		_cond.wakeAll();
		return true;
	}
	//! Sends request message to the respondent thread (response required)
	/*!
	  \param request Request message to send
	  \return Request ID if the request has been accepted or 0 if the requests container has been overflowed
	  \note Call it from requesting thread only!
	*/
	size_t sendRequest(const MessageType& request)
	{
		MutexLocker locker(_cond.mutex());
		if (_requestsQueue.size() >= _maxContainerSize) {
			return 0;
		}
		size_t newRequestId = ++_lastRequestId;
		if (newRequestId <= 0) {
			newRequestId = 1;
		}
		_requestsQueue.push_front(RequestsQueueItem(newRequestId, request, true));
		_cond.wakeAll();
		return newRequestId;
	}
	//! Fetches response from the respondent thread
	/*!
	  \param requestId ID of the request to fetch a response to
	  \return Auto-pointer to the response or to zero if no response available
	  \note Call it from requesting thread only!
	*/
	std::auto_ptr<MessageType> fetchResponse(size_t requestId)
	{
		MutexLocker locker(_cond.mutex());
		if (_responsesMap.find(requestId) == _responsesMap.end()) {
			return std::auto_ptr<MessageType>();
		}
		std::auto_ptr<MessageType> responseAutoPtr(_responsesMap[requestId].second);
		_responsesMap.erase(requestId);
		return responseAutoPtr;
	}
	//! Awaits for the response from the respondent thread and returns it if available
	/*!
	  \param requestId ID of the request to await/fetch a response to
	  \param limit Time limit to await for the response
	  \return Auto-pointer to the response or to zero if no response available

	  \note Call it from requesting thread only!
	*/
	std::auto_ptr<MessageType> awaitResponse(size_t requestId, const Timestamp& limit)
	{
		MutexLocker locker(_cond.mutex());
		do {
			if (_responsesMap.find(requestId) == _responsesMap.end()) {
				continue;
			}
			std::auto_ptr<MessageType> responseAutoPtr(_responsesMap[requestId]);
			_responsesMap.erase(requestId);
			return responseAutoPtr;
		} while (_cond.wait(limit));
		return std::auto_ptr<MessageType>();
	}
	//! Awaits for the response from the respondent thread and returns it
	/*!
	  \param requestId ID of the request to await/fetch a response to
	  \return Auto-pointer to the response
	  \note Call it from requesting thread only!
	*/
	std::auto_ptr<MessageType> awaitResponse(size_t requestId)
	{
		MutexLocker locker(_cond.mutex());
		while (true) {
			if (_responsesMap.find(requestId) != _responsesMap.end()) {
				std::auto_ptr<MessageType> responseAutoPtr(_responsesMap[requestId]);
				_responsesMap.erase(requestId);
				return responseAutoPtr;
			}
			_cond.wait();
		}
	}
	//! Returns a pointer to the current pending request w/o fetching the new one
	/*!
	  \return Pointer to the current pending request or zero if no pending request at the moment
	  \note Call it from respondent thread only!
	*/
	const PendingRequest * pendingRequest() const
	{
		_pendingRequestAutoPtr.get();
	}
	//! Fetches next pending request from requesting thread
	/*!
	  \return Pointer to the next pending request or zero if no pending request at the moment
	  \note Call it from respondent thread only!
	*/
	const PendingRequest * fetchRequest()
	{
		if (_pendingRequestAutoPtr.get()) {
			if (_pendingRequestAutoPtr->responseRequired() && !_pendingRequestAutoPtr->responseSent()) {
				std::ostringstream msg;
				msg << "Unanswered inter-thread request (id = " << _pendingRequestAutoPtr->id() << ") has been discarded";
				Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			_pendingRequestAutoPtr.reset();
		}
		MutexLocker locker(_cond.mutex());
		if (_requestsQueue.empty()) {
			return 0;
		}
		_pendingRequestAutoPtr.reset(new PendingRequest(_requestsQueue.back()));
		_requestsQueue.pop_back();
		return _pendingRequestAutoPtr.get();
	}
	//! Awaits for the next pending request from the requesting thread and returns a pointer to it if available
	/*!
	  \param limit Time limit to await
	  \return Pointer to the next pending request or zero if no pending request available
	  \note Call it from respondent thread only!
	*/
	const PendingRequest * awaitRequest(const Timestamp& limit)
	{
		if (_pendingRequestAutoPtr.get()) {
			if (_pendingRequestAutoPtr->responseRequired() && !_pendingRequestAutoPtr->responseSent()) {
				std::ostringstream msg;
				msg << "Unanswered inter-thread request (id = " << _pendingRequestAutoPtr->id() << ") has been discarded";
				Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			}
			_pendingRequestAutoPtr.reset();
		}
		MutexLocker locker(_cond.mutex());
		do {
			if (_requestsQueue.empty()) {
				continue;
			}
			_pendingRequestAutoPtr.reset(new PendingRequest(_requestsQueue.back()));
			_requestsQueue.pop_back();
			return _pendingRequestAutoPtr.get();
		} while (_cond.wait(limit));
		return 0;
	}
	//! Sends a response to the requesting thread
	/*!
	  \param response Constant reference to the response to send
	  \return TRUE if the response has been accepted or FALSE if the responses container has been overflowed
	  \note Call it from respondent thread only!
	*/
	bool sendResponse(const MessageType& response)
	{
		if (!_pendingRequestAutoPtr.get()) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "No pending request to response to"));
			return false;
		}
		size_t pendingRequestId = _pendingRequestAutoPtr->id();
		if (!_pendingRequestAutoPtr->responseRequired()) {
			std::ostringstream msg;
			msg << "Requesting thread does not suppose a response for the pending request (id = " << pendingRequestId << "\")";
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			return false;
		}
		if (_responsesMap.find(pendingRequestId) != _responsesMap.end()) {
			std::ostringstream msg;
			msg << "Response has been already sent for the pending request (id = " << pendingRequestId << "\")";
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, msg.str()));
			return false;
		}
		MutexLocker locker(_cond.mutex());
		if (_responsesMap.size() >= _maxContainerSize) {
			Log::error().log(LogMessage(SOURCE_LOCATION_ARGS, "Responses container overflow has been detected"));
			return false;
		}
		_pendingRequestAutoPtr->_responseSent = true;
		_responsesMap.insert(typename ResponsesMap::value_type(pendingRequestId, ClonerType::clone(response)));
		_cond.wakeAll();
		return true;
	}
	//! Clears internal containers to reset requester to it's initial state
	/*!
	  \note Thread-unsafe
	*/
	void reset()
	{
		for (typename RequestsQueue::iterator i = _requestsQueue.begin(); i != _requestsQueue.end(); ++i) {
			delete i->requestPtr;
		}
		_requestsQueue.clear();
		for (typename ResponsesMap::iterator i = _responsesMap.begin(); i != _responsesMap.end(); ++i) {
			delete i->second;
		}
		_responsesMap.clear();
		_pendingRequestAutoPtr.reset();
	}
private:

	size_t _maxContainerSize;
	WaitCondition _cond;
	size_t _lastRequestId;
	RequestsQueue _requestsQueue;
	ResponsesMap _responsesMap;
	std::auto_ptr<PendingRequest> _pendingRequestAutoPtr;
};

//! Inter-thread reaquester for use in ISL
typedef BasicInterThreadRequester<AbstractInterThreadMessage, CloneMessageCloner<AbstractInterThreadMessage> > InterThreadRequester;

} // namespace isl

#endif
