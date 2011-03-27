/* realjsformatter.cpp
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
#include <cstdlib>
#include <string>
#include <cstring>

#include "realjsformatter.h"

using namespace std;

RealJSFormatter::RealJSFormatter():
	m_chIndent('\t'),
	m_nChPerInd(1),
	m_bSkipCR(false),
	m_bPutCR(false),
	m_bNLBracket(false)
{
	Init();
}

RealJSFormatter::RealJSFormatter(char chIndent, int nChPerInd):
	m_chIndent(chIndent),
	m_nChPerInd(nChPerInd),
	m_bSkipCR(false),
	m_bPutCR(false),
	m_bNLBracket(false)
{
	Init();
}

RealJSFormatter::RealJSFormatter(bool bSkipCR, bool bPutCR):
	m_chIndent('\t'),
	m_nChPerInd(1),
	m_bSkipCR(bSkipCR),
	m_bPutCR(bPutCR),
	m_bNLBracket(false)
{
	Init();
}

RealJSFormatter::RealJSFormatter(char chIndent, int nChPerInd, bool bSkipCR, bool bPutCR, bool bNLBracket):
	m_chIndent(chIndent),
	m_nChPerInd(nChPerInd),
	m_bSkipCR(bSkipCR),
	m_bPutCR(bPutCR),
	m_bNLBracket(bNLBracket)
{
	Init();
}

void RealJSFormatter::Init()
{
	m_lineBuffer = "";

	m_bRegular = false;
	m_bPosNeg = false;
	m_nIndents = 0;
	m_bNewLine = false;
	m_nIfLikeBlock = 0;
	m_nDoLikeBlock = 0;
	m_nSwitchBlock = 0;
	m_bBlockStmt = true;
	m_bAssign = false;
	m_bEmptyBracket = false;
	m_bCommentPut = false;

	m_blockMap[string("if")] = IF;
	m_blockMap[string("else")] = ELSE;
	m_blockMap[string("for")] = FOR;
	m_blockMap[string("do")] = DO;
	m_blockMap[string("while")] = WHILE;
	m_blockMap[string("switch")] = SWITCH;
	m_blockMap[string("case")] = CASE;
	m_blockMap[string("default")] = CASE;
	m_blockMap[string("try")] = TRY;
	m_blockMap[string("catch")] = CATCH;
	m_blockMap[string("=")] = ASSIGN;
	m_blockMap[string("function")] = FUNCTION;
	m_blockMap[string("{")] = BLOCK;
	m_blockMap[string("(")] = BRACKET;
	m_blockMap[string("[")] = SQUARE;

	m_specKeywordSet.insert("if");
	m_specKeywordSet.insert("for");
	m_specKeywordSet.insert("while");
	m_specKeywordSet.insert("switch");
	m_specKeywordSet.insert("catch");
	m_specKeywordSet.insert("function");
	m_specKeywordSet.insert("with");
	m_specKeywordSet.insert("return");
}

bool RealJSFormatter::IsNormalChar(int ch)
{
	// һ���ַ�
	return ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
		(ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' || ch > 126);
}

bool RealJSFormatter::IsNumChar(int ch)
{
	// ���ֺ�.
	return ((ch >= '0' && ch <= '9') || ch == '.');
}

bool RealJSFormatter::IsBlankChar(int ch)
{
	// �հ��ַ�
	return (ch == ' ' || ch == '\t' || ch == '\r');
}

bool RealJSFormatter::IsSingleOper(int ch)
{
	// ���ַ�����
	return (ch == '.' || ch == '(' || ch == ')' ||
		ch == '[' || ch == ']' || ch == '{' || ch == '}' || 
		ch == ':' || ch == ',' || ch == ';' || ch == '~' || 
		ch == '\n');
}

bool RealJSFormatter::IsQuote(int ch)
{
	// ����
	return (ch == '\'' || ch == '\"');
}

bool RealJSFormatter::IsComment()
{
	// ע��
	return (m_charA == '/' && (m_charB == '/' || m_charB == '*'));
}

bool RealJSFormatter::IsType(const string& str)
{
	// �ַ����ǲ�����������
	return (!str.compare("var") || !str.compare("int") || !str.compare("float") || 
		!str.compare("long") || !str.compare("char") || !str.compare("double") || 
		!str.compare("unsigned"));
}

void RealJSFormatter::GetToken(bool init)
{
	if(init)
	{
		m_charB = GetChar();
	}

	// normal procedure	
	if(!m_bRegular && !m_bPosNeg)
	{
		m_tokenBType = STRING_TYPE;
		m_tokenB = "";
	}
	else if(m_bRegular)
	{
		m_tokenBType = REGULAR_TYPE; // ����
		//m_tokenB.push_back('/');
	}
	else
	{
		m_tokenBType = STRING_TYPE; // ������
	}

	bool bQuote = false;
	bool bComment = false;
	bool bFirst = true;
	bool bNum = false; // �ǲ�������
	bool bLineBegin = false;
	char chQuote; // ��¼�������� ' �� "
	char chComment; // ע������ / �� *

	while(1)
	{
		m_charA = m_charB;
		if(m_charA == EOF)
			return;
		do
		{
			m_charB = GetChar();
		} while(m_bSkipCR && m_charB == '\r');

		/* 
		 * �ο� m_charB ������ m_charA
		 * �������� m_charA
		 * ��һ��ѭ��ʱ�Զ����� m_charB ���� m_charA
		 */

		// ������Ҫ�� token �����ж�
		if(m_bRegular)
		{
			// ����״̬ȫ�������ֱ�� /
			m_tokenB.push_back(m_charA);

			if(m_charA == '\\' && (m_charB == '/' || m_charB == '\\')) // ת���ַ�
			{
				m_tokenB.push_back(m_charB);
				m_charB = GetChar();
			}

			if(m_charA == '/') // �������
			{
				m_bRegular = false;
				return;
			}

			continue;
		}

		if(bQuote)
		{
			// ����״̬��ȫ�������ֱ�����Ž���
			m_tokenB.push_back(m_charA);
			
			if(m_charA == '\\' && (m_charB == chQuote || m_charB == '\\')) // ת���ַ�
			{
				m_tokenB.push_back(m_charB);
				m_charB = GetChar();
			}

			if(m_charA == chQuote) // ���Ž���
				return;

			continue;
		}

		if(bComment)
		{
			// ע��״̬��ȫ�����
			if(m_tokenBType == COMMENT_TYPE_2)
			{
				// /*...*/ÿ��ǰ���\t, ' '��Ҫɾ��
				if(bLineBegin && (m_charA == '\t' || m_charA == ' '))
					continue;
				else if(bLineBegin && m_charA == '*')
					m_tokenB.push_back(' ');

				bLineBegin = false;

				if(m_charA == '\n')
					bLineBegin = true;
			}
			m_tokenB.push_back(m_charA);

			if(chComment == '*')
			{
				// ֱ�� */
				m_tokenBType = COMMENT_TYPE_2;
				if(m_charA == '*' && m_charB == '/')
				{
					m_tokenB.push_back(m_charB);
					m_charB = GetChar();
					return;
				}
			}
			else
			{
				// ֱ������
				m_tokenBType = COMMENT_TYPE_1;
				if(m_charA == '\n')
					return;
			}

			continue;
		}
		
		if(IsNormalChar(m_charA))
		{
			m_tokenBType = STRING_TYPE;
			m_tokenB.push_back(m_charA);
			
			// ������� 82e-2, 442e+6, 555E-6 ������
			// ��Ϊ����������������ֻ�ܷ������µı����ʽ
			bool bNumOld = bNum;
			if(bFirst || bNumOld) // ֻ��֮ǰ�����ֲŸı�״̬
			{
				bNum = IsNumChar(m_charA);
				bFirst = false;
			}
			if(bNumOld && !bNum && (m_charA == 'e' || m_charA == 'E') && 
				(m_charB == '-' || m_charB == '+' || IsNumChar(m_charB)))
			{
				bNum = true;
				if(m_charB == '-' || m_charB == '+')
				{
					m_tokenB.push_back(m_charB);
					m_charB = GetChar();
				}
			}

			if(!IsNormalChar(m_charB)) // loop until m_charB is not normal char
			{
				m_bPosNeg = false;
				return;
			}
		}
		else
		{
			if(IsBlankChar(m_charA))
				continue; // ���Կհ��ַ�

			if(IsQuote(m_charA))
			{
				// ����
				bQuote= true;
				chQuote = m_charA;

				m_tokenBType = STRING_TYPE;
				m_tokenB.push_back(m_charA);
				continue;
			}

			if(IsComment())
			{
				// ע��
				bComment = true;
				chComment = m_charB;

				//m_tokenBType = COMMENT_TYPE;
				m_tokenB.push_back(m_charA);
				continue;
			}

			if( IsSingleOper(m_charA) ||
				IsNormalChar(m_charB) || IsBlankChar(m_charB) || IsQuote(m_charB))
			{
				m_tokenBType = OPER_TYPE;
				m_tokenB = m_charA; // ���ַ�����
				return;
			}

			// ���ַ�����
			if(m_charB == '=' || m_charB == m_charA)
			{
				// ��ȷ�Ƕ��ַ�����
				m_tokenBType = OPER_TYPE;
				m_tokenB.push_back(m_charA);
				m_tokenB.push_back(m_charB);
				m_charB = GetChar();
				if((!m_tokenB.compare("==") || !m_tokenB.compare("!=") ||
					!m_tokenB.compare("<<") || !m_tokenB.compare(">>")) && m_charB == '=')
				{
					// ���ַ� ===, !==, <<=, >>=
					m_tokenB.push_back(m_charB);
					m_charB = GetChar();
				}
				else if(!m_tokenB.compare(">>") && m_charB == '>')
				{
					// >>>, >>>=
					m_tokenB.push_back(m_charB);
					m_charB = GetChar();
					if(m_charB == '=') // >>>=
					{
						m_tokenB.push_back(m_charB);
						m_charB = GetChar();
					}
				}
				return;
			}
			else
			{
				// ���ǵ��ַ���
				m_tokenBType = OPER_TYPE;
				m_tokenB = m_charA; // ���ַ�����
				return;
			}

			// What? How could we come here?
			m_charA = EOF;
			return;
		}
	}
}

