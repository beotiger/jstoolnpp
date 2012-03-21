/* JSParser.cpp
   2010-12-16

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
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <ctime>

#include "jsparser.h"

using namespace std;

template<class T>
bool GetStackTop(stack<T> stk, T& ret)
{
	if(stk.size() == 0)
		return false;
	ret = stk.top();
	return true;
}

template<class T>
bool StackTopEq(stack<T> stk, T eq)
{
	if(stk.size() == 0)
		return false;
	return (eq == stk.top());
}

JSParser::JSParser()
{
	Init();
}

string JSParser::Trim(const string& str)
{
	std::string ret(str);
	ret = ret.erase(ret.find_last_not_of(" \r\n\t") + 1);
	return ret.erase(0, ret.find_first_not_of(" \r\n\t"));
}

string JSParser::TrimSpace(const string& str)
{
	std::string ret(str);
	ret = ret.erase(ret.find_last_not_of(" \t") + 1);
	return ret.erase(0, ret.find_first_not_of(" \t"));
}

string JSParser::TrimRightSpace(const string& str)
{
	std::string ret(str);
	return ret.erase(ret.find_last_not_of(" \t") + 1);
}

void JSParser::StringReplace(string& strBase, const string& strSrc, const string& strDes)
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

void JSParser::Init()
{
	m_debugOutput = false;
	m_tokenCount = 0;

	m_strBeforeReg = "(,=:[!&|?+{};\n";
	m_lineBuffer = "";

	m_bRegular = false;
	m_bPosNeg = false;
	m_bNewLine = false;

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

bool JSParser::IsNormalChar(int ch)
{
	// һ���ַ�
	return ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
		(ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' ||
		ch > 126 || ch < 0);
}

bool JSParser::IsNumChar(int ch)
{
	// ���ֺ�.
	return ((ch >= '0' && ch <= '9') || ch == '.');
}

bool JSParser::IsBlankChar(int ch)
{
	// �հ��ַ�
	return (ch == ' ' || ch == '\t' || ch == '\r');
}

bool JSParser::IsSingleOper(int ch)
{
	// ���ַ�����
	return (ch == '.' || ch == '(' || ch == ')' ||
		ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
		ch == ',' || ch == ';' || ch == '~' ||
		ch == '\n');
}

bool JSParser::IsQuote(int ch)
{
	// ����
	return (ch == '\'' || ch == '\"');
}

bool JSParser::IsComment()
{
	// ע��
	return (m_charA == '/' && (m_charB == '/' || m_charB == '*'));
}

void JSParser::GetToken(bool init)
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
	bool bRegularFlags = false;
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
		} while(m_charB == '\r');

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

			if(m_charA == '/') // ������ܽ���
			{
				if(!bRegularFlags && IsNormalChar(m_charB))
				{
					// ����� flags ����
					bRegularFlags = true;
				}
				else
				{
					// �������
					m_bRegular = false;
					return;
				}
			}

			if(bRegularFlags && !IsNormalChar(m_charB))
			{
				// �������
				bRegularFlags = false;
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
			if((m_charB == '=' || m_charB == m_charA) || 
				(m_charA == '-' && m_charB == '>'))
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

void JSParser::PutString(const string& str)
{
	size_t length = str.size();
	//char topStack = m_blockStack.top();
	for(size_t i = 0; i < length; ++i)
	{
		if(m_bNewLine)
		{
			// ���к��治�ǽ����� {,; ��������
			PutLineBuffer(); // ����л���

			m_lineBuffer = "";
			m_bNewLine = false;
		}

		if(str[i] == '\n')
			m_bNewLine = true;
		else
			m_lineBuffer += str[i];
	}
}

void JSParser::PutLineBuffer()
{
	string line;
	line.append(TrimRightSpace(m_lineBuffer));
	line.append("\n"); //PutChar('\n');

	for(size_t i = 0; i < line.length(); ++i)
		PutChar(line[i]);
}

void JSParser::FlushLineBuffer()
{
	m_lineBuffer = Trim(m_lineBuffer);
	if(m_lineBuffer.length())
		PutLineBuffer();
}

void JSParser::PrepareRegular()
{
	/*
	 * �ȴ���һ������
	 * m_tokenB[0] == /���� m_tokenB ����ע��
	 * m_tokenA ���� STRING (���� m_tokenA == return)
	 * ���� m_tokenA �����һ���ַ���������Щ
	*/
	//size_t last = m_tokenA.size() > 0 ? m_tokenA.size() - 1 : 0;
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

void JSParser::PreparePosNeg()
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

void JSParser::PrepareTokenB()
{
	//char stackTop = m_blockStack.top();

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

void JSParser::Go()
{
	GetToken(true);

	m_startClock = clock();

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

		/*
		 * ���ˣ���ȡ��� m_tokenA �� m_tokenB
		 * �Ѿ��ϲ��������
		 * �Ѿ�ʶ����
		 * �Ѿ�ʶ��������ʽ
		 */
		if(m_tokenA != "\r\n" && m_tokenA != "\n")
		{
			PutString(m_tokenA);
			PutString("\n");
		}
		
	}

	FlushLineBuffer();

	m_endClock = clock();
	m_duration = (double)(m_endClock - m_startClock) / CLOCKS_PER_SEC;
	if(m_debugOutput)
	{
		cout << "Processed tokens: " << m_tokenCount << endl;
		cout << "Time used: " << m_duration << "s" << endl;
		cout << m_tokenCount/ m_duration << " tokens/second" << endl;
	}
}
