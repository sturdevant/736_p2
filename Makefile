MRNETLIBBASE = $(HOME)/private/mrnet
MRNETLIBS = $(HOME)/private/mrnet/lib
MRNETINCLUDE = $(HOME)/private/mrnet/include
XPLATINCLUDE = $(HOME)/private/mrnet/lib/xplat-5.0.0/include
CURDIR = $(shell pwd)
LLIBS = -std=c++0x -g -O0 -L/lrz/sys/libraries/boost/1.58_gcc/lib -lmrnet -lm -lpthread -ldl -lboost_system -lxplat
SHAREDLIBS = -fPIC -shared

all: Test_FE Test_BE Test_Filter Client

Test_FE: Test_FE.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl Test_FE.C $(LLIBS) -o $(CURDIR)/bin/Test_FE -ldl


Test_BE: Test_BE.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl Test_BE.C $(LLIBS) -o $(CURDIR)/bin/Test_BE -ldl


Test_Filter: Test_Filter.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl Test_Filter.C $(LLIBS) -shared -fPIC -o $(CURDIR)/bin/Test_Filter.so -ldl

Client: Client.C
	g++ Client.C -o $(CURDIR)/bin/Client
