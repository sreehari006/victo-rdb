# Stage 1: Build the application

# Use an Alpine-based BusyBox variant as the base image
FROM alpine:latest AS builder

# Install the required packages using the package manager
RUN apk --no-cache add gcc g++ cmake git make openssl-dev openssl

# Set the working directory
WORKDIR /app

# Copy the source code
COPY . /app/source

# Build the application
RUN cmake -S /app/source -B /app/build
RUN make -C /app/build/

# Stage 2: Create a minimal runtime Image

# Use an Alpine-based BusyBox variant as the base image
FROM alpine:latest

# Set the working directory
WORKDIR /app

# Create directory for exe and database base path
RUN mkdir -p /app/victodb
RUN mkdir -p /app/exe

# Copy exe from builder image to runtime image
COPY --from=builder /app/build/victo-exe /app/exe

#Expose port 8080 on runtime image
EXPOSE 8080

# Set the entry point for the container
ENTRYPOINT ["/app/exe/victo-exe", "-d", "/app/victodb", "-i", "0.0.0.0", "-p", "8080", "-a", "false"]