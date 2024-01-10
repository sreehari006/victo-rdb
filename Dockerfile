# Use an Alpine-based BusyBox variant as the base image
FROM alpine:latest

# Install BusyBox package manager
RUN apk --no-cache add busybox

# Install GCC and CMake using the package manager
RUN apk --no-cache add gcc g++ cmake git make openssl-dev openssl

# Set the working directory
WORKDIR /app
RUN mkdir -p /app/build
RUN mkdir -p /app/victodb

# Clone the Git repository
RUN git clone -b docker https://github.com/sreehari006/victo-rdb.git /app/source

RUN cmake -S /app/source -B /app/source/build
RUN make -C /app/source/build/

EXPOSE 2018

# Set the entry point for the container
ENTRYPOINT ["/app/source/build/victo-exe", "/app/victodb"]