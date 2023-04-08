CXX			:= /usr/bin/g++
CXXFLAGS	:= -std=c++14 -Wall -g -pthread -O3
INCLUDE		:= -I./libs 
BIN			:= ./bin
SRC			:= ./src
OBJ			:= ./obj
SRCS		:= $(wildcard $(SRC)/*.cpp)
OBJS		:= $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
OUT			:= $(BIN)/agro-sim

.PHONY: all clean
all: $(OUT)
	
$(OUT): $(OBJS) | $(BIN)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INCLUDE)

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN) $(OBJ):
	mkdir $@

clean:
	rm $(wildcard obj/*.o)
	rm $(BIN)/$(OUT)
