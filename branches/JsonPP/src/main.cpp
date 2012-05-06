// JSMinConsole.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "jsonpp.h"
#include "jsonFileProc.h"

using namespace std;
using namespace sunjwbase;

int main(int argc, char* argv[])
{
	if(argc == 3)
	{
		char* inputFile = argv[1];
		char* outputFile = argv[2];

		try
		{
			tstring fileName = strtotstr(string(inputFile));
			JsonFileProc jfp(fileName);
			jfp.m_debugOutput = true;

			JsonValue jsonValue;
			jfp.GetJsonValue(jsonValue);

			/*JsonUnsortedMap& jmap = jsonValue.GetMapValue()["web-app"].GetMapValue();
			JsonValue& jval = jmap["taglib"];
			JsonValue jvalNew;
			jvalNew.SetValueType(JsonValue::ARRAY_VALUE);
			jvalNew.ArrayPut(JsonValue("test1"));
			jvalNew.ArrayPut(JsonValue("test2"));
			jvalNew.ArrayPut(JsonValue("test3"));
			jval.GetMapValue().push_front(JsonMapPair("xxx2", jvalNew));*/

			jfp.Save(jsonValue);

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
