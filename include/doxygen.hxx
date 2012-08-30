#ifndef ISL__DOXYGEN__HXX
#define ISL__DOXYGEN__HXX 1

//------------------------------------------------------------------------------
// ISL doxygen data header file. Not for any usage.
//
// Copyright (c) 2011-2012, Ilya V. Storozhilov. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------

namespace isl
{

/*! \mainpage ISL - Internet Server Library, C++ server development toolkit

  \section intro_section Introduction

  Every server application should run daemonized 24x7 in memory leak free multithread environment, have it's own UNIX signals
  handler, thread-safe logging subsystem, thread-safe message queues/buses for inter-thread exchange, etc. In general terms
  the architecture of any server application is usually composed from common design elements which are based on
  particular <a href="http://en.wikipedia.org/wiki/Software_design_pattern">design patterns</a>. Server application developers
  facing the similar challenges where design solutions along with versatile tools must be provided by easy to use toolkit. This
  toolkit is aimed to be an ISL project.

  \section features_section Features

  - C++ wrappers for basic inter-thread synchronization objects (thread, mutex, R/W-lock, conditional variable, etc.) and helper classes;
  - Tools for common server application tasks (pidfile saving, daemonizing, logging, etc.);
  - Thread-safe multi-target extensible logging architecture implementation with any kind of targets (stdout, file, syslog, database, etc.)
    and multiple target per log support;
  - <a href="http://en.wikipedia.org/wiki/Active_object">Active object pattern</a> templated extensible implementation;
  - Hierarchially organized extensible server and it's subsystems abstractions design in accordance with
    <a href="http://en.wikipedia.org/wiki/Composite_pattern">Composite design pattern</a>;
  - Buffered I/O-device abstraction and it's implementation for TCP/UDP(TODO) sockets with asynchronous data transmission and SSL (TODO) support;
  - Extensible UNIX-signal handler subsystem implementation;
  - Extensible design for synchronous (one thread per client connection) and asynchronous (two threads per client connection) TCP-service
    subsystems implementation;
  - Extensible message queueing functionality implementation, including thread-safe message queue, thread-safe message bus/fan, asynchronous
    message broker connection subsystem, asynchronous message brocker service subsystem, message routing facilities, etc.;
  - Nanosecond-precision datetime and interval support;
  - Functionally rich HTTP-module with HTTP-message/HTTP-cookie parsers and composers, HTTP-request/HTTP-response stream readers and writers,
    utility methods, etc.

  \section installation_section Installation

  Runtime requirements:

  - libc;
  - libpthread;
  - STL.

  Build requirements:

  - <a href="http://scons.org/doc/production/HTML/scons-user/c95.html">SCons</a>.

  To get ISL's source code do SVN-checkout from the http://svn.storozhilov.com/isl repository:

  \verbatim
  $ svn co http://svn.storozhilov.com/isl
  \endverbatim
  
  To build and install ISL type:

  \verbatim
  $ cd isl/trunk
  $ scons & sudo scons install
  \endverbatim

  To uninstall type:

  \verbatim
  $ scons uninstall
  \endverbatim

  To list build options type:

  \verbatim
  $ scons -h
  \endverbatim

  \section usage_section Usage

  TODO
  
  \section example_section Example
  
  TODO

  \code
// TODO Put an example code here
  \endcode

  \section license_section License

  This software is distributed under &quot;Simplified <a href="http://en.wikipedia.org/wiki/BSD_licenses">BSD-license</a>&quot;
  (A.K.A. &quot;FreeBSD License&quot;) terms. It means you can use it in any application/library you want including commercial one
  with minimum restrictions.
 
  \verbatim
  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
  - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  \endverbatim

  Copyright (c) 2011-2012, <a href="http://storozhilov.com/">Ilya V. Storozhilov</a>. All rights reserved.

*/

} // namespace smxx
