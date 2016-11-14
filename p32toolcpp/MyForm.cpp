#include "MyForm.h"


using namespace System;
using namespace System::Windows::Forms;
using namespace System::Threading;

[STAThread]
void Main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	p32tools::MyForm form;
	Application::Run(%form);
}

void p32tools::MyForm::ShowMsgEventHandler(String^ _str)
{
	this->richTextBox1->AppendText(_str);
}

System::Void p32tools::MyForm::button1_Click(System::Object ^ sender, System::EventArgs ^ e)
{
	Thread^ _t;
	SSPara^ _para = gcnew SSPara();
	_para->_rtb = this->richTextBox1;
	_para->_iResult = new int[1];
	_para->_btn = this->button1;
	_para->_strPort = gcnew String(this->SSSPort);
	_para->_strIP = gcnew String(this->SSSIP);
	if (!CWinSS::bSSOn) {
		this->button1->Enabled = false;
		this->button1->Text = gcnew String("Stop Socket Server");
		this->button1->BackColor = Color::DarkRed;
		if (!_t) {
			_t = gcnew Thread(gcnew ParameterizedThreadStart(CWinSS::StartListening));
		}

		while (_t->IsAlive) {
			CWinSS::StopListening(_para); 
			Thread::Sleep(1000);
		}
		CWinSS::bSSOn = true;  
		_t->Start(_para);
		this->button1->Enabled = true;
	}
	else {
		if (!_t) {
			_t = gcnew Thread(gcnew ParameterizedThreadStart(CWinSS::StartListening));
		}

		while (_t->IsAlive) {
			CWinSS::StopListening(_para);
			Thread::Sleep(1000);
		}
		CWinSS::bSSOn = false;
		this->richTextBox1->AppendText("===\nSocket Server has been stopped.\n===\n");
		this->button1->Text = gcnew String("Start Socket Server");
		this->button1->BackColor = Color::RoyalBlue;
		this->button1->Enabled = true;
	}
	return System::Void();
}

System::Void p32tools::MyForm::button2_Click(System::Object ^ sender, System::EventArgs ^ e)
{
	this->richTextBox1->Clear();
	return System::Void();
}

System::Void p32tools::MyForm::MyForm_Load(System::Object ^ sender, System::EventArgs ^ e)
{
	//XMLDocument doc;
	//doc.LoadFile("config.xml");
	//std::string tempStr;
	if (!_cwSS) {
		this->_cwSS = gcnew CWinSS();
	}
	if (!_ftpCtrl) {
		_ftpCtrl = new FtpCtrl();
	}
	try
	{
		TiXmlDocument *myDocument = new TiXmlDocument("config.xml");
		myDocument->LoadFile();
		TiXmlElement *RootElement = myDocument->RootElement();
		TiXmlElement *FirstNode = RootElement->FirstChildElement("ftpUrl");
		if (FirstNode) {
			this->SFtpUrl = gcnew String(FirstNode->GetText());
		}
		FirstNode = RootElement->FirstChildElement("ftpUsr");
		if (FirstNode) {
			this->SFtpUsr = gcnew String(FirstNode->GetText());
		}
		FirstNode = RootElement->FirstChildElement("ftpPwd");
		if (FirstNode) {
			this->SFtpPwd = gcnew String(FirstNode->GetText());
		}
		FirstNode = RootElement->FirstChildElement("ftpUsr");
		if (FirstNode) {
			this->SFtpPort = gcnew String(FirstNode->GetText());
		}
		FirstNode = RootElement->FirstChildElement("SSPort");
		if (FirstNode) {
			this->SSSPort = gcnew String(FirstNode->GetText());
		}
		FirstNode = RootElement->FirstChildElement("SSIP");
		if (FirstNode) {
			this->SSSIP = gcnew String(FirstNode->GetText());
		}
	}
	catch (System::Exception^ e)
	{
		return;
	}
}

System::Void p32tools::MyForm::button3_Click(System::Object ^ sender, System::EventArgs ^ e)
{
	String^ tempFtpUrl = gcnew String("ftp://127.0.0.1/log.txt");
	String^ tempUsr = gcnew String("root");
	String^ tempPwd = gcnew String("12345678");
	_openFileDialog1 = gcnew OpenFileDialog();
	if (_openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		/*System::IO::StreamReader ^ sr = gcnew
			System::IO::StreamReader(_openFileDialog1->FileName);*/
		this->textBox1->Text = _openFileDialog1->FileName;
		/*MessageBox::Show(sr->ReadToEnd());*/
		/*sr->Close();*/
		this->_ftpCtrl->UploadFile(_openFileDialog1->FileName, tempFtpUrl, tempUsr, tempPwd);
	}
	return System::Void();
}
