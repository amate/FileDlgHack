// FileDlgHack.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"
#include <tchar.h>
#include <atlbase.h>
#include <atlconv.h>
#include "FileDlgHack.h"


// Set/GetProp�Ŏg�p
#define PROP_PROCEDURE "PROP_PROCEDURE" // �T�u�N���X������O�̃v���V�[�W��
#define PROP_CANCEL    "PROP_CANCEL"	// �L�����Z���{�^�������������ǂ���

#define TB_BUTTONID    0x54				// �c�[���o�[�ɒǉ�����{�^����ID

static const int WM_FILEDIALOGEXDETECTMESSAGE = ::RegisterWindowMessage(_T("FileDialogDetectMessage"));

enum { kExOpenDir = 200 };

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL StrReplace( LPSTR str, LPCSTR szFrom, LPCSTR szTo, int size );

void ExplorerPathToFileDlg();


#pragma data_seg(".SHARED_DATA")
HHOOK g_hHook = NULL;					// �t�b�N�v���V�[�W���̃n���h��
#pragma data_seg()

HINSTANCE   g_Inst		= NULL;			// �C���X�^���X�n���h��
HANDLE      g_hShared	= NULL;
SHAREDDATA *g_Shared	= NULL;			// ���L������

// --------------------------------------------------------

// str����szFrom��szTo�Œu��������
BOOL StrReplace( LPSTR str, LPCSTR szFrom, LPCSTR szTo, int size )
{
	int nFromLen = lstrlenA( szFrom );
	int nToLen = lstrlenA( szTo );
	int nLen = lstrlenA( str );
	LPSTR p = str;
	LPSTR f;

	while( ( f = StrStrA( p, szFrom ) ) != NULL )
	{
		if( nLen - nFromLen + nToLen + 1 > size )
			return FALSE;
		MoveMemory( f + nToLen, f + nFromLen, lstrlenA( f ) + 1 );
		MoveMemory( f, szTo, nToLen );

		p = f + nToLen;
	}

	return TRUE;
}


LONG_PTR SetWindowLongX( HWND hWnd, int nIndex, LONG_PTR dwNewLong )
{
	if( IsWindowUnicode( hWnd ) )
		return SetWindowLongPtrW( hWnd, nIndex, dwNewLong );
	else
		return SetWindowLongPtrA( hWnd, nIndex, dwNewLong );
}

LONG_PTR GetWindowLongX( HWND hWnd, int nIndex )
{
	if( IsWindowUnicode( hWnd ) )
		return GetWindowLongPtrW( hWnd, nIndex );
	else
		return GetWindowLongPtrA( hWnd, nIndex );
}

LRESULT CallWindowProcX( WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	if( IsWindowUnicode( hWnd ) )
		return CallWindowProcW( lpPrevWndFunc, hWnd, Msg, wParam, lParam );
	else
		return CallWindowProcA( lpPrevWndFunc, hWnd, Msg, wParam, lParam );
}

// ID�ƃN���X������q�E�B���h�E��T��
HWND FindChildWindowID( HWND hParent, LPSTR szClass, int ID )
{
	HWND hWnd = NULL;
	while( 1 )
	{
		hWnd = FindWindowExA( hParent, hWnd, szClass, NULL );
		if (hWnd == NULL){
			return NULL;
		}
		LONG_PTR tID = GetWindowLongX( hWnd, GWL_ID );
		if( tID == ID ){
			return hWnd;
		}
	}
	return NULL;
}

// ID�ƃN���X���̔z�񂩂�q�E�B���h�E��T��
// �{����szClass,ID���܂Ƃ߂�struct������āA����vector��n�������񂾂��ǁA
// ���ƂȂ�vector�g���̎��d
HWND FindChildWindowIDRecursive(HWND hParent, LPSTR * szClass, const int * ID){
	HWND hWnd = NULL;
	while ( 1 ){
		hWnd = FindWindowExA( hParent, hWnd, *szClass, NULL);

		if(hWnd == NULL){
			return NULL;		// �q�E�B���h�E��������Ȃ�����
		}
		LONG_PTR tID = GetWindowLongX( hWnd, GWL_ID);
		if( tID == *ID){
			//		TRACE(" ID���}�b�`����\n");
			if(szClass + 1 == NULL || *(ID+1) == -1){
				return hWnd;	// �Ō�܂ōs�����̂ŏI���
			}else{
				HWND hWndChild = FindChildWindowIDRecursive(hWnd, szClass + 1, ID + 1);
				if(hWndChild != NULL){
					return hWndChild;
				}
			}
		}
		//	TRACE(" ID���}�b�`���Ȃ�����\n");

	}
	return NULL;
}

HWND GetDefView( HWND hDlg )
{
	HWND hWnd = FindChildWindowID( hDlg, "SHELLDLL_DefView", 0x461 );
	if(hWnd == NULL){
		//For IFileDialog
		LPSTR kls[] = {"DUIViewWndClassName","DirectUIHWND","CtrlNotifySink","SHELLDLL_DefView",NULL};
		const int ids[] = {0,0,0,0x461,-1};
		hWnd = FindChildWindowIDRecursive(hDlg, kls , ids);
	}
	TRACE("GetDefView : %d\n", (int)hWnd);
	return hWnd;
}


// IFileDialog�A�h���X�o�[�̃R���{�{�b�N�X
HWND GetAddressBarTxt( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW", "ReBarWindow32", "Address Band Root", "msctls_progress32", "ComboBoxEx32", NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0xA205, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg,kls,ids);
	TRACE("GetAddressBarTxt : %d\n", (int)hWnd);
	return hWnd;
}


