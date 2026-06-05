FROM ubuntu:24.04

RUN apt-get update -y \
	&& apt-get install -y --no-install-recommends make clang \
	&& rm -rf /var/lib/apt/lists/*
RUN mkdir /Luz

# Copy source files
COPY ./src /Luz/src
COPY ./include /Luz/include
COPY ./tests /Luz/tests
COPY ./assets /Luz/assets
COPY ./Makefile /Luz

# Copy scripts
COPY ./docker/benchmark/benchmark.sh /
RUN chmod +x /benchmark.sh

CMD ["/benchmark.sh"]
