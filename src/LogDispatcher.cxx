#include <isl/LogDispatcher.hxx>
#include <isl/AbstractLogTarget.hxx>
#include <isl/AbstractLogDevice.hxx>
#include <isl/Log.hxx>
#include <isl/AbstractLogMessage.hxx>
#include <isl/Exception.hxx>
#include <isl/Error.hxx>
#include <algorithm>

#ifdef ISL_LOG_DEBUGGING
#include <iostream>
#endif

namespace isl
{

/*------------------------------------------------------------------------------
 * LogDispatcher
 *------------------------------------------------------------------------------*/

LogDispatcher::LogDispatcher() :
	_devices(),
	_connections(),
	_connectionsRWLock()
{}

LogDispatcher::~LogDispatcher()
{
	for (Devices::iterator i = _devices.begin(); i != _devices.end(); ++i) {
		delete (*i);
	}
}

void LogDispatcher::connectLogToDevice(Log *log, const AbstractLogTarget *target)
{
	WriteLocker locker(_connectionsRWLock);
	Devices::iterator devicePos;
	for (devicePos = _devices.begin(); devicePos != _devices.end(); ++devicePos) {
		if ((*devicePos)->serving(target)) {
			break;
		}
	}
	if (devicePos == _devices.end()) {
		std::pair<Devices::iterator, bool> pos = _devices.insert(target->createDevice());
		devicePos = pos.first;
#ifdef ISL_LOG_DEBUGGING
		std::clog << "LogDispatcher::connectLogToDevice(): New log device has been created" << std::endl;
#endif
	} else {
		std::pair<Connections::iterator, Connections::iterator> connectedDevices = _connections.equal_range(log);
		for (Connections::iterator i = connectedDevices.first; i != connectedDevices.second; ++i) {
			if ((*i).second == devicePos) {
				throw Exception(Error(SOURCE_LOCATION_ARGS, "Log is already connected to the log device"));
			}
		}
	}
	_connections.insert(Connections::value_type(log, devicePos));
#ifdef ISL_LOG_DEBUGGING
	std::clog << "LogDispatcher::connectLogToDevice(): Log has been connected to the log device" << std::endl;
#endif
}

void LogDispatcher::disconnectLogFromDevice(Log *log, const AbstractLogTarget *target)
{
	WriteLocker locker(_connectionsRWLock);
	Devices::iterator devicePos;
	for (devicePos = _devices.begin(); devicePos != _devices.end(); ++devicePos) {
		if ((*devicePos)->serving(target)) {
			break;
		}
	}
	if (devicePos == _devices.end()) {
		throw Exception(Error(SOURCE_LOCATION_ARGS, "Log device is not found"));
	}
	std::pair<Connections::iterator, Connections::iterator> connectedDevices = _connections.equal_range(log);
	for (Connections::iterator i = connectedDevices.first; i != connectedDevices.second; ++i) {
		if ((*i).second == devicePos) {
			_connections.erase(i);
#ifdef ISL_LOG_DEBUGGING
			std::clog << "LogDispatcher::disconnectLogFromDevice(): Log has been disconnected from the log device" << std::endl;
#endif
			sweepDevices();
			return;
		}
	}
	throw Exception(Error(SOURCE_LOCATION_ARGS, "Log is already disconnected from the log device"));
}

void LogDispatcher::disconnectLogFromDevices(Log *log)
{
	WriteLocker locker(_connectionsRWLock);
	std::pair<Connections::iterator, Connections::iterator> connectedDevices = _connections.equal_range(log);
	_connections.erase(connectedDevices.first, connectedDevices.second);
#ifdef ISL_LOG_DEBUGGING
	if (connectedDevices.first != _connections.end()) {
		std::clog << "LogDispatcher::disconnectLogFromDevices(): Log has been disconnected from the log devices" << std::endl;
	} else {
		std::clog << "LogDispatcher::disconnectLogFromDevices(): Log is already disconnected from the log devices" << std::endl;
	}
#endif
	sweepDevices();
}

void LogDispatcher::logMessage(Log * log, const AbstractLogMessage& msg)
{
	ReadLocker locker(_connectionsRWLock);
	std::pair<Connections::iterator, Connections::iterator> connectedDevices = _connections.equal_range(log);
	for (Connections::iterator i = connectedDevices.first; i != connectedDevices.second; ++i) {
		(*(*i).second)->logMessage(*log, msg);
	}
}

void LogDispatcher::sweepDevices()
{
	Devices usedDevices;
	for (Connections::iterator i = _connections.begin(); i != _connections.end(); ++i) {
		usedDevices.insert(*(*i).second);
	}
	if (usedDevices.size() == _devices.size()) {
		return;
	}
	Devices::iterator devPos = _devices.begin();
	while (devPos != _devices.end()) {
		if (usedDevices.find(*devPos) == usedDevices.end()) {
			delete (*devPos);
			_devices.erase(*(devPos++));
#ifdef ISL_LOG_DEBUGGING
			std::clog << "Log device destroyed" << std::endl;
#endif
		} else {
			++devPos;
		}
	}
}

} // namespace isl

