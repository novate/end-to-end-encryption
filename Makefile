CXXFLAGS := -std=c++11 -Wall -Wextra

SRC_SERVER := server.cpp
SRC_CLIENT := client.cpp
SRC_TEST_LOG := test_log.cpp

TARGET_SERVER := $(SRC_SERVER:.cpp=)
TARGET_CLIENT := $(SRC_CLIENT:.cpp=)
TARGET_TEST_LOG := $(SRC_TEST_LOG:.cpp=)

SRC_LIB := parse_arguments.cpp shared_library.cpp log.cpp
HEADER_LIB := $(SRC_LIB:.cpp=.hpp)
OBJ_LIB := $(SRC_LIB:.cpp=.o)

RM := rm -rf

all: $(TARGET_SERVER) $(TARGET_CLIENT)

test: $(TARGET_TEST_LOG)

$(TARGET_SERVER): $(SRC_SERVER) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_CLIENT): $(SRC_CLIENT) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_TEST_LOG): $(SRC_TEST_LOG) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean :
	$(RM) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_TEST_LOG) *.o

.PHONY : clean all test