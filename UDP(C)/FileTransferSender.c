#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 1024
typedef unsigned long long u64;
u64 GetMicroCounter();

int main(int argc, char **argv) {
	WSADATA wsaData;
	struct sockaddr_in server_addr;
	SOCKET s;
	u64 start, end;

	if (argc != 4){
		printf("Command parameter does not right.\n");
		exit(1);
	}
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Socket Creat Error.\n");
		exit(1);
	}

	printf("File Send Start");

	int totalBufferNum;
	int BufferNum;
	int sendBytes;
	int file_size;	// total file size
	int totalSendBytes;  // received file size

	char buf[BUF_SIZE];

	FILE *fp;
	fp = fopen(argv[3], "rb");
	if (fp == NULL){
		printf("File not Exist");
		exit(1);
	}
	start = GetMicroCounter();
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	totalBufferNum = file_size / sizeof(buf) + 1;
	fseek(fp, 0, SEEK_SET);
	BufferNum = 0;
	totalSendBytes = 0;

	_snprintf(buf, sizeof(buf), "%d", file_size);
	sendBytes = sendto(s, buf, sizeof(char) * 1024, 0, (SOCKADDR *)&server_addr, sizeof(server_addr));

	while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fp))>0) {
		sendto(s, buf, sendBytes, 0, (SOCKADDR *)&server_addr, sizeof(server_addr));
		BufferNum++;
		totalSendBytes += sendBytes;
		printf("In progress: %d/%dByte(s) [%d%%]\n", totalSendBytes, file_size, ((BufferNum * 100) / totalBufferNum));
		while (recvfrom(s, buf, BUF_SIZE, 0, NULL, NULL) != 10)
		{
			sendto(s, buf, sendBytes, 0, (SOCKADDR *)&server_addr, sizeof(server_addr));
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
