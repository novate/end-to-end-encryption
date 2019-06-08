#CXXFLAGS := -std=c++11 -Wall -Wextra

#SRC_LIB = $(wildcard src/*.cpp)
#HEADER_LIB = $(wildcard include/*.hpp)
#OBJ_LIB := $(SRC_LIB:.cpp=.o)
#TARGET_LIB := server try

#RM := rm -f
# 
all: 
	g++ -g -std=c++11 -o yzmond ./src/yzmond.cpp ./src/types.cpp ./src/log.cpp ./src/parse_arguments.cpp ./src/transfer.cpp ./src/presentation.cpp ./src/DatabaseConnection.cpp -lmysqlclient -I /usr/include/ -L/usr/lib64/mysql

clean:
	$(RM) yzmond *.log
# 
# $(OBJ_LIB) : $(HEADER_LIB)
# 
# %.o: %.cpp $(HEADER_LIB)
# 	$(CXX) $(CXXFLAGS) -c -o $@ $<
# 
# %: %.o %.hpp
# 	$(CXX) $(CXXFLAGS) -o $@ $<
# 
# conf: src/conf.o include/conf.hpp
# 	$(CXX) $(CXXFLAGS) -o try $<
# 
# clean :
# 	$(RM) $(TARGET_LIB) $(OBJ_LIB)
# 
# .PHONY : all clean conf
