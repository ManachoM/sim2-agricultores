SHELL := /bin/bash
CXX			:= mpicxx
CXXFLAGS	:= -std=c++17 -Wall -g3 -pg -O3 -pthread -fpermissive -flto# -fsanitize=address
INCLUDE		:= -I./libs -I/usr/include/postgresql -I./libs/include -I/usr/include/c++ -I/usr/local/bsponmpi/include/
LIBS 		:= -L./libs/lib -lpqxx -lpq -lbsponmpi -L /usr/local/bsponmpi/lib/
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




.PHONY: all clean init_db
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
	rm $(wildcard obj/*.o)
	rm $(OUT)


run_scenarios:
	for file in $(SIM_CONFIG_FILES); do \
    	for i in $$(seq 1 $(n)); do \
        	mpirun -np $(NP) $(OUT) -c $$file >> out.log; \
		done; \
	done
