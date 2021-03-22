#include "MainWindow.h"

#include <locale>
#include <codecvt>
#include <algorithm>
MainWindow::MainWindow(int x, int y, int w, int h) :Fl_Window(x, y, w, h)
{
	int rowY = 5;

	this->BTN_GetPhyDisks = new Fl_Button(5, rowY, 200, 30, "Get Physical Disks");
	this->BTN_GetPhyDisks->callback((Fl_Callback*)BTN_GetPhyDisks_Clicked, this);

	this->Label_TextBox_DataRead = new Fl_Box(320, rowY, 200, 30, "Data Read:");
	this->Label_TextBox_DataRead->align(FL_ALIGN_LEFT);
	this->TextBox_DataRead = new Fl_Multiline_Output(250, rowY + 30, 800, 600);
	this->TextBox_DataRead->align(FL_ALIGN_WRAP);
	this->TextBox_DataRead->type(FL_MULTILINE_OUTPUT_WRAP | FL_MULTILINE_OUTPUT);
	this->TextBox_DataRead->wrap(1);

	rowY = 40;

	this->InChoice_DiskList = new Fl_Input_Choice(5, rowY, 200, 30);

	rowY = 80;

	this->Input_LBAidx = new Fl_Input(5, rowY, 145, 30);

	this->BTN_Read = new Fl_Button(150, rowY, 50, 30, "Read");
	this->BTN_Read->callback((Fl_Callback*)BTN_Read_Clicked, this);
}

MainWindow::~MainWindow()
{
}

void BTN_GetPhyDisks_Clicked(Fl_Widget *, MainWindow * window)
{
	using namespace std;
	if (!Win32IOHelper::instance->WMI_getPhyDisks()) //if success
	{
		window->InChoice_DiskList->clear();

		auto list = Win32IOHelper::instance->getDiskList();
		for (auto v : list)
		{
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;

			string strModel = converter.to_bytes(v.first);
			string strPhyAddr = converter.to_bytes(v.second);
			string btnStr = string(strPhyAddr + " (" + strModel + ")");
			window->InChoice_DiskList->add(btnStr.c_str());
		}
	}

}

void BTN_Read_Clicked(Fl_Widget*, MainWindow * window)
{
	using namespace std;
	string phyAddr = window->InChoice_DiskList->value();
	phyAddr = phyAddr.substr(0, phyAddr.find(' '));
	vector<uint8_t> dataOut;

	string LBAstr = string(window->Input_LBAidx->value());
	int LBA = 23;
	if (!LBAstr.empty()&&LBAstr.find_first_not_of("0123456789") == string::npos)
	{
		LBA = stol(LBAstr);
	}

	
	if (Win32IOHelper::instance->Win32_ReadOneLBA(phyAddr, dataOut, LBA))
	{
		string strFromLBA;
		strFromLBA.insert(strFromLBA.begin(), dataOut.begin(), dataOut.end());
		window->TextBox_DataRead->value(strFromLBA.c_str());
	}
	else
	{
		MessageBox(nullptr, "Create file failed", "Error", MB_OK);
	}
}