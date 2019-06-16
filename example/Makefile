CC := g++
LIBDIR := -L/usr/libmysql -L/usr/lib64/mysql
LIB := -lpthread -lmysqlclient -lprotobuf -lz -lhiredis -lhiredis_vip
IDIR := -I/usr/include/mysql
CFLAG := -std=c++11 -g

SRC = $(wildcard  ../server/src/*.cc)

ALLSRC = $(filter-out ../server/src/main.cc, $(SRC))

ALLSRC += $(wildcard ./*main.cc)

ALLOBJ = $(patsubst %.cc,%.o,$(ALLSRC) )

.PHONY : all
all : ThreadPool MutexLock Timestamp Thread Logging LogFile AsyncLogging

AsyncLogging : $(filter ./%eg.AsyncLogging.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 
LogFile : $(filter ./%eg.LogFile.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

Thread : $(filter ./%eg.Thread.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

Logging : $(filter ./%eg.Logging.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

Timestamp : $(filter ./%eg.Timestamp.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

ThreadPool : $(filter ./%eg.ThreadPool.main.o ../server/src/%.o   , $(ALLOBJ) )
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

MutexLock : $(filter ./%eg.MutexLock.main.o ../server/src/%.o   , $(ALLOBJ))
	$(CC) $^ $(LIBDIR) -o $@ $(LIB) 

%.o : %.cc
	$(CC) -c $^ $(IDIR)  -o $@  $(CFLAG)

ThreadPool_OBJ  =  $(filter %eg.ThreadPool.main.o ../server/src/%.o   , $(ALLOBJ) )
MutexLock_OBJ  = $(filter %eg.MutexLock.main.o ../server/src/%.o   , $(ALLOBJ))




.PHONY:clean
clean:
	rm -rf $(CLR)



#Debug
.PHONY:file
file:
	@echo "ALLOBJ: $(ALLOBJ)"
	@echo $(MutexLock_OBJ)
	@echo $(ThreadPool_OBJ)
	@echo $(filter %eg.ThreadPool.main.o ../server/src/%.o   , $(ALLOBJ) )

CLR := $(filter-out %.cc %Makefile ,  $(wildcard ../server/src/*.o , ./*))


