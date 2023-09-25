#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "WinSock2.h"
#include "iostream"
#include <fstream>
#pragma comment(lib,"ws2_32.lib") 
#define PORT 65035
#define BUFFER_SIZE 1024
using namespace std;

int main() {
	//加载套接字库,约定版本2.2
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	
	//建立客户端数据报套接字
	SOCKET sock_client;
	struct sockaddr_in server_addr;
	int addr_len = sizeof(struct sockaddr_in);
	char buffer[BUFFER_SIZE];
	
	//设置客户端套接字,使用IPv4地址族,UDP协议
	//事先设定服务器地址 130.33.40.2
	sock_client = socket(AF_INET, SOCK_DGRAM, 0);
	memset((void*)&server_addr, 0, addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("130.33.40.2");  //填写服务器IP地址
	
	// 发送连接请求
	sendto(sock_client, "Connect", strlen("Connect"), 0, (struct sockaddr*)&server_addr, addr_len);
	
	//接收连接成功消息
	int size = recvfrom(sock_client, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
	if (size == 0) {
		cout << "    连接失败\n";
		closesocket(sock_client);
		WSACleanup();
		return 0;
	}
	else {
		cout << "    服务器消息: " << buffer << endl;
	}
	
	// 发送申请服务端传输的文件的文件名
	string filename;
	cout << "请输入要发送的文件名：    ";
	cin >> filename;
	
	size = sendto(sock_client, filename.c_str(), filename.length(), 0, (struct sockaddr*)&server_addr, addr_len);
	if (size == SOCKET_ERROR) {
		cout << "    发送文件名失败,错误代码：" << WSAGetLastError() << endl;
		closesocket(sock_client);
		WSACleanup();
		return 0;
	}
	
	// 接收服务端将发送的文件的大小
	streampos fileSize;
	size = recvfrom(sock_client, (char*)&fileSize, sizeof(fileSize), 0, (struct sockaddr*)&server_addr, &addr_len);
	if (size == 0) {
		cout << "    与服务器断开连接\n";
		closesocket(sock_client);
		WSACleanup();
		return 0;
	}
	else {
		cout << "    接收到的文件大小为 :  " << fileSize << " bytes" << endl;
	}
	
	// 创建文件
	ofstream file("received_" + filename, ios::binary);
	
	// 接收文件内容
	int remainingSize = fileSize;
	while (remainingSize > 0) {
		size = recvfrom(sock_client, buffer, min(remainingSize, BUFFER_SIZE), 0, (struct sockaddr*)&server_addr, &addr_len);
		if (size == 0) {
			cout << "    对方已关闭连接！\n";
			break;
		}
		else if (size == SOCKET_ERROR) {
			cout << "    接收文件内容失败,错误代码：" << WSAGetLastError() << endl;
			break;
		}
		else {
			file.write(buffer, size);
			remainingSize -= size;
		}
	}
	
	file.close();
	cout << "    文件接收完成,保存为:  received_" << filename << endl;
	
	//关闭套接字和清理Winsock库的资源
	closesocket(sock_client);
	WSACleanup();
	return 0;
}

