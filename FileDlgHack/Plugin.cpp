/*=========================================================================

                TTBase�v���O�C���e���v���[�g(Plugin.pas)

                ���̃t�@�C���͂ł��邾���ύX���Ȃ��B
                Main.pas�ɏ������������Ƃ������߂��܂��B

 =========================================================================*/

#include "stdafx.h"
#include <assert.h>
#include "Plugin.h"
#include "FileDlgHack.h"
#include "MessageDef.h"

/* ---------------------------------------------------------*/
/*      �{�̑��G�N�X�|�[�g�֐�                              */
/* ---------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN64
PPLUGIN_INFO_W (WINAPI *TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
void (WINAPI *TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PPLUGIN_INFO_W PluginInfo);
void (WINAPI *TTBPlugin_FreePluginInfo)(PPLUGIN_INFO_W PluginInfo);
void (WINAPI *TTBPlugin_SetMenuProperty)(DWORD_PTR hPlugin, int CommandID, DWORD ChangeFlag, DWORD Flag);
PPLUGIN_INFO_ARRAY_W (WINAPI *TTBPlugin_GetAllPluginInfo) ();
void (WINAPI *TTBPlugin_FreePluginInfoArray)(PPLUGIN_INFO_ARRAY_W PluginInfoArray);
void (WINAPI *TTBPlugin_SetTaskTrayIcon)(HICON hIcon, PCTSTR Tips);
void (WINAPI *TTBPlugin_WriteLog)( DWORD_PTR hPlugin, int logLevel, PCTSTR msg );
BOOL (WINAPI *TTBPlugin_ExecuteCommand)( PCTSTR PluginFilename, int CmdID );
#else
PPLUGIN_INFO_A (WINAPI *TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
void (WINAPI *TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PPLUGIN_INFO_A PluginInfo);
void (WINAPI *TTBPlugin_FreePluginInfo)(PPLUGIN_INFO_A PluginInfo);
void (WINAPI *TTBPlugin_SetMenuProperty)(DWORD_PTR hPlugin, int CommandID, DWORD ChangeFlag, DWORD Flag);
PPLUGIN_INFO_ARRAY_A (WINAPI *TTBPlugin_GetAllPluginInfo) ();
void (WINAPI *TTBPlugin_FreePluginInfoArray)(PPLUGIN_INFO_ARRAY_A PluginInfoArray);
void (WINAPI *TTBPlugin_SetTaskTrayIcon)(HICON hIcon, PCTSTR Tips);
void (WINAPI *TTBPlugin_WriteLog)( DWORD_PTR hPlugin, int logLevel, PCTSTR msg );
BOOL (WINAPI *TTBPlugin_ExecuteCommand)( PCTSTR PluginFilename, int CmdID );
#endif
#ifdef __cplusplus
};
#endif
/* ---------------------------------------------------------*/
/*      �O���[�o���ϐ�                                      */
/* ---------------------------------------------------------*/
#ifdef _WIN64
PWCHAR PLUGIN_FILENAME = NULL;		// �v���O�C���̃t�@�C�����BTTBase����̑��΃p�X
#else
PCHAR PLUGIN_FILENAME = NULL;		// �v���O�C���̃t�@�C�����BTTBase����̑��΃p�X
#endif
DWORD_PTR PLUGIN_HANDLE = 0;		// �v���O�C����TTBase�ŔF�����邽�߂̎��ʃR�[�h

