CXXFLAGS := -std=c++11 -Wall -Wextra

SRC_SERVER := server.cpp
SRC_CLIENT := client.cpp client_lib.cpp communicate.cpp

TARGET_SERVER := $(SRC_SERVER:.cpp=)
TARGET_CLIENT := $(SRC_CLIENT:.cpp=)

SRC_LIB := parse_arguments.cpp #shared_library.cpp
HEADER_LIB := $(SRC_LIB:.cpp=.hpp)
OBJ_LIB := $(SRC_LIB:.cpp=.o)

RM := rm -rf

all: $(TARGET_SERVER) $(TARGET_CLIENT) make_dirs

client:
	g++ -std=c++11 -Wall -Wextra -o client \
		client.cpp client_lib.cpp communicate.cpp parse_arguments.cpp

$(TARGET_SERVER): $(SRC_SERVER) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_CLIENT): $(SRC_CLIENT) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

make_dirs :
	mkdir -p server_txt/
	mkdir -p client_txt/

clean :
	$(RM) $(TARGET_SERVER) $(TARGET_CLIENT) *.o
	$(RM) server_txt/ client_txt/

.PHONY : clean all make_dirs