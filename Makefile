CXX_SRCS = cpputil.cpp lexer.cpp parser.cpp parser2.cpp \
	main.cpp buildast.cpp ast.cpp node_base.cpp node.cpp treeprint.cpp \
	location.cpp exceptions.cpp
CXX_OBJS = $(CXX_SRCS:%.cpp=%.o)

CXX = g++
CXXFLAGS = -g -Wall -std=c++17

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

all : astdemo

astdemo : $(CXX_OBJS)
	$(CXX) -o $@ $(CXX_OBJS)

clean :
	rm -f *.o astdemo

depend :
	$(CXX) $(CXXFLAGS) -M $(CXX_SRCS) >> depend.mak

depend.mak :
	touch $@

include depend.mak
