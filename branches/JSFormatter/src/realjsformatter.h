/* realjsformatter.h
   2010-12-16

Copyright (c) 2010-2011 SUN Junwen

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
#include <queue>
#include <map>

using namespace std;

#define STRING_TYPE 0
#define OPER_TYPE 1
#define REGULAR_TYPE 2
#define COMMENT_TYPE_1 9 // ����ע��
#define COMMENT_TYPE_2 10 // ����ע��

/*
 * if-i, else-e, else if-i, 
 * for-f, do-d, while-w, 
 * switch-s, case-c, default-c
 * try-r, catch-h
 * {-BLOCK, (-BRACKET
 */ 
#define IF 'i'
#define ELSE 'e'
#define FOR 'f'
#define DO 'd'
#define WHILE 'w'
#define SWITCH 's'
#define CASE 'c'
#define TRY 'r'
#define CATCH 'h'
#define FUNCTION 'n'
#define BLOCK '{'
#define BRACKET '('
#define SQUARE '['
#define HELPER '\\'

struct TokenAndType
{
	string token;
	int type;
};

class RealJSFormatter
{
public:
	typedef stack<char> CharStack;
	typedef stack<bool> BoolStack;
	typedef queue<TokenAndType> TokenQueue;
	typedef map<string, char> StrCharMap;

	RealJSFormatter();

	void Go();

protected:
	// Should be implemented in derived class
	virtual inline int GetChar() = 0; // JUST get next char from input
	virtual inline void PutChar(int ch) = 0; // JUST put a char to output 

	void ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);
	void ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);

	void GetToken(bool init = false);
	void PutToken(const string& token, 
		const string& leftStyle = string(""), 
		const string& rightStyle = string("")); // Put a token out with style
	void PutString(const string& str);

	bool inline IsNormalChar(int ch);
	bool inline IsBlankChar(int ch);
	bool inline IsSingleOper(int ch);
	bool inline IsQuote(int ch);
	bool inline IsComment(); // Ҫ�����ж� charA, charB
	bool inline IsType(const string& str);

	void PrepareRegular(); // ͨ���ʷ��ж� tokenB ����
	void PreparePosNeg(); // ͨ���ʷ��ж� tokenB ������
	void PrepareTokenB();
	void PopMultiBlock(char previousStackTop);

	bool bRegular; // tokenB ʵ�������� GetToken �õ���������Ա״̬
	bool bPosNeg; // tokenB ʵ����������
	int charA;
	int charB;
	int tokenAType;
	int tokenBType;
	string tokenA;
	string tokenB;
	TokenQueue tokenBQueue;

	StrCharMap blockMap;
	CharStack blockStack; 
	int nIndents; // �������������ü��� blockStack��Ч������

	bool bNewLine; // ׼�����еı�־
	int nIfLikeBlock; // if, for, while, else if
	int nDoLikeBlock; // do, else
	int nSwitchBlock; // switch
	bool bBlockStmt; // block ������ʼ��
	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack brcNeedStack; // if ֮��ĺ��������

	bool bCommentPut; // �ո������ע��
};

#endif
