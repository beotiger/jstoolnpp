/* realjsformatter.cpp
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
#include <cstdlib>
#include <string>
#include <cstring>

#include "realjsformatter.h"

using namespace std;

bool RealJSFormatter::IsNormalChar(int ch)
{
	// һ���ַ�
	return ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
		(ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '$' || ch > 126);
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
		ch == ':' || ch == ',' || ch == ';' || ch == '\n');
}

bool RealJSFormatter::IsQuote(int ch)
{
	// ����
	return (ch == '\'' || ch == '\"');
}

bool RealJSFormatter::IsComment()
{
	// ע��
	return (charA == '/' && (charB == '/' || charB == '*'));
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
		charB = GetChar();
	}

	// normal procedure	
	tokenBType = STRING_TYPE;
	tokenB = "";
	if(bRegular)
	{
		tokenBType = REGULAR_TYPE;
		tokenB.push_back('/'); // ����һ�� /
	}

	bool bQuote = false;
	bool bComment = false;
	char chQuote; // ��¼�������� ' �� "
	char chComment; // ע������ / �� *

	while(1)
	{
		charA = charB;
		if(charA == EOF)
			return;
		charB = GetChar();

		/* 
		 * �ο� charB ������ charA
		 * �������� charA
		 * ��һ��ѭ��ʱ�Զ����� charB ���� charA
		 */

		// ������Ҫ�� token �����ж�
		if(bRegular)
		{
			// ����״̬ȫ�������ֱ�� /
			tokenB.push_back(charA);

			if(charA == '\\' && (charB == '/' || charB == '\\')) // ת���ַ�
			{
				tokenB.push_back(charB);
				charB = GetChar();
			}

			if(charA == '/') // ���Ž���
			{
				bRegular = false;
				return;
			}

			continue;
		}

		if(bQuote)
		{
			// ����״̬��ȫ�������ֱ�����Ž���
			tokenB.push_back(charA);
			
			if(charA == '\\' && (charB == chQuote || charB == '\\')) // ת���ַ�
			{
				tokenB.push_back(charB);
				charB = GetChar();
			}

			if(charA == chQuote) // ���Ž���
				return;

			continue;
		}

		if(bComment)
		{
			// ע��״̬��ȫ�����
			if(tokenBType == COMMENT_TYPE_2 && charA == '\t')
				continue;
			tokenB.push_back(charA);

			if(chComment == '*')
			{
				// ֱ�� */
				tokenBType = COMMENT_TYPE_2;
				if(charA == '*' && charB == '/')
				{
					tokenB.push_back(charB);
					charB = GetChar();
					return;
				}
			}
			else
			{
				// ֱ������
				tokenBType = COMMENT_TYPE_1;
				if(charA == '\n')
					return;
			}

			continue;
		}
		
		if(IsNormalChar(charA))
		{
			tokenBType = STRING_TYPE;
			tokenB.push_back(charA);
			if(!IsNormalChar(charB)) // loop until charB is not normal char
				return;
		}
		else
		{
			if(IsBlankChar(charA))
				continue; // ���Կհ��ַ�

			if(IsQuote(charA))
			{
				// ����
				bQuote= true;
				chQuote = charA;

				tokenBType = STRING_TYPE;
				tokenB.push_back(charA);
				continue;
			}

			if(IsComment())
			{
				// ע��
				bComment = true;
				chComment = charB;

				//tokenBType = COMMENT_TYPE;
				tokenB.push_back(charA);
				continue;
			}

			if( IsSingleOper(charA) ||
				IsNormalChar(charB) || IsBlankChar(charB) || IsQuote(charB))
			{
				tokenBType = OPER_TYPE;
				tokenB = charA; // ���ַ�����
				return;
			}

			// ˫�ַ�����
			if(charB == '=' || charB == charA)
			{
				// ��ȷ��˫�ַ�����
				tokenBType = OPER_TYPE;
				tokenB.push_back(charA);
				tokenB.push_back(charB);
				charB = GetChar();
				if((!tokenB.compare("==") || !tokenB.compare("!=")) && charB == '=')
				{
					// ���ַ� ===, !==
					tokenB.push_back(charB);
					charB = GetChar();
				}
				return;
			}
			else
			{
				// ���ǵ��ַ���
				tokenBType = OPER_TYPE;
				tokenB = charA; // ���ַ�����
				return;
			}

			// What? How could we come here?
			charA = EOF;
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
}

