#ifndef ISL__ABSTRACT_MESSAGE_RECEIVER__HXX
#define ISL__ABSTRACT_MESSAGE_RECEIVER__HXX

namespace isl
{

//! Thread-safe message consumer templated class
/*!
  \tparam Msg Message class with <tt>Msg * Msg::clone() const</tt> method
*/
template <typename Msg> class AbstractMessageConsumer
{
public:
	typedef Msg MessageType;

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
