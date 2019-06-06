CXXFLAGS := -std=c++11 -Wall -Wextra -lpthread

SRC_CLIENT := client.cpp

TARGET_CLIENT := $(SRC_CLIENT:.cpp=)

SRC_LIB := client_communicate.cpp client_lib.cpp log.cpp parse_arguments.cpp shared_library.cpp
HEADER_LIB := $(SRC_LIB:.cpp=.hpp)
OBJ_LIB := $(SRC_LIB:.cpp=.o)

RM := rm -rf

all: $(TARGET_CLIENT)

# client:
# 	g++ -std=c++11 -Wall -Wextra -o client \
# 		client.cpp client_lib.cpp communicate.cpp parse_arguments.cpp

$(TARGET_CLIENT): $(SRC_CLIENT) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean :
	$(RM) $(TARGET_CLIENT) *.o

.PHONY : clean all