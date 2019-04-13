1.install glfw3

1.1.ubuntu16.04~ubuntu18.04
sudo apt-get install libglfw3-dev

1.2.ubuntu12.04
sudo apt-get install libX11-dev
sudo apt-get install libXrandr-dev
sudo apt-get install libXinerama-dev
sudo apt-get install libXcursor-dev
sudo apt-get install libgl1-mesa-dev

download and install cmake-2.8.12
https://cmake.org/files/v2.8/cmake-2.8.12.tar.gz
./configure
make
sudo make install

download and install glfw3
https://codeload.github.com/glfw/glfw/zip/3.2.1
cmake .
make
make install


1.3.ubuntu14.04
sudo apt-get install cmake
sudo apt-get install libX11-dev
sudo apt-get install libXrandr-dev
sudo apt-get install libXinerama-dev
sudo apt-get install libXcursor-dev
sudo apt-get install libgl1-mesa-dev
sudo apt-get install mesa-utils

download and install glfw3
https://codeload.github.com/glfw/glfw/zip/3.2.1
cmake .
make
make install

1.4.centos6.5~centos7
yum install cmake
yum install libX11-devel
yum install libXrandr-devel
yum install libXinerama-devel
yum install libXcursor-devel
yum install mesa-libGL-devel
yum install mesa-utils
download and install glfw3
https://codeload.github.com/glfw/glfw/zip/3.2.1
cmake .
make
make install


2.bulid HDRCaptureRecord
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH (/usr/local/lib/pkgconfig is the dir of glfw3.pc)
make


3.may bulid fail,so you need bulid ffmpeg on you computer by:
sh bulidffmpeg.sh