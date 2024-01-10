# Use an Alpine-based BusyBox variant as the base image
FROM alpine:latest

# Install BusyBox package manager
RUN apk --no-cache add busybox

# Install GCC and CMake using the package manager
RUN apk --no-cache add gcc g++ cmake git make openssl-dev

# Set the working directory
WORKDIR /app

# Clone the Git repository
RUN git clone https://github.com/sreehari006/victo-rdb.git .

cmake -S . -B ./build
make -C ./build/

# Set the entry point for the container
ENTRYPOINT ["./build/victo-exe", "/app/victodb"]