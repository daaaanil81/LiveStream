# WebRTC OpenCV video streaming

## Install Dependencies

### Install NodeJS
```
curl -fsSL https://deb.nodesource.com/setup_19.x | sudo -E bash - &&\
sudo apt-get install -y nodejs
```
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
