#include <iostream>
#include <FL/Fl.H>
#include "MainWindow.h"

int main(int argc, char** argv)
{

	MainWindow mainWindow(100, 100, 1280, 720);
	mainWindow.show();
	//mainWindow.init();	
	Fl::run();


	return 0;
}