/****************************************************************/
/*                                                              */
/*          ���[�e�B���e�B���[�`��                              */
/*                                                              */
/****************************************************************/
/* ---------------------------------------------------------*/
/*      �v���O�C�����\���̂�Src���R�s�[���ĕԂ�           */
/* ---------------------------------------------------------*/
#ifdef _WIN64
PPLUGIN_INFO_W CopyPluginInfo(PPLUGIN_INFO_W Src)
#else
PPLUGIN_INFO_A CopyPluginInfo(PPLUGIN_INFO_A Src)
#endif
{
	DWORD i;
	PPLUGIN_INFO pPinfoResult;

	pPinfoResult = (PPLUGIN_INFO)calloc (1, sizeof(PLUGIN_INFO));
	*pPinfoResult = *Src;
#ifdef _WIN64
	pPinfoResult->Name = (PWCHAR)calloc(lstrlenW(Src->Name) + 1, sizeof(WCHAR));
	lstrcpyW(pPinfoResult->Name, Src->Name);
	pPinfoResult->Filename = (PWCHAR) calloc(lstrlenW(Src->Filename) + 1, sizeof(WCHAR));
	lstrcpyW(pPinfoResult->Filename, Src->Filename);
#else
	pPinfoResult->Name = (PCHAR)calloc(lstrlenA(Src->Name) + 1, sizeof(CHAR));
	lstrcpyA(pPinfoResult->Name, Src->Name);
	pPinfoResult->Filename = (PCHAR) calloc(lstrlenA(Src->Filename) + 1, sizeof(CHAR));
	lstrcpyA(pPinfoResult->Filename, Src->Filename);
#endif
	pPinfoResult->Commands = (PPLUGIN_COMMAND_INFO)calloc(Src->CommandCount, sizeof(PLUGIN_COMMAND_INFO));
	for (i = 0 ; i < Src->CommandCount ; i += 1)
	{
		pPinfoResult->Commands[i] = Src->Commands[i];
#ifdef _WIN64
		pPinfoResult->Commands[i].Name = (PWCHAR)calloc(lstrlenW(Src->Commands[i].Name) + 1, sizeof(WCHAR));
		lstrcpyW(pPinfoResult->Commands[i].Name, Src->Commands[i].Name);
		pPinfoResult->Commands[i].Caption = (PWCHAR)calloc(lstrlenW(Src->Commands[i].Caption) + 1, sizeof(WCHAR));
		lstrcpyW(pPinfoResult->Commands[i].Caption, Src->Commands[i].Caption);
#else
		pPinfoResult->Commands[i].Name = (PCHAR)calloc(lstrlenA(Src->Commands[i].Name) + 1, sizeof(CHAR));
		lstrcpyA(pPinfoResult->Commands[i].Name, Src->Commands[i].Name);
		pPinfoResult->Commands[i].Caption = (PCHAR)calloc(lstrlenA(Src->Commands[i].Caption) + 1, sizeof(CHAR));
		lstrcpyA(pPinfoResult->Commands[i].Caption, Src->Commands[i].Caption);
#endif
	}
	return pPinfoResult;
}

/* ---------------------------------------------------------*/
/*  �v���O�C�����ō쐬���ꂽ�v���O�C�����\���̂�j������  */
/* ---------------------------------------------------------*/
void FreePluginInfo(PPLUGIN_INFO PluginInfo)
{
	TTBEvent_FreePluginInfo(PluginInfo);
}

