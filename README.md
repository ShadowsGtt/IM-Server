# IM 服务器介绍
A Server Instant Messaging System


## SocketsOps [SocketsOps.cpp SocketsOps.h]  
#### 描述  
```
	将socket系统调用封装在namespace sockets中
```
#### 接口  
```
	/* 创建指定地址族的sockfd, AF_INET or AF_INET6*/
	int createNonblockingOrDie(sa_family_t family);

	/* 连接 成功返回0 失败-1 */
	int  connect(int sockfd, const struct sockaddr* addr);
	
	/* 绑定  */
  	void bindOrDie(int sockfd, const struct sockaddr* addr);
	
	/* 监听 */	
	void listenOrDie(int sockfd);
	
	/* 接受新连接，返回新 socketfd */
	int  accept(int sockfd, struct sockaddr_in* addr);
	
	/* 原生读 */
	ssize_t read(int sockfd, void *buf, size_t count);
	
	/* 原生分散读 */
	ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
	
	/* 原生写 */
	ssize_t write(int sockfd, const void *buf, size_t count);
	
	/* 关闭socketfd */
	void close(int sockfd);
	
	/* 关闭写端 */	
	void shutdownWrite(int sockfd);
	

	/* 从 sockfd 中得到本端信息 */
	struct sockaddr_in getLocalAddr(int sockfd);

	/* 从 sockfd 中得到对端信息 */
	struct sockaddr_in getPeerAddr(int sockfd);
```



## InetAddress [InetAddress.cpp InetAddress.h]  
#### 描述  
```
	对IPV4地址结构struct sockaddr_in的封装
```
#### 接口  
```
	/* 构造函数  传入ip 和 port */
	InetAddress(string ip, uint16_t port);
	
	/* 构造函数  传入struct sockaddr_in结构体 */
	InetAddress(const struct sockaddr_in& addr);

	/* 返回地址族 用到的都是IPV4 AF_INET */
  	sa_family_t family();

	 /* 以string形式返回IP地址  */
  	/* eg: 0.0.0.0 */
  	string toIp() const;

  	/* 以string形式返回IP 及 port */
  	/* eg: 0.0.0.0:8888 */
  	string toIpPort();

  	/* 返回port */
  	uint16_t toPort();


  	/* 返回网络端(大端)IP */
  	uint32_t ipNetEndian();
  
  	/* 返回网络端(大端)端口 */
  	uint16_t portNetEndian();

	/* 以struct sockaddr形式返回地址 */
  	const struct sockaddr* getSockAddr();

   	/* 设置sockaddr_in地址 */
   	void setSockAddrInet(const struct sockaddr_in& addr);
```



## Socket [Socket.cpp Socket.h]  
#### 描述  
```
	对socket fd的封装
```
#### 接口  
```
	/* 构造函数  传入socket fd */
	Socket(int sockfd);

	/* 返回socket fd */
	int fd();
	
	/* 获得TCP详细信息  结果存入传入的tcp_info结构地址中  */
	/* 成功返回true 失败返回false */
	bool getTcpInfo(struct tcp_info*);

	/* 获得TCP详细信息  结果存入buf地址中 */
	bool getTcpInfoString(char* buf, int len);

	/* 绑定ip和port给socketfd */
	void bindAddress(const InetAddress& localaddr);

	/* 开始监听 */
	void listen();

	/* 关闭写端 */
	void shutdownWrite();

	/* 是否设置TCP 选项 TCP_NODELAY */
	/* 设置TCP_NODELAY其实就是禁用了Nagle算法，允许小包的发送 */
	/* 如果对延时要求高，则开启TCP_NODELAY */
	void setTcpNoDelay(bool on);

	/* 是否设置地址复用 SO_REUSEADDR选项  */
	/* 设置SO_REUSEADDR为了重启监听服务 */
	/* 端口被释放之后socketfd处于TIME_WAIT状态 正常情况2min內是不可以重新绑定的 */
	/* 而导致bind函数出错 */
	void setReuseAddr(bool on);

	/* 是否设置端口复用 SO_REUSEPORT选项 */
	/* 这个选项允许将相同的ip和port绑定到多个socket上 */
    /* 这些socket可以分布在相同主机的同一个线程、多个线程、乃至多个进程中去 */
	/* 内核会自动把这个端口的请求自动分派到各个socket上面去 */
	/* 而且这个过程没有用户惊群、互斥等问题 */
	void setReusePort(bool on);

	/* 设置保活 */
	void setKeepAlive(bool on);
	
```



