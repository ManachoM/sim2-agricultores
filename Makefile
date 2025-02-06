SHELL := /bin/bash
CXX			:= ./libs_build/bsponmpi/bin/bspcxx
CXXFLAGS	:= -std=c++17 -Wall -g  -O3 -pg  -pthread -fpermissive -flto #--enable-checking -Q -v -da # -fsanitize=address
INCLUDE		:= -I./libs -I/usr/include/postgresql  -I./libs_build/libpqxx/include  -I./libs/include  -I./libs_build/bsponmpi/include/
LIBS 		:= -L./libs/lib -lpqxx -lpq -lbsponmpi -L./libs_build/local/bsponmpi/lib/ -L./libs_build/libpqxx/lib/
NP      := 4
BIN			:=  ./bin
SIM_CONFIG_DIR	:= ./sim_config_files
SIM_CONFIG_FILES := $(wildcard $(SIM_CONFIG_DIR)/*)
SRC			:= ./src
OBJ			:= ./obj
SRCS		:= $(wildcard $(SRC)/*.cpp)
OBJS		:= $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
OUT			:=  $(BIN)/agro-sim
DB_HOST		:= localhost
DB_PORT 	:= 5432
DB_NAME		:= postgres
DB_USER		:= postgres
DB_PASS		:= secret


LIBPQ_BUILD_DIR := $(shell pwd)/libs_build/libpq


.PHONY: all clean init_db libs
all: $(OUT)
	
$(OUT): $(OBJS) | $(BIN)
	$(CXX) $(INCLUDE) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(INCLUDE) -c $< -o $@ $(CXXFLAGS) $(LIBS)

$(BIN) $(OBJ):
	mkdir $@

init_db:
	psql postgresql://$(DB_USER):$(DB_PASS)@$(DB_HOST):$(DB_PORT)/$(DB_NAME) -tc "SELECT 1 FROM pg_database WHERE datname = 'sim-db'" | grep -q 1|| psql postgresql://$(DB_USER):$(DB_PASS)@$(DB_HOST):$(DB_PORT)/$(DB_NAME) -c 'create database "sim-db";'
	psql postgresql://$(DB_USER):$(DB_PASS)@$(DB_HOST):$(DB_PORT)/sim-db -f ./database/create_sim_db.sql


clean:
	rm -f $(OBJ)/*.o $(OUT) 

clean_libs:
	rm -rf libs_build/

# Regla para compilar las librerías en `libs`
libs: libs_build/bsponmpi/lib/libbsponmpi.a libs_build/libpqxx/lib/libpqxx.a libs_build/libpq/lib/libpq.a

# Descarga y compila bsponmpi si no está ya compilado
libs_build/bsponmpi/lib/libbsponmpi.a:
		mkdir -p libs_build/bsponmpi
	[ -d libs/bsponmpi ] || (git clone https://github.com/wijnand-suijlen/bsponmpi libs/bsponmpi && cd libs/bsponmpi && git checkout tags/v1.1.1)
	# Modificaciones programáticas para deshabilitar pruebas, documentación y post-install-check
	sed -i 's/^enable_testing/#enable_testing/' libs/bsponmpi/CMakeLists.txt
	sed -i '/add_test(/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/add_unit_test(/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/add_unit_ctest(/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/add_mpi_test(/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/set_tests_properties(/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/install(SCRIPT/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/Doxygen/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/add_custom_target(docs/s/^/#/' libs/bsponmpi/CMakeLists.txt
	sed -i '/BSPONMPI_A2A_METHOD/s/^/#/' libs/bsponmpi/CMakeLists.txt
	# Configura e instala bsponmpi
	cd libs/bsponmpi && ./configure --prefix=../../libs_build/bsponmpi && make && make install
 
libs_build/libpqxx/lib/libpqxx.a:
	mkdir -p $(shell pwd)/libs_build/libpqxx
  [ -d libs/libpqxx ] || (curl -L https://github.com/jtv/libpqxx/archive/refs/tags/7.9.2.tar.gz -o libpqxx-7.9.2.tar.gz && \
	tar -xzf libpqxx-7.9.2.tar.gz -C libs && mv libs/libpqxx-7.9.2 libs/libpqxx)
	cd libs/libpqxx && CXXFLAGS="-O3" ./configure --prefix=$(shell pwd)/libs_build/libpqxx --disable-documentation && make && make install


# Regla para descargar y compilar PostgreSQL completo, luego instalar solo libpq
libs_build/libpq/lib/libpq.a:
	mkdir -p $(LIBPQ_BUILD_DIR)
	# Descarga y extrae PostgreSQL si no existe ya
	[ -d libs/postgresql-15.3 ] || (curl -L https://ftp.postgresql.org/pub/source/v15.3/postgresql-15.3.tar.gz -o postgresql-15.3.tar.gz && \
	 tar -xzf postgresql-15.3.tar.gz -C libs)
	# Configuración solo para libpq
	cd libs/postgresql-15.3 && ./configure --prefix=$(LIBPQ_BUILD_DIR) --without-readline --without-zlib --without-server
	# Compilar e instalar solo libpq en el directorio de destino
	cd libs/postgresql-15.3/src/interfaces/libpq && make MAKELEVEL=0 && make install


run_scenarios:
	for file in $(filter %.json,$(SIM_CONFIG_FILES)); do \
	    base=$$(basename $$file .json); \
	    logfile=$$base.log; \
	    echo "Processing file: $$file" | tee -a $$logfile; \
	    for i in $$(seq 1 15); do \
	        mpirun -np $(NP) $(OUT) -c $$file >> $$logfile; \
	        # :; \
	    done; \
	done

