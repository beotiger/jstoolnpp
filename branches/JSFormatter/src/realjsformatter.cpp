/* realjsformatter.cpp
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
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <ctime>

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

void RealJSFormatter::StringReplace(string &strBase, string strSrc, string strDes)
{
	string::size_type pos = 0;
	string::size_type srcLen = strSrc.size();
	string::size_type desLen = strDes.size();
	pos = strBase.find(strSrc, pos); 
	while((pos != string::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, pos + desLen);
	}
}

void RealJSFormatter::Init()
{
	m_debugOutput = false;
	m_tokenCount = 0;

	m_strBeforeReg = "(,=:[!&|?+{};\n";
	m_lineBuffer = "";

	m_bRegular = false;
	m_bPosNeg = false;
	m_nIndents = 0;
	m_nLineIndents = 0;
	m_bNewLine = false;
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
		if(m_charA == 0)
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
				if((m_tokenB == "==" || m_tokenB == "!=" ||
					m_tokenB == "<<" || m_tokenB == ">>") && m_charB == '=')
				{
					// ���ַ� ===, !==, <<=, >>=
					m_tokenB.push_back(m_charB);
					m_charB = GetChar();
				}
				else if(m_tokenB == ">>" && m_charB == '>')
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
			m_charA = 0;
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
		}

		if(m_bNewLine && !m_bCommentPut &&  
			((!m_bNLBracket && str[i] == '{') || str[i] == ',' || str[i] == ';'))
			m_bNewLine = false;

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
	if(tokenBFirst == '/' && m_tokenBType != COMMENT_TYPE_1 && 
		m_tokenBType != COMMENT_TYPE_2 && 
		((m_tokenAType != STRING_TYPE && m_strBeforeReg.find(tokenALast) != string::npos) || 
			m_tokenA == "return"))
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
	if(m_tokenBType == OPER_TYPE && (m_tokenB == "-" || m_tokenB == "+") && 
		(m_tokenAType != STRING_TYPE || m_tokenA == "return") && m_tokenAType != REGULAR_TYPE &&
		m_tokenA != "++" && m_tokenA != "--" && 
		m_tokenA != "]" && m_tokenA != ")" && 
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

	/*
	 * ���� else, while, catch, ',', ';', ')', { ֮ǰ�Ļ���
	 * ����������Ĳ��������Ǽ������ٰ�ȥ���Ļ��в���
	 */
	int c = 0;
	while(m_tokenB == "\n" || m_tokenB == "\r\n") 
	{
		++c;
		GetToken();
	}
		
	if(m_tokenB != "else" && m_tokenB != "while" && m_tokenB != "catch" && 
		m_tokenB != "," && m_tokenB != ";" && m_tokenB != ")")
	{
		// ��ȥ���Ļ���ѹ����У��ȴ���
		if(m_bNLBracket && m_tokenB == "{")
			return;

		if(m_tokenA == "{" && m_tokenB == "}")
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
}

