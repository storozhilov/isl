#ifndef ISL__ABSTRACT_MESSAGE_CONSUMER__HXX
#define ISL__ABSTRACT_MESSAGE_CONSUMER__HXX

namespace isl
{

//! Message cloner, which calls copying constructor for cloning
template <typename Msg> class CopyMessageCloner
{
public:
	//! Clones the message
	/*!
	  \param msg Constant reference to the message to clone
	  \return Pointer to the copy of the message
	*/
	static Msg * clone(const Msg& msg)
	{
		return new Msg(msg);
	}
};

//! Message cloner, which calls <tt>Msg * Msg::clone() const</tt> method for cloning
template <typename Msg> class CloneMessageCloner
{
public:
	//! Clones the message
	/*!
	  \param msg Constant reference to the message to clone
	  \return Pointer to the copy of the message
	*/
	static Msg * clone(const Msg& msg)
	{
		return msg.clone();
	}
};

//! Thread-safe message consumer templated class
/*!
  \tparam Msg Message class
*/
template <typename Msg> class AbstractMessageConsumer
{
public:
	typedef Msg MessageType;				//!< Message type

	//! Constructor
	AbstractMessageConsumer()
	{}
	//! Destructor
	virtual ~AbstractMessageConsumer()
	{}
	//! Pushes a message to the consumer
	/*!
	  \param msg Constant reference to a message to push
	  \return True id the message has been accepted by the consumer
	*/
	virtual bool push(const Msg& msg) = 0;
private:
	AbstractMessageConsumer(const AbstractMessageConsumer&);			// No copy

	AbstractMessageConsumer& operator=(const AbstractMessageConsumer&);		// No copy
};

} // namespace isl

#endif
