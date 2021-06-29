# Common

## About

Common is a collection of utilities that I have written that tends to be used across all projects that I work on.

### Core

The Core contains two helpers:
- `Core`: A set of common types and macros used to streamline development.
- `SysMemory`: Interface for application memory. Internally, a free list allocator is used to manage memory.

### Scripts

Scripts contain two useful scripts:
- `setup_cl.bat`: Searches for the version of visual studio installed on host system and sets up the `cl` suite of command line tools.
- `machine.sh`: Determines OS of the host machine. Useful for defining command line tools for Mac and Linux.

### Util

A collection of header only files for common use data structures. 
- `HashFunctions`: Thin wrapper around MummurHash (64 and 128 bit). MummurHash is a hashing algorithm focused on speed and reduced collision.
- `MapleMath`: Custom veector math library supporting basic vector, matrix, and quaternion types.
- `Memory`: Free List Allocator that focuses on speed and a reduced memory overhead.
- `String`: Immutable string library that focuses on reduced memory overhead.
- `StrPool`: An immutable string library that stores strings within a memory arena and returns an unique identifier rather than the string.