// IFileDialog�A�h���X�o�[�̃{�^��
HWND GetAddressBarBtn( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW", "ReBarWindow32", "Address Band Root", "msctls_progress32", "ToolbarWindow32", NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg, kls, ids);
	TRACE("GetAddressBarBtn : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialog�̃A�h���X�o�[��Breadcumb���(?)�ɂ���Ƃ���ComboBox�̑���ɕ\�������ToolBar
HWND GetAddressBarBread( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW","ReBarWindow32","Address Band Root","msctls_progress32","Breadcrumb Parent","ToolbarWindow32",NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0, 0x3E9, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg,kls,ids);
	TRACE("GetAddressBarBread : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialog�̃t�@�C�����G�f�B�b�g�{�b�N�X
HWND GetIFileDialogFileEditBox(HWND hDlg)
{
	LPSTR kls[] = { "ComboBoxEx32", "ComboBox", "Edit", NULL };
	const int ids[] = { 0x47C, 0x47C, 0x47C, -1 };
	HWND hWnd = FindChildWindowIDRecursive(hDlg, kls, ids);
	TRACE("GetIFileDialogFileEditBox : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialog�̕ۑ��t�@�C�����G�f�B�b�g�{�b�N�X
HWND GetIFileDialogSaveFileEditBox(HWND hDlg)
{
	LPSTR kls[] = { "DUIViewWndClassName", "DirectUIHWND", "FloatNotifySink", "ComboBox", "Edit", NULL };
	const int ids[] = { 0, 0, 0, 0, 0x3E9, -1 };
	HWND hWnd = FindChildWindowIDRecursive(hDlg, kls, ids);
	TRACE("GetIFileDialogSaveFileEditBox : %d\n", (int)hWnd);
	return hWnd;
}



HWND GetToolbar( HWND hDlg )
{
	HWND hToolbar = FindChildWindowID( hDlg, "ToolbarWindow32", 0x440 );
	if( hToolbar == NULL )
		hToolbar = FindChildWindowID( hDlg, "ToolbarWindow32", 0x001 );
	TRACE("GetToolbar : %d\n", (int)hToolbar);
	return hToolbar;
}


HWND GetNameEdit( HWND hDlg )
{
	HWND hName = GetIFileDialogSaveFileEditBox(hDlg);
	if (hName)
		return hName;
		
	hName = FindChildWindowID(hDlg, "ComboBoxEx32", 0x47C);
	if( hName == NULL ) {
		// �]���̃_�C�A���O�H
		hName = FindChildWindowID( hDlg, "Edit", 0x480 );
	} else {
		// Win7�p
		HWND hCombo = FindChildWindowID( hName, "ComboBox", 0x47C );
		if (hCombo != NULL) {
			hName = FindChildWindowID( hCombo, "Edit", 0x47C );
		}
	}
	TRACE("GetNameEdit : %d\n", (int)hName);
	return hName;
}

// �t�@�C���_�C�A���O�̌��݂̃t�H���_���擾����
BOOL GetDlgCurDir( HWND hDlgWnd, LPSTR lpbuff )
{
	/*	// �������ł͌Â��̂������Ȃ�
	HWND hAddrT;
	if ((hAddrT = GetAddressBarTxt( hDlgWnd )) != NULL) {
	// IFileDialog
	GetWindowText(hAddrT, lpbuff, MAX_PATH);
	return TRUE;
	}
	*/

	// IFileDlg�p
	HWND hAddrBrd = GetAddressBarBread( hDlgWnd );
	if (hAddrBrd != NULL) {
		char strtemp[MAX_PATH];
		GetWindowTextA(hAddrBrd, strtemp, MAX_PATH);
		lstrcpyA(lpbuff, &strtemp[10]);		// "�A�h���X: "�̕�������菜��
		return TRUE;
	}

	// CDM_GETFOLDERPATH ����UNICODE�֌W�ŕ����������邱�Ƃ����邽�� CDM_GETFOLDERIDLIST ��
	int size = (int)SendMessage( hDlgWnd, CDM_GETFOLDERIDLIST, 0, NULL );
	if ( size <= 0 )
		return FALSE;

	LPITEMIDLIST lpListBuff = (LPITEMIDLIST)CoTaskMemAlloc( size );
	if ( lpListBuff == NULL )
		return FALSE;

	LRESULT ret = SendMessage( hDlgWnd, CDM_GETFOLDERIDLIST, size, (LPARAM)lpListBuff );
	if ( ret <= 0 )
	{
		CoTaskMemFree( lpListBuff );
		return FALSE;
	}

	BOOL re = SHGetPathFromIDListA( lpListBuff, lpbuff );
	CoTaskMemFree( lpListBuff );
	if ( !re )
		return FALSE;

	return TRUE;
}

typedef struct {
	HWND hName;
	char szBuff[MAX_PATH];
} ND;

void EditBoxWatch(LPVOID pData)
{
	ND *pND;
	pND = (ND*)pData;
	char temp[MAX_PATH] = "";
	for (int i = 0; i < 20; i++) {	// �������[�v���Ȃ����߂ɂQ�O��񂵂���I���
		::GetWindowTextA( pND->hName, temp, MAX_PATH);
		TRACE("���[�v%d : %s\n",i, temp);
		if (temp[0] == '\0') {
			break;
		}
		Sleep(50);
	}
	::SetWindowTextA(pND->hName, pND->szBuff);
	SendMessage(pND->hName, EM_SETSEL, (WPARAM)0L, (LPARAM)-1L);
	//SendMessage( pND->hName, WM_SETTEXT, 0, (LPARAM)(LPWSTR)ATL::CA2W(pND->szBuff) );

	delete pND;
	_endthread();
}

// �t�@�C���_�C�A���O�̌��݂̃t�H���_��ύX����
BOOL SetDlgCurDir( HWND hDlg, LPCSTR lpdir )
{
	char currentFolderPath[MAX_PATH] = "";
	GetDlgCurDir(hDlg, currentFolderPath);
	if (::lstrcmpiA(lpdir, currentFolderPath) == 0)
		return TRUE;

	//HWND hAddrT = GetAddressBarTxt( hDlg );
	//HWND hAddrB = GetAddressBarBtn( hDlg );
	//HWND hAddrBrd = GetAddressBarBread( hDlg );
	HWND hAddBrd = GetAddressBarBread(hDlg);

#if 0
	if( hAddrT != NULL && hAddrB != NULL ) {
		// IFileDialog
		if(hAddrBrd != NULL){
			// Breadcrumb��Ԃ�����
			SendMessage(hAddrBrd,WM_SETFOCUS,0,0);
			SendMessage(hAddrBrd,WM_KEYDOWN,VK_RETURN,0);
			SendMessage(hAddrBrd,WM_KEYUP,VK_RETURN,0);
		}

		SendMessage(hAddrT,WM_SETTEXT,0,(LPARAM)(LPWSTR)ATL::CA2W(lpdir));

		SendMessage(hAddrB,WM_SETFOCUS,0,0);
		SendMessage(hAddrB,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrB,WM_KEYUP,VK_RETURN,0);
		return TRUE;
	}
	if (hAddrBrd != NULL && hAddrB != NULL) {
		// Breadcrumb��Ԃ�����
		SendMessage(hAddrBrd,WM_SETFOCUS,0,0);
		SendMessage(hAddrBrd,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrBrd,WM_KEYUP,VK_RETURN,0);

		HWND hAddrT = GetAddressBarTxt( hDlg );
		if (hAddrT == NULL) {
			TRACE("SetDlgCurDir �Ɏ��s�H\n");
		}
		SendMessage(hAddrT,WM_SETTEXT,0,(LPARAM)(LPWSTR)ATL::CA2W(lpdir));

		SendMessage(hAddrB,WM_SETFOCUS,0,0);
		SendMessage(hAddrB,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrB,WM_KEYUP,VK_RETURN,0);
		return TRUE;
	}
#endif

	TCHAR szBuff[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	lstrcpy( szDir, (LPWSTR)ATL::CA2W(lpdir) );
	PathAddBackslash( szDir );

	HWND hName = GetNameEdit( hDlg );
	if (hName == NULL)
		return FALSE;

	HWND hListView = FindWindowExA( GetDefView( hDlg ), NULL, "SysListView32", NULL );

	if( hListView != NULL )// ���X�g�r���[�̑I��������
		ListView_SetItemState( hListView, -1, 0 ,LVIS_SELECTED );

	//SendMessage( hName, WM_GETTEXT, MAX_PATH, (LPARAM)szBuff );
	//SendMessage( hName, WM_SETTEXT, 0, (LPARAM)szDir );
	::GetWindowTextW(hName, szBuff, MAX_PATH);
	HWND cmbBox = ::GetParent(hName);
	SendMessage(cmbBox, CB_SETCURSEL, -1, 0);
	::SetWindowTextW(hName, szDir);
	SendMessage( hDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0 );
	
	WCHAR title[MAX_PATH] = L"";
	::GetWindowText(hDlg, title, MAX_PATH);
	const int titleLen = ::lstrlen(title);
	if (titleLen > 2) {
		if (::lstrcmp(title + (titleLen - 2), L"�J��") == 0)
			return TRUE;
	}

	ND *pND = new ND;
	pND->hName = hName;
	lstrcpyA(pND->szBuff, (LPSTR)ATL::CW2A(szBuff));
	_beginthread(EditBoxWatch, 0, pND);

	return TRUE;
}


// �ݒ�ɉ����ă_�C�A���O�̈ʒu�𒲐�
void AdjustWinPos( HWND hWnd )
{
	RECT Wndrc, Listrc, Deskrc;
	POINT Pos, Size;
	int flag = SWP_NOZORDER;
	HWND hDefView = GetDefView( hWnd );

	GetWindowRect( hWnd, &Wndrc );
	GetWindowRect( hDefView, &Listrc );
	SystemParametersInfo( SPI_GETWORKAREA, 0, &Deskrc, 0 );

	if( g_Shared->bListSize && ( GetWindowLongX( hWnd, GWL_STYLE ) & WS_THICKFRAME ) ) {
		Size.x = g_Shared->ListSize.x;
		Size.y = g_Shared->ListSize.y;
		/*
		Size.x = ( Wndrc.right - Wndrc.left ) - ( Listrc.right - Listrc.left ) + g_Shared->ListSize.x;
		if( Size.x > Deskrc.right - Deskrc.left )
		Size.x = Deskrc.right - Deskrc.left;
		Size.y = ( Wndrc.bottom - Wndrc.top ) - ( Listrc.bottom - Listrc.top ) + g_Shared->ListSize.y;
		if( Size.y > Deskrc.bottom - Deskrc.top )
		Size.y = Deskrc.bottom - Deskrc.top;
		*/
	} else {
		flag |= SWP_NOSIZE;
		Size.x = Wndrc.right - Wndrc.left;
		Size.y = Wndrc.bottom - Wndrc.top;
	}

	switch( g_Shared->nDialogPos )
	{
	case 0: //�ʒu���w�肵�Ȃ�
		flag |= SWP_NOMOVE;
		break;
	case 1: //�ʒu���w�肷��
		Pos.x = g_Shared->DialogPos.x;
		Pos.y = g_Shared->DialogPos.y;
		break;
	case 2: //������
		Pos.x = ( Deskrc.right + Deskrc.left - Size.x ) / 2;
		Pos.y = ( Deskrc.bottom + Deskrc.top - Size.y ) / 2;
		break;
	}


	if( !( flag & SWP_NOSIZE) || !( flag & SWP_NOMOVE ) ) {
		SetWindowPos( hWnd, NULL, Pos.x, Pos.y, Size.x, Size.y, flag );
		TRACE("AdjustWinPos : x : %d  y : %d  �� : %d  ���� : %d  NOSIZE : %d  NOMOVE : %d\n", Pos.x, Pos.y, Size.x, Size.y, flag & SWP_NOSIZE, flag & SWP_NOMOVE);
	}
}

// EnumChildWindows�Ŏg��
BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam )
{
	char szClass[64];
	GetClassNameA( hwnd, szClass, 64 );

	// �E���� �܂� �͖���
	if( lstrcmpA( szClass, "ScrollBar" ) == 0 && GetWindowLongX( hwnd, GWL_STYLE ) & SBS_SIZEGRIP )
		return TRUE;

	std::pair< std::map< HWND, RECT >, BOOL > *p = (std::pair< std::map< HWND, RECT >, BOOL > *)lParam;
	std::map< HWND, RECT > &Map = p->first;

	if( p->second ) {
		RECT rc;
		GetWindowRect( hwnd, &rc );
		MapWindowPoints( HWND_DESKTOP, GetParent( hwnd ), (LPPOINT)&rc, 2 );
		Map.insert( std::make_pair( hwnd, rc ) );
	} else {
		RECT &rc = Map[hwnd];
		MoveWindow( hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE );
	}

	return TRUE;
}

// �V�����_�C�A���O�Ńc�[���o�[���쐬
void CreateExToolBar(HWND hWnd)
{
	// �c�[���o�[���쐬
	HWND ToolBar = ::CreateToolbarEx(
		hWnd, //�e�E�B���h�E�̃n���h��    
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TRANSPARENT | TBSTYLE_LIST |CCS_NODIVIDER,  //�E�B���h�E�X�^�C��
		ID_TOOLBAR,  //�c�[���o�[�̃R���g���[���h�c     
		0, //hBMIns��wBMID�œ��肳�ꂽ�r�b�g�}�b�v�ɂ͂����Ă���{�^���C���[�W�̐�
		NULL, //�r�b�g�}�b�v���\�[�X�������Ă��郂�W���[���̃C���X�^���X�n���h��     
		0, //���\�[�X�h�c    
		NULL,//TBBUTTON�\���̂̔z��̃|�C���^     
		0, //�c�[���o�[�̃{�^���̐�    
		0, //�{�^���̕�    
		0, //�{�^���̍���
		0, //�{�^���C���[�W�̕�
		0, //�{�^���C���[�W�̍���
		sizeof(TBBUTTON) //TBBUTTON�\���̂̑傫��
		); 
	::SetWindowLongPtr(ToolBar, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	::SetWindowText(ToolBar, _T("�g���c�[���o�["));

	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("�V���[�g�J�b�g"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("�ŋߎg�����t�H���_"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("�c�[��"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("�ݒ�"));

	TBBUTTON tbb[] = {
		{I_IMAGENONE, ID_SHORTCUT	, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 0},
		{I_IMAGENONE, ID_HISTORY	, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 1},
		{I_IMAGENONE, ID_TOOL		, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 2},
		{I_IMAGENONE, ID_SETTEI		, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 3}
	};
	::SendMessage(ToolBar, TB_ADDBUTTONS, 4, (LPARAM)tbb);

	// �c�[���o�[�̈ʒu���ړ�
	HWND DUI = FindChildWindowID(hWnd, "DUIViewWndClassName", 0);
	HWND WorkerW = FindChildWindowID(hWnd, "WorkerW", 0);
	RECT rcDUI, rcToolBar, rcWorker;
	::GetClientRect(DUI, &rcDUI);
	::GetClientRect(ToolBar, &rcToolBar);
	::GetClientRect(WorkerW, &rcWorker);
	// ���Ɉړ�
	::SetWindowPos(ToolBar, HWND_TOP, 0, rcWorker.bottom, 0, 0, SWP_NOSIZE);
	::SetWindowPos(DUI, NULL, 0, rcWorker.bottom + rcToolBar.bottom, rcDUI.right, rcDUI.bottom - rcToolBar.bottom, SWP_NOZORDER);
}

// �t�@�C���_�C�A���O�̏�����
BOOL InitFileDialog( HWND hWnd )
{
	// �T�u�N���X��
	WNDPROC DefWndProc = (WNDPROC)SetWindowLongX( hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc );
	SetPropA( hWnd, PROP_PROCEDURE, DefWndProc );

	if (GetAddressBarBread(hWnd)) {
		// �V�����t�@�C���_�C�A���O�̏ꍇ
		CreateExToolBar(hWnd);

	} else {
		// �]���̃_�C�A���O�̏ꍇ

		if( !g_Shared->bToolbar ) {
			// �c�[���o�[�ɒǉ��͂��Ȃ�

			// ���j���[���Z�b�g
			SetMenu(hWnd, LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU1 ) ) );

			// �R���g���[�����͂ݏo���Ȃ��悤�ɒ���
			if( GetWindowLongX( hWnd, GWL_STYLE ) & WS_THICKFRAME ) {
				/*
				RECT rc;
				GetWindowRect( hWnd, &rc );
				// �_�C�A���O���J�����тɏ����Â傫���Ȃ�̂�h�����߂ɁA�������������Ă���
				SetWindowPos( hWnd, NULL, 0, 0,
				rc.right - rc.left, rc.bottom - rc.top - GetSystemMetrics( SM_CYMENU ),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

				std::pair< std::map< HWND, RECT >, BOOL > p;
				p.second = TRUE;
				EnumChildWindows( hWnd, EnumChildProc, (LPARAM)&p );

				GetWindowRect( hWnd, &rc );
				SetWindowPos( hWnd, NULL, 0, 0,
				rc.right - rc.left, rc.bottom - rc.top + GetSystemMetrics( SM_CYMENU ),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );

				p.second = FALSE;
				EnumChildWindows( hWnd, EnumChildProc, (LPARAM)&p );
				*/

			} else {
				RECT rc;
				GetWindowRect( hWnd, &rc );
				SetWindowPos( hWnd, NULL, 0, 0,
					rc.right - rc.left, rc.bottom - rc.top + GetSystemMetrics( SM_CYMENU ),
					SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );
			}

		} else {
			// �c�[���o�[�Ƀ{�^����ǉ�
			RECT rc;
			HWND hToolbar = GetToolbar( hWnd );
			TBBUTTON TBA[] = {
				{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0 },
				{ HIST_FAVORITES, TB_BUTTONID, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, 0, 0, 0 }
			};
			TOOLINFO ti = { sizeof(TOOLINFO), TTF_SUBCLASS, hToolbar };
			HWND hTooltip = (HWND)SendMessage( hToolbar, TB_GETTOOLTIPS , 0, 0 );

			TBA[1].iBitmap += (int)SendMessage( hToolbar, TB_LOADIMAGES, IDB_HIST_SMALL_COLOR, (LPARAM)HINST_COMMCTRL );

			int btncnt = (int)SendMessage( hToolbar, TB_BUTTONCOUNT, 0, 0 );
			for( int i = 0; i < sizeof( TBA ) / sizeof( TBA[0] ); i++ )
				SendMessage( hToolbar, TB_INSERTBUTTON, btncnt++, (LPARAM)&TBA[i] );

			SendMessage( hToolbar, TB_GETITEMRECT, btncnt - 1, (LPARAM)&ti.rect);
			ti.uId = TB_BUTTONID;
			ti.lpszText = _T("�t�@�C���_�C�A���O�g��(ALT+M)");
			SendMessage( hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti );

			// �c�[���o�[�̃T�C�Y����
			SendMessage( hToolbar, TB_GETITEMRECT, btncnt - 1, (LPARAM)&rc );
			SetWindowPos( hToolbar, 0, 0, 0,
				rc.right, rc.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER );
		}
	}

	// �ʒu�A�T�C�Y����
	AdjustWinPos( hWnd );

	// �\����ݒ�
	HWND hDefView = GetDefView( hWnd );
	if( g_Shared->ListStyle )
		SendMessage( hDefView, WM_COMMAND, g_Shared->ListStyle, 0 );
	// �\�[�g���鍀�ڂ�ݒ�
	if( g_Shared->ListSort )
		SendMessage( hDefView, WM_COMMAND, g_Shared->ListSort, 0 );

	g_Shared->cDialog++;

#if 1
	if (!(::GetAsyncKeyState(VK_CONTROL) < 0)) {
		//ExplorerPathToFileDlg();
		IShellBrowser* shellBrowser = (IShellBrowser*)::SendMessage(hWnd, WM_USER + 7, 0, 0);
		if (shellBrowser) {
			PIDLIST_ABSOLUTE pidl = nullptr;
			DWORD attr = 0;
			HRESULT hr = ::SHILCreateFromPath(L"F:\\�A�v���P�[�V����", &pidl, &attr);
			hr = shellBrowser->BrowseObject(pidl, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
			::ILFree(pidl);
		}
	}
#endif
	return TRUE;
}

/*
// �t�@�C���_�C�A���O�̏�����
BOOL InitFileDialog( HWND hWnd )
{
// �T�u�N���X��
WNDPROC DefWndProc = (WNDPROC)SetWindowLongX( hWnd, GWL_WNDPROC, (LONG)WndProc );
SetProp( hWnd, PROP_PROCEDURE, DefWndProc );

if( !g_Shared->bToolbar ) {
// ���j���[���Z�b�g
//      SetMenu(hWnd, LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU1 ) ) );
HWND hWndChild = GetDirectUIHWND(hWnd);
SetMenu(hWndChild,  LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU1 ) ));
// �R���g���[�����͂ݏo���Ȃ��悤�ɒ���
if( GetWindowLongX( hWnd, GWL_STYLE ) & WS_THICKFRAME ) {
RECT rc;
GetWindowRect( hWnd, &rc );
// �_�C�A���O���J�����тɏ����Â傫���Ȃ�̂�h�����߂ɁA�������������Ă���
SetWindowPos( hWnd, NULL, 0, 0,
rc.right - rc.left, rc.bottom - rc.top - GetSystemMetrics( SM_CYMENU ),
SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

std::pair< std::map< HWND, RECT >, BOOL > p;
p.second = TRUE;
EnumChildWindows( hWnd, EnumChildProc, (LPARAM)&p );

GetWindowRect( hWnd, &rc );
SetWindowPos( hWnd, NULL, 0, 0,
rc.right - rc.left, rc.bottom - rc.top + GetSystemMetrics( SM_CYMENU ),
SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );

p.second = FALSE;
EnumChildWindows( hWnd, EnumChildProc, (LPARAM)&p );

} else {
RECT rc;
GetWindowRect( hWnd, &rc );
SetWindowPos( hWnd, NULL, 0, 0,
rc.right - rc.left, rc.bottom - rc.top + GetSystemMetrics( SM_CYMENU ),
SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );
}
} else {
// �c�[���o�[�Ƀ{�^����ǉ�
RECT rc;
HWND hToolbar = GetToolbar( hWnd );
TBBUTTON TBA[] = {
{ 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0 },
{ HIST_FAVORITES, TB_BUTTONID, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, 0, 0, 0 }
};
TOOLINFO ti = { sizeof(TOOLINFO), TTF_SUBCLASS, hToolbar };
HWND hTooltip = (HWND)SendMessage( hToolbar, TB_GETTOOLTIPS , 0, 0 );

TBA[1].iBitmap += SendMessage( hToolbar, TB_LOADIMAGES, IDB_HIST_SMALL_COLOR, (LPARAM)HINST_COMMCTRL );

int btncnt = SendMessage( hToolbar, TB_BUTTONCOUNT, 0, 0 );
for( int i = 0; i < sizeof( TBA ) / sizeof( TBA[0] ); i++ )
SendMessage( hToolbar, TB_INSERTBUTTON, btncnt++, (LPARAM)&TBA[i] );

SendMessage( hToolbar, TB_GETITEMRECT, btncnt - 1, (LPARAM)&ti.rect);
ti.uId = TB_BUTTONID;
ti.lpszText = "�t�@�C���_�C�A���O�g��(ALT+M)";
SendMessage( hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti );

// �c�[���o�[�̃T�C�Y����
SendMessage( hToolbar, TB_GETITEMRECT, btncnt - 1, (LPARAM)&rc );
SetWindowPos( hToolbar, 0, 0, 0,
rc.right, rc.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER );
}

// �ʒu�A�T�C�Y����
AdjustWinPos( hWnd );

// �\����ݒ�
HWND hDefView = GetDefView( hWnd );
if( g_Shared->ListStyle )
SendMessage( hDefView, WM_COMMAND, g_Shared->ListStyle, 0 );
// �\�[�g���鍀�ڂ�ݒ�
if( g_Shared->ListSort )
SendMessage( hDefView, WM_COMMAND, g_Shared->ListSort, 0 );

g_Shared->cDialog++;

return TRUE;
}
*/


// ���݊J���Ă���t�H���_���V���[�g�J�b�g�ɒǉ�����
BOOL AppendShortcut( LPCSTR pszPath, HWND hWnd )
{
	char szName[80];
	PathCompactPathExA( szName, pszPath, 64, 0 );
	if( g_Shared->shortcut.count < SHORTCUT_MAX )
	{
		lstrcpyA( g_Shared->shortcut.data[g_Shared->shortcut.count].szName, szName );
		lstrcpyA( g_Shared->shortcut.data[g_Shared->shortcut.count].szPath, pszPath );
		g_Shared->shortcut.count++;

		// ini�ɂ���������
		char szKey[32];
		wsprintfA( szKey, "Name%d", g_Shared->shortcut.count - 1 );
		WritePrivateProfileStringA( "Shortcut", szKey, szName, g_Shared->szIniPath );
		wsprintfA( szKey, "Path%d", g_Shared->shortcut.count - 1 );
		WritePrivateProfileStringA( "Shortcut", szKey, pszPath, g_Shared->szIniPath );
	}
	else
	{
		wsprintfA( szName, "�V���[�g�J�b�g��%d�܂ł����쐬�ł��܂���" , SHORTCUT_MAX );
		MessageBoxA( hWnd, szName, nullptr, 0 );
		return FALSE;
	}
	return TRUE;
}

// �����ɒǉ�
BOOL AppendHistory( LPCSTR pszPath )
{
	// �f�[�^������ɂ��炷
	int i;
	for( i = 0; i < g_Shared->history.count; i++ )
		if( lstrcmpA( g_Shared->history.data[i].szPath, pszPath ) == 0 )
			break;
	if( i == g_Shared->history.count && g_Shared->history.count < g_Shared->history.max )
		g_Shared->history.count++;
	if( i == g_Shared->history.max )
		i--;
	for( ; i > 0; i-- )
		lstrcpyA( g_Shared->history.data[i].szPath, g_Shared->history.data[i - 1].szPath );

	// �����̐擪�ɂɒǉ�
	lstrcpyA( g_Shared->history.data[0].szPath, pszPath );

	return TRUE;
}

// ini�t�@�C������V���[�g�J�b�g��ǂݍ���
BOOL LoadShortcut( LPCSTR szIni )
{
	char szKey[32];
	for( int i = 0; i < SHORTCUT_MAX; i++ )
	{
		wsprintfA( szKey, "Name%d", i );
		GetPrivateProfileStringA( "Shortcut", szKey, "none", g_Shared->shortcut.data[i].szName, MAX_PATH, szIni );
		if( lstrcmpA( g_Shared->shortcut.data[i].szName, "none" ) == 0 )
			break;
		wsprintfA( szKey, "Path%d", i );
		GetPrivateProfileStringA( "Shortcut", szKey, "", g_Shared->shortcut.data[i].szPath, MAX_PATH, szIni );
		g_Shared->shortcut.count++;
	}

	return TRUE;
}

// ini �t�@�C�����痚����ǂݍ���
BOOL LoadHistory( LPCSTR szIni )
{
	char szKey[32];
	for( int i = 0; i < HISTORY_MAX; i++ )
	{
		wsprintfA( szKey, "hist%d", i );
		GetPrivateProfileStringA( "History", szKey, "none", g_Shared->history.data[i].szPath, MAX_PATH, szIni );
		if( lstrcmpA( g_Shared->history.data[i].szPath, "none" ) == 0 )
			break;
		g_Shared->history.count++;
	}

	return TRUE;
}

// ini�t�@�C������c�[����ǂݍ���
BOOL LoadTool( LPCSTR szIni )
{
	char szKey[32];
	for( int i = 0; i < TOOL_MAX; i++ )
	{
		wsprintfA( szKey, "Name%d", i );
		GetPrivateProfileStringA( "Tool", szKey, "none", g_Shared->tool.data[i].szName, MAX_PATH, szIni );
		if( lstrcmpA( g_Shared->tool.data[i].szName, "none" ) == 0 )
			break;
		wsprintfA( szKey, "Path%d", i );
		GetPrivateProfileStringA( "Tool", szKey, "", g_Shared->tool.data[i].szPath, MAX_PATH, szIni );
		wsprintfA( szKey, "Param%d", i );
		GetPrivateProfileStringA( "Tool", szKey, "", g_Shared->tool.data[i].szParam, MAX_PATH, szIni );
		g_Shared->tool.count++;
	}

	return TRUE;
}

// ini �t�@�C���ɗ�����ۑ�
BOOL SaveHistory( LPCSTR szIni )
{
	WritePrivateProfileSectionA( "History", "\0\0", szIni );
	char szKey[32];
	for( int i = 0; i < g_Shared->history.count; i++ )
	{
		wsprintfA( szKey, "hist%d", i );
		WritePrivateProfileStringA( "History", szKey, g_Shared->history.data[i].szPath, szIni );
	}

	return TRUE;
}

// ���j���[�̍��ڂ�������

BOOL InitMenu( HMENU hMenu )
{
	HMENU hShortcutMenu = GetSubMenu( hMenu, 0 );
	HMENU hHistoryMenu = GetSubMenu( hMenu, 1 );
	HMENU hToolMenu = GetSubMenu( hMenu, 2 );
	UINT uID;

	// �V���[�g�J�b�g�̃��j���[�����
	while( uID = GetMenuItemID( hShortcutMenu, 0 ),
		(ID_SHORTCUT_FIRST <= uID && uID < ID_SHORTCUT_FIRST + SHORTCUT_MAX) )
		DeleteMenu( hShortcutMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->shortcut.count; i++ )
		InsertMenuA( hShortcutMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
		ID_SHORTCUT_FIRST + i, g_Shared->shortcut.data[i].szName );

	// �����̃��j���[�����
	while( uID = GetMenuItemID( hHistoryMenu, 0 ),
		( ID_HISTORY_FIRST <= uID && uID < ID_HISTORY_FIRST + HISTORY_MAX ) )
		DeleteMenu( hHistoryMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->history.count; i++ )
	{
		char szBuff[MAX_PATH];
		PathCompactPathExA( szBuff, g_Shared->history.data[i].szPath, 64, 0);
		InsertMenuA( hHistoryMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
			ID_HISTORY_FIRST + i, szBuff );
	}

	// �c�[���̃��j���[�����
	while( uID = GetMenuItemID( hToolMenu, 0 ),
		(ID_TOOL_FIRST <= uID && uID < ID_TOOL_FIRST + TOOL_MAX) )
		DeleteMenu( hToolMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->tool.count; i++ )
		InsertMenuA( hToolMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
		ID_TOOL_FIRST + i, g_Shared->tool.data[i].szName );

	return TRUE;
}

BOOL PopupMenu( HWND hWnd, HWND hToolbar )
{
	TPMPARAMS tpm = { sizeof(TPMPARAMS) };
	SendMessage( hToolbar, TB_GETRECT, (WPARAM)TB_BUTTONID, (LPARAM)&tpm.rcExclude);
	MapWindowPoints( hToolbar, HWND_DESKTOP, (LPPOINT)&tpm.rcExclude, 2 );

	HMENU hMenu = LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU2 ) );
	HMENU hSubMenu = GetSubMenu( hMenu, 0 );
	InitMenu( hSubMenu );
	BOOL ret = TrackPopupMenuEx( hSubMenu, TPM_RECURSE, tpm.rcExclude.left, tpm.rcExclude.bottom, hWnd, &tpm );
	DestroyMenu( hMenu );
	if(!ret)
	{
		char sz[111];
		wsprintfA(sz,"%d", GetLastError());
		MessageBoxA( hWnd, sz,sz,0);
	}

	return ret;
}


HHOOK g_LLMouseHook = NULL;
HWND  g_wndToolbar = NULL;
int	  g_nowPopupIndex = -1;

LRESULT CALLBACK LowLevelMouseProc(
  int nCode,     // �t�b�N�R�[�h
  WPARAM wParam, // ���b�Z�[�W���ʎq
  LPARAM lParam  // ���b�Z�[�W�f�[�^
)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_MOUSEMOVE) {
			LPMSLLHOOKSTRUCT pmsllhk = (LPMSLLHOOKSTRUCT)lParam;
			HWND hWndpt = ::WindowFromPoint(pmsllhk->pt);
			if (hWndpt == g_wndToolbar) {
				POINT pt = pmsllhk->pt;
				::ScreenToClient(g_wndToolbar, &pt);
				int nIndex = (int)::SendMessage(g_wndToolbar, TB_HITTEST, 0, (LPARAM)&pt);
				if (0 <= nIndex && nIndex < 3 && g_nowPopupIndex != nIndex) {
					TRACE("�\�����郁�j���[��؂�ւ��܂�");
					::EndMenu();
					g_nowPopupIndex = nIndex;
					::PostMessage(::GetParent(g_wndToolbar), WM_COMMAND, ID_SHORTCUT + nIndex, 1);
				}
			}
		}
	}

	return ::CallNextHookEx(g_LLMouseHook, nCode, wParam, lParam);
}

BOOL PopupMenu( HWND hWnd, HWND hToolbar, int iID )
{
	int	nIndex;
	switch ( iID ) 
	{
	case ID_SHORTCUT:
		nIndex = 0;
		break;
	case ID_HISTORY:
		nIndex = 1;
		break;
	case ID_TOOL:
		nIndex = 2;
		break;
	default:
		return FALSE;
	}
	g_nowPopupIndex = nIndex;
	TPMPARAMS tpm = { sizeof(TPMPARAMS) };
	SendMessage( hToolbar, TB_GETRECT, (WPARAM)iID, (LPARAM)&tpm.rcExclude);
	MapWindowPoints( hToolbar, HWND_DESKTOP, (LPPOINT)&tpm.rcExclude, 2 );

	HMENU hMenu = LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU2 ) );
	HMENU hSubMenu = GetSubMenu( hMenu, 0 );
	InitMenu( hSubMenu );
	HMENU hSubSubMenu = GetSubMenu( hSubMenu, nIndex );

	HHOOK hHook = ::SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_Inst, 0);
	g_LLMouseHook = hHook;
	g_wndToolbar = hToolbar;

	::SendMessage(hToolbar, TB_PRESSBUTTON, iID, MAKELPARAM(1, 0));
	BOOL ret = TrackPopupMenuEx( hSubSubMenu, TPM_RETURNCMD, tpm.rcExclude.left, tpm.rcExclude.bottom, hWnd, &tpm );
	::SendMessage(hToolbar, TB_PRESSBUTTON, iID, MAKELPARAM(0, 0));

	g_wndToolbar = NULL;
	::UnhookWindowsHookEx(hHook);

	DestroyMenu( hMenu );
	if (ret != 0) {
		::SendMessage(hWnd, WM_COMMAND, ret, 0);
	}
	g_nowPopupIndex = -1;

	return ret;
}


