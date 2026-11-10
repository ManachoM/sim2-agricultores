FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    g++ make libpqxx-dev libpq-dev python3 postgresql-client ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /opt/sim
COPY Makefile ./
COPY src ./src
COPY includes ./includes
COPY libs/json.hpp ./libs/json.hpp
RUN make CXX=g++ CXXFLAGS='-std=c++17 -O2 -Wall -pthread -fpermissive' \
    INCLUDE='-I./libs -I/usr/include/postgresql' LIBS='-lpqxx -lpq'
WORKDIR /workspace
CMD ["/opt/sim/bin/agro-sim", "-c", "runs/local-config.json"]
