MRNETLIBBASE = $(HOME)/public/736/mrnet
MRNETLIBS = $(HOME)/public/736/mrnet/lib
MRNETINCLUDE = $(HOME)/public/736/mrnet/include
XPLATINCLUDE = $(HOME)/public/736/mrnet/lib/xplat-5.0.0/include
UTILINCLUDE = ../util
NODESINCLUDE = ../nodes
LOGGERINCLUDE = ../
CURDIR = $(shell pwd)
LLIBS = -std=c++0x -g -O0 -L/lrz/sys/libraries/boost/1.58_gcc/lib -lmrnet -lm -lpthread -ldl -lboost_system -lxplat
SHAREDLIBS = -fPIC -shared

all: IntegerAddition_FE IntegerAddition_BE IntegerAdditionFilter partial_FE

IntegerAddition_FE: IntegerAddition_FE.C
	g++ -L$(MRNETLIBS) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAddition_FE.C $(LLIBS) -o $(CURDIR)/IntegerAddition_FE -ldl

partial_FE: logger timedqueue IntegerAdditionFilter IntegerAddition_BE
	g++ -L$(MRNETLIBS) logger.o timedqueue.o -I$(LOGGERINCLUDE) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl partial_FE.C $(LLIBS) -o $(CURDIR)/partial_FE -ldl -lrt

LeafNode: Cell Response Request
	g++ -o LeafNode.o -c -fPIC ../nodes/LeafNode.C -I$(UTILINCLUDE)

InternalNode: Cell Response Request
	g++ -o InternalNode.o -c -fPIC ../nodes/InternalNode.C -I$(UTILINCLUDE)

Cell: Point
	g++ -o Cell.o -c -fPIC ../nodes/Cell.C

Response: Point
	g++ -o Response.o -c -fPIC ../nodes/Response.C

Request: Point
	g++ -o Request.o -c -fPIC ../nodes/Request.C

Point: Cluster
	g++ -o Point.o -c -fPIC ../nodes/Point.C
	
Cluster:
	g++ -o Cluster.o -c -fPIC ../nodes/Cluster.C

logger: timedqueue
	g++ -o logger.o -c ../logger.C

timedqueue:
	g++ -o timedqueue.o -c ../timedqueue.C -lrt

IntegerAddition_BE: IntegerAddition_BE.C LeafNode
	g++ -L$(MRNETLIBS) -I$(NODESINCLUDE) -I$(UTILINCLUDE) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAddition_BE.C $(LLIBS) -o $(CURDIR)/IntegerAddition_BE -ldl LeafNode.o Cell.o Response.o Request.o Point.o Cluster.o


IntegerAdditionFilter: IntegerAdditionFilter.C InternalNode
	g++ -L$(MRNETLIBS) -I$(UTILINCLUDE) -I$(NODESINCLUDE) -I$(MRNETLIBBASE) -I$(MRNETINCLUDE) -I$(XPLATINCLUDE) -ldl IntegerAdditionFilter.C $(LLIBS) -shared -fPIC -o $(CURDIR)/IntegerAdditionFilter.so -ldl InternalNode.o Cell.o Response.o Request.o Point.o Cluster.o