// --------------------------------------------------------
//    �T�u�N���X���p�̃v���V�[�W��
// --------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC DefProc = (WNDPROC)GetPropA( hWnd, PROP_PROCEDURE );
	if (DefProc == NULL)
		return 0;

	if (message == WM_FILEDIALOGEXDETECTMESSAGE)
		return 1;

	__declspec(thread) static bool	bFirstOpen = true;

	switch (message)
	{
	case WM_COPYDATA:
		{
			COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
			if (pcds->dwData == kExOpenDir) {
				SetDlgCurDir(hWnd, static_cast<LPCSTR>(pcds->lpData));
				return 1;
			}
			break;
		}

	case WM_ACTIVATE:
		{
			BOOL bActive = wParam & 0xFFFF;
			if (bActive && bFirstOpen && !(::GetAsyncKeyState(VK_CONTROL) < 0))
				ExplorerPathToFileDlg();
			bFirstOpen = false;
		}
		break;

	case WM_COMMAND:
		if( lParam == 0 ) // ���j���[����
		{
			WORD wID = LOWORD( wParam );
			switch( wID )
			{
			case ID_APPEND:
				{
					char szPath[MAX_PATH];
					if( GetDlgCurDir( hWnd, szPath ) )
						AppendShortcut( szPath, hWnd );
				}
				return 0;
			case ID_CLEAR:
				g_Shared->history.count = 0;
				SaveHistory( g_Shared->szIniPath );
				return 0;
			case ID_SETTING:
				SettingDialog( hWnd );
				return 0;
			default:
				if( ID_SHORTCUT_FIRST <= wID && wID < ID_SHORTCUT_FIRST + SHORTCUT_MAX )
				{
					SetDlgCurDir( hWnd, g_Shared->shortcut.data[ wID - ID_SHORTCUT_FIRST ].szPath );
					return 0;
				}
				else if( ID_HISTORY_FIRST <= wID && wID < ID_HISTORY_FIRST + HISTORY_MAX )
				{
					SetDlgCurDir( hWnd, g_Shared->history.data[ wID - ID_HISTORY_FIRST ].szPath );
					return 0;
				}
				else if( ID_TOOL_FIRST <= wID && wID < ID_TOOL_FIRST + TOOL_MAX )
				{
					char szParam[MAX_PATH*2];
					char szDir[MAX_PATH];

					lstrcpyA( szParam, g_Shared->tool.data[wID - ID_TOOL_FIRST].szParam );
					if( StrStrA( szParam, "%1" ) != 0 )
					{ // "%1"���܂�ł����猻�݂̃t�H���_�̃p�X�Œu������B
						if( GetDlgCurDir( hWnd, szDir ) )
							StrReplace( szParam, "%1", szDir, MAX_PATH*2 );
						else
							return 0;
					}

					if( (int)ShellExecuteA( NULL, NULL, g_Shared->tool.data[wID - ID_TOOL_FIRST].szPath,
						szParam, NULL, SW_SHOWDEFAULT ) <= 32 )
						MessageBoxA( hWnd, "���s�ł��܂���B�p�X���m�F���Ă��������B", "�G���[", 0 );
				}
				break;
			}
		} else {

			WORD wID = LOWORD( wParam );
			switch( wID ) 
			{
			case ID_SHORTCUT:
			case ID_HISTORY:
			case ID_TOOL:
				{
					HWND ToolBar = FindWindowExA(hWnd, NULL, "ToolbarWindow32", "�g���c�[���o�[");
					PopupMenu(hWnd, ToolBar, wID);
					break;
				}
			case ID_SETTEI:
				SettingDialog( hWnd );
				return 0;
			case IDCANCEL: // �L�����Z���{�^��
				SetPropA( hWnd, PROP_CANCEL, (HANDLE)TRUE );
				break;
			}
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pnmd = (LPNMHDR)lParam;
			switch( pnmd->code )
			{
			case TBN_DROPDOWN:
				{
					LPNMTOOLBAR pnmt = (LPNMTOOLBAR)lParam;
					if( pnmt->iItem == TB_BUTTONID ) {
						// �c�[���o�[�̃{�^�����N���b�N���ꂽ
						PopupMenu( hWnd, pnmt->hdr.hwndFrom );
						return TBDDRET_DEFAULT;
					}
				}
				break;
			}
		}
		break;
	case WM_MENUCHAR:
		if( LOWORD( wParam ) == 'm' && g_Shared->bToolbar )
		{
			// ���j���[��\��
			static int i = 0;
			if( i == 0 ) // ���j���[��2�d�ɕ\������Ȃ��悤��
			{
				i++;
				HWND hToolbar = GetToolbar( hWnd );
				SendMessage( hToolbar, TB_PRESSBUTTON , TB_BUTTONID, MAKELPARAM( TRUE, 0 ) );
				PopupMenu( hWnd, hToolbar );
				SendMessage( hToolbar, TB_PRESSBUTTON , TB_BUTTONID, MAKELPARAM( FALSE, 0 ) );
				i--;
			}
			return MAKELONG( 0, MNC_CLOSE );
		}
		break;
	case WM_INITMENU:
		InitMenu( (HMENU)wParam );
		break;
	case WM_SIZE:
		{
			HWND ToolBar = FindWindowExA(hWnd, NULL, "ToolbarWindow32", "�g���c�[���o�[");
			if (ToolBar != NULL) {
				// �V�����t�@�C���_�C�A���O�̏ꍇ
				// �c�[���o�[��\�����邽�߂Ƀr���[�����ɂ��炷
				::SendMessage(hWnd, WM_SETREDRAW, (WPARAM)FALSE, 0);
				CallWindowProcX( DefProc, hWnd, message, wParam, lParam );

				HWND DUI = FindChildWindowID(hWnd, "DUIViewWndClassName", 0);
				HWND WorkerW = FindChildWindowID(hWnd, "WorkerW", 0);
				RECT rcDUI, rcToolBar, rcWorker;
				::GetClientRect(DUI, &rcDUI);
				::GetClientRect(ToolBar, &rcToolBar);
				::GetClientRect(WorkerW, &rcWorker);
				// ���Ɉړ�
				::SetWindowPos(ToolBar, HWND_TOP, 0, rcWorker.bottom, 0, 0, SWP_NOSIZE);
				::SetWindowPos(DUI, NULL, 0, rcWorker.bottom + rcToolBar.bottom, rcDUI.right, rcDUI.bottom - rcToolBar.bottom, SWP_NOZORDER);

				::SendMessage(hWnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
				::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
				return 0;
			}
		}
		break;
	case WM_DESTROY:
		bFirstOpen = true;

		// �����ɒǉ�
		if( !GetPropA( hWnd, PROP_CANCEL) ) {
			// �L�����Z���ŏI���ł͂Ȃ��̂ŗ�����ۑ�
			char szBuff[MAX_PATH];
			if( GetDlgCurDir( hWnd, szBuff ) ) {
				AppendHistory( szBuff );
				SaveHistory( g_Shared->szIniPath );
			}
		}

		// ���j���[���폜
		HMENU hMenu = GetMenu( hWnd );
		if( hMenu ) {
			SetMenu( hWnd, NULL );
			DestroyMenu( hMenu );
			//�E�B���h�E�T�C�Y��߂�
			//RECT rc;
			//GetWindowRect( hWnd, &rc );
			//SetWindowPos( hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top - GetSystemMetrics( SM_CYMENU ),
			// SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );
		}

		//�T�u�N���X��������
		SetWindowLongX( hWnd, GWLP_WNDPROC, (LONG_PTR)DefProc );

		g_Shared->cDialog--;

		RemovePropA( hWnd, PROP_PROCEDURE );
		RemovePropA( hWnd, PROP_CANCEL );

		break;
	}

	return CallWindowProcX( DefProc, hWnd, message, wParam, lParam );
}

