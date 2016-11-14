#include "FtpCtrl.h"

using namespace System;
using namespace System::Net;
using namespace System::Threading;
using namespace System::IO;

p32tools::FtpCtrl::FtpCtrl()
{
}

p32tools::FtpCtrl::~FtpCtrl()
{
}

bool p32tools::FtpCtrl::DeleteFileOnServer(Uri^ serverUri,String^ _ftpUsr,String^ _ftpPwd)
{
	// The serverUri parameter should use the ftp:// scheme.
	// It contains the name of the server file that is to be deleted.
	// Example: ftp://contoso.com/someFile.txt.
	// 
	if (serverUri->Scheme != Uri::UriSchemeFtp)
	{
		return false;
	}

	// Get the object used to communicate with the server.
	FtpWebRequest^ request = dynamic_cast<FtpWebRequest^>(WebRequest::Create(serverUri));
	request->Credentials = gcnew NetworkCredential(_ftpUsr, _ftpPwd);
	request->Method = WebRequestMethods::Ftp::DeleteFile;
	FtpWebResponse^ response = dynamic_cast<FtpWebResponse^>(request->GetResponse());
	Console::WriteLine("Delete status: {0}", response->StatusDescription);
	response->Close();
	return true;
}

bool p32tools::FtpCtrl::DisplayFileFromServer(Uri^ serverUri,String^ _ftpUsr,String^ _ftpPwd)
{
	// The serverUri parameter should start with the ftp:// scheme.
	if (serverUri->Scheme != Uri::UriSchemeFtp)
	{
		return false;
	}

	// Get the object used to communicate with the server.
	WebClient^ request = gcnew WebClient;

	// This example assumes the FTP site uses anonymous logon.
	//request->Credentials = gcnew NetworkCredential("anonymous", "janeDoe@contoso.com");
	request->Credentials = gcnew NetworkCredential(_ftpUsr, _ftpPwd);
	try
	{
		array<Byte>^newFileData = request->DownloadData(serverUri->ToString());
		String^ fileString = System::Text::Encoding::UTF8->GetString(newFileData);
		Console::WriteLine(fileString);
	}
	catch (WebException^ e)
	{
		Console::WriteLine(e);
	}

	return true;
}

/// <summary>
/// Methods to upload file to FTP Server
/// </summary>
/// <param name="_FileName">local source file name</param>
/// <param name="_UploadPath">Upload FTP path including Host name</param>
/// <param name="_FTPUser">FTP login username</param>
/// <param name="_FTPPass">FTP login password</param>
bool p32tools::FtpCtrl::UploadFile(System::String ^_FileName, System::String ^_UploadPath, System::String ^_FTPUser, System::String ^_FTPPass)
{
	System::IO::FileInfo ^_FileInfo = gcnew System::IO::FileInfo(_FileName);
	// Create FtpWebRequest object from the Uri provided
	System::Net::FtpWebRequest ^_FtpWebRequest = safe_cast<System::Net::FtpWebRequest^>(System::Net::FtpWebRequest::Create(gcnew Uri(_UploadPath)));
	// Provide the WebPermission Credintials
	_FtpWebRequest->Credentials = gcnew System::Net::NetworkCredential(_FTPUser, _FTPPass);
	// By default KeepAlive is true, where the control connection is not closed
	// after a command is executed.
	_FtpWebRequest->KeepAlive = false;
	// set timeout for 20 seconds
	_FtpWebRequest->Timeout = 20000;
	// Specify the command to be executed.
	_FtpWebRequest->Method = WebRequestMethods::Ftp::UploadFile;
	// Specify the data transfer type.
	_FtpWebRequest->UseBinary = true;
	// Notify the server about the size of the uploaded file
	_FtpWebRequest->ContentLength = _FileInfo->Length;
	// The buffer size is set to 2kb
	int buffLength = 2048;
	array<System::Byte> ^buff = gcnew array<System::Byte>(buffLength);
	// Opens a file stream (System.IO.FileStream) to read the file to be uploaded
	System::IO::FileStream ^_FileStream = _FileInfo->OpenRead();
	try
	{
		// Stream to which the file to be upload is written
		System::IO::Stream ^_Stream = _FtpWebRequest->GetRequestStream();
		// Read from the file stream 2kb at a time
		int contentLen = _FileStream->Read(buff, 0, buffLength);
		// Till Stream content ends
		while (contentLen != 0)
		{
				// Write Content from the file stream to the FTP Upload Stream
				_Stream->Write(buff, 0, contentLen);
				contentLen = _FileStream->Read(buff, 0, buffLength);
		}
		// Close the file stream and the Request Stream
		_Stream->Close();
		delete _Stream;
		_FileStream->Close();
		delete _FileStream;
	}
	catch (Exception ^ex)
	{
		return false;
	}
	return true;
}

