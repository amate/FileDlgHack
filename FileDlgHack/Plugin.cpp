/*=========================================================================

                TTBaseプラグインテンプレート(Plugin.pas)

                このファイルはできるだけ変更しない。
                Main.pasに処理を書くことをお勧めします。

 =========================================================================*/

#include "stdafx.h"
#include <assert.h>
#include "Plugin.h"
#include "FileDlgHack.h"
#include "MessageDef.h"

/* ---------------------------------------------------------*/
/*      本体側エクスポート関数                              */
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
/*      グローバル変数                                      */
/* ---------------------------------------------------------*/
#ifdef _WIN64
PWCHAR PLUGIN_FILENAME = NULL;		// プラグインのファイル名。TTBaseからの相対パス
#else
PCHAR PLUGIN_FILENAME = NULL;		// プラグインのファイル名。TTBaseからの相対パス
#endif
DWORD_PTR PLUGIN_HANDLE = 0;		// プラグインをTTBaseで認識するための識別コード

/****************************************************************/
/*                                                              */
/*          ユーティリティルーチン                              */
/*                                                              */
/****************************************************************/
/* ---------------------------------------------------------*/
/*      プラグイン情報構造体のSrcをコピーして返す           */
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
/*  プラグイン側で作成されたプラグイン情報構造体を破棄する  */
/* ---------------------------------------------------------*/
void FreePluginInfo(PPLUGIN_INFO PluginInfo)
{
	TTBEvent_FreePluginInfo(PluginInfo);
}

/* ---------------------------------------------------------*/
/*      バージョン情報を返す                                */
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
	// ------- ファイルにバージョン番号を埋め込んでいる場合
	// ------- このルーチンを使えば、そのバージョン番号を渡すことができる
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
/*      ログを出力する                                      */
/* ---------------------------------------------------------*/
void WriteLog( int logLevel, PCTSTR msg )
{
	//TTBase が TTBPlugin_WriteLog をエクスポートしていない場合は何もしない
	if (TTBPlugin_WriteLog == NULL)
		return;
	TTBPlugin_WriteLog( PLUGIN_HANDLE, logLevel, msg );
}

/* ---------------------------------------------------------*/
/*      ほかのプラグインのコマンドを実行する                */
/* ---------------------------------------------------------*/
BOOL ExecutePluginCommand( LPCTSTR pluginName, int CmdID )
{
	BOOL Result = FALSE;
	//TTBase が TTBPlugin_ExecuteCommand をエクスポートしていない場合は何もしない
	if (TTBPlugin_ExecuteCommand == NULL)
		return FALSE;
	Result = TTBPlugin_ExecuteCommand( pluginName, CmdID );
	return Result;
}

// ****************************************************************
// *
// *         プラグイン イベント
// *
/* ---------------------------------------------------------*/
/*      プラグイン情報構造体のセット                        */
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

	PLUGIN_INFO		*result = NULL;	/* 返値	*/

	/* ファイル名（相対パス）をコピー */
#ifdef _WIN64
	PLUGIN_FILENAME = (PWCHAR)malloc((lstrlenW(PluginFilename) + 1) * sizeof(WCHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyW(PLUGIN_FILENAME, PluginFilename);
#else
	PLUGIN_FILENAME = (PCHAR)malloc((lstrlenA(PluginFilename) + 1) * sizeof(CHAR));
	if (PLUGIN_FILENAME != NULL)
		lstrcpyA(PLUGIN_FILENAME, PluginFilename);
#endif

	/* プラグイン情報構造体の生成 */
	result = (PLUGIN_INFO *)malloc(sizeof(PLUGIN_INFO));
	if (result == NULL) return NULL;
	
	/* プラグインの名前 */
#ifdef _WIN64
	result->Name = (PWCHAR)malloc((lstrlenW( PLUGIN_NAME ) + 1) * sizeof(WCHAR));
	if (result->Name != NULL)
		lstrcpyW(result->Name, PLUGIN_NAME);
#else
	result->Name = (PCHAR)malloc((lstrlenA( PLUGIN_NAME ) + 1) * sizeof(CHAR));
	if (result->Name != NULL)
		lstrcpyA(result->Name, PLUGIN_NAME);
#endif
	/* プラグインファイル名 */
#ifdef _WIN64
	result->Filename = (PWCHAR)malloc((lstrlenW(PLUGIN_FILENAME) + 1) * sizeof(WCHAR));
	if (result->Filename != NULL)
		lstrcpyW(result->Filename, PLUGIN_FILENAME);
#else
	result->Filename = (PCHAR)malloc((lstrlenA(PLUGIN_FILENAME) + 1) * sizeof(CHAR));
	if (result->Filename != NULL)
		lstrcpyA(result->Filename, PLUGIN_FILENAME);
#endif
	/* プラグインタイプ */
	result->PluginType = PLUGIN_TYPE;
	/* バージョン情報の取得 */
	GetVersion(PLUGIN_FILENAME, &result->VersionMS, &result->VersionLS);
	/* コマンドの数 */
	result->CommandCount = COMMAND_COUNT;
	/* コマンド情報構造体配列の作成 */
#if COMMAND_COUNT > 0
	result->Commands = (PLUGIN_COMMAND_INFO *)malloc(sizeof(PLUGIN_COMMAND_INFO) * COMMAND_COUNT);
	if (result->Commands != NULL)
	{
		/* コマンド情報構造体の作成 */
		for (i = 0; i < COMMAND_COUNT; i++)
		{
			pCI = &result->Commands[i];
			*pCI = COMMAND_INFO[i];
			/* コマンド名		*/
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
/*      プラグイン情報構造体の破棄                          */
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
/*      プラグイン初期化                                    */
/* ---------------------------------------------------------*/
#ifdef _WIN64
BOOL WINAPI TTBEvent_Init (PWSTR PluginFilename, DWORD_PTR hPlugin)
#else
BOOL WINAPI TTBEvent_Init (PSTR PluginFilename, DWORD_PTR hPlugin)
#endif
{
	HMODULE hModule;

	RegisterMessages();
	// キャッシュのために、TTBPlugin_InitPluginInfoは呼ばれない場合がある
	// そのため、InitでもPLUGIN_FILENAMEの初期化を行う
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
	// TTBaseから、プラグインを認識するための識別コード
	PLUGIN_HANDLE = hPlugin;
	// API関数の取得
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
/*      ラグインアンロード時の処理                          */
/* ---------------------------------------------------------*/
void WINAPI TTBEvent_Unload (void)
{
	Unload();
	free(PLUGIN_FILENAME);
	PLUGIN_FILENAME = NULL;
}

/* ---------------------------------------------------------*/
/*      コマンド実行                                        */
/* ---------------------------------------------------------*/
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd)
{
	return Execute(CommandID, hWnd);
}

/* ---------------------------------------------------------*/
/*      フック（ShellHook,MouseHook)                        */
/* ---------------------------------------------------------*/
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam)
{
	Hook(Msg, wParam, lParam);
}
