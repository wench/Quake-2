// DisAPE.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "..\APElib\APEFILE.h"
#include "..\APElib\FILEREADER.h"
#include "..\APElib\BUFFEREADER.h"
int main()
{
	const char* fn =  "C:\\GOG Galaxy\\Games\\Anachronox\\anachrodox\\Anoxfiles\\anoxdata\\gameflow\\hephaestus.ape";
		auto f = fopen(fn, "rb");
	FILEREADER file =f;
	READER& r = file;
	uint8_t* buffer = new uint8_t[r.Length];
	r.read(r.Length, buffer);
	BUFFEREADER breader(buffer, r.Length);
	fclose(f);
	APEFILE af;

	af.Parse(breader);
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