void RealJSFormatter::PutToken(const string& token, 
		const string& leftStyle, 
		const string& rightStyle)
{
	// debug
	/*size_t length = token.size();
	for(size_t i = 0; i < length; ++i)
		PutChar(token[i]);
	PutChar('\n');*/
	// debug
	PutString(leftStyle);
	PutString(token);
	PutString(rightStyle);
	m_bCommentPut = false; // ���һ���ᷢ����ע��֮����κ��������
}

void RealJSFormatter::PutString(const string& str)
{
	size_t length = str.size();
	char topStack = m_blockStack.top();
	for(size_t i = 0; i < length; ++i)
	{
		if(m_bNewLine && (m_bCommentPut || 
			((m_bNLBracket || str[i] != '{') && str[i] != ',' && str[i] != ';')))
		{
			// ���к��治�ǽ����� {,; ��������
			PutLineBuffer(); // ����л���

			m_lineBuffer = "";
			m_bNewLine = false;
			m_nIndents = m_nIndents < 0 ? 0 : m_nIndents; // ��������
			m_nLineIndents = m_nIndents;
			if(str[i] == '{' || str[i] == ',' || str[i] == ';') // �н�β��ע�ͣ�ʹ��{,;���ò�����
				--m_nLineIndents;
			/*for(int c = 0; c < inds; ++c)
				for(int c2 = 0; c2 < m_nChPerInd; ++c2)
					PutChar(m_chIndent);*/
		}

		if(m_bNewLine && !m_bCommentPut &&  
			((!m_bNLBracket && str[i] == '{') || str[i] == ',' || str[i] == ';'))
			m_bNewLine = false;
		//if(m_bNewLine && !m_bCommentPut && str[i] == '{')
		//{
		//	m_bNewLine = false;
		//	PutChar(' '); // ������ { ��һ���ո�,; ֱ�Ӳ����о�����
		//}

		if(str[i] == '\n')
			m_bNewLine = true;
		else
			m_lineBuffer += str[i];
	}
}

