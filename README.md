# select-epoll
Linux网络编程中select/epoll的比较

编译命令：
gcc select-server.c -o select-server
gcc epoll-server.c -o epoll-server
gcc select-client.c -o client

运行命令：
./select-server 7838 2
./epoll-server 7838 2
./client 127.0.0.1 7838