// Command line arguments are two strings:
// 1. The url that is the name of the file being uploaded to the server.
// 2. The name of the file on the local machine.
//
void p32tools::FtpCtrl::MainProcedure()
{
	array<String^>^args = Environment::GetCommandLineArgs();

	// Create a Uri instance with the specified URI string.
	// If the URI is not correctly formed, the Uri constructor
	// will throw an exception.
	ManualResetEvent^ waitObject;
	Uri^ target = gcnew Uri(args[1]);
	String^ fileName = args[2];
	FtpState^ state = gcnew FtpState;
	FtpWebRequest ^ request = dynamic_cast<FtpWebRequest^>(WebRequest::Create(target));
	request->Method = WebRequestMethods::Ftp::UploadFile;

	// This example uses anonymous logon.
	// The request is anonymous by default; the credential does not have to be specified. 
	// The example specifies the credential only to
	// control how actions are logged on the server.
	request->Credentials = gcnew NetworkCredential("anonymous", "janeDoe@contoso.com");

	// Store the request in the object that we pass into the
	// asynchronous operations.
	state->Request = request;
	state->FileName = fileName;

	// Get the event to wait on.
	waitObject = state->OperationComplete;

	// Asynchronously get the stream for the file contents.
	request->BeginGetRequestStream(gcnew AsyncCallback(EndGetStreamCallback), state);

	// Block the current thread until all operations are complete.
	waitObject->WaitOne();

	// The operations either completed or threw an exception.
	if (state->OperationException != nullptr)
	{
		throw state->OperationException;
	}
	else
	{
		Console::WriteLine("The operation completed - {0}", state->StatusDescription);
	}
}


void p32tools::FtpCtrl::EndGetStreamCallback(IAsyncResult^ ar)
{
	FtpState^ state = dynamic_cast<FtpState^>(ar->AsyncState);
	Stream^ requestStream = nullptr;

	// End the asynchronous call to get the request stream.
	try
	{
		requestStream = state->Request->EndGetRequestStream(ar);

		// Copy the file contents to the request stream.
		const int bufferLength = 2048;
		array<Byte>^buffer = gcnew array<Byte>(bufferLength);
		int count = 0;
		int readBytes = 0;
		FileStream^ stream = File::OpenRead(state->FileName);
		do
		{
			readBytes = stream->Read(buffer, 0, bufferLength);
			requestStream->Write(buffer, 0, bufferLength);
			count += readBytes;
		} while (readBytes != 0);
		Console::WriteLine("Writing {0} bytes to the stream.", count);

		// IMPORTANT: Close the request stream before sending the request.
		requestStream->Close();

		// Asynchronously get the response to the upload request.
		state->Request->BeginGetResponse(gcnew AsyncCallback(EndGetResponseCallback), state);
	}
	// Return exceptions to the main application thread.
	catch (Exception^ e)
	{
		Console::WriteLine("Could not get the request stream.");
		state->OperationException = e;
		state->OperationComplete->Set();
		return;
	}
}

// The EndGetResponseCallback method  
// completes a call to BeginGetResponse.
void p32tools::FtpCtrl::EndGetResponseCallback(IAsyncResult^ ar)
{
	FtpState^ state = dynamic_cast<FtpState^>(ar->AsyncState);
	FtpWebResponse ^ response = nullptr;
	try
	{
		response = dynamic_cast<FtpWebResponse^>(state->Request->EndGetResponse(ar));
		response->Close();
		state->StatusDescription = response->StatusDescription;

		// Signal the main application thread that 
		// the operation is complete.
		state->OperationComplete->Set();
	}
	// Return exceptions to the main application thread.
	catch (Exception^ e)
	{
		Console::WriteLine("Error getting response.");
		state->OperationException = e;
		state->OperationComplete->Set();
	}
}
