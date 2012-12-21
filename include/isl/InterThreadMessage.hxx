#ifndef ISL__INTER_THREAD_MESSAGE
#define ISL__INTER_THREAD_MESSAGE

class AbstractInterThreadMessage
{
public:
	virtual ~AbstractInterThreadMessage()
	{}

	template <typename T> bool instanceOf() const
	{
		return dynamic_cast<const T *>(this);
	}

	template <typename T> T * cast() const
	{
		return dynamic_cast<T *>(this);
	}

	virtual const char * name() const = 0;

	virtual AbstractInterThreadMessage * clone() const = 0;
private:
};

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

#endif
