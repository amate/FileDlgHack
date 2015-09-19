// stdafx.cpp : �W���C���N���[�h FileDlgHack.pch �݂̂�
// �܂ރ\�[�X �t�@�C���́A�v���R���p�C���ς݃w�b�_�[�ɂȂ�܂��B
// stdafx.obj �ɂ̓v���R���p�C���ς݌^��񂪊܂܂�܂��B

#include "stdafx.h"

// TODO: ���̃t�@�C���ł͂Ȃ��ASTDAFX.H �ŕK�v��
// �ǉ��w�b�_�[���Q�Ƃ��Ă��������B

/*	// �f�o�b�O�E�B���h�E�ɏo�͂���ver
void DebugTrace(const char *format, ...)
{
	char buf[1024];
	va_list list;
	va_start(list, format);

	wvsprintf( buf,format, list);

	OutputDebugString(buf);
	va_end(list);
}

void DebugTrace(const char *str)
{
	OutputDebugString(str);
}
*/

#include "FileDlgHack.h"

void LogFilePath(TCHAR* path)
{
	static TCHAR LogPath[MAX_PATH] = L"\0";
	if (LogPath[0] == L'\0') {
		lstrcpy(LogPath, g_Shared->szIniPath);
		int len = lstrlen(LogPath);
	 	LogPath[len - 3] = L'l';
	 	LogPath[len - 2] = L'o';
		LogPath[len - 1] = L'g';
	}
	lstrcpy(path, LogPath); 
}

void DebugTrace(const char *format, ...)
{
	char str[1024];
	va_list list;
	va_start(list, format);

	wvsprintfA(str, format, list);
	va_end(list);

	FILE *fp = nullptr;
	TCHAR LogPath[MAX_PATH];
	LogFilePath(LogPath);
	if (::strcmp(str, "open") == 0) {	// ��̃t�@�C�����쐬����
		fp = _wfopen(LogPath, L"w");
		fclose(fp);
		return ;
	}
	fp = _wfopen(LogPath, L"a");

	fputs(str, fp);				// �t�@�C���ɏ�������

	fclose(fp);
}

#if 0
void DebugTrace(const char *str)
{
	FILE *fp;
	char LogPath[MAX_PATH];
	LogFilePath(LogPath);

	fp = fopen(LogPath, "a");

	fputs(str, fp);				// �t�@�C���ɏ�������

	fclose(fp);
}
#endif