void RealJSFormatter::PutLineBuffer()
{
	string line;
	for(int c = 0; c < m_nLineIndents; ++c)
		for(int c2 = 0; c2 < m_nChPerInd; ++c2)
			PutChar(m_chIndent);
	
	line.append(TrimRightSpace(m_lineBuffer));
	if(m_bPutCR)
		line.append("\r"); //PutChar('\r');
	line.append("\n"); //PutChar('\n');

	for(size_t i = 0; i < line.length(); ++i)
		PutChar(line[i]);
}

string RealJSFormatter::Trim(string& str)
{
	std::string ret = str.erase(str.find_last_not_of(" \r\n\t") + 1);
	return ret.erase(0, ret.find_first_not_of(" \r\n\t"));
}

string RealJSFormatter::TrimSpace(string& str)
{
	std::string ret = str.erase(str.find_last_not_of(" \t") + 1);
	return ret.erase(0, ret.find_first_not_of(" \t"));
}

string RealJSFormatter::TrimRightSpace(string& str)
{
	return str.erase(str.find_last_not_of(" \t") + 1);
}

void RealJSFormatter::PrepareRegular()
{
	/* 
	 * �ȴ���һ������
	 * m_tokenB[0] == /���� m_tokenB ����ע��
	 * m_tokenA ���� STRING (���� m_tokenA == return)
	 * ���� m_tokenA �����һ���ַ���������Щ
	*/
	size_t last = m_tokenA.size() > 0 ? m_tokenA.size() - 1 : 0;
	char tokenALast = m_tokenA.size() > 0 ? m_tokenA[m_tokenA.size() - 1] : 0;
	char tokenBFirst = m_tokenB[0];
	if(m_tokenBType != COMMENT_TYPE_1 && m_tokenBType != COMMENT_TYPE_2 && 
		(m_tokenAType != STRING_TYPE && tokenBFirst == '/' && 
		(tokenALast == '(' || tokenALast == ',' || tokenALast == '=' ||
			tokenALast == ':' || tokenALast == '[' || tokenALast == '!' ||
               tokenALast == '&' || tokenALast == '|' || tokenALast == '?' ||
               tokenALast == '+' || tokenALast == '{' || tokenALast == '}' || 
			tokenALast == ';' || tokenALast == '\n')) || 
			(!m_tokenA.compare("return") && tokenBFirst == '/'))
	{
		m_bRegular = true;
		GetToken(); // ���������ݼӵ� m_tokenB
	}
}

