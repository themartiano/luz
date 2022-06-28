FROM ubuntu:latest

RUN apt-get update -y && apt-get install -y make clang
RUN mkdir /Luz

COPY ./srcs /Luz/srcs
COPY ./includes /Luz/includes
COPY ./Makefile /Luz

CMD cd /Luz && make && ./Luz
# CMD bash
