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
	chIndent('\t'),
	nChPerInd(1),
	bSkipCR(false),
	bPutCR(false),
	bNLBracket(false)
{
	Init();
}

RealJSFormatter::RealJSFormatter(char chIndent, int nChPerInd):
	bSkipCR(false),
	bPutCR(false),
	bNLBracket(false)
{
	this->chIndent = chIndent;
	this->nChPerInd = nChPerInd;
	Init();
}

RealJSFormatter::RealJSFormatter(bool bSkipCR, bool bPutCR):
	chIndent('\t'),
	nChPerInd(1),
	bNLBracket(false)
{
	this->bSkipCR = bSkipCR;
	this->bPutCR = bPutCR;
	Init();
}

RealJSFormatter::RealJSFormatter(char chIndent, int nChPerInd, bool bSkipCR, bool bPutCR, bool bNLBracket)
{
	this->chIndent = chIndent;
	this->nChPerInd = nChPerInd;
	this->bSkipCR = bSkipCR;
	this->bPutCR = bPutCR;
	this->bNLBracket = bNLBracket;
	Init();
}

void RealJSFormatter::Init()
{
	bRegular = false;
	bPosNeg = false;
	nIndents = 0;
	bNewLine = false;
	nIfLikeBlock = 0;
	nDoLikeBlock = 0;
	nSwitchBlock = 0;
	bBlockStmt = true;
	bAssign = false;
	bEmptyBracket = false;
	bCommentPut = false;

	blockMap[string("if")] = IF;
	blockMap[string("else")] = ELSE;
	blockMap[string("for")] = FOR;
	blockMap[string("do")] = DO;
	blockMap[string("while")] = WHILE;
	blockMap[string("switch")] = SWITCH;
	blockMap[string("case")] = CASE;
	blockMap[string("default")] = CASE;
	blockMap[string("try")] = TRY;
	blockMap[string("catch")] = CATCH;
	blockMap[string("function")] = FUNCTION;
	blockMap[string("{")] = BLOCK;
	blockMap[string("(")] = BRACKET;
	blockMap[string("[")] = SQUARE;

	specKeywordSet.insert("if");
	specKeywordSet.insert("for");
	specKeywordSet.insert("while");
	specKeywordSet.insert("switch");
	specKeywordSet.insert("catch");
	specKeywordSet.insert("function");
	specKeywordSet.insert("with");
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
	if(!bRegular && !bPosNeg)
	{
		tokenBType = STRING_TYPE;
		tokenB = "";
	}
	else if(bRegular)
	{
		tokenBType = REGULAR_TYPE; // ����
		//tokenB.push_back('/');
	}
	else
	{
		tokenBType = STRING_TYPE; // ������
	}

	bool bQuote = false;
	bool bComment = false;
	bool bFirst = true;
	bool bNum = false; // �ǲ�������
	char chQuote; // ��¼�������� ' �� "
	char chComment; // ע������ / �� *

	while(1)
	{
		charA = charB;
		if(charA == EOF)
			return;
		do
		{
			charB = GetChar();
		} while(bSkipCR && charB == '\r');

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
			
			// ������� 82e-2 442e+6 ������
			// ��Ϊ����������������ֻ�ܷ������µı����ʽ
			bool bNumOld = bNum;
			if(bFirst || bNumOld) // ֻ��֮ǰ�����ֲŸı�״̬
			{
				bNum = IsNumChar(charA);
				bFirst = false;
			}
			if(bNumOld && !bNum && charA == 'e' && 
				(charB == '-' || charB == '+' || IsNumChar(charB)))
			{
				bNum = true;
				if(charB == '-' || charB == '+')
				{
					tokenB.push_back(charB);
					charB = GetChar();
				}
			}

			if(!IsNormalChar(charB)) // loop until charB is not normal char
			{
				bPosNeg = false;
				return;
			}
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

			// ���ַ�����
			if(charB == '=' || charB == charA)
			{
				// ��ȷ�Ƕ��ַ�����
				tokenBType = OPER_TYPE;
				tokenB.push_back(charA);
				tokenB.push_back(charB);
				charB = GetChar();
				if((!tokenB.compare("==") || !tokenB.compare("!=") ||
					!tokenB.compare("<<") || !tokenB.compare(">>")) && charB == '=')
				{
					// ���ַ� ===, !==, <<=, >>=
					tokenB.push_back(charB);
					charB = GetChar();
				}
				else if(!tokenB.compare(">>") && charB == '>')
				{
					// >>>, >>>=
					tokenB.push_back(charB);
					charB = GetChar();
					if(charB == '=') // >>>=
					{
						tokenB.push_back(charB);
						charB = GetChar();
					}
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
	bCommentPut = false; // ���һ���ᷢ����ע��֮����κ��������
}

void RealJSFormatter::PutString(const string& str)
{
	size_t length = str.size();
	for(size_t i = 0; i < length; ++i)
	{
		if(bNewLine && (bCommentPut || 
			((bNLBracket || str[i] != '{') && str[i] != ',' && str[i] != ';')))
		{
			// ���к��治�ǽ����� {,; ��������
			if(bPutCR)
				PutChar('\r');
			PutChar('\n');
			bNewLine = false;
			int inds = nIndents;
			if(str[i] == '{' || str[i] == ',' || str[i] == ';') // �н�β��ע�ͣ�ʹ��{,;���ò�����
				--inds;
			if(bAssign)
				++inds;
			for(int c = 0; c < inds; ++c)
				for(int c2 = 0; c2 < nChPerInd; ++c2)
					PutChar(chIndent);
		}

		if(bNewLine && !bCommentPut &&  
			((!bNLBracket && str[i] == '{') || str[i] == ',' || str[i] == ';'))
			bNewLine = false;
		//if(bNewLine && !bCommentPut && str[i] == '{')
		//{
		//	bNewLine = false;
		//	PutChar(' '); // ������ { ��һ���ո�,; ֱ�Ӳ����о�����
		//}

		if(str[i] == '\n')
			bNewLine = true;
		else
			PutChar(str[i]);
	}
}

void RealJSFormatter::PrepareRegular()
{
	/* 
	 * �ȴ���һ������
	 * tokenB[0] == /���� tokenB ����ע��
	 * tokenA ���� STRING (���� tokenA == return)
	 * ���� tokenA �����һ���ַ���������Щ
	*/
	size_t last = tokenA.size() > 0 ? tokenA.size() - 1 : 0;
	char tokenALast = tokenA[last];
	char tokenBFirst = tokenB[0];
	if(tokenBType != COMMENT_TYPE_1 && tokenBType != COMMENT_TYPE_2 && 
		(tokenAType != STRING_TYPE && tokenBFirst == '/' && 
		(tokenALast == '(' || tokenALast == ',' || tokenALast == '=' ||
			tokenALast == ':' || tokenALast == '[' || tokenALast == '!' ||
               tokenALast == '&' || tokenALast == '|' || tokenALast == '?' ||
               tokenALast == '+' || tokenALast == '{' || tokenALast == '}' || 
			tokenALast == ';' || tokenALast == '\n')) || 
			(!tokenA.compare("return") && tokenBFirst == '/'))
	{
		bRegular = true;
		GetToken(); // ���������ݼӵ� TokenB
	}
}

void RealJSFormatter::PreparePosNeg()
{
	/*
	 * ��� tokenB �� -,+ ��
	 * ���� tokenA �����ַ�����Ҳ����������ʽ
	 * ���� tokenA ���� ++, --, ], )
	 * ���� charB ��һ�� NormalChar
	 * ��ô tokenB ʵ������һ��������
	 */
	if(tokenBType == OPER_TYPE && (!tokenB.compare("-") || !tokenB.compare("+")) && 
		tokenAType != STRING_TYPE && tokenAType != REGULAR_TYPE &&
		tokenA.compare("++") && tokenA.compare("--") && 
		tokenA.compare("]") && tokenA.compare(")") && 
		IsNormalChar(charB))
	{
		// tokenB ʵ������������
		bPosNeg = true;
		GetToken();
	}
}

void RealJSFormatter::PrepareTokenB()
{
	char stackTop = blockStack.top();
	/*if(((stackTop == 'i' || stackTop == 'e' || stackTop == 'd' || 
		stackTop == 'f' || stackTop == 'w') && !tokenA.compare(";")) ||
		((nIfLikeBlock || nDoLikeBlock) && stackTop == '{' && !tokenA.compare("}")))
	{*/
	/*
	 * ���� else, while, catch, ',', ';', ')', { ֮ǰ�Ļ���
	 * ����������Ĳ��������Ǽ������ٰ�ȥ���Ļ��в���
	 */
	int c = 0;
	while(!tokenB.compare("\n") || !tokenB.compare("\r\n")) 
	{
		++c;
		GetToken();
	}
		
	if(tokenB.compare("else") && tokenB.compare("while") && tokenB.compare("catch") && 
		tokenB.compare(",") && tokenB.compare(";") && tokenB.compare(")"))
	{
		// ��ȥ���Ļ���ѹ����У��ȴ���
		if(bNLBracket && !tokenB.compare("{"))
			return;

		if(!tokenA.compare("{") && !tokenB.compare("}"))
			return; // �� {}

		TokenAndType temp;
		c = c > 2 ? 2 : c;
		for(; c > 0; --c)
		{
			temp.token = string("\n");
			temp.type = OPER_TYPE;
			tokenBQueue.push(temp);
		}
		temp.token = tokenB;
		temp.type = tokenBType;
		tokenBQueue.push(temp);
		temp = tokenBQueue.front();
		tokenBQueue.pop();
		tokenB = temp.token;
		tokenBType = temp.type;
	}
	//}
}

void RealJSFormatter::PopMultiBlock(char previousStackTop)
{
	if(!tokenB.compare(";")) // ��� tokenB �� ;����������������������
		return;

	if(!((previousStackTop == IF && !tokenB.compare("else")) || 
		(previousStackTop == DO && !tokenB.compare("while")) ||
		(previousStackTop == TRY && !tokenB.compare("catch"))))
	{
		char topStack = blockStack.top();
		// ; �����ܿ��ܽ������ if, do, while, for, try, catch
		while(topStack == IF || topStack == FOR || topStack == WHILE || 
			topStack == DO || topStack == ELSE || topStack == TRY || topStack == CATCH)
		{
			if(topStack == IF || topStack == FOR || topStack == WHILE || topStack == CATCH)
			{
				--nIfLikeBlock;
				blockStack.pop();
				--nIndents;
			}
			else if(topStack == ELSE || topStack == TRY)
			{
				--nDoLikeBlock;
				blockStack.pop();
				--nIndents;
			}
			else if(topStack == DO)
			{
				--nIndents;
			}

			if((topStack == IF && !tokenB.compare("else")) ||
				(topStack == DO && !tokenB.compare("while")) ||
				(topStack == TRY && !tokenB.compare("catch")))
				break; // ֱ���ոս���һ�� if...else, do...while, try...catch
			topStack = blockStack.top();
		}
	}
}

void RealJSFormatter::Go()
{
	blockStack.push(' ');
	brcNeedStack.push(true);
	GetToken(true);

	bool bHaveNewLine;
	char tokenAFirst;
	char tokenBFirst;

	while(charA != EOF)
	{
		PrepareRegular(); // �ж�����
		PreparePosNeg(); // �ж�������

		tokenA = tokenB;
		tokenAType = tokenBType;
		
		if(tokenBQueue.size() == 0)
		{
			GetToken();
			PrepareTokenB(); // �����ǲ���Ҫ��������
		}
		else
		{
			// ���ŶӵĻ���
			TokenAndType temp;
			temp = tokenBQueue.front();
			tokenBQueue.pop();
			tokenB = temp.token;
			tokenBType = temp.type;
		}

		bHaveNewLine = false;
		tokenAFirst = tokenA[0];
		tokenBFirst = tokenB[0];
		if(tokenBFirst == '\r')
			tokenBFirst = '\n';
		if(tokenBFirst == '\n' || tokenBType == COMMENT_TYPE_1)
			bHaveNewLine = true;

		if(!bBlockStmt && tokenA.compare("{") && tokenA.compare("\n") 
			&& tokenAType != COMMENT_TYPE_1 && tokenAType != COMMENT_TYPE_2)
			bBlockStmt = true;

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
					PutToken(tokenA, string(""), string("\n")); // ��Ҫ����
				else
					PutToken(tokenA);
			}
			else
			{
				// ����ע��
				PutToken(tokenA); // �϶��ỻ�е�
			}
			bCommentPut = true;
			break;
		case OPER_TYPE:
			ProcessOper(bHaveNewLine, tokenAFirst, tokenBFirst);

			break;
		case STRING_TYPE:
			ProcessString(bHaveNewLine, tokenAFirst, tokenBFirst);
			break;
		}
	}
}

void RealJSFormatter::ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	string strRight = bNLBracket ? string() : string(" ");

	if(!tokenA.compare("(") || !tokenA.compare(")") || 
		!tokenA.compare("[") || !tokenA.compare("]") ||
		!tokenA.compare("!") || !tokenA.compare("!!") ||
		!tokenA.compare("~") || !tokenA.compare("^") ||
		!tokenA.compare("."))
	{
		// ()[]!. ����ǰ��û����ʽ�������
		if((!tokenA.compare(")") && blockStack.top() == BRACKET) ||
			(!tokenA.compare("]") && blockStack.top() == SQUARE))
		{
			bAssign = false;
			// )] ��Ҫ��ջ����������
			blockStack.pop();
			--nIndents;
		}

		if(!tokenA.compare(")") && (nIfLikeBlock || nSwitchBlock) && !brcNeedStack.top() &&
			(blockStack.top() == IF || blockStack.top() == FOR || blockStack.top() == WHILE ||
			blockStack.top() == SWITCH || blockStack.top() == CATCH)) 
		{
			// ջ���� if, for, while, switch, catch ���ڵȴ� )��֮������������
			// ) { ֮��Ŀո����������ʱ�ᴦ��
			// ����Ŀո������Ŀո������� { �ģ�bNLBracket Ϊ true ����Ҫ�ո���
			string rightDeco = tokenB.compare(";") ? strRight : string();
			if(!bHaveNewLine)
				rightDeco.append("\n"); 
			PutToken(tokenA, string(""), rightDeco);
			//bBracket = true;
			brcNeedStack.pop();
			bBlockStmt = false; // �ȴ� statment
			if(blockStack.top() == WHILE)
			{
				blockStack.pop();
				if(blockStack.top() == DO)
				{
					// ���� do...while
					--nIfLikeBlock;
					--nDoLikeBlock;
					blockStack.pop();

					PopMultiBlock(WHILE);
				}
				else
				{
					blockStack.push(WHILE);
					++nIndents;
				}
			}
			else
				++nIndents;
		}
		else if(!tokenA.compare(")") && (!tokenB.compare("{") || bHaveNewLine))
			PutToken(tokenA, string(""), strRight); // ����Ŀո�Ҳ������ { ��
		else
			PutToken(tokenA); // �������

		if(!tokenA.compare("(") || !tokenA.compare("["))
		{
			// ([ ��ջ����������
			blockStack.push(blockMap[tokenA]);
			++nIndents;
		}

		return;
	}
			
	if(!tokenA.compare(";"))
	{
		bAssign = false;
		char topStack = blockStack.top();
		if(nIfLikeBlock || nDoLikeBlock)
		{
			// ; ���� if, else, while, for, try, catch
			if(topStack == IF || topStack == FOR || 
				topStack == WHILE || topStack == CATCH)
			{
				--nIfLikeBlock;
				blockStack.pop();
				--nIndents;
			}
			if(topStack == ELSE || topStack == TRY)
			{
				--nDoLikeBlock;
				blockStack.pop();
				--nIndents;
			}
			if(topStack == DO)
				--nIndents;
			// do �ڶ�ȡ�� while ����޸ļ���
			// ���� do{} Ҳһ��

			// ����� } ��ͬ���Ĵ���
			PopMultiBlock(topStack);
		}

		//if(blockStack.top() == 't')
			//blockStack.pop(); // ; Ҳ����������������ʱ��ѹ�� t

		topStack = blockStack.top();
		if(topStack != BRACKET && !bHaveNewLine)
			PutToken(tokenA, string(""), string("\n")); // ������� () ��� ; �ͻ���
		else if(topStack == BRACKET || tokenBType == COMMENT_TYPE_1)
			PutToken(tokenA, string(""), string(" ")); // (; ) �ո�
		else
			PutToken(tokenA);

		return;
	}

	if(!tokenA.compare(","))
	{
		bAssign = false;
		if(blockStack.top() == BLOCK && !bHaveNewLine)
			PutToken(tokenA, string(""), string("\n")); // ����� {} ���
		else
			PutToken(tokenA, string(""), string(" "));

		return;
	}
			
	if(!tokenA.compare("{"))
	{
		bAssign = false;
		char topStack = blockStack.top();
		if((nIfLikeBlock || nDoLikeBlock || nSwitchBlock) && 
			(topStack == IF || topStack == FOR || 
			topStack == WHILE || topStack == DO || 
			topStack == ELSE || topStack == SWITCH ||
			topStack == TRY || topStack == CATCH))
		{
			if(!bBlockStmt)
			{
				//blockStack.pop(); // �����Ǹ������������� } ʱһ��
				--nIndents;
				bBlockStmt = true;
			}
			else
			{
				blockStack.push(HELPER); // ѹ��һ�� HELPER �����ж�
			}
		}	

		blockStack.push(blockMap[tokenA]); // ��ջ����������
		++nIndents;

		if(!tokenB.compare("}"))
		{
			// �� {}
			bEmptyBracket = true;
			if(bNewLine == false && bNLBracket && 
				(topStack == IF || topStack == FOR || 
				topStack == WHILE || topStack == SWITCH ||
				topStack == CATCH || topStack == FUNCTION))
			{
				PutToken(tokenA, string(" "));
			}
			else
			{
				PutToken(tokenA);
			}
		}
		else
		{
			string strLeft = (bNLBracket && !bNewLine) ? string("\n") : string("");
			if(!bHaveNewLine) // ��Ҫ����
				PutToken(tokenA, strLeft, string("\n"));
			else if(tokenBType == COMMENT_TYPE_1)
				PutToken(tokenA, strLeft, string(" "));
			else
				PutToken(tokenA, strLeft);
		}

		return;
	}

	if(!tokenA.compare("}"))
	{
		bAssign = false;
		char topStack = blockStack.top();

		// �����Ĳ��ԣ�} һֱ���� {
		// ���������ٿ���ʹ�� {} ֮������ȷ��
		while(1)
		{
			if(topStack == BLOCK)
				break;

			blockStack.pop();

			switch(topStack)
			{
			case IF:
			case FOR:
			case WHILE:
			case CATCH:
				--nIfLikeBlock;
				--nIndents;
				break;
			case DO:
			case ELSE:
			case TRY:
				--nDoLikeBlock;
				--nIndents;
				break;
			case SWITCH:
				--nSwitchBlock;
				--nIndents;
				break;
			}

			topStack = blockStack.top();
		}

		if(topStack == BLOCK)
		{
			// ��ջ����С����
			blockStack.pop();
			--nIndents;
			topStack = blockStack.top();

			switch(topStack)
			{
			case IF:
			case FOR:
			case WHILE:
			case CATCH:
				--nIfLikeBlock;
				blockStack.pop();
				break;
			case ELSE:
			case TRY:
				--nDoLikeBlock;
				blockStack.pop();
				break;
			case DO:
				// �����Ѿ�����do ���� while
				break;
			case SWITCH:
				--nSwitchBlock;
				blockStack.pop();
				break;
			case FUNCTION:
			case HELPER:
				blockStack.pop();
				break;
			}
			//topStack = blockStack.top();
		}

		string leftStyle("");
		if(!bNewLine)
			leftStyle = "\n";
		if(bEmptyBracket)
		{
			leftStyle = "";
			strRight = "\n";
			bEmptyBracket = false;
		}

		if((!bHaveNewLine && tokenBFirst != ';' && tokenBFirst != ',')
			&& !(!bNLBracket && topStack == DO && !tokenB.compare("while")) && 
			!(!bNLBracket && topStack == IF && !tokenB.compare("else")) &&
			!(!bNLBracket && topStack == TRY && !tokenB.compare("catch")) &&
			!(!bNLBracket && !tokenB.compare(")")))
			PutToken(tokenA, leftStyle, string("\n")); // һЩ�������
		else if(tokenBType == STRING_TYPE || tokenBType == COMMENT_TYPE_1)
			PutToken(tokenA, leftStyle, strRight); // Ϊ else ׼���Ŀո�
		else
			PutToken(tokenA, leftStyle); // }, }; })
		// ע�� ) ��Ҫ�����ʱ���� ,; ȡ��ǰ��Ļ���

		PopMultiBlock(topStack);

		return;
	}

	if(!tokenA.compare("++") || !tokenA.compare("--") || 
		!tokenA.compare("\n") || !tokenA.compare("\r\n"))
	{
		PutToken(tokenA);
		return;
	}

	if(!tokenA.compare(":") && blockStack.top() == CASE)
	{
		// case, default
		if(!bHaveNewLine)
			PutToken(tokenA, string(""), string(" \n"));
		else
			PutToken(tokenA, string(""), string(" "));
		blockStack.pop();
		return;
	}

	if(!tokenA.compare("="))
		bAssign = true;

	PutToken(tokenA, string(" "), string(" ")); // ʣ��Ĳ��������� �ո�oper�ո�
}

