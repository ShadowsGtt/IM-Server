//
// Created by shadowgao on 2019/5/22.
//
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>

void setDaemon()
{
    pid_t pid;

    pid = fork();    /* 创建子进程 */
    if (pid < 0)
        std::cerr << "set daemon failed\nserver exit..." << std::endl;
    if (pid > 0)     /* 父进程退出,子进程继续运行 */
    {
        _exit(0);
    }
    if (setsid() < 0)  /* 创建会话,此时子进程是该会话的头进程以及新进程组的进程组头进程,因此不在拥有控制终端 */
        std::cerr << "set daemon failed\nserver exit..." << std::endl;

    signal(SIGHUP, SIG_IGN);

    pid = fork();      /* 创建子进程的子进程 */
    if (pid < 0)
        std::cerr << "set daemon failed\nserver exit..." << std::endl;
    if (pid > 0)         /* 子进程退出,子进程的子进程继续运行 */
        _exit(0);
    close(0);
    close(1);
    close(2);
    open("/dev/null", O_RDONLY);     /* 将0号文件描述符(标准输入)重定向到/dev/null */
    open("/dev/null", O_RDWR);       /* 将1号文件描述符(标准输出)重定向到/dev/null */
    open("/dev/null", O_RDWR);       /* 将2号文件描述符(标准错误)定向到/dev/null */

    std::cout << "server running as daemon" << std::endl;
}
