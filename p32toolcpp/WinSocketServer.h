#ifndef H_WINSOCKETSERVER
#define H_WINSOCKETSERVER

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <msclr\marshal.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6666"

namespace p32tools {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace msclr::interop;

	public ref class SSPara{
		public:
			SSPara();
			~SSPara();
		public:
			int *_iResult;
			String^ _strPort;
			Button^ _btn;
			RichTextBox^ _rtb;
			String^ _strIP;
	};

	public ref class CWinSS {
		public:
			static bool bSSOn = false;
		private:
			static SOCKET ListenSocket;
			static SOCKET ClientSocket;
			static SSPara^ para;
		public:
			CWinSS();
			~CWinSS();
		private:
		public:
			static void StartListening(Object^ obj);
			static void StopListening(Object^ obj);
		private:
			// This delegate enables asynchronous calls for setting
			// the text property on a TextBox control.
			delegate void SetTextDelegate(String^ text);
		public:
			static void Set_RTB_Text(String^ text)
			{
				// InvokeRequired required compares the thread ID of the
				// calling thread to the thread ID of the creating thread.
				// If these threads are different, it returns true.
				if (CWinSS::para->_rtb->InvokeRequired)
				{
					SetTextDelegate^ d =
						gcnew SetTextDelegate(&CWinSS::Set_RTB_Text);
					CWinSS::para->_rtb->Invoke(d, gcnew array<Object^> { text });
				}
				else
				{
					CWinSS::para->_rtb->AppendText(text);
				}
			}
	};
}
#endif