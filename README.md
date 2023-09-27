# AAri

AAri is a (very) work in progress realtime sample by sample audio graph system written in C++ and Python.
It is intended for live coding and interactive audio applications.\
The intention is that C++ provides a low level backend that runs continuously in a callback loop
and Python provides a high level frontend for the user to interact with the graph.

## Development Progress

The project is undergoing a transition to use the EnTT library for the graph backend and currently whatever little is
there is broken

## Dependencies

- CMake (version 3.14 or higher)
- A C++ compiler supporting C++20
- Python (version 3.11)
- pybind11 (version 2.11)
- Catch2 (version 3.0.0-preview3)
- EnTT (version 3.12.2)
- miniaudio (header-only library included in the project) (version 0.11.18)

## Building the Project

### Prerequisites

- Install CMake (version 3.14 or higher)
- Install Python (version 3.11) including development headers
- Ensure that a C++ compiler supporting C++20 is available
- ALSA development libraries on linux ( for not yet implemented midi support using python rtmidi )

### Build Instructions

1. Clone the repository:
   ```sh
   git clone https://github.com/username/AAri.git
   cd AAri
   ```

2. Create a build directory and navigate to it:
   ```sh
   mkdir build
   cd build
   ```

3. Configure the project using CMake:
   ```sh
   cmake ..
   ```

4. Build the project:
   ```sh
   cmake --build . --config Release
   ```

5. (Optional) Run the tests:
   ```sh
   ctest --schedule-random
   ```

## Usage

## License

MIT License

## Contributing

Too early for now, but feel free to open an issue if you have any questions or suggestions.
