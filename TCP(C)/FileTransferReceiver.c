#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN	
#pragma comment(lib, "Ws2_32.lib")
#define sleep(sec)  Sleep((sec)*1000)
#endif
typedef unsigned long long u64;
u64 GetMicroCounter();
#define BUF_SIZE 1024


int main(int argc, char **argv) {
	u64 start, end;
	WSADATA wsaData;
	struct sockaddr_in local_addr;
	SOCKET	s_listen;

	if (argc != 3){
		printf("Command parameter does not right.\n");
		exit(1);
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Socket Creat Error.\n");
		exit(1);
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(atoi(argv[1]));

	if (bind(s_listen, (SOCKADDR *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR){
		printf("Socket Bind Error.\n");
		exit(1);
	}

	if (listen(s_listen, 5) == SOCKET_ERROR){
		printf("Socket Listen Error.\n");
		exit(1);
	}


	printf("This server is listening... \n");

	struct sockaddr_in client_addr;
	int	len_addr = sizeof(client_addr);
	SOCKET	s_accept;
	int totalBufferNum;
	int BufferNum;
	int readBytes;
	long file_size;
	long totalReadBytes;


	char buf[BUF_SIZE];

	FILE * fp;
	fp = fopen(argv[2], "wb");

	s_accept = accept(s_listen, (SOCKADDR *)&client_addr, &len_addr);
	if (s_accept) {
		start = GetMicroCounter();
		printf("Connection Request from Client [IP:%s, Port:%d] has been Accepted\n",
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		readBytes = recv(s_accept, buf, BUF_SIZE, 0);
		file_size = atol(buf);
		totalBufferNum = file_size / BUF_SIZE + 1;
		BufferNum = 0;
		totalReadBytes = 0;

		while (BufferNum != totalBufferNum) {
			readBytes = recv(s_accept, buf, BUF_SIZE, 0);
			BufferNum++;
			totalReadBytes += readBytes;
			printf("In progress: %d/%dByte(s) [%d%%]\n", totalReadBytes, file_size, ((BufferNum * 100) / totalBufferNum));
			fwrite(buf, sizeof(char), readBytes, fp);

			if (readBytes == SOCKET_ERROR) {
				printf("File Receive Errpr");
				exit(1);
			}
		}
		closesocket(s_accept);
		end = GetMicroCounter();
		printf("time: %d second(s)", end - start);
	}
	else {
		printf("File Accept Error");
	}

	closesocket(s_listen);
	WSACleanup();

	return 0;
}

u64 GetMicroCounter()
{
	u64 Counter;

#if defined(_WIN32)
	u64 Frequency;
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER *)&Counter);
	Counter = 1000000 * Counter / Frequency;
#elif defined(__linux__) 
	struct timeval t;
	gettimeofday(&t, 0);
	Counter = 1000000 * t.tv_sec + t.tv_usec;
#endif

	return Counter;
}
