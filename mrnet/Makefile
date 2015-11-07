MRNETLIBBASE = $(HOME)/private/mrnet
MRNETLIBS = $(HOME)/private/mrnet/lib
MRNETINCLUDE = $(HOME)/private/mrnet/include
XPLATINCLUDE = $(HOME)/private/mrnet/lib/xplat-5.0.0/include
CURDIR = $(shell pwd)
LLIBS = -std=c++0x -g -O0 -L/lrz/sys/libraries/boost/1.58_gcc/lib -lmrnet -lm -lpthread -ldl -lboost_system -lxplat
SHAREDLIBS = -fPIC -shared

all: IntegerAddition_FE IntegerAddition_BE IntegerAdditionFilter

IntegerAddition_FE: IntegerAddition_FE.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAddition_FE.C $(LLIBS) -o $(CURDIR)/bin/IntegerAddition_FE -ldl


IntegerAddition_BE: IntegerAddition_BE.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAddition_BE.C $(LLIBS) -o $(CURDIR)/bin/IntegerAddition_BE -ldl


IntegerAdditionFilter: IntegerAddition_Filter.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAdditionFilter.C $(LLIBS) -shared -fPIC -o $(CURDIR)/bin/IntegerAdditionFilter.so -ldl
