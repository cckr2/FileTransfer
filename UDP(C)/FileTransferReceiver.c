#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#if defined(_WIN32)
#define sleep(sec)  Sleep((sec)*1000)
#endif
typedef unsigned long long u64;
u64 GetMicroCounter();

#define BUF_SIZE 1024

void printError(char * message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char **argv) {
	u64 start, end;
	WSADATA wsaData;
	struct sockaddr_in local_addr;
	SOCKET s;

	if (argc != 3){
		printf("Command parameter does not right.\n");
		exit(1);
	}
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) <0){
		printf("Socket Creat Error.\n");
		exit(1);
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(atoi(argv[1]));

	if (bind(s, (SOCKADDR *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
		printError("BIND ERROR");

	printf("This server is waiting... \n");

	struct sockaddr_in client_addr;
	int	len_addr = sizeof(client_addr);
	int totalBufferNum;
	int BufferNum;
	int readBytes;
	long file_size;
	long totalReadBytes;

	char buf[BUF_SIZE];

	start = GetMicroCounter();

	FILE * fp;
	fp = fopen(argv[2], "wb");

	readBytes = recvfrom(s, buf, BUF_SIZE, 0, &client_addr, &len_addr);
	file_size = atol(buf);
	totalBufferNum = file_size / BUF_SIZE + 1;
	BufferNum = 0;
	totalReadBytes = 0;

	while (BufferNum != totalBufferNum) {
		readBytes = recvfrom(s, buf, BUF_SIZE, 0, &client_addr, &len_addr);
		BufferNum++;
		totalReadBytes += readBytes;
		printf("In progress: %d/%dByte(s) [%d%%]\n", totalReadBytes, file_size, ((BufferNum * 100) / totalBufferNum));
		if (readBytes > 0) {
			fwrite(buf, sizeof(char), readBytes, fp);
			readBytes = sendto(s, buf, 10, 0, (SOCKADDR *)&client_addr, sizeof(client_addr));
		}
		if (readBytes == SOCKET_ERROR)
		{
			printError("ERROR");
			break;
		}
	}
	end = GetMicroCounter();
	printf("Elapsed Time (micro seconds) : %d", end - start);

	closesocket(s);
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
