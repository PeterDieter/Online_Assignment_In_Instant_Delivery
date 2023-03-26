all : optimalAssignment

CCC = g++
# CCC = clang++-10
CCFLAGS = -O3 -Wall -std=c++2a
# CCFLAGS = -Wall -std=c++2a -g
TARGETDIR=.

OBJS2 = \
        $(TARGETDIR)/Data.o \
        $(TARGETDIR)/Environment.o \
        $(TARGETDIR)/main.o

$(TARGETDIR)/optimalAssignment: $(OBJS2)
	$(CCC) $(CCFLAGS) -o $(TARGETDIR)/optimalAssignment $(OBJS2)
	
$(TARGETDIR)/Data.o: Data.h Data.cpp
	$(CCC) $(CCFLAGS) -c Data.cpp -o $(TARGETDIR)/Data.o

$(TARGETDIR)/Environment.o: Environment.h Environment.cpp
	$(CCC) $(CCFLAGS) -c Environment.cpp -o $(TARGETDIR)/Environment.o

$(TARGETDIR)/main.o: main.cpp
	$(CCC) $(CCFLAGS) -c main.cpp -o $(TARGETDIR)/main.o

# test: optimalAssignment
# 	./optimalAssignment ../../../

clean:
	$(RM) \
    $(TARGETDIR)/main.o \
    $(TARGETDIR)/Data.o \
    $(TARGETDIR)/Environment.o \
    $(TARGETDIR)/optimalAssignment
     