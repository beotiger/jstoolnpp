#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>

using namespace std;

#if defined(UNICODE) || defined(_UNICODE)
typedef wstring tstring;
#else
typedef string tstring;
#endif

struct StruOptions
{
	bool bPutCR; // �Ƿ���� \r\n
	char chIndent; // �����õ��ַ�
	int nChPerInd; // ÿ������ʹ�ü����ַ�
	bool bNLBracket; // { ֮ǰ�Ƿ���
	bool bKeepTopComt; // �Ƿ���ͷ����ע��
};

const string keyPutCR("Put CR");
const string keyChIndent("Indent char");
const string keyChPerInd("Chars per indent");
const string keyNLBracket("New line before {");
const string keyKeepTopComt("Kepp top comment");

tstring GetConfigFilePath(HWND nppHandle);

void loadOption(HWND nppHandle, StruOptions& struOptions);
void loadDefaultOption(StruOptions& struOptions);

void saveOption(HWND nppHandle, StruOptions struOptions);

#endif
