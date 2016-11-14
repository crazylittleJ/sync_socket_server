#include "WinSocketServer.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Threading;


p32tools::CWinSS::CWinSS()
{
}

p32tools::CWinSS::~CWinSS()
{
}


void p32tools::CWinSS::StartListening(Object^ obj)
{

	WSADATA wsaData;
	int iResult;

	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	String^ _tempStr;

	struct addrinfo *result = NULL;
	struct addrinfo *p = NULL;
	struct addrinfo hints;
	marshal_context mc;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	if (CWinSS::para) {
		if (CWinSS::para->_iResult) {
			delete[] CWinSS::para->_iResult;
			CWinSS::para->_iResult = new int[1];
		}
	}
	CWinSS::para = (SSPara^)obj;
	
	// Initialize Winsock
	_tempStr = String::Format("Initialize Winsock...\n");
	Set_RTB_Text(_tempStr);
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		_tempStr = String::Format("WSAStartup failed with error: %d\n", iResult);
		Set_RTB_Text(_tempStr);
		//printf("WSAStartup failed with error: %d\n", iResult);
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	_tempStr = String::Format("Resolve the server address and port...\n");
	Set_RTB_Text(_tempStr);

	CWinSS::para->_strIP = gcnew String("127.0.0.1"); CWinSS::para->_strPort = gcnew String("7755");
	PCSTR sP1 = mc.marshal_as<PCSTR>(CWinSS::para->_strIP);
	PCSTR sP2 = mc.marshal_as<PCSTR>(CWinSS::para->_strPort);
	iResult = getaddrinfo(sP1, sP2, &hints, &result);
	if (iResult != 0) {
		_tempStr = String::Format("getaddrinfo failed with error: %d\n", iResult);
		Set_RTB_Text(_tempStr);
		//printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}
	for (p = result; p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
		char ipstringbuffer[46];
		DWORD ipbufferlength = 46;

		// 取得本身位址的指標，
		// 在 IPv4 與 IPv6 中的欄位不同：
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = inet_ntoa(ipv4->sin_addr);
			//addr = &(ipv4->sin_addr);
			ipver = "IPv4";
			_tempStr = String::Format("{0}:{1},Port:{2}\n", gcnew String(ipver), gcnew String((char*)addr), gcnew String(CWinSS::para->_strPort));
			Set_RTB_Text(_tempStr);
		}
		else { // IPv6
			LPSOCKADDR sockaddr_ip = (LPSOCKADDR)p->ai_addr;
			int iRetval = WSAAddressToString(sockaddr_ip, (DWORD)p->ai_addrlen, NULL,
				(LPWSTR)ipstringbuffer, &ipbufferlength);
			//struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			//addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
			_tempStr = String::Format("{0}:{1},Port:{2}\n", gcnew String(ipver), gcnew String((char*)ipstringbuffer), gcnew String(DEFAULT_PORT));
			Set_RTB_Text(_tempStr);
		}
	}
	
	

	// Create a SOCKET for connecting to server
	_tempStr = String::Format("Create a SOCKET for connecting to server...\n");
	Set_RTB_Text(_tempStr);
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		_tempStr = String::Format("socket failed with error: %ld\n", WSAGetLastError());
		Set_RTB_Text(_tempStr);
		//printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}

	// Setup the TCP listening socket
	_tempStr = String::Format("Setup the TCP listening socket...\n");
	Set_RTB_Text(_tempStr);
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		_tempStr = String::Format("bind failed with error: %d\n", WSAGetLastError());
		Set_RTB_Text(_tempStr);
		//printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}

	freeaddrinfo(result);

	_tempStr = String::Format("listening...\n");
	Set_RTB_Text(_tempStr);
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		_tempStr = String::Format("listen failed with error: %d\n", WSAGetLastError());
		Set_RTB_Text(_tempStr);
		//printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}


	// Accept a client socket
	_tempStr = String::Format("Accept a client socket...\n");
	Set_RTB_Text(_tempStr);
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		_tempStr = String::Format("accept failed with error: %d\n", WSAGetLastError());
		Set_RTB_Text(_tempStr);
		//printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}

	_tempStr = String::Format(" No longer need server socket\n");
	Set_RTB_Text(_tempStr);
	// No longer need server socket
	closesocket(ListenSocket);


	_tempStr = String::Format("Receive until the peer shuts down the connection\n");
	Set_RTB_Text(_tempStr);
	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			_tempStr = String::Format("Bytes received: %d\n", iResult);
			Set_RTB_Text(_tempStr);
			//printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				_tempStr = String::Format("send failed with error: %d\n", WSAGetLastError());
				Set_RTB_Text(_tempStr);
				//printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				CWinSS::para->_iResult[0] = 1;
				return;// return 1;
			}
			_tempStr = String::Format("Bytes sent: %d\n", iSendResult);
			Set_RTB_Text(_tempStr);
			//printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0) {
			_tempStr = String::Format("Connection closing...\n");
			Set_RTB_Text(_tempStr);
			//printf("Connection closing...\n");
		}
		else {
			_tempStr = String::Format("recv failed with error: %d\n", WSAGetLastError());
			Set_RTB_Text(_tempStr);
			//printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			CWinSS::para->_iResult[0] = 1;
			return;// return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		CWinSS::para->_iResult[0] = 1;
		return;// return 1;
	}


	WSACleanup();
	CWinSS::para->_iResult[0] = 0;
	return;// return 0;
}

void p32tools::CWinSS::StopListening(Object^ obj)
{
	closesocket(ListenSocket); ListenSocket = INVALID_SOCKET;
	closesocket(ClientSocket); ClientSocket = INVALID_SOCKET;
	WSACleanup();
	CWinSS::bSSOn = false;
	return ;
}

p32tools::SSPara::SSPara()
{
	
}

p32tools::SSPara::~SSPara()
{
	
}
