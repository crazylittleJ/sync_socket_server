#ifndef H_FTPCTRL
#define H_FTPCTRL
#using <System.dll>
namespace p32tools {
	using namespace System;
	using namespace System::Net;
	using namespace System::Threading;
	using namespace System::IO;

	public ref class FtpState
	{
	private:
		ManualResetEvent^ wait;
		FtpWebRequest^ request;
		String^ fileName;
		Exception^ operationException;
		String^ status;

	public:
		FtpState()
		{
			wait = gcnew ManualResetEvent(false);
		}

		property ManualResetEvent^ OperationComplete
		{
			ManualResetEvent^ get()
			{
				return wait;
			}

		}

		property FtpWebRequest^ Request
		{
			FtpWebRequest^ get()
			{
				return request;
			}

			void set(FtpWebRequest^ value)
			{
				request = value;
			}

		}

		property String^ FileName
		{
			String^ get()
			{
				return fileName;
			}

			void set(String^ value)
			{
				fileName = value;
			}

		}

		property Exception^ OperationException
		{
			Exception^ get()
			{
				return operationException;
			}

			void set(Exception^ value)
			{
				operationException = value;
			}

		}

		property String^ StatusDescription
		{
			String^ get()
			{
				return status;
			}

			void set(String^ value)
			{
				status = value;
			}

		}
	};

	public class FtpCtrl
	{
		public:
			FtpCtrl();
			~FtpCtrl();
		private:
		public:
			static bool DeleteFileOnServer(Uri^ serverUri, String^ _ftpUsr, String^ _ftpPwd);
			static bool DisplayFileFromServer(Uri^ serverUri, String^ _ftpUsr, String^ _ftpPwd);
			/// <summary>
			/// Methods to upload file to FTP Server
			/// </summary>
			/// <param name="_FileName">local source file name</param>
			/// <param name="_UploadPath">Upload FTP path including Host name</param>
			/// <param name="_FTPUser">FTP login username</param>
			/// <param name="_FTPPass">FTP login password</param>
			static bool UploadFile(System::String ^_FileName, System::String ^_UploadPath, System::String ^_FTPUser, System::String ^_FTPPass);
		public:

			// Command line arguments are two strings:
			// 1. The url that is the name of the file being uploaded to the server.
			// 2. The name of the file on the local machine.
			//
			static void MainProcedure();

	private:
		static void EndGetStreamCallback(IAsyncResult^ ar);

		// The EndGetResponseCallback method  
		// completes a call to BeginGetResponse.
		static void EndGetResponseCallback(IAsyncResult^ ar);
	};


}

#endif