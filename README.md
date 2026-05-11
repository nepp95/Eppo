## Install

**Linux:**

*Install Prerequisites (if needed):*
- vcpkg
    - `git clone https://github.com/microsoft/vcpkg.git`
    - `cd vcpkg && ./bootstrap-vcpkg.sh`
    - Set `VCPKG_ROOT` environment variable to root path and add this to your path
- packages
    - `sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config`

*Instructions:*
- `cmake --preset=debug`