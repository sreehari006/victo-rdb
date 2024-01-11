# Use an Alpine-based BusyBox variant as the base image
FROM alpine:latest

# Install BusyBox package manager
RUN apk --no-cache add busybox

# Install GCC and CMake using the package manager
RUN apk --no-cache add gcc g++ cmake git make openssl-dev openssl

# Set the working directory
WORKDIR /app
RUN mkdir -p /app/victodb

# Clone the Git repository
RUN git clone https://github.com/sreehari006/victo-rdb.git /app/source
# COPY . /app/source

RUN cmake -S /app/source -B /app/build
RUN make -C /app/build/

EXPOSE 8080

# Set the entry point for the container
ENTRYPOINT ["/app/build/victo-exe", "-d", "/app/victodb/", "-i", "0.0.0.0", "-p", "8080"]