if [ ! -d build ]; then
    sudo apt update
    sudo apt install -y \
        libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
        libfreetype-dev libflac-dev libvorbis-dev \
        libgl1-mesa-dev libegl1-mesa-dev libharfbuzz-dev
    mkdir build
fi
cd build
cmake ..
cmake --build . --target run
