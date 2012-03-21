/* JSParser.h
   2012-3-11

Copyright (c) 2010-2012 SUN Junwen

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
#ifndef _JS_PARSER_H_
#define _JS_PARSER_H_
#include <ctime>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <set>

#include "jsonValue.h"

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
#define EMPTY ' '

template<class T>
bool GetStackTop(stack<T> stk, T& ret);
template<class T>
bool StackTopEq(stack<T> stk, T eq);

struct TokenAndType
{
	string token;
	int type;
};

class JSParser
{
public:
	typedef stack<char> CharStack;
	typedef stack<bool> BoolStack;
	typedef queue<TokenAndType> TokenQueue;
	typedef map<string, char> StrCharMap;
	typedef set<string> StrSet;

	explicit JSParser();

	virtual ~JSParser()
	{}

	inline void Go(JsonValue& jsonValue)
	{ RecursiveProc(jsonValue); }

	bool m_debugOutput;

private:
	void Init();

	// Should be implemented in derived class
	virtual int GetChar() = 0; // JUST get next char from input
	virtual void PutChar(int ch) = 0; // JUST put a char to output

	void PutString(const string& str);
	void PutLineBuffer();
	void FlushLineBuffer();

	bool inline IsNormalChar(int ch);
	bool inline IsNumChar(int ch);
	bool inline IsBlankChar(int ch);
	bool inline IsSingleOper(int ch);
	bool inline IsQuote(int ch);
	bool inline IsComment(); // Ҫ�����ж� charA, charB

	void GetTokenRaw(bool init = false);
	void GetToken(); // ���������, ����ȵȵ� GetToken ����

	void PrepareRegular(); // ͨ���ʷ��ж� tokenB ����
	void PreparePosNeg(); // ͨ���ʷ��ж� tokenB ������
	void PrepareTokenB();

	void RecursiveProc(JsonValue& jsonValue);

	int m_tokenCount;
	clock_t m_startClock;
	clock_t m_endClock;
	double m_duration;

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

	CharStack m_blockStack;

	bool m_bNewLine; // ׼�����еı�־
	string m_lineBuffer;

	StrSet m_specKeywordSet; // ����Ҫ�������ŵĹؼ��ּ���
	StrCharMap m_blockMap;

	int m_nRecuLevel; // ��ݹ���

private:
	// ��ֹ����
	JSParser(const JSParser&);
	JSParser& operator=(const JSParser&);
};

#endif
