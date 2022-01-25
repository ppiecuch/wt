FROM ubuntu:20.04
MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG en_US.UTF-8

WORKDIR /build

# Install all the dependencies
RUN apt-get update \
    && apt-get upgrade -y --no-install-recommends \
    && apt-get install -y --no-install-recommends apt-transport-https ca-certificates \
    && update-ca-certificates \
    && apt-get install -y --no-install-recommends curl git make cmake g++ \
    && apt-get install -y --no-install-recommends libboost-dev libboost-program-options-dev libboost-filesystem-dev libboost-thread-dev libunwind-dev \
    && apt-get purge --auto-remove -y \
    && apt-get clean

# Compile the server
COPY . /build
RUN ls -l /build && mkdir /build/work && cd /build/work && \
    g++ --version && \
    cmake .. && make -j $(nproc)

WORKDIR /var/source

# Create the actual image
FROM ubuntu:20.04
MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

RUN apt-get update \
    && apt-get install -y --no-install-recommends supervisor \
    && apt-get purge --auto-remove -y && apt-get clean

# Pack everything we need for the game server
COPY --from=0 /build/work/src/server/gameserver /build/resources /var/app/

WORKDIR /var/app

EXPOSE 21080/tcp
EXPOSE 21443/tcp

CMD cd /var/app && ./gameserver --doc-root . --http-listen 0.0.0.0:21080
