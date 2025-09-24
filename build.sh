sudo apt install -y libxrandr-dev libxcursor-dev libxi-dev libudev-dev
                                        libfreetype-dev libflac-dev libvorbis-dev
                                        libgl1-mesa-dev libegl1-mesa-dev libharfbuzz-dev
mkdir build
cd build
cmake ..
make
