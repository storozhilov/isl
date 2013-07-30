#ifndef ISL__SCADA_PROGRAM__HXX
#define ISL__SCADA_PROGRAM__HXX

#include <isl/ScadaTimer.hxx>

namespace isl
{

//! Abstract SCADA program which tasks are to be executed by SCADA timer
class ScadaProgram
{
public:
	//! Constructs a SCADA program
	/*!
	  \param scadaTimer SCADA timer to register a program in
	*/
	ScadaProgram(ScadaTimer& scadaTimer);
	virtual ~ScadaProgram();

	//! Returns a reference to the SCADA timer
	inline ScadaTimer& scadaTimer() const
	{
		return _scadaTimer;
	}

	//! Sends request to the SCADA program
	/*!
	  \param request Request message to send
	  \param limit Limit to await for the response
	  \return Auto-pointer to the response or to 0 if no response has been provided
	  \note Do not call it from the SCADA timer thread -> call onRequest() directly instead
	*/
	std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> sendRequest(const Subsystem::ThreadRequesterType::MessageType& request,
			const Timestamp& awaitResponseLimit);
protected:
	virtual std::auto_ptr<Subsystem::ThreadRequesterType::MessageType> onRequest(const Subsystem::ThreadRequesterType::MessageType& request, bool responseRequired)
	{
		return std::auto_ptr<Subsystem::ThreadRequesterType::MessageType>();
	}
private:
	ScadaTimer& _scadaTimer;

	friend class ScadaTimer;
};

} // namespace isl

#endif
