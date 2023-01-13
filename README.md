# WebRTC OpenCV video streaming

## Install Dependencies

### Install NodeJS
```
curl -fsSL https://deb.nodesource.com/setup_19.x | sudo -E bash - &&\
sudo apt-get install -y nodejs
```

### Install Cuda

[Cuda download](https://developer.nvidia.com/cuda-downloads)
[Cudnn install guide](https://docs.nvidia.com/deeplearning/cudnn/install-guide/index.html)
[Cuda installation guide](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)

### Build and Install OpenCV
```
sudo apt-get install -y cmake gcc g++ python3 python3-dev python3-numpy libavcodec-dev \
                        libavformat-dev libswscale-dev libgstreamer-plugins-base1.0-dev \
                        libgstreamer1.0-dev libgtk-3-dev libpng-dev libjpeg-dev \
                        libopenexr-dev libtiff-dev libwebp-dev
```

1. `git clone https://github.com/opencv/opencv.git`
2. `git clone https://github.com/opencv/opencv_contrib.git`
3. Check Nvidia video device: `nvidia-smi -L`
4. Check Comput Capability for your device: https://developer.nvidia.com/cuda-gpus
5. `cd opencv && mkdir build && cd build` 
6. Cmake command:
```
cmake -D CMAKE_BUILD_TYPE=RELEASE \
       -D CMAKE_INSTALL_PREFIX=/usr/local \
       -D WITH_CUDA=ON \                                                                                      
       -D WITH_CUDNN=ON \
       -D OPENCV_DNN_CUDA=ON \
       -D ENABLE_FAST_MATH=1 \
       -D CUDA_FAST_MATH=1 \
       -D CUDA_ARCH_BIN=<Comput Capability version> \
       -D WITH_CUBLAS=1 \
       -D OPENCV_EXTRA_MODULES_PATH=<path to opencv_contrib>/opencv_contrib/modules \
       -D HAVE_opencv_python3=ON .. 
```
7. Check after cmake the similar:
```
--   NVIDIA CUDA:                   YES (ver 11.6, CUFFT CUBLAS)
--     NVIDIA GPU arch:             61
--     NVIDIA PTX archs:
--
--   cuDNN:                         YES (ver 8.3.2)
```

8. `make -j12 && sudo make install && sudo ldconfig`

### Build and Install Ffmpeg

[Ffmpeg Instruction](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu)


## Start application

### Client part:
```
cd http_server && \
npm install . && \
node server.js localhost
```

### Daemon part:
```
mkdir build && cd build && cmake .. && make && \
./daemon
```
