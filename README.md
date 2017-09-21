# Bitprim Core <a target="_blank" href="https://gitter.im/bitprim/Lobby">![Gitter Chat][badge.Gitter]</a>

*Core Bitcoin functionality*

| **master(linux/osx)** | **dev(linux/osx)**   | **master(windows)**   | **dev(windows)** |
|:------:|:-:|:-:|:-:|
| [![Build Status](https://travis-ci.org/bitprim/bitprim-core.svg)](https://travis-ci.org/bitprim/bitprim-core)       | [![Build StatusB](https://travis-ci.org/bitprim/bitprim-core.svg?branch=dev)](https://travis-ci.org/bitprim/bitprim-core?branch=dev)  | [![Appveyor Status](https://ci.appveyor.com/api/projects/status/github/bitprim/bitprim-core?svg=true)](https://ci.appveyor.com/project/bitprim/bitprim-core)  | [![Appveyor StatusB](https://ci.appveyor.com/api/projects/status/github/bitprim/bitprim-core?branch=dev&svg=true)](https://ci.appveyor.com/project/bitprim/bitprim-core?branch=dev)  |

Table of Contents
=================

   * [Bitprim Core](#bitprim-core)
      * [Installation](#installation)
        * [Using Conan](#using-conan-recommended)
        * [Build from source](#build-from-source)
            * [Debian/Ubuntu](#debianubuntu)
            * [MacOS](#macintosh)
                * [Homebrew](#using-homebrew)
                * [Macports](#using-macports)
            * [Windows with Visual Studio](#windows-with-visual-studio)
        

## Installation

### Using Conan (recommended)

Conan is a Python package for dependency management; it only requires Python and Pip.
With Conan, install can be performed on any OS. If there are no prebuilt binaries for a given
OS-compiler-arch combination, Conan will build from source.

```
pip install conan
conan remote add bitprim https://api.bintray.com/conan/bitprim/bitprim
conan install bitprim-core/0.1@bitprim/stable
```

The last step will install binaries and headers in Conan's cache, a directory outside the usual
system paths. This will avoid conflict with system packages such as boost.
Also, notice it references the stable version 0.1. To see which versions are available,
please check [Bintray](https://bintray.com/bitprim/bitprim/bitprim-core%3Abitprim).

### Build from source

A build requires boost and libsecp256k1. The [bitprim/secp256k1](https://github.com/bitprim/secp256k1) repository is forked from [bitcoin-core/secp256k1](https://github.com/bitcoin-core/secp256k1) in order to control for changes and to incorporate the necessary Visual Studio build. The original repository can be used directly but recent changes to the public interface may cause build breaks. The `--enable-module-recovery` switch is required.

Detailed instructions are provided below.
  * [Debian/Ubuntu](#debianubuntu)
  * [Macintosh](#macintosh)
  * [Windows with Visual Studio](#windows-with-visual-studio)

#### Debian/Ubuntu

Bitprim requires a C++11 compiler, currently minimum [GCC 4.8.0](https://gcc.gnu.org/projects/cxx0x.html) or Clang based on [LLVM 3.5](http://llvm.org/releases/3.5.0/docs/ReleaseNotes.html).

To check your GCC version:
```sh
$ g++ --version
```
```
g++ (Ubuntu 4.8.2-19ubuntu1) 4.8.2
Copyright (C) 2013 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
If necessary, upgrade your compiler as follows:
```sh
$ sudo apt-get install g++-4.8
$ sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
$ sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-4.8 50
```
Next, install the [build system](http://wikipedia.org/wiki/GNU_build_system) (Automake minimum 1.14) and git:
```sh
$ sudo apt-get install build-essential autoconf automake libtool pkg-config git
```
Next, install the [Boost](http://www.boost.org) (minimum 1.56.0) development package:
```sh
$ sudo apt-get install libboost-all-dev
```
Next, install the [CMake](https://cmake.org/) (minimum 3.7.0-rc1) development package:
```sh
$ wget https://cmake.org/files/v3.7/cmake-3.7.0-rc1.tar.gz
$ tar -xvzf cmake-3.7.0-rc1.tar.gz
$ cd cmake-3.7.0-rc1
$ ./bootstrap
$ make
$ sudo make install
$ sudo ln -s /usr/local/bin/cmake /usr/bin/cmake
```

Next, install [bitprim/secp256k1](https://github.com/bitprim/secp256k1)

Finally, install bitprim-core:
```sh
$ git clone https://github.com/bitprim/bitprim-core
$ cd bitprim-core
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles" -DWITH_TESTS=OFF -DWITH_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++11"
$ make
$ sudo make install
$ sudo ldconfig
```
Bitprim-core is now installed in `/usr/local/`.

#### Macintosh

The OSX installation differs from Linux in the installation of the compiler and packaged dependencies. Bitprim supports both [Homebrew](http://brew.sh) and [MacPorts](https://www.macports.org) package managers. Both require Apple's [Xcode](https://developer.apple.com/xcode) command line tools. Neither requires Xcode as the tools may be installed independently.

Bitprim compiles with Clang on OSX and requires C++11 support. Installation has been verified using Clang based on [LLVM 3.5](http://llvm.org/releases/3.5.0/docs/ReleaseNotes.html). This version or newer should be installed as part of the Xcode command line tools.

To check your Clang/LLVM  version:
```sh
$ clang++ --version
```
```
Apple LLVM version 6.0 (clang-600.0.54) (based on LLVM 3.5svn)
Target: x86_64-apple-darwin14.0.0
Thread model: posix
```
If required, update your version of the command line tools as follows:
```sh
$ xcode-select --install
```

##### Using Homebrew

First, install Homebrew. Installation requires [Ruby](https://www.ruby-lang.org/en) and [cURL](http://curl.haxx.se), which are pre-installed on OSX.
```sh
$ ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```
You may encounter a prompt to install the Xcode command line developer tools, in which case accept the prompt.

Next install the [build system](http://wikipedia.org/wiki/GNU_build_system) (Automake minimum 1.14) and [wget](http://www.gnu.org/software/wget):
```sh
$ brew install autoconf automake libtool pkgconfig wget
```
Next install the [Boost](http://www.boost.org) (1.56.0 or newer) development package:
```sh
$ brew install boost
```

Next, install [bitprim/secp256k1](https://github.com/bitprim/secp256k1)

Finally, install bitprim-core:
```sh
$ git clone https://github.com/bitprim/bitprim-core
$ cd bitprim-core
$ mkdir build
$ cd build
$ cmake .. -DWITH_TESTS=OFF -DWITH_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++11"
$ make
$ sudo make install
```
Bitprim-core is now installed in `/usr/local/`.

##### Using MacPorts

First, install [MacPorts](https://www.macports.org/install.php).

Next, install the [build system](http://wikipedia.org/wiki/GNU_build_system) (Automake minimum 1.14) and [wget](http://www.gnu.org/software/wget):
```sh
$ sudo port install autoconf automake libtool pkgconfig wget
```
Next, install the [Boost](http://www.boost.org) (1.56.0 or newer) development package. The `-` options remove MacPort defaults that are not Boost defaults:
```sh
$ sudo port install boost -no_single -no_static -python27
```

Next, install [bitprim/secp256k1](https://github.com/bitprim/secp256k1)

Finally, install bitprim-core:
```sh
$ git clone https://github.com/bitprim/bitprim-core
$ cd bitprim-core
$ mkdir build
$ cd build
$ cmake .. -DWITH_TESTS=OFF -DWITH_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++11"
$ make
$ sudo make install
```
Bitprim-core is now installed in `/usr/local/`.

#### Windows with Visual Studio

This project, unlike secp256k1, has external dependencies such as boost.
The easiest way to build them is to use Conan from the CMake script,
which will install boost and other libraries in non-system directories.
Asides from the Conan flags, the BOOST_INCLUDE_DIR variable is required.
A sample value is shown below, but you will need to check your system in `C:\.conan`
after running `conan install ..` to see where the boost headers get installed.

From a [Visual Studio Developer Command Prompt](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs):

```
$ pip install conan
$ git clone https://github.com/bitprim/bitprim-core.git
$ cd bitprim-core
$ mkdir build
$ cd build
$ conan install ..
$ cmake .. -DENABLE_MODULE_RECOVERY=ON -DUSE_CONAN=ON -DNO_CONAN_AT_ALL=OFF -DBOOST_INCLUDE_DIR=C:\.conan\qtu9qvxl\1\include
$ msbuild ALL_BUILD.vcxproj
```

[badge.Gitter]: https://img.shields.io/badge/gitter-join%20chat-blue.svg

