
####################
# TOOL SECTION
####################
CC = gcc
CXX = g++
MKDEPEND = $(CC) -MM

$(PTHREAD_FLAG)
####################
# CXXFLAGS SECTION
####################
WARNING = -W -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wno-long-long
COMMON_CXXFLAGS = -Wall -g -pipe  -O0 -std=c++11

############################
# CXXFLAGS / CONFIG SECTION
############################
CXXFLAGS = $(INCLUDE) $(COMMON_CXXFLAGS) 
LDFLAGS = 

####################
# TARGET SECTION
####################
OBJS = gason.o arrange.o

MAKE_TARGET = arrange

SRCS = $(OBJS:.o=.cpp)

all	: $(MAKE_TARGET)

$(MAKE_TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^  -o $@ $(LDFLAGS)

clean	:
	rm -f $(OBJS) $(MAKE_TARGET)
	rm -f .depend



.depend :
	$(CXX) -MM $(INCLUDE) $(SRCS) > .depend

include .depend