void RealJSFormatter::PreparePosNeg()
{
	/*
	 * ��� m_tokenB �� -,+ ��
	 * ���� m_tokenA �����ַ�����Ҳ����������ʽ
	 * ���� m_tokenA ���� ++, --, ], )
	 * ���� m_charB ��һ�� NormalChar
	 * ��ô m_tokenB ʵ������һ��������
	 */
	if(m_tokenBType == OPER_TYPE && (!m_tokenB.compare("-") || !m_tokenB.compare("+")) && 
		(m_tokenAType != STRING_TYPE || m_tokenB.compare("return")) && m_tokenAType != REGULAR_TYPE &&
		m_tokenA.compare("++") && m_tokenA.compare("--") && 
		m_tokenA.compare("]") && m_tokenA.compare(")") && 
		IsNormalChar(m_charB))
	{
		// m_tokenB ʵ������������
		m_bPosNeg = true;
		GetToken();
	}
}

void RealJSFormatter::PrepareTokenB()
{
	char stackTop = m_blockStack.top();
	/*if(((stackTop == 'i' || stackTop == 'e' || stackTop == 'd' || 
		stackTop == 'f' || stackTop == 'w') && !m_tokenA.compare(";")) ||
		((m_nIfLikeBlock || m_nDoLikeBlock) && stackTop == '{' && !m_tokenA.compare("}")))
	{*/
	/*
	 * ���� else, while, catch, ',', ';', ')', { ֮ǰ�Ļ���
	 * ����������Ĳ��������Ǽ������ٰ�ȥ���Ļ��в���
	 */
	int c = 0;
	while(!m_tokenB.compare("\n") || !m_tokenB.compare("\r\n")) 
	{
		++c;
		GetToken();
	}
		
	if(m_tokenB.compare("else") && m_tokenB.compare("while") && m_tokenB.compare("catch") && 
		m_tokenB.compare(",") && m_tokenB.compare(";") && m_tokenB.compare(")"))
	{
		// ��ȥ���Ļ���ѹ����У��ȴ���
		if(m_bNLBracket && !m_tokenB.compare("{"))
			return;

		if(!m_tokenA.compare("{") && !m_tokenB.compare("}"))
			return; // �� {}

		TokenAndType temp;
		c = c > 2 ? 2 : c;
		for(; c > 0; --c)
		{
			temp.token = string("\n");
			temp.type = OPER_TYPE;
			m_tokenBQueue.push(temp);
		}
		temp.token = m_tokenB;
		temp.type = m_tokenBType;
		m_tokenBQueue.push(temp);
		temp = m_tokenBQueue.front();
		m_tokenBQueue.pop();
		m_tokenB = temp.token;
		m_tokenBType = temp.type;
	}
	//}
}