BOOL IsFileDialog( HWND hWnd )
{
	TRACE("IsFileDialog���J�n--------------\n");
	if( (((GetAddressBarBread(hWnd) || GetAddressBarTxt(hWnd)) && GetAddressBarBtn(hWnd) != NULL)
		|| (GetDefView( hWnd ) != NULL && GetNameEdit( hWnd ) != NULL)) ){
			TRACE("  �t�@�C���_�C�A���O�ł����B�t�b�N���J�n���܂�\n");
			return TRUE;
	} else {
		TRACE("  �t�@�C���_�C�A���O�ł͂Ȃ�����\n");
		return FALSE;
	}
}

// --------------------------------------------------------
//    �t�b�N�v���V�[�W��
// --------------------------------------------------------
LRESULT CALLBACK CallWndRetProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if ( nCode == HC_ACTION ) {
		LPCWPRETSTRUCT pcw = (LPCWPRETSTRUCT)lParam;
		if( pcw->message == WM_INITDIALOG ) {
			if( IsFileDialog( pcw->hwnd ) ) {
				InitFileDialog( pcw->hwnd );
			}
		}
	}

	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}



// --------------------------------------------------------
//    �v���O�C�������[�h���ꂽ�Ƃ��ɌĂ΂��
// --------------------------------------------------------

