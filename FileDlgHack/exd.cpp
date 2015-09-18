
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

// IDとクラス名の配列から子ウィンドウを探す
// 本当はszClass,IDをまとめたstructを作って、そのvectorを渡したいんだけど、
// 何となくvector使うの自重
HWND FindChildWindowIDRecursive(HWND hParent, LPTSTR * szClass, const int * ID){
	HWND hWnd = NULL;
	while ( 1 ){
		hWnd = FindWindowEx(hParent, hWnd, *szClass, NULL);

		if(hWnd == NULL){
			return NULL;		// 子ウィンドウが見つからなかった
		}
		int tID = GetWindowLongX( hWnd, GWL_ID);
		if( tID == *ID){
			//		TRACE(" IDがマッチした\n");
			if(szClass + 1 == NULL || *(ID+1) == -1){
				return hWnd;	// 最後まで行ったので終わり
			}else{
				HWND hWndChild = FindChildWindowIDRecursive(hWnd, szClass + 1, ID + 1);
				if(hWndChild != NULL){
					return hWndChild;
				}
			}
		}
		//	TRACE(" IDがマッチしなかった\n");

	}
	return NULL;
}

// IFileDialogアドレスバーのコンボボックス
HWND GetAddressBarTxt(HWND hDlg);

// IFileDialogアドレスバーのボタン
HWND GetAddressBarBtn(HWND hDlg);

// IFileDialogのアドレスバーがBreadcumb状態(?)にあるときにComboBoxの代わりに表示されるToolBar
HWND GetAddressBarBread(HWND hDlg);

// ファイルダイアログの現在のフォルダを変更する
BOOL SetDlgCurDir( HWND hDlg, LPCTSTR lpdir )
{
	HWND hAddrT = GetAddressBarTxt( hDlg );
	HWND hAddrB = GetAddressBarBtn( hDlg );
	HWND hAddrBrd = GetAddressBarBread( hDlg );
	if( hAddrT != NULL && hAddrB != NULL ) {
		// IFileDialog
		if(hAddrBrd != NULL){
			// Breadcrumb状態を解除
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
		// Breadcrumb状態を解除
		SendMessage(hAddrBrd,WM_SETFOCUS,0,0);
		SendMessage(hAddrBrd,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrBrd,WM_KEYUP,VK_RETURN,0);

		HWND hAddrT = GetAddressBarTxt( hDlg );
		if (hAddrT == NULL) {
			ATLTRACE("SetDlgCurDir に失敗？\n");
		}
		SendMessage(hAddrT,WM_SETTEXT,0,(LPARAM)lpdir);

		SendMessage(hAddrB,WM_SETFOCUS,0,0);
		SendMessage(hAddrB,WM_KEYDOWN,VK_RETURN,0);
		SendMessage(hAddrB,WM_KEYUP,VK_RETURN,0);
		return TRUE;
	}
	return FALSE;
}

// エクスプローラーを見つける
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

// ファイルダイアログを見つける
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
	// エクスプローラーを見つける
	EnumWindows( EnumWindowsProcExp, (LPARAM)&hExplorer );
	// ファイルダイアログを見つける
	EnumWindows( EnumWindowsProcDlg, (LPARAM)&hFileDlg );

	if ( hExplorer && hFileDlg ) {
		// エクスプローラーから現在表示中のフォルダのパスを得る
		LPTSTR kls[] = {_T("WorkerW"), _T("ReBarWindow32"), _T("Address Band Root"), _T("msctls_progress32"), _T("Breadcrumb Parent"), _T("ToolbarWindow32"), NULL};
		const int ids[] = {0, 0xA005, 0xA205, 0, 0, 0x3E9, -1};
		HWND hWnd = FindChildWindowIDRecursive(hExplorer, kls, ids);
		if (hWnd) {
			CString strUrl;
			GetWindowText(hWnd, strUrl.GetBuffer(1000), 1000);
			strUrl.ReleaseBuffer();
			strUrl = strUrl.Mid(6);
			if (::PathIsDirectory(strUrl)) {
				// ファイルダイログに設定
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
