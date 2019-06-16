CC := g++
LIBDIR := -L/usr/libmysql -L/usr/lib64/mysql/   #ubuntu , centos  
LIB := -lpthread -lmysqlclient -lprotobuf -lz -lhiredis -lhiredis_vip
IDIR := -I/usr/include/mysql
CFLAG := -std=c++11 -g 
BINDIR := ./bin/

serv_src = $(wildcard ./src/*.cc)

serv_obj = $(patsubst %.cc,%.o,$(serv_src))

.PHONY : all
all : build serv  

.PHONY : build
build :
	cd proto;protoc IM.proto --cpp_out=.;cp *.pb.* ../src/;cd .. 
	if [ ! -d "bin" ]; then mkdir bin; fi
	

serv : $(serv_obj)
	$(CC) $^ $(LIBDIR) -o $(BINDIR)$@ $(LIB) 

%.o : %.cc
	$(CC) -c $^ $(CFLAG) $(IDIR)  -o $@


#CLR = $(wildcard ./src/*.o)
#CLR_EXEC = $(wildcard ./serv)
#$(if $(CLR) $(shell rm ./src/*.o ),) 
#$(if $(CLR_EXEC) $(shell rm ./serv ),)

.PHONY:clean
clean:
	rm -rf ./src/*.o
	rm -rf ./bin