BOOL Init(void)
{
	int len = GetModuleFileNameA( g_Inst, g_Shared->szIniPath, MAX_PATH);

	g_Shared->szIniPath[len - 3] = 'i';
	g_Shared->szIniPath[len - 2] = 'n';
	g_Shared->szIniPath[len - 1] = 'i';

	TRACE("open");	// log�t�@�C���쐬

	g_Shared->nDialogPos    = GetPrivateProfileIntA("Setting", "Pos", FALSE, g_Shared->szIniPath);
	g_Shared->DialogPos.x   = GetPrivateProfileIntA("Setting", "PosX", 100, g_Shared->szIniPath);
	g_Shared->DialogPos.y   = GetPrivateProfileIntA("Setting", "PosY", 100, g_Shared->szIniPath);
	g_Shared->bListSize     = GetPrivateProfileIntA("Setting", "Size", FALSE, g_Shared->szIniPath);
	g_Shared->ListSize.x    = GetPrivateProfileIntA("Setting", "SizeX", 640, g_Shared->szIniPath);
	g_Shared->ListSize.y    = GetPrivateProfileIntA("Setting", "SizeY", 480, g_Shared->szIniPath);
	g_Shared->ListStyle     = GetPrivateProfileIntA("Setting", "ListStyle", 0, g_Shared->szIniPath);
	g_Shared->ListSort      = GetPrivateProfileIntA("Setting", "ListSort", 0, g_Shared->szIniPath);
	g_Shared->bToolbar      = GetPrivateProfileIntA("Setting", "Toolbar", 0, g_Shared->szIniPath);


	// ����
	g_Shared->history.count = 0;
	g_Shared->history.max = GetPrivateProfileIntA( "Setting", "MaxHistory", 5, g_Shared->szIniPath);
	LoadHistory( g_Shared->szIniPath );

	// �V���[�g�J�b�g
	g_Shared->shortcut.count = 0;
	LoadShortcut( g_Shared->szIniPath );

	// �c�[��
	g_Shared->tool.count = 0;
	LoadTool( g_Shared->szIniPath );

	g_hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_Inst, 0);
	if(!g_hHook) {
		TRACE(" SetWindowsHookEx�Ɏ��s(%#x), inst(%#x)\n", GetLastError(), g_Inst);		
		return FALSE;
	}

