// JSMinConsole.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept> 

#include "jsfStream.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(argc == 3)
	{
		char* inputFile = argv[1];
		char* outputFile = argv[2];

		ifstream inFileStream(inputFile);
		ofstream outFileStream(outputFile);

		ostringstream outStrStream;

		try
		{
			JSFormatterStream jsf(inFileStream, outStrStream, '\t', 1, false);
			jsf.Go();

			string output = outStrStream.str();
			
			// ������ļ�
			outFileStream << output;

			outFileStream.close();
			inFileStream.close();

			cout << "Done" << endl;
		}
		catch(runtime_error ex)
		{
			cout << "Error: " << ex.what() << endl;
		}
	}
	else
	{
		cout << "Usage: jsformatter [input file] [output file]" << endl;
	}
    
	
    return 0;
}

