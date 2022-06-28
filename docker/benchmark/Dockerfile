FROM ubuntu:latest

# Install dependencies
RUN apt-get update -y && apt-get install -y make clang bc
RUN mkdir /Luz

# Copy source files
COPY ./srcs /Luz/srcs
COPY ./includes /Luz/includes
COPY ./Makefile /Luz

# Copy scripts
COPY ./docker/build.sh /
RUN chmod +x /build.sh
COPY ./docker/benchmark/run.sh /
RUN chmod +x /run.sh

CMD /build.sh && /run.sh
