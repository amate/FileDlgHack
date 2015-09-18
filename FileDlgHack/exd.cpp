
#include "stdafx.h"
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
	
#include <atlbase.h>
#include <atlcom.h>
#include <atlapp.h>
#include <atlmisc.h>

static const int WM_FILEDIALOGEXDETECTMESSAGE = ::RegisterWindowMessage(_T("FileDialogDetectMessage"));

enum { kExOpenDir = 200 };


HWND FindChildWindowID( HWND hParent, LPCTSTR szClass, int ID )
{
	HWND hWnd = NULL;
	while( 1 )
	{
		hWnd = FindWindowEx( hParent, hWnd, szClass, NULL );
		int tID = GetWindowLong( hWnd, GWL_ID );
		if( hWnd == NULL || tID == ID )
			return hWnd;
	}
}

LONG_PTR GetWindowLongX(HWND hWnd, int nIndex);

// ID�ƃN���X���̔z�񂩂�q�E�B���h�E��T��
// �{����szClass,ID���܂Ƃ߂�struct������āA����vector��n�������񂾂��ǁA
// ���ƂȂ�vector�g���̎��d
HWND FindChildWindowIDRecursive(HWND hParent, LPTSTR * szClass, const int * ID){
	HWND hWnd = NULL;
	while ( 1 ){
		hWnd = FindWindowEx(hParent, hWnd, *szClass, NULL);

		if(hWnd == NULL){
			return NULL;		// �q�E�B���h�E��������Ȃ�����
		}
		int tID = GetWindowLongX( hWnd, GWL_ID);
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

// IFileDialog�A�h���X�o�[�̃R���{�{�b�N�X
HWND GetAddressBarTxt(HWND hDlg);

// IFileDialog�A�h���X�o�[�̃{�^��
HWND GetAddressBarBtn(HWND hDlg);

// IFileDialog�̃A�h���X�o�[��Breadcumb���(?)�ɂ���Ƃ���ComboBox�̑���ɕ\�������ToolBar
HWND GetAddressBarBread(HWND hDlg);

// �t�@�C���_�C�A���O�̌��݂̃t�H���_��ύX����
BOOL SetDlgCurDir( HWND hDlg, LPCTSTR lpdir )
{
	HWND hAddrT = GetAddressBarTxt( hDlg );
	HWND hAddrB = GetAddressBarBtn( hDlg );
	HWND hAddrBrd = GetAddressBarBread( hDlg );
	if( hAddrT != NULL && hAddrB != NULL ) {
		// IFileDialog
		if(hAddrBrd != NULL){
			// Breadcrumb��Ԃ�����
			SendMessage(hAddrBrd,WM_SETFOCUS,0,0);
			SendMessage(hAddrBrd,WM_KEYDOWN,VK_RETURN,0);
			SendMessage(hAddrBrd,WM_KEYUP,VK_RETURN,0);
		}

		SendMessage(hAddrT,WM_SETTEXT,0,(LPARAM)lpdir);

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
			ATLTRACE("SetDlgCurDir �Ɏ��s�H\n");
		}
		SendMessage(hAddrT,WM_SETTEXT,0,(LPARAM)lpdir);

		SendMessage(hAddrB,WM_SETFOCUS,0,0);
		SendMessage(hAddrB,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrB,WM_KEYUP,VK_RETURN,0);
		return TRUE;
	}
	return FALSE;
}

// �G�N�X�v���[���[��������
BOOL CALLBACK EnumWindowsProcExp( HWND hwnd, LPARAM lParam )
{
	TCHAR szClass[MAX_PATH];
	GetClassName(hwnd, szClass, MAX_PATH);
	if(lstrcmp(szClass, _T("CabinetWClass")) == 0 || lstrcmp(szClass, _T("ExploreWClass")) == 0)
	{
		*((HWND*)lParam) = hwnd;
		return FALSE;
	}
	return TRUE;
}

// �t�@�C���_�C�A���O��������
BOOL CALLBACK EnumWindowsProcDlg( HWND hwnd, LPARAM lParam )
{
	TCHAR szClass[MAX_PATH];
	GetClassName(hwnd, szClass, MAX_PATH);
	if( lstrcmp(szClass, _T("#32770") ) == 0 )
	{
		if (::SendMessage(hwnd, WM_FILEDIALOGEXDETECTMESSAGE, 0, 0) != 0) {
			*((HWND*)lParam) = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}



void ExplorerPathToFileDlg()
{
	HWND hExplorer = NULL;
	HWND hFileDlg = NULL;
	// �G�N�X�v���[���[��������
	EnumWindows( EnumWindowsProcExp, (LPARAM)&hExplorer );
	// �t�@�C���_�C�A���O��������
	EnumWindows( EnumWindowsProcDlg, (LPARAM)&hFileDlg );

	if ( hExplorer && hFileDlg ) {
		// �G�N�X�v���[���[���猻�ݕ\�����̃t�H���_�̃p�X�𓾂�
		LPTSTR kls[] = {_T("WorkerW"), _T("ReBarWindow32"), _T("Address Band Root"), _T("msctls_progress32"), _T("Breadcrumb Parent"), _T("ToolbarWindow32"), NULL};
		const int ids[] = {0, 0xA005, 0xA205, 0, 0, 0x3E9, -1};
		HWND hWnd = FindChildWindowIDRecursive(hExplorer, kls, ids);
		if (hWnd) {
			CString strUrl;
			GetWindowText(hWnd, strUrl.GetBuffer(1000), 1000);
			strUrl.ReleaseBuffer();
			strUrl = strUrl.Mid(6);
			if (::PathIsDirectory(strUrl)) {
				// �t�@�C���_�C���O�ɐݒ�
				char temp[MAX_PATH] = "";
				::strcpy(temp, ATL::CW2A(strUrl));
				COPYDATASTRUCT cds = { 0 };
				cds.dwData = kExOpenDir;
				cds.cbData = ::strlen(temp) + 1;
				cds.lpData = static_cast<LPVOID>(temp);
				::SendMessage(hFileDlg, WM_COPYDATA, NULL, (LPARAM)&cds);
				
				//SetDlgCurDir( hFileDlg, strUrl );
				SetForegroundWindow( hFileDlg );
			}
		}
	}
}
