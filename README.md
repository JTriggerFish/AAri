# AAri Project

This project is built using the Meson build system and depends on Python 3.11, SDL2, and pybind11.

## Prerequisites

- Python 3.11
- Meson (will be installed in the Python virtual environment created by the setup scripts)
- A compiler (MSVC on Windows, GCC on Linux)

## Setup

### Windows

1. Ensure you have Python 3.11 installed and available in your system's PATH.
2. Open a command prompt in the project's root directory.
3. Run the setup script:

   ```
   setup.bat
   ```

### Linux

1. Ensure you have Python 3.11 installed and available in your system's PATH.
2. Open a terminal in the project's root directory.
3. Give execution permission to the setup script:

   ```
   chmod +x setup.sh
   ```

4. Run the setup script:

   ```
   ./setup.sh
   ```

## Building

After running the setup script, you can build the project using Meson:

```
meson compile -C debug
meson compile -C release
```

## Testing

To run the C++ tests, use the following command:

```
meson test -C debug
```

