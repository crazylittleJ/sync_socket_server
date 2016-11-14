#ifndef H_WINSOCKETCLIENT
#define H_WINSOCKETCLIENT

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

namespace p32tools {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class CWinSC {
		private:
			WSADATA *wsaDataC = NULL;
			SOCKET ConnectSocket = INVALID_SOCKET;
			struct addrinfo *result = NULL,
				*ptr = NULL,
				*hints = NULL;
			char *sendbuf = "this is a test";
			char *recvbuf = NULL;// [DEFAULT_BUFLEN];
			int iResult;
			int recvbuflen = DEFAULT_BUFLEN;
			RichTextBox^ _scRtb;
			String^ _scPort;
		public:
			CWinSC(RichTextBox^ _rtb, String^ _port);
			~CWinSC();
		private:
		public:
			int StartClientService(int argc, char **argv);
	};
}
#endif
