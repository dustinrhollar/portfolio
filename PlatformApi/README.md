# Platform API

## About

Platform API seeks to create a lightweight library for cross-platform development. The API uses a unity build system so that a user does not have to consistently check the platform the code is running on. All platform code is routed through a series of functions prefixed with `Platform*`. The full API can be found in `Platform.h`. Source code is compiled within `Platform.cpp`, which is the only source file that needs to be directly added to a project's build system.

## Supported Features

### Windows

- Window API
- Keyboard Input
- Logging
- High performance timer
- File Management (API and hotreloading)
- Thread Pool
- Fiber Scheduler (in progress)

### Linux
- Window API
- Keyboard Input
- Logging
- File API
- OpenGL loader
