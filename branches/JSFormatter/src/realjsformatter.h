/* realjsformatter.h
   2010-12-16

Copyright (c) 2010 SUN Junwen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef _REAL_JSFORMATTER_H_
#define _REAL_JSFORMATTER_H_
#include <string>
#include <stack>

using namespace std;

#define STRING_TYPE 0
#define OPER_TYPE 1
#define REGULAR_TYPE 2
#define COMMENT_TYPE_1 9 // ����ע��
#define COMMENT_TYPE_2 10 // ����ע��

class RealJSFormatter
{
public:
	typedef stack<char> CharStack;

	RealJSFormatter():
		bRegular(false),
		nIndents(0),
		bNewLine(false),
		bIfLikeBlock(false),
		bDoLikeBlock(false),
		bSwitchBlock(false),
		bBracket(false)
	{}

	void Go();

private:
	// Should be implemented in derived class
	virtual inline int GetChar() = 0; // JUST get next char from input
	virtual inline void PutChar(int ch) = 0; // JUST put a char to output 

	void GetToken(bool init = false);
	void PutToken(const string& token, 
		const string& leftStyle = string(""), 
		const string& rightStyle = string("")); // Put a token out with style
	void PutString(const string& str);

	bool IsNormalChar(int ch);
	bool IsBlankChar(int ch);
	bool IsSingleOper(int ch);
	bool IsQuote(int ch);
	bool IsComment(); // Ҫ�����ж� charA, charB
	bool IsType(const string& str);

	int charA;
	int charB;
	bool bRegular; // GetToken �õ���Ψһһ����Ա״̬
	string tokenA;
	string tokenB;
	int tokenAType;
	int tokenBType;

	CharStack blockStack;
	int nIndents; // �������������ü��� blockStack��Ч������

	bool bNewLine; // ׼�����еı�־
	bool bIfLikeBlock; // if, for, while, else if
	bool bDoLikeBlock; // do, else
	bool bSwitchBlock; // switch
	bool bBracket; // if ֮��ĺ��������
};

#endif
