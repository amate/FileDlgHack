// FileDlgHack.h

#include "Plugin.h"
#include <process.h>


// --------------------------------------------------------
//    プラグインの情報
// --------------------------------------------------------
/* プラグインの名前（２バイトも可能） */
#ifdef _WIN64
#define PLUGIN_NAME L"ファイルダイアログ拡張7"
#else
#define	PLUGIN_NAME	"ファイルダイアログ拡張7"
#endif

/* プラグインのタイプ */
#define	PLUGIN_TYPE	ptAlwaysLoad

// --------------------------------------------------------
//    コマンドの情報
// --------------------------------------------------------
/* コマンドの数 */
#define COMMAND_COUNT	0

/* コマンドID */

// ★コマンド内容については、Main.cppを参照

// --------------------------------------------------------
//    グローバル変数
// --------------------------------------------------------
//#if COMMAND_COUNT > 0
//extern PLUGIN_COMMAND_INFO	COMMAND_INFO[COMMAND_COUNT];
//#endif

// --------------------------------------------------------
//    関数定義
// --------------------------------------------------------
BOOL StrReplace( LPSTR str, LPCSTR szFrom, LPCSTR szTo, int size );

BOOL Init(void);
void Unload(void);
BOOL Execute(int, HWND); 
void Hook(UINT Msg, DWORD wParam, DWORD lParam);

void AdjustWinPos( HWND hWnd );
BOOL SettingDialog( HWND );

// --------------------------------------------------------
// 
// --------------------------------------------------------

enum {
	WIN98	= 0,
	WINME	= 1,
	WIN2K	= 2,
	WINXP	= 3,
	WINVT	= 4,
	WIN7	= 5,
};

/// 共有メモリ名
#define FILEMAP_SHARED _T("FILEDIALOGEXSHARED")

#define SHORTCUT_MAX 100	// ショートカット
#define HISTORY_MAX  100	// 履歴
#define TOOL_MAX     100	// ツール

#define ID_SHORTCUT_FIRST 10000
#define ID_HISTORY_FIRST  15000
#define ID_TOOL_FIRST     20000

typedef struct _SHORTCUTDATA
{
	int  count;	
	struct ShortcutItem {
		TCHAR szName[MAX_PATH];
		TCHAR szPath[MAX_PATH];
	} data[SHORTCUT_MAX];
} SHORTCUTDATA;

typedef struct _HISTORYDATA
{
	int  count;
	int  max;
	struct {
		TCHAR szPath[MAX_PATH];
	} data[HISTORY_MAX];
} HISTORYDATA;

typedef struct _TOOLDATA
{
	int count;
	int max;
	struct ToolItem {
		TCHAR szName[MAX_PATH];
		TCHAR szPath[MAX_PATH];
		TCHAR szParam[MAX_PATH];
	} data[TOOL_MAX];
} TOOLDATA;


typedef struct _SHAREDDATA
{
	int          nDialogPos;          // ダイアログの位置を指定するかどうか(0:指定しない 1:指定する 2:中央に)
	POINT        DialogPos;           // ダイアログの位置
	BOOL         bListSize;           // リストビューのサイズを指定するかどうか
	POINT        ListSize;            // リストビューのサイズ
	int          ListStyle;           // 表示
	int          ListSort;            // アイコンの整理
	BOOL         bToolbar;            // ツールバーにアイコンを追加するかどうか
	BOOL		 bExplorerFolderOpenOnActive;
	TCHAR        szIniPath[MAX_PATH]; // iniのパス
	int          cDialog;             // サブクラス化しているダイアログの数
	SHORTCUTDATA shortcut;            // ショートカットのデータ
	HISTORYDATA  history;             // 最近使ったフォルダのデータ
	TOOLDATA     tool;                // ツールのデータ
} SHAREDDATA;


extern HINSTANCE   g_Inst;			// インスタンスハンドル
extern HANDLE      g_hShared;
extern SHAREDDATA *g_Shared;		// 共有メモリ
