#ifndef _JSMINSTREAM_H_
#define _JSMINSTREAM_H_

#include <iostream>

#include "realjsformatter.h"

class JSFormatterStream: public RealJSFormatter
{
public:
	JSFormatterStream(std::istream& input, std::ostream& output)
		:RealJSFormatter('\t', 1, false, false, false, false), in(input), out(output)
	{}
	
	JSFormatterStream(std::istream& input, std::ostream& output, 
		char chIndent, int nChPerInd, 
		bool bNLBracket, bool bIndentInEmpty)
		:RealJSFormatter(chIndent, nChPerInd, 
		false, false, 
		bNLBracket, bIndentInEmpty), in(input), out(output)
	{}

private:
	std::istream& in;
	std::ostream& out;

	virtual int GetChar();
	virtual void PutChar(int ch);
};

#endif
