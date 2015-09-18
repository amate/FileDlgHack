/* ===========================================================================
                           TTB Plugin Template(VC++)

                                 Plugin.h

   =========================================================================== */

#pragma once

/* プラグインのロードタイプ */
#define	ptAlwaysLoad		0			/* 常駐型プラグイン				*/
#define	ptLoadAtUse			1			/* 一発起動型プラグイン			*/
#define	ptSpecViolation		0xFFFF		/* TTBaseプラグイン以外のDLL	*/

/* メニュー表示に関する定数 */
#define dmNone 				0	/* 何も出さない 	*/
#define dmSystemMenu		1	/* システムメニュー	*/
#define dmToolMenu			2	/* ツールメニュー	*/
#define dmHotKeyMenu		4	/* ホットキー		*/
#define dmChecked			8	/* メニューのチェックマーク */
#define dmUnchecked			0	/* メニューのチェックマークをつけない */
#define dmEnabled			0	/* メニューをEnableに */
#define dmDisabled			16	/* メニューをDisableする */
#define DISPMENU_MENU		dmToolMenu|dmSystemMenu
#define	DISPMENU_ENABLED	dmDisabled
#define DISPMENU_CHECKED	dmChecked

/* ログ出力に関する定数 */
#define elNever				0	/* 出力しない */
#define elError				1	/* エラー */
#define elWarning			2	/* 警告 */
#define elInfo				3	/* 情報 */
#define elDebug				4	/* デバッグ */


// 構造体アライメント圧縮
#pragma pack(push,1)
/* ---------------------------------------------------------*/
/*      構造体定義                                          */
/* ---------------------------------------------------------*/
/* コマンド情報構造体 */
typedef struct
{
	PWCHAR Name;			/* コマンドの名前（英名）								*/
	PWCHAR Caption;			/* コマンドの説明（日本語）								*/
	int CommandID;			/* コマンド番号											*/
	int Attr;				/* アトリビュート（未使用）								*/
	int ResID;				/* リソース番号（未使用）								*/
	int DispMenu;			/*  システムメニューが1、ツールメニューが2、			*/
							/*  表示なしは0、ホットキーメニューは4					*/
	DWORD TimerInterval;	/*  コマンド実行タイマー間隔[msec] 0で機能を使わない。	*/
	DWORD TimerCounter;		/*  システム内部で使用									*/
}PLUGIN_COMMAND_INFO_W, *PPLUGIN_COMMAND_INFO_W, **PPLUGIN_COMMAND_INFO_ARRAY_W;

typedef struct
{
	PCHAR Name;				/* コマンドの名前（英名）								*/
	PCHAR Caption;			/* コマンドの説明（日本語）								*/
	int CommandID;			/* コマンド番号											*/
	int Attr;				/* アトリビュート（未使用）								*/
	int ResID;				/* リソース番号（未使用）								*/
	int DispMenu;			/*  システムメニューが1、ツールメニューが2、			*/
							/*  表示なしは0、ホットキーメニューは4					*/
	DWORD TimerInterval;	/*  コマンド実行タイマー間隔[msec] 0で機能を使わない。	*/
	DWORD TimerCounter;		/*  システム内部で使用									*/
}PLUGIN_COMMAND_INFO_A, *PPLUGIN_COMMAND_INFO_A, **PPLUGIN_COMMAND_INFO_ARRAY_A;

#ifdef _WIN64
typedef PLUGIN_COMMAND_INFO_W PLUGIN_COMMAND_INFO;
typedef PPLUGIN_COMMAND_INFO_W PPLUGIN_COMMAND_INFO;
typedef PPLUGIN_COMMAND_INFO_ARRAY_W PPLUGIN_COMMAND_INFO_ARRAY;
#else
typedef PLUGIN_COMMAND_INFO_A PLUGIN_COMMAND_INFO;
typedef PPLUGIN_COMMAND_INFO_A PPLUGIN_COMMAND_INFO;
typedef PPLUGIN_COMMAND_INFO_ARRAY_A PPLUGIN_COMMAND_INFO_ARRAY;
#endif

/* プラグイン情報構造体 */
typedef struct
{
	WORD NeedVersion;				/* プラグインI/F要求バージョン					*/
	PWCHAR Name;					/* プラグインの説明（日本語）					*/
	PWCHAR Filename;				/* プラグインのファイル名（相対パス）			*/
	WORD PluginType;				/* プラグインのロードタイプ						*/
	DWORD VersionMS;				/* バージョン									*/
	DWORD VersionLS;				/* バージョン									*/
	DWORD CommandCount;				/* コマンド個数									*/
	PPLUGIN_COMMAND_INFO_W Commands;	/* コマンド										*/
	/* 以下システムで、TTBase本体で使用する */
	DWORD LoadTime;					/* ロードにかかった時間（msec）					*/
}PLUGIN_INFO_W, *PPLUGIN_INFO_W, **PPLUGIN_INFO_ARRAY_W;

typedef struct
{
	WORD NeedVersion;				/* プラグインI/F要求バージョン					*/
	PCHAR Name;						/* プラグインの説明（日本語）					*/
	PCHAR Filename;					/* プラグインのファイル名（相対パス）			*/
	WORD PluginType;				/* プラグインのロードタイプ						*/
	DWORD VersionMS;				/* バージョン									*/
	DWORD VersionLS;				/* バージョン									*/
	DWORD CommandCount;				/* コマンド個数									*/
	PPLUGIN_COMMAND_INFO_A Commands;	/* コマンド										*/
	/* 以下システムで、TTBase本体で使用する */
	DWORD LoadTime;					/* ロードにかかった時間（msec）					*/
}PLUGIN_INFO_A, *PPLUGIN_INFO_A, **PPLUGIN_INFO_ARRAY_A;