void RealJSFormatter::PopMultiBlock(char previousStackTop)
{
	if(!m_tokenB.compare(";")) // ��� m_tokenB �� ;����������������������
		return;

	if(!((previousStackTop == IF && !m_tokenB.compare("else")) || 
		(previousStackTop == DO && !m_tokenB.compare("while")) ||
		(previousStackTop == TRY && !m_tokenB.compare("catch"))))
	{
		char topStack = m_blockStack.top();
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		while(topStack == IF || topStack == FOR || topStack == WHILE || 
			topStack == DO || topStack == ELSE || topStack == TRY || topStack == CATCH)
		{
			if(topStack == IF || topStack == FOR || topStack == WHILE || topStack == CATCH)
			{
				--m_nIfLikeBlock;
				m_blockStack.pop();
				--m_nIndents;
			}
			else if(topStack == ELSE || topStack == TRY)
			{
				--m_nDoLikeBlock;
				m_blockStack.pop();
				--m_nIndents;
			}
			else if(topStack == DO)
			{
				--m_nIndents;
			}

			if((topStack == IF && !m_tokenB.compare("else")) ||
				(topStack == DO && !m_tokenB.compare("while")) ||
				(topStack == TRY && !m_tokenB.compare("catch")))
				break; // ֱ���ոս���һ�� if...else, do...while, try...catch
			topStack = m_blockStack.top();
		}
	}
}

void RealJSFormatter::Go()
{
	m_blockStack.push(' ');
	m_brcNeedStack.push(true);
	GetToken(true);

	bool bHaveNewLine;
	char tokenAFirst;
	char tokenBFirst;

	while(m_charA != EOF)
	{
		PrepareRegular(); // �ж�����
		PreparePosNeg(); // �ж�������

		m_tokenA = m_tokenB;
		m_tokenAType = m_tokenBType;
		
		if(m_tokenBQueue.size() == 0)
		{
			GetToken();
			PrepareTokenB(); // �����ǲ���Ҫ��������
		}
		else
		{
			// ���ŶӵĻ���
			TokenAndType temp;
			temp = m_tokenBQueue.front();
			m_tokenBQueue.pop();
			m_tokenB = temp.token;
			m_tokenBType = temp.type;
		}

		bHaveNewLine = false; // bHaveNewLine ��ʾ���潫Ҫ���У�m_bNewLine ��ʾ�Ѿ�������
		tokenAFirst = m_tokenA[0];
		tokenBFirst = m_tokenB.size() ? m_tokenB[0] : 0;
		if(tokenBFirst == '\r')
			tokenBFirst = '\n';
		if(tokenBFirst == '\n' || m_tokenBType == COMMENT_TYPE_1)
			bHaveNewLine = true;

		if(!m_bBlockStmt && m_tokenA.compare("{") && m_tokenA.compare("\n") 
			&& m_tokenAType != COMMENT_TYPE_1 && m_tokenAType != COMMENT_TYPE_2)
			m_bBlockStmt = true;

		/* 
		 * �ο� m_tokenB ������ m_tokenA
		 * �������� m_tokenA
		 * ��һ��ѭ��ʱ�Զ����� m_tokenB ���� m_tokenA
		 */
		//PutToken(m_tokenA);
		switch(m_tokenAType)
		{
		case REGULAR_TYPE:
			PutToken(m_tokenA); // ������ʽֱ�������ǰ��û���κ���ʽ
			break;
		case COMMENT_TYPE_1:
		case COMMENT_TYPE_2:
			if(m_tokenA[2] == '*')
			{
				// ����ע��
				if(!bHaveNewLine)
					PutToken(m_tokenA, string(""), string("\n")); // ��Ҫ����
				else
					PutToken(m_tokenA);
			}
			else
			{
				// ����ע��
				PutToken(m_tokenA); // �϶��ỻ�е�
			}
			m_bCommentPut = true;
			break;
		case OPER_TYPE:
			ProcessOper(bHaveNewLine, tokenAFirst, tokenBFirst);

			break;
		case STRING_TYPE:
			ProcessString(bHaveNewLine, tokenAFirst, tokenBFirst);
			break;
		}
	}

	if(m_lineBuffer.length())
		PutLineBuffer();
}

