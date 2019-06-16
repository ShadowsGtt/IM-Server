CC := g++
LIBDIR := -L/usr/libmysql -L/usr/lib64/mysql/   #ubuntu , centos
LIB := -lpthread -lmysqlclient -lprotobuf -lz  -lhiredis -lhiredis_vip
IDIR := -I/usr/include/mysql
CFLAG := -std=c++11 -g 

client_src =  $(filter-out %main.cc , $(wildcard ../server/src/*.cc) )
#client_src += $(filter-out main.cc , $(client_src))
client_src += $(wildcard ./src/*.cc)
client_obj = $(patsubst %.cc,%.o,$(client_src))

.PHONY : all
all : client


client : $(client_obj)
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

%.o : %.cc
	$(CC) -c $^ $(CFLAG) $(IDIR)  -o $@


CLR = $(wildcard ./src/*.o)
CLR_EXEC = $(wildcard ./client)

.PHONY:clean
clean:
	#$(if $(CLR) $(shell rm -rf ./src/*.o ),) 
	#$(if $(CLR_EXEC) $(shell rm -rf ./client ),)
	rm -rf ./src/*.o 
	rm -rf ./client

