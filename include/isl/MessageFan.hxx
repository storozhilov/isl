#ifndef ISL__MESSAGE_FAN__HXX
#define ISL__MESSAGE_FAN__HXX

#include <isl/MessageProvider.hxx>

namespace isl
{

//! Thread-safe message fanout templated class
/*!
  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class MessageFan : public MessageProvider<Msg>, public AbstractMessageConsumer<Msg>
{
public:
	typedef Msg MessageType;
	typedef MessageProvider<Msg> MessageProviderType;
	typedef AbstractMessageConsumer<Msg> AbstractMessageConsumerType;

	//! Constructor
	MessageFan() :
		MessageProviderType(),
		AbstractMessageConsumerType()
	{}
	//! Constructor
	/*!
	  \param maxConsumersAmount Maximum consumers amount
	*/
	MessageFan(size_t maxConsumersAmount) :
		MessageProviderType(maxConsumersAmount),
		AbstractMessageConsumerType()
	{}
	//! Drops a message to fun
	/*!
	  \param msg Constant reference to a message to drop
	  \return True if message has been accepted by the fan's filter
	*/
	virtual bool push(const Msg& msg)
	{
		if (!isAccepting(msg)) {
			return false;
		}
		provideToOne(msg);
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
