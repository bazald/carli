#C++ Artificial Reinforcement Learning Intelligence (Carli)

##Welcome

This is my C++ reinforcement learning testbed. If you find it useful, great. Please let me know. I've made little effort to document it for others' understanding, but I'm now making it public.

~[bazald](mailto:bazald@gmail.com)

##Building

The simplest way to build Carli is using [premake4](http://industriousone.com/premake-quick-start).

    premake4 gmake
    make config=release

I use a fair number of C++11 features in Carli, so compilation in Visual Studio is not supported.
I recommend the use of [Code::Blocks](http://www.codeblocks.org/), which uses the high quality [TDM-GCC](http://tdm-gcc.tdragon.net/) compilation suite.
To build the Code::Blocks project, simply run:
    premake4 codeblocks
I've tested the project on Windows and Linux.

##License

Copyright (c) 2013, Mitchell Keith Bloch  
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