void RealJSFormatter::PutString(const string& str)
{
	size_t length = str.size();
	for(size_t i = 0; i < length; ++i)
	{
		if(bNewLine && 
			str[i] != '{' && str[i] != ',' && str[i] != ';')
		{
			// ���к��治�ǽ����� {,; ��������
			PutChar('\n');
			bNewLine = false;
			for(int c = 0; c < nIndents; ++c)
				PutChar('\t');
		}

		if(bNewLine && 
			(str[i] == ',' || str[i] == ';'))
			bNewLine = false;
		if(bNewLine && str[i] == '{')
		{
			bNewLine = false;
			PutChar(' '); // ������ { ��һ���ո�
		}

		if(str[i] == '\n')
			bNewLine = true;
		else
			PutChar(str[i]);
	}
}

void RealJSFormatter::Go()
{
	blockStack.push(' ');
	GetToken(true);

	while(charA != EOF)
	{
		// �ȴ���һ������
		size_t last = tokenA.size() > 0 ? tokenA.size() - 1 : 0;
		char tokenALast = tokenA[last];
		if(tokenBType != COMMENT_TYPE_1 && tokenBType != COMMENT_TYPE_2 && 
			(tokenAType != STRING_TYPE && tokenB[0] == '/' && 
			(tokenALast == '(' || tokenALast == ',' || tokenALast == '=' ||
				tokenALast == ':' || tokenALast == '[' || tokenALast == '!' ||
                tokenALast == '&' || tokenALast == '|' || tokenALast == '?' ||
                tokenALast == '+' || tokenALast == '{' || tokenALast == '}' || 
				tokenALast == ';' || tokenALast == '\n')) || 
				(!tokenA.compare("return") && !tokenB.compare("/")))
		{
			bRegular = true;
			GetToken(); // ���������ݼӵ� TokenB
		}

		tokenA = tokenB;
		tokenAType = tokenBType;
		GetToken();
		//if(nIfLikeBlock || nDoLikeBlock)
		if(((blockStack.top() == 'i' || blockStack.top() == 'd') && !tokenA.compare(";")) ||
			((nIfLikeBlock || nDoLikeBlock) && blockStack.top() == '{' && !tokenA.compare("}")))
		{
			while(!tokenB.compare("\n") || !tokenB.compare("\r\n"))
				GetToken(); // ��ѭ�����������к��Զ���Ļ���
		}

		bool bHaveNewLine = false;
		char tokenAFirst = tokenA[0];
		char tokenBFirst = tokenB[0];
		if(tokenBFirst == '\r')
			tokenBFirst = '\n';
		if(tokenBFirst == '\n' || tokenBType == COMMENT_TYPE_1)
			bHaveNewLine = true;

		/* 
		 * �ο� tokenB ������ tokenA
		 * �������� tokenA
		 * ��һ��ѭ��ʱ�Զ����� tokenB ���� tokenA
		 */
		//PutToken(tokenA);
		switch(tokenAType)
		{
		case REGULAR_TYPE:
			PutToken(tokenA); // ������ʽֱ�������ǰ��û���κ���ʽ
			break;
		case COMMENT_TYPE_1:
		case COMMENT_TYPE_2:
			if(tokenA[2] == '*')
			{
				// ����ע��
				if(!bHaveNewLine)
					PutToken(tokenA, string(""), string(" \n")); // ��Ҫ����
				else
					PutToken(tokenA);
			}
			else
			{
				// ����ע��
				PutToken(tokenA); // �϶��ỻ�е�
			}
			break;
		case OPER_TYPE:
			if(tokenA.compare("(") == 0 || tokenA.compare(")") == 0 || 
				tokenA.compare("[") == 0 || tokenA.compare("]") == 0 ||
				tokenA.compare("!") == 0 || tokenA.compare(".") == 0)
			{
				// ()[]!. ����ǰ��û����ʽ�������
				if((tokenA.compare(")") == 0 && blockStack.top() == '(') ||
					(tokenA.compare("]") == 0 && blockStack.top() == '['))
				{
					// )] ��Ҫ��ջ����������
					blockStack.pop();
					--nIndents;
				}

				if(tokenA.compare(")") == 0 && (nIfLikeBlock || nSwitchBlock) && !bBracket &&
					(blockStack.top() == 'i' || blockStack.top() == 'f' || blockStack.top() == 'w' ||
					blockStack.top() == 's')) 
				{
					// if, switch ���ڵȴ� )��֮������������
					// ) { ֮��Ŀո����������ʱ�ᴦ��
					if(!bHaveNewLine)
						PutToken(tokenA, string(""), string("\n"));
					else
						PutToken(tokenA);
					bBracket = true;
					++nIndents;
				}
				else
					PutToken(tokenA); // �������

				if(tokenA.compare("(") == 0 || tokenA.compare("[") == 0)
				{
					// ([ ��ջ����������
					blockStack.push(tokenAFirst);
					++nIndents;
				}

				continue;
			}
			
			if(tokenA.compare(";") == 0)
			{
				if(nIfLikeBlock || nDoLikeBlock)
				{
					// ; ���� if, do, while, for
					char topStack = blockStack.top();
					if(topStack == 'i' || topStack == 'f' || topStack == 'w')
					{
						--nIfLikeBlock;
						blockStack.pop();
						--nIndents;
					}
					if(topStack == 'd' || topStack == 'e')
					{
						--nDoLikeBlock;
						blockStack.pop();
						--nIndents;
					}
					
					if(!((topStack == 'i' && !tokenB.compare("else")) || 
						(topStack == 'd' && !tokenB.compare("while"))))
					{
						topStack = blockStack.top();
						// ; �����ܿ��ܽ������ if, do, while, for
						while(topStack == 'i' || topStack == 'f' || topStack == 'w' || 
							topStack == 'd' || topStack == 'e')
						{
							if(topStack == 'i' || topStack == 'f' || topStack == 'w')
							{
								--nIfLikeBlock;
								blockStack.pop();
								--nIndents;
							}
							else if(topStack == 'd' || topStack == 'e')
							{
								--nDoLikeBlock;
								blockStack.pop();
								--nIndents;
							}
							if(topStack == 'i' && !tokenB.compare("else"))
								break;
							topStack = blockStack.top();
						}
					}
				}

				if(blockStack.top() == 't')
					blockStack.pop(); // ; Ҳ������������

				if(blockStack.top() != '(' && !bHaveNewLine)
					PutToken(tokenA, string(""), string(" \n")); // ������� () ���;�ͻ���
				else
					PutToken(tokenA, string(""), string(" "));

				continue;
			}

			if(tokenA.compare(",") == 0)
			{
				if(blockStack.top() == '{' && !bHaveNewLine)
					PutToken(tokenA, string(""), string(" \n")); // ����� {} ���,���Ҳ����ڱ��������ͻ���
				else
					PutToken(tokenA, string(""), string(" "));

				continue;
			}
			
			if(tokenA.compare("{") == 0)
			{
				if((nIfLikeBlock || nDoLikeBlock || nSwitchBlock) && 
					(blockStack.top() == 'i' || blockStack.top() == 'f' || 
					blockStack.top() == 'w' || blockStack.top() == 'd' || 
					blockStack.top() == 'e' || blockStack.top() == 's'))
				{
					// { ���ԭ���� if ֮���
					/*if(blockStack.top() == 'i' || blockStack.top() == 'f' || blockStack.top() == 'w')
						bIfLikeBlock = false;
					else if(blockStack.top() == 'd' || blockStack.top() == 'e')
						bDoLikeBlock = false;
					else if(blockStack.top() == 's')
						bSwitchBlock = false;*/
					//blockStack.pop(); // �����Ǹ������������� } ʱһ��
					--nIndents;
				}

				blockStack.push(tokenAFirst); // ��ջ����������
				++nIndents;

				if(!bHaveNewLine) // ��Ҫ����
					PutToken(tokenA, string(""), string(" \n"));
				else
					PutToken(tokenA, string(""), string(" "));

				continue;
			}

			if(tokenA.compare("}") == 0)
			{
				char topStack = blockStack.top();
				if(blockStack.top() == '{')
				{
					// ��ջ����С����
					blockStack.pop();
					--nIndents;
					topStack = blockStack.top();
					if(topStack == 'i' || topStack == 'f' || topStack == 'w')
					{
						--nIfLikeBlock;
						blockStack.pop();
					}
					else if(topStack == 'd' || topStack == 'e')
					{
						--nDoLikeBlock;
						blockStack.pop();
						
					}
					else if(topStack == 's')
					{
						--nSwitchBlock;
						blockStack.pop();
					}
					//topStack = blockStack.top();
				}

				if((!bHaveNewLine && tokenBFirst != ';' && tokenBFirst != ',' && tokenBFirst != ')')
					&& !(topStack == 'd' && !tokenB.compare("while")) && 
					!(topStack == 'i' && !tokenB.compare("else")))
					PutToken(tokenA, string(""), string(" \n")); // һЩ�������
				else if(tokenBType == STRING_TYPE)
					PutToken(tokenA, string(""), string(" ")); // Ϊ else ׼���Ŀո�
				else
					PutToken(tokenA); // }, }; })

				if(!((topStack == 'i' && !tokenB.compare("else")) || 
						(topStack == 'd' && !tokenB.compare("while"))))
				{
					topStack = blockStack.top();
					// ; �����ܿ��ܽ������ if, do, while, for
					while(topStack == 'i' || topStack == 'f' || topStack == 'w' || 
						topStack == 'd' || topStack == 'e')
					{
						if(topStack == 'i' || topStack == 'f' || topStack == 'w')
						{
							--nIfLikeBlock;
							blockStack.pop();
							--nIndents;
						}
						else if(topStack == 'd' || topStack == 'e')
						{
							--nDoLikeBlock;
							blockStack.pop();
							--nIndents;
						}
						if(topStack == 'i' && !tokenB.compare("else"))
							break;
						topStack = blockStack.top();
					}
				}

				continue;
			}

			if(!tokenA.compare("++") || !tokenA.compare("--") || 
				!tokenA.compare("\n") || !tokenA.compare("\r\n"))
			{
				PutToken(tokenA);
				continue;
			}

			if(!tokenA.compare(":") && blockStack.top() == 'c')
			{
				// case, default
				if(!bHaveNewLine)
					PutToken(tokenA, string(""), string("\n"));
				else
					PutToken(tokenA);
				blockStack.pop();
				continue;
			}

			PutToken(tokenA, string(" "), string(" ")); // ʣ��Ĳ��������� �ո�oper�ո�
			break;
		case STRING_TYPE:
			if((tokenA.compare("case") == 0 || tokenA.compare("default") == 0) && bNewLine)
			{
				// case, default ��������һ��
				--nIndents;
				PutToken(tokenA, string(""), string(" "));
				++nIndents;
				blockStack.push('c');
				continue;
			}
			
			if((tokenA.compare("do") == 0 || tokenA.compare("else") == 0) && tokenB.compare("if"))
			{
				PutToken(tokenA);

				++nDoLikeBlock;
				blockStack.push(tokenA[0]);
				++nIndents; // ���� ()��ֱ������
				
				if(tokenBType == STRING_TYPE)
				{
					PutString(string(" \n"));
				}
				else
				{
					if(!tokenB.compare("{"))
						PutString(string(" "));
				}
				continue;
			}

			if(tokenBType == STRING_TYPE)
			{
				PutToken(tokenA, string(""), string(" "));

				//if(blockStack.top() != 't' && IsType(tokenA))
					//blockStack.push('t'); // ��������
			}
			else
			{
				PutToken(tokenA);

				if(tokenA.compare("if") == 0 || tokenA.compare("for") == 0 || tokenA.compare("while") == 0)
				{
					++nIfLikeBlock;
					bBracket = false; // �ȴ� ()��() ��������ܼ�����
					blockStack.push(tokenA[0]);
				}

				if(tokenA.compare("switch") == 0)
				{
					++nSwitchBlock;
					bBracket = false;
					blockStack.push('s');
				}
			}
			break;
		}
	}
}