## Thread [Thread.cpp Thread.h]  
#### 描述  
```
	对线程的封装
```
#### 接口  
```
	/* 新建一个线程类 参数1:设置回调函数  参数2:线程名字 */
	/* 为传入name时会使用默认名字 Thread1 Thread2... */
	Thread(const boost::function<void ()>&, const string& name = string());
	
	/* 线程开始运行 */
	void Thread::start()
	
	/* 等待线程运行结束 return 0 函数调用成功  */
	int Thread::join();
	
	/* 返回线程id */
	pid_t Thread::tid();
	
	/* 返回线程名字 */
	const Thread::string& name();

    /* 返回线程序列号 */
	const int32_t sequence();

```



## Mutex  [Mutex.h]  
#### 描述  
```
	对互斥锁的封装
```
#### 接口  
```
	/* 新建MutexLock，无参构造 */
	MutexLock();
	
	/* 本线程是否拥有锁 */
	bool MutexLock::isLockedByThisThread();

	/* 是否加锁，锁没有被锁住时会程序会退出 */
	void MutexLock::assertLocked();

	/* 锁住锁 */
	void MutexLock::lock();

	/* 释放锁 */
	void MutexLock::unlock();

	/* 获得锁 */
	pthread_mutex_t* MutexLock::getPthreadMutex();

	/* 构造时会对mutex上锁,析构时会解锁 */
	MutexLockGuard(MutexLock& mutex);
	
	/* 析构时解锁mutex */
	~MutexLockGuard();
	
```



## ThreadPool  [ThreadPool.h  ThreadPool.cpp]
#### 描述  
```
	静态线程池的实现
```
#### **接口**  
```
	/* 新建线程池,并为其命名 */
	ThreadPool(const string& nameArg = string("ThreadPool"));	

    /* 设置任务队列大小 */
    void setMaxQueueSize(int maxSize);
    
    /* 设置线程池中线程启动时运行的函数 */
    void setThreadInitCallback(const Task& cb);

    /* 设置线程池大小并启动 */
    void start(int numThreads);

    /* 线程池停止 */
    void stop();

    /* 返回线程池名字 */
    const string& name();
    
    /* 返回任务队列中任务个数 */
    size_t queueSize();
```

## Timestamp  [Timestamp.h  Timestamp.cpp]
#### 描述  
```
	时间戳的封装
```
#### **接口**  
```
	/* 构造一个微秒数为0的无效类 */
  	Timestamp();

  	/* 构造一个微秒数为microSecondsSinceEpochArg的Timestamp */
  	explicit Timestamp(int64_t microSecondsSinceEpochArg);

  	/* this->microSecondsSinceEpoch_ 与 that.microSecondsSinceEpoch_互换 */
  	void swap(Timestamp& that);

  	/* 将microSecondsSinceEpoch_转换成 [秒.微秒]格式的 string */
  	string toString() const;

  	/* 转换成   xxxx-xx-xx xx:xx:xx.xxxxxx 格式的string */
  	string toFormattedString(bool showMicroseconds = true) const;

  	/* 判断是否有效 microSecondsSinceEpoch_值大于0则有效*/
  	bool valid() ;

  	/* 得到私有变量microSecondsSinceEpoch_值 */
  	int64_t microSecondsSinceEpoch();
  
  	/* 将microSecondsSinceEpoch_值转换成秒数并返回 */
  	time_t secondsSinceEpoch();
  	
	/* 得到系统当前时间并存成微秒数 返回一个新Timestamp */
  	static Timestamp now();

  	/* 返回一个微秒数为0的无效类 */
  	static Timestamp invalid();

  	/* Unix时间戳经历的秒数记录为微秒数，返回Timestamp类 */
  	static Timestamp fromUnixTime(time_t t);
  
	/* Unix时间戳经历的秒数及微秒数记录为微秒数，返回Timestamp类 */
  	static Timestamp fromUnixTime(time_t t, int microseconds);

	//下面这两个时全局函数  作为辅助函数
	/* 返回两个时间差秒数 high - low */
	inline double timeDifference(Timestamp high, Timestamp low);

	/* 计算 timestamp + 秒数seconds  ，返回新Timestamp类 */
	inline Timestamp addTime(Timestamp timestamp, double seconds);

```
## Timer  [Timer.h  Timer.cpp]
#### 描述  
```
	定时器的实现
```
#### **接口**  
```
	/* 构造函数，传入定时器到时的回调，到时的时间点 */
	/* interval为时间间隔，表示定时器多长时间调用一次回调 */
	/* 一般要么设置when  要么设置interval */
	Timer(const TimerCallback& cb, Timestamp when, double interval);
	
	/* 运行回调 */
  	void run();

	/* 返回定时器的截止时间 */
  	Timestamp expiration();
 
	/* 是否设置以一定的时间间隔重复激活的定时器 */
  	bool repeat();
  
  	/* 返回定时器的序列号 */
  	int64_t sequence();

  	/* 重启定时器  用于设置repeat的Timer */
  	void restart(Timestamp now);

```


