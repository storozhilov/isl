#ifndef ISL__MESSAGE_BUS__HXX
#define ISL__MESSAGE_BUS__HXX

#include <isl/MessageProvider.hxx>

namespace isl
{

//! Thread-safe message bus templated class
/*!
  \tparam Msg Message class
*/
template <typename Msg> class MessageBus : public MessageProvider<Msg>, public AbstractMessageConsumer<Msg>
{
public:
	typedef Msg MessageType;
	typedef MessageProvider<Msg> MessageProviderType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;

	//! Constructor
	MessageBus() :
		MessageProviderType(),
		AbstractMessageConsumerType()
	{}
	//! Constructor
	/*!
	  \param maxConsumersAmount Maximum consumers amount
	*/
	MessageBus(size_t maxConsumersAmount) :
		MessageProviderType(maxConsumersAmount),
		AbstractMessageConsumerType()
	{}
	//! Pushes a message to bus
	/*!
	  \param msg Constant reference to a message to push
	  \return True if message has been accepted by the bus's filter
	*/
	virtual bool push(const Msg& msg)
	{
		if (!isAccepting(msg)) {
			return false;
		}
		provideToAll(msg);
		return true;
	}
protected:
	//! Messages filter method
	/*!
	  \param msg Constant reference to a message to filter
	  \return True id the message has been accepted by the filter
	*/
	virtual bool isAccepting(const Msg& msg)
	{
		return true;
	}
};

} // namespace isl

#endif
