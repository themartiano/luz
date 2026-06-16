FROM ubuntu:24.04

RUN apt-get update -y \
	&& apt-get install -y --no-install-recommends make clang \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /luz

COPY . .
RUN chmod +x /luz/docker/benchmark/benchmark.sh

CMD ["/luz/docker/benchmark/benchmark.sh"]