void RealJSFormatter::ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	char topStack = m_blockStack.top();
	string strRight(" ");

	if(!m_tokenA.compare("(") || !m_tokenA.compare(")") || 
		!m_tokenA.compare("[") || !m_tokenA.compare("]") ||
		!m_tokenA.compare("!") || !m_tokenA.compare("!!") ||
		!m_tokenA.compare("~") || !m_tokenA.compare("^") ||
		!m_tokenA.compare("."))
	{
		// ()[]!. ����ǰ��û����ʽ�������
		if((!m_tokenA.compare(")") || !m_tokenA.compare("]")) &&
			topStack == ASSIGN || topStack == HELPER)
		{
			if(topStack == ASSIGN)
				--m_nIndents;
			m_blockStack.pop();
		}
		if((!m_tokenA.compare(")") && m_blockStack.top() == BRACKET) ||
			(!m_tokenA.compare("]") && m_blockStack.top() == SQUARE))
		{
			// )] ��Ҫ��ջ����������
			m_blockStack.pop();
			--m_nIndents;
			topStack = m_blockStack.top();
			if(topStack == ASSIGN || topStack == HELPER)
				m_blockStack.pop();
		}

		if(!m_tokenA.compare(")") && (m_nIfLikeBlock || m_nSwitchBlock) && !m_brcNeedStack.top() &&
			(m_blockStack.top() == IF || m_blockStack.top() == FOR || m_blockStack.top() == WHILE ||
			m_blockStack.top() == SWITCH || m_blockStack.top() == CATCH)) 
		{
			// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
			// ����Ŀո������Ŀո������� { �ģ�m_bNLBracket Ϊ true ����Ҫ�ո���
			string rightDeco = strRight;
			if(!bHaveNewLine)
				rightDeco.append("\n"); 
			PutToken(m_tokenA, string(""), rightDeco);
			//bBracket = true;
			m_brcNeedStack.pop();
			m_bBlockStmt = false; // �ȴ� statment
			if(m_blockStack.top() == WHILE)
			{
				m_blockStack.pop();
				if(m_blockStack.top() == DO)
				{
					// ���� do...while
					--m_nIfLikeBlock;
					--m_nDoLikeBlock;
					m_blockStack.pop();

					PopMultiBlock(WHILE);
				}
				else
				{
					m_blockStack.push(WHILE);
					++m_nIndents;
				}
			}
			else
				++m_nIndents;
		}
		else if(!m_tokenA.compare(")") && (!m_tokenB.compare("{") || bHaveNewLine))
			PutToken(m_tokenA, string(""), strRight); // { ���߻���֮ǰ�����ո�
		else
			PutToken(m_tokenA); // �������

		if(!m_tokenA.compare("(") || !m_tokenA.compare("["))
		{
			// ([ ��ջ����������
			topStack = m_blockStack.top();
			if(topStack == ASSIGN)
			{
				if(!m_bAssign)
					--m_nIndents;
				else
					m_blockStack.push(HELPER);
			}
			m_blockStack.push(m_blockMap[m_tokenA]);
			++m_nIndents;
		}

		return;
	}
			
	if(!m_tokenA.compare(";"))
	{
		topStack = m_blockStack.top();
		if(topStack == ASSIGN)
		{
			--m_nIndents;
			m_blockStack.pop();
		}

		topStack = m_blockStack.top();
		if(m_nIfLikeBlock || m_nDoLikeBlock)
		{
			// ; ���� if, else, while, for, try, catch
			if(topStack == IF || topStack == FOR || 
				topStack == WHILE || topStack == CATCH)
			{
				--m_nIfLikeBlock;
				m_blockStack.pop();
				--m_nIndents;
			}
			if(topStack == ELSE || topStack == TRY)
			{
				--m_nDoLikeBlock;
				m_blockStack.pop();
				--m_nIndents;
			}
			if(topStack == DO)
				--m_nIndents;
			// do �ڶ�ȡ�� while ����޸ļ���
			// ���� do{} Ҳһ��

			// ����� } ��ͬ���Ĵ���
			PopMultiBlock(topStack);
		}

		//if(m_blockStack.top() == 't')
			//m_blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t

		topStack = m_blockStack.top();
		if(topStack != BRACKET && !bHaveNewLine)
			PutToken(m_tokenA, string(""), strRight.append("\n")); // ������� () ��� ; �ͻ���
		else if(topStack == BRACKET || m_tokenBType == COMMENT_TYPE_1)
			PutToken(m_tokenA, string(""), strRight); // (; ) �ո�
		else
			PutToken(m_tokenA);

		return; // ;
	}

	if(!m_tokenA.compare(","))
	{
		if(m_blockStack.top() == ASSIGN)
		{
			--m_nIndents;
			m_blockStack.pop();
		}
		if(m_blockStack.top() == BLOCK && !bHaveNewLine)
			PutToken(m_tokenA, string(""), strRight.append("\n")); // ����� {} ���
		else
			PutToken(m_tokenA, string(""), strRight);

		return; // ,
	}
			
	if(!m_tokenA.compare("{"))
	{
		topStack = m_blockStack.top();
		if((m_nIfLikeBlock || m_nDoLikeBlock || m_nSwitchBlock) && 
			(topStack == IF || topStack == FOR || 
			topStack == WHILE || topStack == DO || 
			topStack == ELSE || topStack == SWITCH ||
			topStack == TRY || topStack == CATCH) || 
			topStack == ASSIGN)
		{
			if(!m_bBlockStmt || (topStack == ASSIGN && !m_bAssign))
			{
				//m_blockStack.pop(); // �����Ǹ������������� } ʱһ��
				--m_nIndents;
				m_bBlockStmt = true;
			}
			else
			{
				m_blockStack.push(HELPER); // ѹ��һ�� HELPER ͳһ״̬
			}
		}	

		m_blockStack.push(m_blockMap[m_tokenA]); // ��ջ����������
		++m_nIndents;

		/*
		 * { ֮��Ŀո�����֮ǰ�ķ���׼���õ�
		 * ����Ϊ�˽�� { ������ʱ��ǰ����һ���ո������
		 * ��Ϊ�㷨ֻ����󣬲�����ǰ��
		 */
		if(!m_tokenB.compare("}"))
		{
			// �� {}
			m_bEmptyBracket = true;
			if(m_bNewLine == false && m_bNLBracket && 
				(topStack == IF || topStack == FOR || 
				topStack == WHILE || topStack == SWITCH ||
				topStack == CATCH || topStack == FUNCTION))
			{
				PutToken(m_tokenA, string(" ")); // ��Щ����£�ǰ�油һ���ո�
			}
			else
			{
				PutToken(m_tokenA);
			}
		}
		else
		{
			string strLeft = (m_bNLBracket && !m_bNewLine) ? string("\n") : string("");
			if(!bHaveNewLine) // ��Ҫ����
				PutToken(m_tokenA, strLeft, strRight.append("\n"));
			else
				PutToken(m_tokenA, strLeft, strRight);
		}

		return; // {
	}

	if(!m_tokenA.compare("}"))
	{
		topStack = m_blockStack.top();

		// �����Ĳ��ԣ�} һֱ���� {
		// ���������ٿ���ʹ�� {} ֮������ȷ��
		while(1)
		{
			if(topStack == BLOCK)
				break;

			m_blockStack.pop();

			switch(topStack)
			{
			case IF:
			case FOR:
			case WHILE:
			case CATCH:
				--m_nIfLikeBlock;
				--m_nIndents;
				break;
			case DO:
			case ELSE:
			case TRY:
				--m_nDoLikeBlock;
				--m_nIndents;
				break;
			case SWITCH:
				--m_nSwitchBlock;
				--m_nIndents;
				break;
			case ASSIGN:
			case FUNCTION:
			case HELPER:
				--m_nIndents;
				break;
			}

			topStack = m_blockStack.top();
		}

		if(topStack == BLOCK)
		{
			// ��ջ����С����
			m_blockStack.pop();
			--m_nIndents;
			topStack = m_blockStack.top();

			switch(topStack)
			{
			case IF:
			case FOR:
			case WHILE:
			case CATCH:
				--m_nIfLikeBlock;
				m_blockStack.pop();
				break;
			case ELSE:
			case TRY:
				--m_nDoLikeBlock;
				m_blockStack.pop();
				break;
			case DO:
				// �����Ѿ�����do ���� while
				break;
			case SWITCH:
				--m_nSwitchBlock;
				m_blockStack.pop();
				break;
			case ASSIGN:
			case FUNCTION:
			case HELPER:
				m_blockStack.pop();
				break;
			}
			//topStack = m_blockStack.top();
		}

		string leftStyle("");
		if(!m_bNewLine)
			leftStyle = "\n";
		if(m_bEmptyBracket)
		{
			leftStyle = "";
			strRight.append("\n");
			m_bEmptyBracket = false;
		}

		if((!bHaveNewLine && tokenBFirst != ';' && tokenBFirst != ',')
			&& !(!m_bNLBracket && topStack == DO && !m_tokenB.compare("while")) && 
			!(!m_bNLBracket && topStack == IF && !m_tokenB.compare("else")) &&
			!(!m_bNLBracket && topStack == TRY && !m_tokenB.compare("catch")) &&
			!(!m_bNLBracket && !m_tokenB.compare(")")))
			PutToken(m_tokenA, leftStyle, strRight.append("\n")); // һЩ�������
		else if(m_tokenBType == STRING_TYPE || m_tokenBType == COMMENT_TYPE_1)
			PutToken(m_tokenA, leftStyle, strRight); // Ϊ else ׼���Ŀո�
		else
			PutToken(m_tokenA, leftStyle); // }, }; })
		// ע�� ) ��Ҫ�����ʱ���� ,; ȡ��ǰ��Ļ���

		PopMultiBlock(topStack);

		return; // }
	}

	if(!m_tokenA.compare("++") || !m_tokenA.compare("--") || 
		!m_tokenA.compare("\n") || !m_tokenA.compare("\r\n"))
	{
		PutToken(m_tokenA);
		return;
	}

	if(!m_tokenA.compare(":") && m_blockStack.top() == CASE)
	{
		// case, default
		if(!bHaveNewLine)
			PutToken(m_tokenA, string(""), strRight.append("\n"));
		else
			PutToken(m_tokenA, string(""), strRight);
		m_blockStack.pop();
		return;
	}

	if(m_blockStack.top() == ASSIGN)
		m_bAssign = true;

	if(!m_tokenA.compare("=") && m_blockStack.top() != ASSIGN)
	{
		m_blockStack.push(m_blockMap[m_tokenA]);
		++m_nIndents;
		m_bAssign = false;
	}

	PutToken(m_tokenA, string(" "), string(" ")); // ʣ��Ĳ��������� �ո�oper�ո�
}

