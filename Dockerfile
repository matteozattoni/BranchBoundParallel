# DEBIAN with OPENMPI 4.1.5 
# user: mpi 
# password: mpi

FROM debian:latest
RUN export DEBIAN_FRONTEND=noninteractive
ARG username=mpi
RUN apt-get update && \
apt-get -y install build-essential && \
apt-get -y install automake && \
apt-get -y install autoconf && \
apt-get -y install libtool && \
apt-get -y install flex && \
apt-get -y install autotools-dev && \
apt-get -y install ntp

COPY openmpi-4.1.5.tar /openmpi/
WORKDIR /openmpi
RUN tar -xf openmpi-4.1.5.tar
WORKDIR /openmpi/openmpi-4.1.5
RUN ./configure --prefix=/usr/local
RUN make all install
RUN ldconfig

RUN apt-get -y install sudo openssh-server && \
service ssh start

RUN useradd -ms /bin/bash -G sudo $username
RUN echo "${username}:mpi" | chpasswd
USER $username
WORKDIR /home/${username}