#if 0
	::CoInitialize(NULL);
	{
		CComPtr<IShellWindows> spShellWindows;
		HRESULT hr = ::CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL, IID_IShellWindows, (void**)&spShellWindows);

		long count = 0;
		spShellWindows->get_Count(&count);
		for (long i = 0; i < count; ++i) {
			CComVariant vIndex(i);
			CComPtr<IDispatch> spDisp;
			hr = spShellWindows->Item(vIndex, &spDisp);
			if (SUCCEEDED(hr)) {
				CComPtr<IShellBrowser> spShellBrowser;
				hr = IUnknown_QueryService(spDisp, SID_STopLevelBrowser, IID_IShellBrowser, (void**)&spShellBrowser);
				if (SUCCEEDED(hr)) {
					HWND hwnd = NULL;
					spShellBrowser->GetWindow(&hwnd);
					int a = 0;
				}
			}
		}
	}
	::CoUninitialize();
#endif

	::PostMessageA(HWND_BROADCAST, WM_NULL, 0, 0);	// ���ׂẴE�B���h�E�Ƀt�b�N���d�|����

	return TRUE;
}

// --------------------------------------------------------
//    �v���O�C�����A�����[�h���ꂽ�Ƃ��ɌĂ΂��
// --------------------------------------------------------
void Unload(void)
{
#if 0
	while (g_Shared->cDialog) {
		int ret = MessageBoxA(NULL, "TTBase���I������O�ɂ��ׂẴt�@�C���_�C�A���O����Ă�������",
			"�t�@�C���_�C�A���O�g��7", MB_YESNO | MB_ICONEXCLAMATION);
		if (ret == IDNO)
			g_Shared->cDialog = 0;
	}
#endif

	UnhookWindowsHookEx( g_hHook );
	g_hHook = NULL;

	// UnHook�����̂�dll��unload�𑣂�
	::PostMessageA(HWND_BROADCAST, WM_NULL, 0, 0);
}

// --------------------------------------------------------
//    �R�}���h���s���ɌĂ΂��
// --------------------------------------------------------
BOOL Execute(int CmdId, HWND hWnd)
{
	return TRUE;
}

// --------------------------------------------------------
//    �O���[�o���t�b�N���b�Z�[�W������Ă���
// --------------------------------------------------------
void Hook(UINT Msg, DWORD wParam, DWORD lParam)
{
}