void RealJSFormatter::ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	if(!m_tokenA.compare("case") || !m_tokenA.compare("default"))
	{
		// case, default ��������һ��
		--m_nIndents;
		string rightDeco = m_tokenA.compare("default") ? string(" ") : string();
		PutToken(m_tokenA, string(""), rightDeco);
		++m_nIndents;
		m_blockStack.push(m_blockMap[m_tokenA]);
		return;
	}
			
	if(!m_tokenA.compare("do") || 
		(!m_tokenA.compare("else") && m_tokenB.compare("if")) ||
		!m_tokenA.compare("try"))
	{
		// do, else (NOT else if), try
		PutToken(m_tokenA);

		++m_nDoLikeBlock;
		m_blockStack.push(m_blockMap[m_tokenA]);
		++m_nIndents; // ���� ()��ֱ������
		m_bBlockStmt = false; // �ȴ� block �ڲ��� statment
			
		PutString(string(" "));
		if((m_tokenBType == STRING_TYPE || m_bNLBracket) && !bHaveNewLine)
			PutString(string("\n"));

		return;
	}

	if(!m_tokenA.compare("function"))
	{
		if(m_blockStack.top() == ASSIGN)
		{
			--m_nIndents;
			m_blockStack.pop();
		}
		m_blockStack.push(m_blockMap[m_tokenA]); // �� function Ҳѹ��ջ������ } ����
	}

	if(m_blockStack.top() == ASSIGN)
		m_bAssign = true;

	if(m_tokenBType == STRING_TYPE)
	{
		PutToken(m_tokenA, string(""), string(" "));

		//if(m_blockStack.top() != 't' && IsType(m_tokenA))
			//m_blockStack.push('t'); // ��������
		return;
	}

	if(m_specKeywordSet.find(m_tokenA) != m_specKeywordSet.end())
		PutToken(m_tokenA, string(""), string(" "));
	else
		PutToken(m_tokenA);

	if(!m_tokenA.compare("if") || !m_tokenA.compare("for") || 
		!m_tokenA.compare("while") || !m_tokenA.compare("catch"))
	{
		++m_nIfLikeBlock;
		//bBracket = false; // �ȴ� ()��() ��������ܼ�����
		m_brcNeedStack.push(false);
		//if(m_tokenA.compare("catch"))
		//	m_blockStack.push(m_tokenA[0]);
		//else
		//	m_blockStack.push('h');
		m_blockStack.push(m_blockMap[m_tokenA]);

	}

	if(!m_tokenA.compare("switch"))
	{
		++m_nSwitchBlock;
		//bBracket = false;
		m_brcNeedStack.push(false);
		m_blockStack.push(m_blockMap[m_tokenA]);
	}
}

