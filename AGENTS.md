# Project: Aircraft Tracking System

Mock surface-to-air 1:many ATS in C++20. Simulates aircraft sending GPS coordinates to an ATS server via custom TCP packets over loopback. Demonstrates Transport Canada / FAA compliant ATS design with MISRA C++:2008 adherence. Non-concurrent communication.

## Commands

- Configure: `cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build cmake-build-debug`
- Test: `cd cmake-build-debug && ctest --output-on-failure`
- Run tests directly: `./cmake-build-debug/ats_tests`
- Run server: `./cmake-build-debug/ats_server`
- Run client: `./cmake-build-debug/ats_client`
- PVS-Studio MISRA check: `cmake -B cmake-build-pvs -G Ninja -DENABLE_PVS_STUDIO=ON && cmake --build cmake-build-pvs --target pvs_analysis`

## Structure

- `src/client/` — aircraft simulator, sends GPS data to ATS
- `src/server/` — ATS server, receives and processes GPS data
- `src/common/` — shared packet definitions and logging
- `tests/` — Googletest unit tests and fixtures
- `cmake/` — CMake configuration files

## Conventions

- C++20. Must build on both Windows and POSIX.
- Networking code uses `#ifdef` for platform-specific headers and socket APIs.
- Google clang-format and clang-tidy style. All code must pass both before merge.
- Tests use Googletest.
- clang-tidy runs automatically during build via CMake integration. Do not run it manually.

## Coding Rules (MISRA C++:2008)

- No dynamic memory allocation (`new`, `delete`, `malloc`, `free`). Use fixed-size buffers and stack allocation.
- If dynamic allocation is unavoidable, use smart pointers (`unique_ptr`, `shared_ptr`).
- No global variables. Pass data via function parameters or class members.
- Single return statement per function.
- Minimize complexity. Document complex code with rationale comments.

## Do Not Touch

- `cmake-build-*` directories — CMake generated, do not modify or run linting/formatting on these.
- `.PVS-Studio/` directory — analyzer output, do not parse or interpret.