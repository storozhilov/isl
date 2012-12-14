#ifndef ISL__IO_ERROR__HXX
#define ISL__IO_ERROR__HXX

// TODO Remove this file from the repo!

//#include <isl/AbstractError.hxx>
//
//namespace isl
//{
//
////! I/O error
//class IOError : public AbstractError
//{
//public:
//	//! Error type
//	enum Type {
//		ConnectionAborted,			//!< Connection aborted
//		DeviceIsNotOpen				//!< Device is not open
//	};
//	//! Constructs I/O error
//	/*!
//	  You should use SOURCE_LOCATION_ARGS macro as a value for the first parameter
//
//	  \param type Error type
//	  \param info User info
//	*/
//	IOError(SOURCE_LOCATION_ARGS_DECLARATION, Type type, const std::string& info = std::string()) :
//		AbstractError(SOURCE_LOCATION_ARGS_PASSTHRU, info),
//		_type(type)
//	{}
//	//! Returns error type
//	inline Type type() const
//	{
//		return _type;
//	}
//	//! Clones error
//	virtual AbstractError * clone() const
//	{
//		return new IOError(*this);
//	}
//private:
//	IOError();
//
//	virtual std::string composeMessage() const
//	{
//		switch (_type) {
//			case ConnectionAborted:
//				return "Connection aborted on I/O-device";
//				break;
//			case DeviceIsNotOpen:
//				return "I/O-device is not open";
//				break;
//			default:
//				return "Unknown I/O-error";
//		}
//	}
//
//	Type _type;
//};
//
//} // namespace isl

#endif

