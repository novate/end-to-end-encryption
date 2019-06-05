CXXFLAGS := -std=c++11 -Wall -Wextra

SRC_TEST := test.cpp

TARGET_TEST := $(SRC_TEST:.cpp=)

SRC_LIB := log.cpp
HEADER_LIB := $(SRC_LIB:.cpp=.hpp)
OBJ_LIB := $(SRC_LIB:.cpp=.o)

RM := rm -rf

all: $(TARGET_TEST)

$(TARGET_TEST): $(SRC_TEST) $(OBJ_LIB) $(HEADER_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean :
	$(RM) $(TARGET_TEST) *.o

.PHONY : clean all