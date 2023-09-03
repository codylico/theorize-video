# PNG Parts: parts of a Portable Network Graphics implementation

The PNG Parts repository aims to provide a working implementation of
the Portable Network Graphics image format as specified in RFC 2083.
The parts of the implementation, to be provided in both C and C++,
should be more or less usable independently of each other.

## Goals

The repository currently contains source code in C for the following:

* DEFLATE compression implementation (both read and write)
  according to [RFC 1951](https://www.ietf.org/rfc/rfc1951.txt)

* ZLIB compressed data format implementation (both read and write)
  according to [RFC 1950](https://www.ietf.org/rfc/rfc1950.txt)

* PNG image format basic implementation (both read and write)
  according to [RFC 2083](https://www.ietf.org/rfc/rfc2083.txt)

Future goals of this project include:

* A C++ port of this library

* support for the tRNS chunk for transparency (both read and write)

* support for additional user-defined chunks (both read and write)

## Build and Installation

This project uses the CMake build system, for easier cross-platform
development. CMake can be found at the following URL:

[https://cmake.org/download/](https://cmake.org/download/)

To use CMake with this project, first make a diretory to hold the build results.
Then run CMake in the directory with a path to the source code.
On UNIX, the commands would look like the following:
```
mkdir build
cd build
cmake ../png-parts/src
```

Running CMake should create a build project, which then can be processed using other 
tools. Usually, the tool would be Makefile or a IDE project.
For Makefiles, use the following command to build the project:
```
make
```
For IDE projects, the IDE must be installed and ready to use. Open the project
within the IDE.

## License

The source code within this project is provided under the MIT license.