void RealJSFormatter::ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst)
{
	if((!tokenA.compare("case") || !tokenA.compare("default")) && bNewLine)
	{
		// case, default ��������һ��
		--nIndents;
		string rightDeco = tokenA.compare("default") ? string(" ") : string();
		PutToken(tokenA, string(""), rightDeco);
		++nIndents;
		blockStack.push(blockMap[tokenA]);
		return;
	}
			
	if(!tokenA.compare("do") || 
		(!tokenA.compare("else") && tokenB.compare("if")) ||
		!tokenA.compare("try"))
	{
		// do, else (NOT else if), try
		PutToken(tokenA);

		++nDoLikeBlock;
		blockStack.push(blockMap[tokenA]);
		++nIndents; // ���� ()��ֱ������
		bBlockStmt = false; // �ȴ� block �ڲ��� statment
				
		if((tokenBType == STRING_TYPE || bNLBracket) && !bHaveNewLine)
		{
			PutString(string("\n"));
		}
		else
		{
			PutString(string(" "));
		}
		return;
	}

	if(!tokenA.compare("function"))
	{
		bAssign = false;
		blockStack.push(blockMap[tokenA]); // �� function Ҳѹ��ջ������ } ����
	}

	if(tokenBType == STRING_TYPE)
	{
		PutToken(tokenA, string(""), string(" "));

		//if(blockStack.top() != 't' && IsType(tokenA))
			//blockStack.push('t'); // ��������
	}
	else
	{
		if(specKeywordSet.find(tokenA) != specKeywordSet.end() &&
			!tokenB.compare("("))
			PutToken(tokenA, string(""), string(" "));
		else
			PutToken(tokenA);

		if(!tokenA.compare("if") || !tokenA.compare("for") || 
			!tokenA.compare("while") || !tokenA.compare("catch"))
		{
			++nIfLikeBlock;
			//bBracket = false; // �ȴ� ()��() ��������ܼ�����
			brcNeedStack.push(false);
			//if(tokenA.compare("catch"))
			//	blockStack.push(tokenA[0]);
			//else
			//	blockStack.push('h');
			blockStack.push(blockMap[tokenA]);

		}

		if(!tokenA.compare("switch"))
		{
			++nSwitchBlock;
			//bBracket = false;
			brcNeedStack.push(false);
			blockStack.push(blockMap[tokenA]);
		}
	}
}