## Condition  [Condition.h  Condition.cpp]
#### 描述  
```
	条件变量的封装
```
#### **接口**  
```
	/* 构造函数，传入一个互斥锁 */ 
 	/*因为条件变量总是与互斥锁搭配使用 */
	Condition(MutexLock& mutex)
	
	/* 睡眠等待条件变量 */
	void wait();

	/* 唤醒等待条件变量的一个线程 */
  	void notify();

	/* 唤醒等待条件变量的全部线程 */
 	void notifyAll();

	/* 睡眠等待条件变量senconds秒 */
	/* 超时返回true  */	
	bool waitForSeconds(double seconds);

```



## CountDownLatch  [CountDownLatch.h  CountDownLatch.cpp]
#### 描述  
```
	一个辅助类,用于同步多个线程	
```
#### **接口**  
```
	/* 构造函数，设置门闩值  */ 
	CountDownLatch(int count);
	
	/* 睡眠等待，直到count值为0时才开始运行*/
	void wait();

	/* 将count值减1 */
  	void countDown();

	/* 返回count值 */
 	int getCount() const;


```

## Logging  [Logging.h  Logging.cpp]
#### 描述  
```
	将日志消息保存到4M的缓冲区內
```
#### **接口**  
``` 
	LOG_TRACE << [content];  	//需设置日志级别为TRACE才会记录
	LOG_DEBUG << [content];  	//需设置日志级别为DEBUG或TRACE才会记录
	LOG_INFO  << [content];		//默认日志级别 
	LOG_WARN  << [content];		//
	LOG_ERROR << [content];		//
	LOG_FATAL << [content];  	//记录日志并Abort()退出程序 
	LOG_SYSERR << [content]; 	//当errno不为0时会 记录错误信息
	LOG_SYSFATAL << [content]; 	//记录日志并退出程序 当errno不为0时会记录错误信息
	
	/* 获得内部stream_ */
	/* stream_才是真正持有缓冲区的成员 */
	/* 通过stream_的const Buffer& buffer()函数获得缓冲区  */ 
	/*Buffer是 LogStream::Buffer类型 */
	/*  通过stream_的void resetBuffer()函数清空缓冲区 */
	LogStream& stream();	
	
	/* 获得当前日志级别 */
	static LogLevel logLevel();

	/* 设置日志级别 */
	static void setLogLevel(LogLevel level);

	/* 设置日志输出的回调函数 */
	/* 当LOG_*调用结束后会调用该回调函数 */
	/* msg指针会指向日志缓冲区的起始位置  缓冲区的长度为len */
	static void setOutput(void (*OutputFunc)(const char* msg, int len));

	/* 设置日志刷新函数 */
	static void setFlush(void (*FlushFunc)());
```




## LogFile  [LogFile.h  LogFile.cpp]
#### 描述  
```
	将日志信息记录到文件中
	文件名格式:文件名.年月日-十分秒.主机名.进程id.log
	附：每到新的一天就会创建新文件去记录日志内容,这是不可更改的
```
#### **接口**  
```	
	/* 构造函数  basename:文件名  rollSize:文件大小达到多色Byte时创建新文件 */
	/* threadSafe:是否需要加锁写(默认加锁)  flushInterval 每多少秒刷新日志内容到磁盘(默认3秒) */
	/* 每多少次写操作就检查是否要回滚 */
	LogFile( const string& basename,   off_t rollSize,
              bool threadSafe = true, int flushInterval = 3,
          	  int checkEveryN = 1024 );
	
	/* 向文件中追加内容 */
	void append(const char* logline, int len);

	/* 刷新文件缓冲区内容到磁盘 */
	void flush();

	/* 回滚  即创建新的日志文件 但是同一秒內不会创建新文件的 */
	bool rollFile();

```