/* ---------------------------------------------------------*/
/*      �o�[�W��������Ԃ�                                */
/* ---------------------------------------------------------*/
#ifdef _WIN64
void GetVersion(PWSTR Filename, DWORD* VersionMS, DWORD* VersionLS)
#else
void GetVersion(PSTR Filename, DWORD* VersionMS, DWORD* VersionLS)
#endif
{
	DWORD VersionHandle;
	DWORD VersionSize;
	LPVOID pVersionInfo;
	UINT itemLen;
	VS_FIXEDFILEINFO *FixedFileInfo;

	HMODULE hModule = LoadLibrary(TEXT("version.dll"));
	if (hModule == NULL)
		return;
#ifdef _WIN64
typedef DWORD (APIENTRY *GetFileVersionInfoSizeW)(__in LPCWSTR lptstrFilename, __out_opt LPDWORD lpdwHandle);
typedef BOOL (APIENTRY *GetFileVersionInfoW)(__in LPCWSTR lptstrFilename, __in DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData);
typedef BOOL (APIENTRY *VerQueryValueW)(const LPVOID pBlock, LPWSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);
GetFileVersionInfoSizeW _GetFileVersionInfoSize;
GetFileVersionInfoW _GetFileVersionInfo;
VerQueryValueW _VerQueryValue;
(FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeW");
(FARPROC&)_GetFileVersionInfo = GetProcAddress(hModule, "GetFileVersionInfoW");
(FARPROC&)_VerQueryValue = GetProcAddress(hModule, "VerQueryValueW");
PWSTR pSubBlock = L"\\";
#else
typedef DWORD (APIENTRY *GetFileVersionInfoSizeA)(__in LPCSTR lptstrFilename, __out_opt LPDWORD lpdwHandle);
typedef BOOL (APIENTRY *GetFileVersionInfoA)(__in LPCSTR lptstrFilename, __in DWORD dwHandle, __in DWORD dwLen, __out_bcount(dwLen) LPVOID lpData);
typedef BOOL (APIENTRY *VerQueryValueA)(const LPVOID pBlock, LPSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen);
GetFileVersionInfoSizeA _GetFileVersionInfoSize;
GetFileVersionInfoA _GetFileVersionInfo;
VerQueryValueA _VerQueryValue;
(FARPROC&)_GetFileVersionInfoSize = GetProcAddress(hModule, "GetFileVersionInfoSizeA");
(FARPROC&)_GetFileVersionInfo = GetProcAddress(hModule, "GetFileVersionInfoA");
(FARPROC&)_VerQueryValue = GetProcAddress(hModule, "VerQueryValueA");
PSTR pSubBlock = "\\";
#endif
	if (_GetFileVersionInfoSize == NULL ||
		_GetFileVersionInfo == NULL ||
		_VerQueryValue == NULL)
		return;
	// ------- �t�@�C���Ƀo�[�W�����ԍ��𖄂ߍ���ł���ꍇ
	// ------- ���̃��[�`�����g���΁A���̃o�[�W�����ԍ���n�����Ƃ��ł���
	*VersionMS = 0;
	*VersionLS = 0;
	VersionSize = _GetFileVersionInfoSize(Filename, &VersionHandle);
	if (VersionSize == 0)
		return;
	pVersionInfo = malloc(VersionSize);
	if (pVersionInfo == NULL)
		return;
	if (_GetFileVersionInfo(Filename, VersionHandle, VersionSize, pVersionInfo))
		if (_VerQueryValue(pVersionInfo,pSubBlock,(void **)&FixedFileInfo,&itemLen))
		{
			*VersionMS = FixedFileInfo->dwFileVersionMS;
			*VersionLS = FixedFileInfo->dwFileVersionLS;
		}
	free(pVersionInfo);
	FreeLibrary(hModule);
}

/* ---------------------------------------------------------*/
/*      ���O���o�͂���                                      */
/* ---------------------------------------------------------*/
void WriteLog( int logLevel, PCTSTR msg )
{
	//TTBase �� TTBPlugin_WriteLog ���G�N�X�|�[�g���Ă��Ȃ��ꍇ�͉������Ȃ�
	if (TTBPlugin_WriteLog == NULL)
		return;
	TTBPlugin_WriteLog( PLUGIN_HANDLE, logLevel, msg );
}

/* ---------------------------------------------------------*/
/*      �ق��̃v���O�C���̃R�}���h�����s����                */
/* ---------------------------------------------------------*/
BOOL ExecutePluginCommand( LPCTSTR pluginName, int CmdID )
{
	BOOL Result = FALSE;
	//TTBase �� TTBPlugin_ExecuteCommand ���G�N�X�|�[�g���Ă��Ȃ��ꍇ�͉������Ȃ�
	if (TTBPlugin_ExecuteCommand == NULL)
		return FALSE;
	Result = TTBPlugin_ExecuteCommand( pluginName, CmdID );
	return Result;
}

// ****************************************************************
// *
// *         �v���O�C�� �C�x���g
// *
/* ---------------------------------------------------------*/
/*      �v���O�C�����\���̂̃Z�b�g                        */
/* ---------------------------------------------------------*/
#ifdef _WIN64
PPLUGIN_INFO WINAPI TTBEvent_InitPluginInfo (PWSTR PluginFilename)
#else
PPLUGIN_INFO WINAPI TTBEvent_InitPluginInfo (PSTR PluginFilename)
#endif
{
#if COMMAND_COUNT > 0
	int i = 0;
	PLUGIN_COMMAND_INFO *pCI = NULL;
#endif

	PLUGIN_INFO		*result = NULL;	/* �Ԓl	*/

	/* �t�@�C�����i���΃p�X�j���R�s�[ */
#ifdef _WIN64
	PLUGIN_FILENAME = (PWCHAR)malloc((lstrlenW(PluginFilename) + 1) * sizeof(WCHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyW(PLUGIN_FILENAME, PluginFilename);
#else
	PLUGIN_FILENAME = (PCHAR)malloc((lstrlenA(PluginFilename) + 1) * sizeof(CHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyA(PLUGIN_FILENAME, PluginFilename);
#endif

	/* �v���O�C�����\���̂̐��� */
	result = (PLUGIN_INFO *)malloc(sizeof(PLUGIN_INFO));
	if (result == NULL) return NULL;
	
	/* �v���O�C���̖��O */
#ifdef _WIN64
	result->Name = (PWCHAR)malloc((lstrlenW( PLUGIN_NAME ) + 1) * sizeof(WCHAR));
	if (result->Name != NULL)
		lstrcpyW(result->Name, PLUGIN_NAME);
#else
	result->Name = (PCHAR)malloc((lstrlenA( PLUGIN_NAME ) + 1) * sizeof(CHAR));
	if (result->Name != NULL)
		lstrcpyA(result->Name, PLUGIN_NAME);
#endif
	/* �v���O�C���t�@�C���� */
#ifdef _WIN64
	result->Filename = (PWCHAR)malloc((lstrlenW(PLUGIN_FILENAME) + 1) * sizeof(WCHAR));
	if (result->Filename != NULL)
		lstrcpyW(result->Filename, PLUGIN_FILENAME);
#else
	result->Filename = (PCHAR)malloc((lstrlenA(PLUGIN_FILENAME) + 1) * sizeof(CHAR));
	if (result->Filename != NULL)
		lstrcpyA(result->Filename, PLUGIN_FILENAME);
#endif
	/* �v���O�C���^�C�v */
	result->PluginType = PLUGIN_TYPE;
	/* �o�[�W�������̎擾 */
	GetVersion(PLUGIN_FILENAME, &result->VersionMS, &result->VersionLS);
	/* �R�}���h�̐� */
	result->CommandCount = COMMAND_COUNT;
	/* �R�}���h���\���̔z��̍쐬 */
#if COMMAND_COUNT > 0
	result->Commands = (PLUGIN_COMMAND_INFO *)malloc(sizeof(PLUGIN_COMMAND_INFO) * COMMAND_COUNT);
	if (result->Commands != NULL)
	{
		/* �R�}���h���\���̂̍쐬 */
		for (i = 0; i < COMMAND_COUNT; i++)
		{
			pCI = &result->Commands[i];
			*pCI = COMMAND_INFO[i];
			/* �R�}���h��		*/
#ifdef _WIN64
			pCI->Name = (PWCHAR)malloc((lstrlenW(COMMAND_INFO[i].Name) + 1) * sizeof(WCHAR));
			if (pCI->Name != NULL)
				lstrcpyW(pCI->Name, COMMAND_INFO[i].Name);
			pCI->Caption = (PWCHAR)malloc((lstrlenW(COMMAND_INFO[i].Caption) + 1) * sizeof(WCHAR));
			if (pCI->Caption != NULL)
				lstrcpyW(pCI->Caption, COMMAND_INFO[i].Caption);
#else
			pCI->Name = (PCHAR)malloc((lstrlenA(COMMAND_INFO[i].Name) + 1) * sizeof(CHAR));
			if (pCI->Name != NULL)
				lstrcpyA(pCI->Name, COMMAND_INFO[i].Name);
			pCI->Caption = (PCHAR)malloc((lstrlenA(COMMAND_INFO[i].Caption) + 1) * sizeof(CHAR));
			if (pCI->Caption != NULL)
				lstrcpyA(pCI->Caption, COMMAND_INFO[i].Caption);
#endif
		}
	}
#else
	result->Commands = NULL;
#endif

	return result;
}

/* ---------------------------------------------------------*/
/*      �v���O�C�����\���̂̔j��                          */
/* ---------------------------------------------------------*/
#ifdef _WIN64
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_W PluginInfo)
#else
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_A PluginInfo)
#endif
{
	DWORD i;
	PLUGIN_COMMAND_INFO *pCI;

	for (i = 0; i < PluginInfo->CommandCount; i++)
	{
		pCI = &PluginInfo->Commands[i];
		free(pCI->Name);
		free(pCI->Caption);
	}
	free(PluginInfo->Commands);
	free(PluginInfo->Filename);
	free(PluginInfo->Name);
	free(PluginInfo);
}

/* ---------------------------------------------------------*/
/*      �v���O�C��������                                    */
/* ---------------------------------------------------------*/
#ifdef _WIN64
BOOL WINAPI TTBEvent_Init (PWSTR PluginFilename, DWORD_PTR hPlugin)
#else
BOOL WINAPI TTBEvent_Init (PSTR PluginFilename, DWORD_PTR hPlugin)
#endif
{
	HMODULE hModule;

	RegisterMessages();
	// �L���b�V���̂��߂ɁATTBPlugin_InitPluginInfo�͌Ă΂�Ȃ��ꍇ������
	// ���̂��߁AInit�ł�PLUGIN_FILENAME�̏��������s��
	if (PLUGIN_FILENAME != NULL) free(PLUGIN_FILENAME);
#ifdef _WIN64
	PLUGIN_FILENAME = (PWCHAR)malloc((lstrlenW(PluginFilename) + 1) * sizeof(WCHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyW(PLUGIN_FILENAME, PluginFilename);
#else
	PLUGIN_FILENAME = (PCHAR)malloc((lstrlenA(PluginFilename) + 1) * sizeof(CHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyA(PLUGIN_FILENAME, PluginFilename);
#endif
	// TTBase����A�v���O�C����F�����邽�߂̎��ʃR�[�h
	PLUGIN_HANDLE = hPlugin;
	// API�֐��̎擾
	hModule = GetModuleHandle(NULL);
#ifdef _WIN64
	(FARPROC&)TTBPlugin_GetAllPluginInfo = GetProcAddress(hModule, "TTBPlugin_GetAllPluginInfo");
	(FARPROC&)TTBPlugin_FreePluginInfoArray = GetProcAddress(hModule, "TTBPlugin_FreePluginInfoArray");
	(FARPROC&)TTBPlugin_GetPluginInfo = GetProcAddress(hModule, "TTBPlugin_GetPluginInfo");
	(FARPROC&)TTBPlugin_SetPluginInfo = GetProcAddress(hModule, "TTBPlugin_SetPluginInfo");
	(FARPROC&)TTBPlugin_FreePluginInfo = GetProcAddress(hModule, "TTBPlugin_FreePluginInfo");
	(FARPROC&)TTBPlugin_SetMenuProperty = GetProcAddress(hModule, "TTBPlugin_SetMenuProperty");
	(FARPROC&)TTBPlugin_SetTaskTrayIcon = GetProcAddress(hModule, "TTBPlugin_SetTaskTrayIcon");
	(FARPROC&)TTBPlugin_WriteLog = GetProcAddress(hModule, "TTBPlugin_WriteLog");
	(FARPROC&)TTBPlugin_ExecuteCommand = GetProcAddress(hModule, "TTBPlugin_ExecuteCommand");
#else
	(FARPROC&)TTBPlugin_GetAllPluginInfo = GetProcAddress(hModule, "TTBPlugin_GetAllPluginInfo");
	(FARPROC&)TTBPlugin_FreePluginInfoArray = GetProcAddress(hModule, "TTBPlugin_FreePluginInfoArray");
	(FARPROC&)TTBPlugin_GetPluginInfo = GetProcAddress(hModule, "TTBPlugin_GetPluginInfo");
	(FARPROC&)TTBPlugin_SetPluginInfo = GetProcAddress(hModule, "TTBPlugin_SetPluginInfo");
	(FARPROC&)TTBPlugin_FreePluginInfo = GetProcAddress(hModule, "TTBPlugin_FreePluginInfo");
	(FARPROC&)TTBPlugin_SetMenuProperty = GetProcAddress(hModule, "TTBPlugin_SetMenuProperty");
	(FARPROC&)TTBPlugin_SetTaskTrayIcon = GetProcAddress(hModule, "TTBPlugin_SetTaskTrayIcon");
	(FARPROC&)TTBPlugin_WriteLog = GetProcAddress(hModule, "TTBPlugin_WriteLog");
	(FARPROC&)TTBPlugin_ExecuteCommand = GetProcAddress(hModule, "TTBPlugin_ExecuteCommand");
#endif

	return Init();
}

/* ---------------------------------------------------------*/
/*      ���O�C���A�����[�h���̏���                          */
/* ---------------------------------------------------------*/
void WINAPI TTBEvent_Unload (void)
{
	Unload();
	free(PLUGIN_FILENAME);
	PLUGIN_FILENAME = NULL;
}

/* ---------------------------------------------------------*/
/*      �R�}���h���s                                        */
/* ---------------------------------------------------------*/
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd)
{
	return Execute(CommandID, hWnd);
}

/* ---------------------------------------------------------*/
/*      �t�b�N�iShellHook,MouseHook)                        */
/* ---------------------------------------------------------*/
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam)
{
	Hook(Msg, wParam, lParam);
}
