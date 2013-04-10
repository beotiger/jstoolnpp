/* realjsformatter.h
   2010-12-16

Copyright (c) 2010-2013 SUN Junwen

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
#include <ctime>
#include <string>
#include <map>
#include <set>

#include "jsparser.h"

using namespace std;

class RealJSFormatter: public JSParser
{
public:
	typedef map<string, char> StrCharMap;
	typedef set<string> StrSet;

	/*
	 * CR_READ
	 *   READ_CR ��ȡ \r
	 *   SKIP_READ_CR ��ȡʱ���� \r
	 */
	enum CR_READ { SKIP_READ_CR, READ_CR };
	/*
	 * CR_PUT
	 *   PUT_CR ����ʹ�� \r\n
	 *   NOT_PUT_CR ����ʹ�� \n
	 */
	enum CR_PUT { NOT_PUT_CR, PUT_CR };
	/*
	 * BRAC_NEWLINE
	 *   NEWLINE_BRAC ����ǰ����
	 *   NO_NEWLINE_BRAC ����ǰ������
	 */
	enum BRAC_NEWLINE { NO_NEWLINE_BRAC, NEWLINE_BRAC };
	/*
	 * INDENT_IN_EMPTYLINE
	 *   INDENT_IN_EMPTYLINE ������������ַ�
	 *   NO_INDENT_IN_EMPTYLINE ���в���������ַ�
	 */
	enum EMPTYLINE_INDENT { NO_INDENT_IN_EMPTYLINE, INDENT_IN_EMPTYLINE };

	struct FormatterOption 
	{
		char chIndent;
		int nChPerInd;
		CR_READ eCRRead;
		CR_PUT eCRPut;
		BRAC_NEWLINE eBracNL;
		EMPTYLINE_INDENT eEmpytIndent;

		FormatterOption():
			chIndent('\t'),
			nChPerInd(1),
			eCRRead(SKIP_READ_CR),
			eCRPut(NOT_PUT_CR),
			eBracNL(NO_NEWLINE_BRAC),
			eEmpytIndent(NO_INDENT_IN_EMPTYLINE)
		{}

		FormatterOption(char op_chIndent,
						int op_nChPerInd,
						CR_READ op_eCRRead,
						CR_PUT op_eCRPut,
						BRAC_NEWLINE op_eBracNL,
						EMPTYLINE_INDENT op_eEmpytIndent):
			chIndent(op_chIndent),
			nChPerInd(op_nChPerInd),
			eCRRead(op_eCRRead),
			eCRPut(op_eCRPut),
			eBracNL(op_eBracNL),
			eEmpytIndent(op_eEmpytIndent)
		{}
	};

	RealJSFormatter(FormatterOption option);

	virtual ~RealJSFormatter()
	{}

	inline void SetInitIndent(const string& initIndent)
	{ m_initIndent = initIndent; }

	void Go();

	static string Trim(const string& str);
	static string TrimSpace(const string& str);
	static string TrimRightSpace(const string& str);
	void StringReplace(string& strBase, const string& strSrc, const string& strDes);

	bool m_debugOutput;

private:
	void Init();

	virtual void PutChar(int ch) = 0;

	void PopMultiBlock(char previousStackTop);
	void ProcessOper(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);
	void ProcessString(bool bHaveNewLine, char tokenAFirst, char tokenBFirst);

	void PutToken(const string& token,
		const string& leftStyle = string(""),
		const string& rightStyle = string("")); // Put a token out with style
	void PutString(const string& str);
	void PutLineBuffer();

	clock_t m_startClock;
	clock_t m_endClock;
	double m_duration;

	int m_nLineIndents;
	string m_lineBuffer;

	StrSet m_specKeywordSet; // ����Ҫ�������ŵĹؼ��ּ���
	StrCharMap m_blockMap;
	CharStack m_blockStack;
	int m_nIndents; // �������������ü��� blockStack��Ч������

	// ʹ��ջ��Ϊ�˽�����ж������г���ѭ��������
	BoolStack m_brcNeedStack; // if ֮��ĺ��������

	bool m_bNewLine; // ׼�����еı�־
	bool m_bBlockStmt; // block ������ʼ��
	bool m_bAssign;
	bool m_bEmptyBracket; // �� {}

	bool m_bCommentPut; // �ո������ע��

	string m_initIndent; // ��ʼ����

	// ����Ϊ������
	FormatterOption m_struOption;

private:
	// ��ֹ����
	RealJSFormatter(const RealJSFormatter&);
	RealJSFormatter& operator=(const RealJSFormatter&);
};

#endif
