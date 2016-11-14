#include "WinSocketClient.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

p32tools::CWinSC::CWinSC(RichTextBox^ _rtb, String^ _port)
{
	this->_scRtb = _rtb;
	this->_scPort = gcnew String(_port);
	this->hints = new struct addrinfo();
	this->wsaDataC = new WSADATA();
	this->recvbuf = new char[DEFAULT_BUFLEN];
}

p32tools::CWinSC::~CWinSC()
{
	if (this->hints)
		delete this->hints;
	if (this->wsaDataC)
		delete this->wsaDataC;
	if (this->recvbuf)
		delete[] this->recvbuf;
}

int p32tools::CWinSC::StartClientService(int argc, char **argv)
{
	String^ tempMsg;
	this->ConnectSocket = INVALID_SOCKET;
	if (this->result)
		free(this->result);
	this->result = NULL;
	if(this->ptr)
		free(this->ptr);
	this->ptr = NULL;
	if (this->hints)
		free(this->hints);
	this->hints = NULL;
	if (this->recvbuf)
		delete[] this->recvbuf;
	this->recvbuf = NULL;
	this->recvbuf = new char[DEFAULT_BUFLEN];
	this->recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	if (argc != 2) {
		tempMsg = String::Format("usage: {0} server-name\n", gcnew String(argv[0]));
		//printf("usage: %s server-name\n", argv[0]);
		this->_scRtb->AppendText(tempMsg);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), wsaDataC);
	if (iResult != 0) {
		tempMsg = String::Format("usage: {0} server-name\n", iResult);
		//printf("WSAStartup failed with error: %d\n", iResult);
		this->_scRtb->AppendText(tempMsg);
		return 1;
	}

	ZeroMemory((void*)hints, sizeof(hints));
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, hints, (PADDRINFOA*)result);
	if (iResult != 0) {
		tempMsg = String::Format("getaddrinfo failed with error: %d\n", iResult);
		//printf("getaddrinfo failed with error: %d\n", iResult);
		this->_scRtb->AppendText(tempMsg);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			tempMsg = String::Format("socket failed with error: %ld\n", WSAGetLastError());
			//printf("socket failed with error: %ld\n", WSAGetLastError());
			this->_scRtb->AppendText(tempMsg);
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result); result = NULL;

	if (ConnectSocket == INVALID_SOCKET) {
		tempMsg = String::Format("Unable to connect to server!\n");
		//printf("Unable to connect to server!\n");
		this->_scRtb->AppendText(tempMsg);
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		tempMsg = String::Format("send failed with error: %d\n", WSAGetLastError());
		//printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		this->_scRtb->AppendText(tempMsg);
		WSACleanup();
		return 1;
	}
	tempMsg = String::Format("send failed with error: %d\n", WSAGetLastError());
	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