#ifdef _WIN64
typedef PLUGIN_INFO_W PLUGIN_INFO;
typedef PPLUGIN_INFO_W PPLUGIN_INFO;
typedef PPLUGIN_INFO_ARRAY_W PPLUGIN_INFO_ARRAY;
#else
typedef PLUGIN_INFO_A PLUGIN_INFO;
typedef PPLUGIN_INFO_A PPLUGIN_INFO;
typedef PPLUGIN_INFO_ARRAY_A PPLUGIN_INFO_ARRAY;
#endif
#pragma pack(pop)

/* ---------------------------------------------------------*/
/*      プラグイン側エクスポート関数                        */
/* ---------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN64
/* 必須 */
PPLUGIN_INFO_W WINAPI TTBEvent_InitPluginInfo (PWSTR PluginFilename);
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_W PLUGIN_INFO);
/* 任意 */
BOOL WINAPI TTBEvent_Init (PWSTR PluginFilename, DWORD_PTR hPlugin);
void WINAPI TTBEvent_Unload (void);
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd);
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam);
#else
/* 必須 */
PPLUGIN_INFO_A WINAPI TTBEvent_InitPluginInfo (PSTR PluginFilename);
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_A PLUGIN_INFO);
/* 任意 */
BOOL WINAPI TTBEvent_Init (PSTR PluginFilename, DWORD_PTR hPlugin);
void WINAPI TTBEvent_Unload (void);
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd);
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam);
#endif
#ifdef __cplusplus
};
#endif

/* ---------------------------------------------------------*/
/*      本体側エクスポート関数                              */
/* ---------------------------------------------------------*/
#ifdef _WIN64
extern PPLUGIN_INFO_W (WINAPI *TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
extern void (WINAPI *TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PPLUGIN_INFO_W PLUGIN_INFO);
extern void (WINAPI *TTBPlugin_FreePluginInfo)(PPLUGIN_INFO_W PLUGIN_INFO);
extern void (WINAPI *TTBPlugin_SetMenuProperty)(DWORD_PTR hPlugin, int CommandID, DWORD ChangeFlag, DWORD Flag);
extern PPLUGIN_INFO_ARRAY_W (WINAPI *TTBPlugin_GetAllPluginInfo) ();
extern void (WINAPI *TTBPlugin_FreePluginInfoArray)(PPLUGIN_INFO_ARRAY_W PluginInfoArray);
extern void (WINAPI *TTBPlugin_SetTaskTrayIcon)(HICON hIcon, PCTSTR Tips);
extern void (WINAPI *TTBPlugin_WriteLog)( DWORD_PTR hPlugin, int logLevel, PCTSTR msg );
extern BOOL (WINAPI *TTBPlugin_ExecuteCommand)( PCTSTR PluginFilename, int CmdID );
#else
extern PPLUGIN_INFO_A (WINAPI *TTBPlugin_GetPluginInfo)(DWORD_PTR hPlugin);
extern void (WINAPI *TTBPlugin_SetPluginInfo)(DWORD_PTR hPlugin, PPLUGIN_INFO_A PLUGIN_INFO);
extern void (WINAPI *TTBPlugin_FreePluginInfo)(PPLUGIN_INFO_A PLUGIN_INFO);
extern void (WINAPI *TTBPlugin_SetMenuProperty)(DWORD_PTR hPlugin, int CommandID, DWORD ChangeFlag, DWORD Flag);
extern PPLUGIN_INFO_ARRAY_A (WINAPI *TTBPlugin_GetAllPluginInfo) ();
extern void (WINAPI *TTBPlugin_FreePluginInfoArray)(PPLUGIN_INFO_ARRAY_A PluginInfoArray);
extern void (WINAPI *TTBPlugin_SetTaskTrayIcon)(HICON hIcon, PCTSTR Tips);
extern void (WINAPI *TTBPlugin_WriteLog)( DWORD_PTR hPlugin, int logLevel, PCTSTR msg );
extern BOOL (WINAPI *TTBPlugin_ExecuteCommand)( PCTSTR PluginFilename, int CmdID );
#endif

/* ---------------------------------------------------------*/
/*      ユーティリティルーチン                              */
/* ---------------------------------------------------------*/
#ifdef _WIN64
PPLUGIN_INFO_W CopyPluginInfo(PPLUGIN_INFO_W Src);
void FreePluginInfo(PPLUGIN_INFO_W PLUGIN_INFO);
void GetVersion(PWSTR Filename, DWORD* VersionMS, DWORD* VersionLS);
void WriteLog( int logLevel, PCWSTR msg );
BOOL ExecutePluginCommand( PWSTR pluginName, int CmdID );
#else
PPLUGIN_INFO_A CopyPluginInfo(PPLUGIN_INFO_A Src);
void FreePluginInfo(PPLUGIN_INFO_A PLUGIN_INFO);
void GetVersion(PSTR Filename, DWORD* VersionMS, DWORD* VersionLS);
void WriteLog( int logLevel, PCSTR msg );
BOOL ExecutePluginCommand( LPSTR pluginName, int CmdID );
#endif

/* ---------------------------------------------------------*/
/*      グローバル変数                                      */
/* ---------------------------------------------------------*/
#ifdef _WIN64
extern PWCHAR PLUGIN_FILENAME;		/* プラグインのファイル名。TTBaseフォルダからの相対パス */
#else
extern PCHAR PLUGIN_FILENAME;		/* プラグインのファイル名。TTBaseフォルダからの相対パス */
#endif
extern DWORD_PTR PLUGIN_HANDLE;		/* TTBaseがプラグインを識別するためのコード */

