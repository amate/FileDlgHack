/* ===========================================================================
                           TTB Plugin Template(VC++)

                                 Plugin.h

   =========================================================================== */

#pragma once

/* �v���O�C���̃��[�h�^�C�v */
#define	ptAlwaysLoad		0			/* �풓�^�v���O�C��				*/
#define	ptLoadAtUse			1			/* �ꔭ�N���^�v���O�C��			*/
#define	ptSpecViolation		0xFFFF		/* TTBase�v���O�C���ȊO��DLL	*/

/* ���j���[�\���Ɋւ���萔 */
#define dmNone 				0	/* �����o���Ȃ� 	*/
#define dmSystemMenu		1	/* �V�X�e�����j���[	*/
#define dmToolMenu			2	/* �c�[�����j���[	*/
#define dmHotKeyMenu		4	/* �z�b�g�L�[		*/
#define dmChecked			8	/* ���j���[�̃`�F�b�N�}�[�N */
#define dmUnchecked			0	/* ���j���[�̃`�F�b�N�}�[�N�����Ȃ� */
#define dmEnabled			0	/* ���j���[��Enable�� */
#define dmDisabled			16	/* ���j���[��Disable���� */
#define DISPMENU_MENU		dmToolMenu|dmSystemMenu
#define	DISPMENU_ENABLED	dmDisabled
#define DISPMENU_CHECKED	dmChecked

/* ���O�o�͂Ɋւ���萔 */
#define elNever				0	/* �o�͂��Ȃ� */
#define elError				1	/* �G���[ */
#define elWarning			2	/* �x�� */
#define elInfo				3	/* ��� */
#define elDebug				4	/* �f�o�b�O */


// �\���̃A���C�����g���k
#pragma pack(push,1)
/* ---------------------------------------------------------*/
/*      �\���̒�`                                          */
/* ---------------------------------------------------------*/
/* �R�}���h���\���� */
typedef struct
{
	PWCHAR Name;			/* �R�}���h�̖��O�i�p���j								*/
	PWCHAR Caption;			/* �R�}���h�̐����i���{��j								*/
	int CommandID;			/* �R�}���h�ԍ�											*/
	int Attr;				/* �A�g���r���[�g�i���g�p�j								*/
	int ResID;				/* ���\�[�X�ԍ��i���g�p�j								*/
	int DispMenu;			/*  �V�X�e�����j���[��1�A�c�[�����j���[��2�A			*/
							/*  �\���Ȃ���0�A�z�b�g�L�[���j���[��4					*/
	DWORD TimerInterval;	/*  �R�}���h���s�^�C�}�[�Ԋu[msec] 0�ŋ@�\���g��Ȃ��B	*/
	DWORD TimerCounter;		/*  �V�X�e�������Ŏg�p									*/
}PLUGIN_COMMAND_INFO_W, *PPLUGIN_COMMAND_INFO_W, **PPLUGIN_COMMAND_INFO_ARRAY_W;

typedef struct
{
	PCHAR Name;				/* �R�}���h�̖��O�i�p���j								*/
	PCHAR Caption;			/* �R�}���h�̐����i���{��j								*/
	int CommandID;			/* �R�}���h�ԍ�											*/
	int Attr;				/* �A�g���r���[�g�i���g�p�j								*/
	int ResID;				/* ���\�[�X�ԍ��i���g�p�j								*/
	int DispMenu;			/*  �V�X�e�����j���[��1�A�c�[�����j���[��2�A			*/
							/*  �\���Ȃ���0�A�z�b�g�L�[���j���[��4					*/
	DWORD TimerInterval;	/*  �R�}���h���s�^�C�}�[�Ԋu[msec] 0�ŋ@�\���g��Ȃ��B	*/
	DWORD TimerCounter;		/*  �V�X�e�������Ŏg�p									*/
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

/* �v���O�C�����\���� */
typedef struct
{
	WORD NeedVersion;				/* �v���O�C��I/F�v���o�[�W����					*/
	PWCHAR Name;					/* �v���O�C���̐����i���{��j					*/
	PWCHAR Filename;				/* �v���O�C���̃t�@�C�����i���΃p�X�j			*/
	WORD PluginType;				/* �v���O�C���̃��[�h�^�C�v						*/
	DWORD VersionMS;				/* �o�[�W����									*/
	DWORD VersionLS;				/* �o�[�W����									*/
	DWORD CommandCount;				/* �R�}���h��									*/
	PPLUGIN_COMMAND_INFO_W Commands;	/* �R�}���h										*/
	/* �ȉ��V�X�e���ŁATTBase�{�̂Ŏg�p���� */
	DWORD LoadTime;					/* ���[�h�ɂ����������ԁimsec�j					*/
}PLUGIN_INFO_W, *PPLUGIN_INFO_W, **PPLUGIN_INFO_ARRAY_W;

typedef struct
{
	WORD NeedVersion;				/* �v���O�C��I/F�v���o�[�W����					*/
	PCHAR Name;						/* �v���O�C���̐����i���{��j					*/
	PCHAR Filename;					/* �v���O�C���̃t�@�C�����i���΃p�X�j			*/
	WORD PluginType;				/* �v���O�C���̃��[�h�^�C�v						*/
	DWORD VersionMS;				/* �o�[�W����									*/
	DWORD VersionLS;				/* �o�[�W����									*/
	DWORD CommandCount;				/* �R�}���h��									*/
	PPLUGIN_COMMAND_INFO_A Commands;	/* �R�}���h										*/
	/* �ȉ��V�X�e���ŁATTBase�{�̂Ŏg�p���� */
	DWORD LoadTime;					/* ���[�h�ɂ����������ԁimsec�j					*/
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
/*      �v���O�C�����G�N�X�|�[�g�֐�                        */
/* ---------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN64
/* �K�{ */
PPLUGIN_INFO_W WINAPI TTBEvent_InitPluginInfo (PWSTR PluginFilename);
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_W PLUGIN_INFO);
/* �C�� */
BOOL WINAPI TTBEvent_Init (PWSTR PluginFilename, DWORD_PTR hPlugin);
void WINAPI TTBEvent_Unload (void);
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd);
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam);
#else
/* �K�{ */
PPLUGIN_INFO_A WINAPI TTBEvent_InitPluginInfo (PSTR PluginFilename);
void WINAPI TTBEvent_FreePluginInfo (PPLUGIN_INFO_A PLUGIN_INFO);
/* �C�� */
BOOL WINAPI TTBEvent_Init (PSTR PluginFilename, DWORD_PTR hPlugin);
void WINAPI TTBEvent_Unload (void);
BOOL WINAPI TTBEvent_Execute (int CommandID, HWND hWnd);
void WINAPI TTBEvent_WindowsHook (UINT Msg, DWORD wParam, DWORD lParam);
#endif
#ifdef __cplusplus
};
#endif

/* ---------------------------------------------------------*/
/*      �{�̑��G�N�X�|�[�g�֐�                              */
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
/*      ���[�e�B���e�B���[�`��                              */
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
/*      �O���[�o���ϐ�                                      */
/* ---------------------------------------------------------*/
#ifdef _WIN64
extern PWCHAR PLUGIN_FILENAME;		/* �v���O�C���̃t�@�C�����BTTBase�t�H���_����̑��΃p�X */
#else
extern PCHAR PLUGIN_FILENAME;		/* �v���O�C���̃t�@�C�����BTTBase�t�H���_����̑��΃p�X */
#endif
extern DWORD_PTR PLUGIN_HANDLE;		/* TTBase���v���O�C�������ʂ��邽�߂̃R�[�h */

