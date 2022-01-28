FROM ubuntu:20.04
MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG en_US.UTF-8

WORKDIR /build

# Install all the dependencies
RUN apt-get update \
  && apt-get upgrade -y --no-install-recommends \
  && apt-get install -y --no-install-recommends apt-transport-https ca-certificates locales \
  && update-ca-certificates \
  && apt-get install -y --no-install-recommends curl git pkg-config autogen autoconf automake libtool make cmake g++ \
  && apt-get install -y --no-install-recommends libboost-dev libboost-program-options-dev libboost-filesystem-dev libboost-thread-dev libunwind-dev libstatgrab-dev liblzma-dev libssl-dev \
  && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
  && dpkg-reconfigure --frontend=noninteractive locales \
  && update-locale LANG=en_US.UTF-8 \
  && apt-get purge --auto-remove -y \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Compile the server
COPY . /build
RUN ls -l /build && mkdir /build/work && cd /build/work \
  && g++ --version \
  && cmake .. && make -j $(nproc)

WORKDIR /var/source

# Create the actual image
FROM ubuntu:20.04
MAINTAINER Pawel Piecuch <piecuch.pawel@gmail.com>

RUN apt-get update \
  && apt-get install -y --no-install-recommends supervisor \
  && apt-get install -y --no-install-recommends libboost libboost-program-options libboost-filesystem libboost-thread libunwind libstatgrab liblzma libssl \
  && apt-get purge --auto-remove -y && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Pack everything we need for the game server
COPY --from=0 /build/work/src/server/gameserver /build/resources /var/app/

WORKDIR /var/app

EXPOSE 21080/tcp
EXPOSE 21443/tcp

CMD cd /var/app && ./gameserver --doc-root . --http-listen 0.0.0.0:21080
