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

  TODO

  \section installation_section Installation

  Runtime requirements:

  TODO

  Build requirements:

  TODO

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

  \section features_section Features

  TODO

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
