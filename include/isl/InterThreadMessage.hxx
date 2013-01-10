#ifndef ISL__INTER_THREAD_MESSAGE
#define ISL__INTER_THREAD_MESSAGE

namespace isl
{

//! Abstract inter-thread message
class AbstractInterThreadMessage
{
public:
	virtual ~AbstractInterThreadMessage()
	{}
	//! Inspects inter-thread message to be instance of a class
	/*!
	  \tparam T Class to inspect for
	  \return TRUE if an inter-thread message is an instance of a class
	*/
	template <typename T> bool instanceOf() const
	{
		return dynamic_cast<const T *>(this);
	}
	//! Casts inter-thread message
	/*!
	  \tparam T Class to cast to
	  \return Pointer to casted inter-thread message or to zero if typecasting is not possible
	*/
	template <typename T> T * cast() const
	{
		return dynamic_cast<T *>(this);
	}
	//! Returns inter-thread message name
	virtual const char * name() const = 0;
	//! Clones inter-thread message
	/*!
	  \return Pointer to new cloned inter-thread message
	*/
	virtual AbstractInterThreadMessage * clone() const = 0;
private:
};

//! Termination request inter-thread message
class TerminateRequestMessage : public AbstractInterThreadMessage
{
public:
	virtual const char * name() const
	{
		static const char * n = "Termination Request";
		return n;
	}
	virtual AbstractInterThreadMessage * clone() const
	{
		return new TerminateRequestMessage(*this);
	}
};

//! OK response inter-thread message
class OkResponseMessage : public AbstractInterThreadMessage
{
public:
	virtual const char * name() const
	{
		static const char * n = "OK Response";
		return n;
	}
	virtual AbstractInterThreadMessage * clone() const
	{
		return new OkResponseMessage(*this);
	}
};

} // namespace isl

#endif
