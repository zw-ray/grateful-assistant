---
description: The rule only for this project.
---

# Project Architecture

This is a C++ 17 application with:

- Header files in `/include`
- Source files in `/source`
- Application kernel headers in '/include/kernel'
- Application kernel source files in '/source/kernel'
- Application gui headers in '/include/gui'
- Application gui source files in '/source/gui'
- Package management using vcpkg
- Build system using CMake
- Test suite using Google Test
- Documentation using Doxygen
- CI/CD using Gitlab Actions

## Coding Standards

- Use C++17 for all new files
- Follow the existing naming conventions
- Ensure compile can pass after your changes