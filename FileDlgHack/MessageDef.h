/* ===========================================================================
                           TTB Plugin Template(C++)

                                MessageDef.h

   =========================================================================== */

// ----  Message��`  -----------------------------
// TTBPlugin_WindowsHook�ŁAMsg�p�����[�^�ɂ���炪����Ă��܂��B
// ���e�́AMSDN�ŁAWH_SHELL�̊Y��ID, WH_MOUSE��HC_ACTION���Q�Ƃ��Ă��������B
extern UINT TTB_HSHELL_ACTIVATESHELLWINDOW;
extern UINT TTB_HSHELL_GETMINRECT;
extern UINT TTB_HSHELL_LANGUAGE;
extern UINT TTB_HSHELL_REDRAW;
extern UINT TTB_HSHELL_TASKMAN;
extern UINT TTB_HSHELL_WINDOWACTIVATED;
extern UINT TTB_HSHELL_WINDOWCREATED;
extern UINT TTB_HSHELL_WINDOWDESTROYED;
extern UINT TTB_HMOUSE_ACTION;

// �����g�p�BTaskTray�A�C�R���֌W�̃��b�Z�[�W�ł�
extern UINT	TTB_ICON_NOTIFY;
// TTBase.dat��TTBase�Ƀ��[�h�����܂�
extern UINT TTB_LOAD_DATA_FILE;
// TTBase.dat��TTBase�ɃZ�[�u�����܂�
extern UINT TTB_SAVE_DATA_FILE;

// �擾�p�֐�
void RegisterMessages(void);