void RealJSFormatter::PopMultiBlock(char previousStackTop)
{
	if(m_tokenB == ";") // ��� m_tokenB �� ;����������������������
		return;

	if(!((previousStackTop == IF && m_tokenB == "else") || 
		(previousStackTop == DO && m_tokenB == "while") ||
		(previousStackTop == TRY && m_tokenB == "catch")))
	{
		char topStack = m_blockStack.top();
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		while(topStack == IF || topStack == FOR || topStack == WHILE || 
			topStack == DO || topStack == ELSE || topStack == TRY || topStack == CATCH)
		{
			if(topStack == IF || topStack == FOR || 
				topStack == WHILE || topStack == CATCH ||
				topStack == ELSE || topStack == TRY)
			{
				m_blockStack.pop();
				--m_nIndents;
			}
			else if(topStack == DO)
			{
				--m_nIndents;
			}

			if((topStack == IF && m_tokenB == "else") ||
				(topStack == DO && m_tokenB == "while") ||
				(topStack == TRY && m_tokenB == "catch"))
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

	time(&m_startTime);

	while(m_charA != 0)
	{
		PrepareRegular(); // �ж�����
		PreparePosNeg(); // �ж�������

		++m_tokenCount;
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

		if(!m_bBlockStmt && m_tokenA != "{" && m_tokenA != "\n" 
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

	m_lineBuffer = Trim(m_lineBuffer);
	if(m_lineBuffer.length())
		PutLineBuffer();

	time(&m_endTime);
	if(m_debugOutput)
	{
		cout << "Processed tokens: " << m_tokenCount << endl;
		cout << "Time used: " << m_endTime - m_startTime << "s" << endl;
	}
}

void RealJSFormatter::ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	char topStack = m_blockStack.top();
	string strRight(" ");

	if(m_tokenA == "(" || m_tokenA == ")" || 
		m_tokenA == "[" || m_tokenA == "]" ||
		m_tokenA == "!" || m_tokenA == "!!" ||
		m_tokenA == "~" || m_tokenA == "^" ||
		m_tokenA == ".")
	{
		// ()[]!. ����ǰ��û����ʽ�������
		if((m_tokenA == ")" || m_tokenA == "]") &&
			(topStack == ASSIGN || topStack == HELPER))
		{
			if(topStack == ASSIGN)
				--m_nIndents;
			m_blockStack.pop();
		}
		if((m_tokenA == ")" && m_blockStack.top() == BRACKET) ||
			(m_tokenA == "]" && m_blockStack.top() == SQUARE))
		{
			// )] ��Ҫ��ջ����������
			m_blockStack.pop();
			--m_nIndents;
			topStack = m_blockStack.top();
			if(topStack == ASSIGN || topStack == HELPER)
				m_blockStack.pop();
		}

		if(m_tokenA == ")" && !m_brcNeedStack.top() &&
			(m_blockStack.top() == IF || m_blockStack.top() == FOR || m_blockStack.top() == WHILE ||
			m_blockStack.top() == SWITCH || m_blockStack.top() == CATCH)) 
		{
			// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
			// ����Ŀո������Ŀո������� { �ģ�m_bNLBracket Ϊ true ����Ҫ�ո���
			string rightDeco = m_tokenB != ";" ? strRight : "";
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
		else if(m_tokenA == ")" && (m_tokenB == "{" || bHaveNewLine))
			PutToken(m_tokenA, string(""), strRight); // { ���߻���֮ǰ�����ո�
		else
			PutToken(m_tokenA); // �������

		if(m_tokenA == "(" || m_tokenA == "[")
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
			
	if(m_tokenA == ";")
	{
		topStack = m_blockStack.top();
		if(topStack == ASSIGN)
		{
			--m_nIndents;
			m_blockStack.pop();
		}

		topStack = m_blockStack.top();

		// ; ���� if, else, while, for, try, catch
		if(topStack == IF || topStack == FOR || 
			topStack == WHILE || topStack == CATCH)
		{
			m_blockStack.pop();
			--m_nIndents;
			// ����� } ��ͬ���Ĵ���
			PopMultiBlock(topStack);
		}
		if(topStack == ELSE || topStack == TRY)
		{
			m_blockStack.pop();
			--m_nIndents;
			PopMultiBlock(topStack);
		}
		if(topStack == DO)
		{
			--m_nIndents;
			PopMultiBlock(topStack);
		}
		// do �ڶ�ȡ�� while ����޸ļ���
		// ���� do{} Ҳһ��

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

	if(m_tokenA == ",")
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
			
	if(m_tokenA == "{")
	{
		topStack = m_blockStack.top();
		if(topStack == IF || topStack == FOR || 
			topStack == WHILE || topStack == DO || 
			topStack == ELSE || topStack == SWITCH ||
			topStack == TRY || topStack == CATCH || 
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
		if(m_tokenB == "}")
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

	if(m_tokenA == "}")
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
			case DO:
			case ELSE:
			case TRY:
			case SWITCH:
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
			case ELSE:
			case TRY:
			case SWITCH:
			case ASSIGN:
			case FUNCTION:
			case HELPER:
				m_blockStack.pop();
				break;
			case DO:
				// �����Ѿ�����do ���� while
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
			&& (m_bNLBracket || !((topStack == DO && m_tokenB == "while") || 
			(topStack == IF && m_tokenB == "else") ||
			(topStack == TRY && m_tokenB == "catch") ||
			m_tokenB == ")")))
			PutToken(m_tokenA, leftStyle, strRight.append("\n")); // һЩ�������
		else if(m_tokenBType == STRING_TYPE || m_tokenBType == COMMENT_TYPE_1)
			PutToken(m_tokenA, leftStyle, strRight); // Ϊ else ׼���Ŀո�
		else
			PutToken(m_tokenA, leftStyle); // }, }; })
		// ע�� ) ��Ҫ�����ʱ���� ,; ȡ��ǰ��Ļ���

		PopMultiBlock(topStack);

		return; // }
	}

	if(m_tokenA == "++" || m_tokenA == "--" || 
		m_tokenA == "\n" || m_tokenA == "\r\n")
	{
		PutToken(m_tokenA);
		return;
	}

	if(m_tokenA == ":" && m_blockStack.top() == CASE)
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

	if(m_tokenA == "=" && m_blockStack.top() != ASSIGN)
	{
		m_blockStack.push(m_blockMap[m_tokenA]);
		++m_nIndents;
		m_bAssign = false;
	}

	PutToken(m_tokenA, string(" "), string(" ")); // ʣ��Ĳ��������� �ո�oper�ո�
}

void RealJSFormatter::ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	if(m_tokenA == "case" || m_tokenA == "default")
	{
		// case, default ��������һ��
		--m_nIndents;
		string rightDeco = m_tokenA != "default" ? string(" ") : string();
		PutToken(m_tokenA, string(""), rightDeco);
		++m_nIndents;
		m_blockStack.push(m_blockMap[m_tokenA]);
		return;
	}
			
	if(m_tokenA == "do" || 
		(m_tokenA == "else" && m_tokenB != "if") ||
		m_tokenA == "try")
	{
		// do, else (NOT else if), try
		PutToken(m_tokenA);

		m_blockStack.push(m_blockMap[m_tokenA]);
		++m_nIndents; // ���� ()��ֱ������
		m_bBlockStmt = false; // �ȴ� block �ڲ��� statment
			
		PutString(string(" "));
		if((m_tokenBType == STRING_TYPE || m_bNLBracket) && !bHaveNewLine)
			PutString(string("\n"));

		return;
	}

	if(m_tokenA == "function")
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

	if(m_specKeywordSet.find(m_tokenA) != m_specKeywordSet.end() && 
		m_tokenB != ";")
		PutToken(m_tokenA, string(""), string(" "));
	else
		PutToken(m_tokenA);

	if(m_tokenA == "if" || m_tokenA == "for" || 
		m_tokenA == "while" || m_tokenA == "catch")
	{
		// �ȴ� ()��() ��������ܼ�����
		m_brcNeedStack.push(false);
		m_blockStack.push(m_blockMap[m_tokenA]);

	}

	if(m_tokenA == "switch")
	{
		//bBracket = false;
		m_brcNeedStack.push(false);
		m_blockStack.push(m_blockMap[m_tokenA]);
	}
}

