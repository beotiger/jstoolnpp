/* realjsformatter.h
   2010-12-16

Copyright (c) 2010-2011 SUN Junwen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _REAL_JSFORMATTER_H_
#define _REAL_JSFORMATTER_H_
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <set>

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
#define ASSIGN '='
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
	typedef set<string> StrSet;

	RealJSFormatter();
	RealJSFormatter(char chIndent, int nChPerInd);
	RealJSFormatter(bool bSkipCR, bool bPutCR);
	RealJSFormatter(char chIndent, int nChPerInd, bool bSkipCR, bool bPutCR, bool bNLBracket);

	virtual ~RealJSFormatter()
	{}

	void Go();

	static string Trim(const string& str);
	static string TrimSpace(const string& str);
	static string TrimRightSpace(const string& str);
	void StringReplace(string& strBase, const string& strSrc, const string& strDes);

	bool m_debugOutput;

protected:
	void Init();

	// Should be implemented in derived class
	virtual int GetChar() = 0; // JUST get next char from input
	virtual void PutChar(int ch) = 0; // JUST put a char to output 

	void ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);
	void ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);

	void GetToken(bool init = false);
	void PutToken(const string& token, 
		const string& leftStyle = string(""), 
		const string& rightStyle = string("")); // Put a token out with style
	void PutString(const string& str);
	void PutLineBuffer();

	bool inline IsNormalChar(int ch);
	bool inline IsNumChar(int ch);
	bool inline IsBlankChar(int ch);
	bool inline IsSingleOper(int ch);
	bool inline IsQuote(int ch);
	bool inline IsComment(); // Ҫ�����ж� charA, charB

	void PrepareRegular(); // ͨ���ʷ��ж� tokenB ����
	void PreparePosNeg(); // ͨ���ʷ��ж� tokenB ������
	void PrepareTokenB();
	void PopMultiBlock(char previousStackTop);

	int m_tokenCount;
	time_t m_startTime;
	time_t m_endTime;

	string m_strBeforeReg; // �ж�����ʱ������ǰ����Գ��ֵ��ַ�

	bool m_bRegular; // tokenB ʵ�������� GetToken �õ���������Ա״̬
	bool m_bPosNeg; // tokenB ʵ����������
	int m_charA;
	int m_charB;
	int m_tokenAType;
	int m_tokenBType;
	string m_tokenA;
	string m_tokenB;
	TokenQueue m_tokenBQueue;

	int m_nLineIndents;
	string m_lineBuffer;

	StrSet m_specKeywordSet; // ����Ҫ�������ŵĹؼ��ּ���
	StrCharMap m_blockMap;
	CharStack m_blockStack; 
	int m_nIndents; // �������������ü��� blockStack��Ч������

	bool m_bNewLine; // ׼�����еı�־
	bool m_bBlockStmt; // block ������ʼ��
	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack m_brcNeedStack; // if ֮��ĺ��������
	bool m_bAssign;
	bool m_bEmptyBracket; // �� {}

	bool m_bCommentPut; // �ո������ע��

	char m_chIndent; // ��Ϊ�������ַ�
	int m_nChPerInd; // ÿ�����������ַ�����

	bool m_bSkipCR; // ��ȡʱ���� \r 
	bool m_bPutCR; // ʹ�� \r\n ��Ϊ����

	bool m_bNLBracket; // { ֮ǰ�Ƿ���

private:
	// ��ֹ����
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter& operator=(const RealJSFormatter&);
};

#endif
