#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Window.H>


#include <FL/Fl_Button.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Output.H>

#include "Win32IOHelper.h"
class MainWindow :
	public Fl_Window
{
public:
	MainWindow(int x, int y, int w, int h);
	~MainWindow();

	Fl_Button* BTN_GetPhyDisks = nullptr;
	Fl_Input_Choice* InChoice_DiskList = nullptr;

	Fl_Input* Input_LBAidx = nullptr;
	Fl_Button* BTN_Read = nullptr;

	Fl_Box* Label_TextBox_DataRead = nullptr;
	Fl_Multiline_Output* TextBox_DataRead = nullptr;

};

void BTN_GetPhyDisks_Clicked(Fl_Widget*, MainWindow* window);

void BTN_Read_Clicked(Fl_Widget *, MainWindow * window);
