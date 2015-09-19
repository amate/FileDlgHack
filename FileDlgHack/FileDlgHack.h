// FileDlgHack.h

#include "Plugin.h"
#include <process.h>


// --------------------------------------------------------
//    �v���O�C���̏��
// --------------------------------------------------------
/* �v���O�C���̖��O�i�Q�o�C�g���\�j */
#ifdef _WIN64
#define PLUGIN_NAME L"�t�@�C���_�C�A���O�g��7"
#else
#define	PLUGIN_NAME	"�t�@�C���_�C�A���O�g��7"
#endif

/* �v���O�C���̃^�C�v */
#define	PLUGIN_TYPE	ptAlwaysLoad

// --------------------------------------------------------
//    �R�}���h�̏��
// --------------------------------------------------------
/* �R�}���h�̐� */
#define COMMAND_COUNT	0

/* �R�}���hID */

// ���R�}���h���e�ɂ��ẮAMain.cpp���Q��

// --------------------------------------------------------
//    �O���[�o���ϐ�
// --------------------------------------------------------
//#if COMMAND_COUNT > 0
//extern PLUGIN_COMMAND_INFO	COMMAND_INFO[COMMAND_COUNT];
//#endif

// --------------------------------------------------------
//    �֐���`
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

/// ���L��������
#define FILEMAP_SHARED _T("FILEDIALOGEXSHARED")

#define SHORTCUT_MAX 100	// �V���[�g�J�b�g
#define HISTORY_MAX  100	// ����
#define TOOL_MAX     100	// �c�[��

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
	int          nDialogPos;          // �_�C�A���O�̈ʒu���w�肷�邩�ǂ���(0:�w�肵�Ȃ� 1:�w�肷�� 2:������)
	POINT        DialogPos;           // �_�C�A���O�̈ʒu
	BOOL         bListSize;           // ���X�g�r���[�̃T�C�Y���w�肷�邩�ǂ���
	POINT        ListSize;            // ���X�g�r���[�̃T�C�Y
	int          ListStyle;           // �\��
	int          ListSort;            // �A�C�R���̐���
	BOOL         bToolbar;            // �c�[���o�[�ɃA�C�R����ǉ����邩�ǂ���
	BOOL		 bExplorerFolderOpenOnActive;
	TCHAR        szIniPath[MAX_PATH]; // ini�̃p�X
	int          cDialog;             // �T�u�N���X�����Ă���_�C�A���O�̐�
	SHORTCUTDATA shortcut;            // �V���[�g�J�b�g�̃f�[�^
	HISTORYDATA  history;             // �ŋߎg�����t�H���_�̃f�[�^
	TOOLDATA     tool;                // �c�[���̃f�[�^
} SHAREDDATA;


extern HINSTANCE   g_Inst;			// �C���X�^���X�n���h��
extern HANDLE      g_hShared;
extern SHAREDDATA *g_Shared;		// ���L������
