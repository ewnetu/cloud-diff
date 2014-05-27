

CXXFLAGS=-g -std=c++0x -Wall  -pedantic -Wno-vla -Wno-deprecated -I./jsoncpp-src-0.5.0/include/ -I./flopc/include/coin -I./dlib-18.5  

#dlib
LFLAGS = -lpthread   -lnsl -lX11

LDLIBS=-lboost_program_options -lboost_thread -lvirt -ltinyxml -lcurl -lblas -L . -ljson_linux-gcc-4.6_libmt  -L./flopc/lib   -lFlopCpp -lOsiCbc -lCbc -lOsiClp -lClp -lCgl -lOsi -lCoinUtils -lz -lm  
all: actuator diffcloudcontroller httpmon #least_squares_ex

 

actuator: actuator.o

diffcloudcontroller: diffcloudcontroller.o VirtualManager.o Application.o PriorityClass.o Optimizer.o PriorityManager.o  Model.o Penalty.o ApacheMonitor.o QueueLength.o #./ least_squares.o dlib-18.5/dlib/all/source.cpp

 #PriorityManager.o: PriorityManager.cc  PriorityManager.hh
 
#least_squares_ex: least_squares_ex.cpp  
#lib/all/source.o: lib/all/source.cpp
httpmon: httpmon.cc




clean:
	rm -f *.o actuator

