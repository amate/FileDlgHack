// FileDlgHack.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "FileDlgHack.h"
#include <tchar.h>
#include <string>
#include <atlbase.h>
#include <atlconv.h>
#include <boost\optional.hpp>
#include <boost\algorithm\string.hpp>

// Set/GetPropで使用
#define PROP_PROCEDURE "PROP_PROCEDURE" // サブクラス化する前のプロシージャ
#define PROP_CANCEL    "PROP_CANCEL"	// キャンセルボタンを押したかどうか

#define TB_BUTTONID    0x54				// ツールバーに追加するボタンのID

static const int WM_FILEDIALOGEXDETECTMESSAGE = ::RegisterWindowMessage(_T("FileDialogDetectMessage"));

enum { kExOpenDir = 200 };

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL StrReplace( LPSTR str, LPCSTR szFrom, LPCSTR szTo, int size );

void ExplorerPathToFileDlg();

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);


#pragma data_seg(".SHARED_DATA")
HHOOK g_hHook = NULL;					// フックプロシージャのハンドル
#pragma data_seg()

HINSTANCE   g_Inst		= NULL;			// インスタンスハンドル
HANDLE      g_hShared	= NULL;
SHAREDDATA *g_Shared	= NULL;			// 共有メモリ


HHOOK g_LLMouseHook = NULL;
HWND  g_wndToolbar = NULL;
int	  g_nowPopupIndex = -1;

// --------------------------------------------------------

// str中のszFromをszToで置き換える
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

// IDとクラス名から子ウィンドウを探す
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

// IDとクラス名の配列から子ウィンドウを探す
// 本当はszClass,IDをまとめたstructを作って、そのvectorを渡したいんだけど、
// 何となくvector使うの自重
HWND FindChildWindowIDRecursive(HWND hParent, LPSTR * szClass, const int * ID){
	HWND hWnd = NULL;
	while ( 1 ){
		hWnd = FindWindowExA( hParent, hWnd, *szClass, NULL);

		if(hWnd == NULL){
			return NULL;		// 子ウィンドウが見つからなかった
		}
		LONG_PTR tID = GetWindowLongX( hWnd, GWL_ID);
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


// IFileDialogアドレスバーのコンボボックス
HWND GetAddressBarTxt( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW", "ReBarWindow32", "Address Band Root", "msctls_progress32", "ComboBoxEx32", NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0xA205, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg,kls,ids);
	TRACE("GetAddressBarTxt : %d\n", (int)hWnd);
	return hWnd;
}


// IFileDialogアドレスバーのボタン
HWND GetAddressBarBtn( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW", "ReBarWindow32", "Address Band Root", "msctls_progress32", "ToolbarWindow32", NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg, kls, ids);
	TRACE("GetAddressBarBtn : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialogのアドレスバーがBreadcumb状態(?)にあるときにComboBoxの代わりに表示されるToolBar
HWND GetAddressBarBread( HWND hDlg )
{
	LPSTR kls[] = {"WorkerW","ReBarWindow32","Address Band Root","msctls_progress32","Breadcrumb Parent","ToolbarWindow32",NULL};
	const int ids[] = {0, 0xA005, 0xA205, 0, 0, 0x3E9, -1};
	HWND hWnd = FindChildWindowIDRecursive(hDlg,kls,ids);
	TRACE("GetAddressBarBread : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialogのファイル名エディットボックス
HWND GetIFileDialogFileEditBox(HWND hDlg)
{
	LPSTR kls[] = { "ComboBoxEx32", "ComboBox", "Edit", NULL };
	const int ids[] = { 0x47C, 0x47C, 0x47C, -1 };
	HWND hWnd = FindChildWindowIDRecursive(hDlg, kls, ids);
	TRACE("GetIFileDialogFileEditBox : %d\n", (int)hWnd);
	return hWnd;
}

// IFileDialogの保存ファイル名エディットボックス
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
		// 従来のダイアログ？
		hName = FindChildWindowID( hDlg, "Edit", 0x480 );
	} else {
		// Win7用
		HWND hCombo = FindChildWindowID( hName, "ComboBox", 0x47C );
		if (hCombo != NULL) {
			hName = FindChildWindowID( hCombo, "Edit", 0x47C );
		}
	}
	TRACE("GetNameEdit : %d\n", (int)hName);
	return hName;
}

/// 現在エクスプローラーで表示中のフォルダのアイテムＩＤリストを作成する
PIDLIST_ABSOLUTE GetCurIDList(IShellBrowser* pShellBrowser)
{
	PIDLIST_ABSOLUTE	pidl;
	HRESULT	hr;
	CComPtr<IShellView>	pShellView;
	CComQIPtr<IFolderView>	pFolderView;
	CComQIPtr<IPersistFolder2>	pPersistFolder2;
	hr = pShellBrowser->QueryActiveShellView(&pShellView);
	pFolderView = pShellView;
	if (hr == S_OK && pFolderView) {
		hr = pFolderView->GetFolder(IID_IPersistFolder2, (void**)&pPersistFolder2);
		if (hr == S_OK && pPersistFolder2) {
			hr = pPersistFolder2->GetCurFolder(&pidl);
			if (hr == S_OK) {
				return pidl;
			}
		}
	}

	return NULL;
}

/// アイテムＩＤリストからフルパスを返す
std::wstring	GetFullPathFromIDList(PCIDLIST_ABSOLUTE pidl)
{
	PWSTR	strFullPath;
	std::wstring FullPath;
	if (::SHGetNameFromIDList(pidl, SIGDN_DESKTOPABSOLUTEPARSING, &strFullPath) == S_OK) {
		FullPath = strFullPath;
		::CoTaskMemFree(strFullPath);
	} else {
		ATLASSERT(FALSE);
	}
	return FullPath;
}


/// フルパスからアイテムIDリストを作成する
/// 作成したアイテムIDリストはちゃんと解放すること！
PIDLIST_ABSOLUTE CreateIDListFromFullPath(LPCTSTR strFullPath)
{
	LPITEMIDLIST pidl;
	if (::SHILCreateFromPath(strFullPath, &pidl, NULL) == S_OK) {
		return pidl;
	}
	return NULL;
}


///　エクスプローラーで表示中のフォルダのパスを返す
boost::optional<std::wstring>	GetExplorerFolderPath()
{
	auto funcGetExplorerFolder = []() -> boost::optional<std::wstring> 
	{
		CComPtr<IShellWindows> spShellWindows;
		HRESULT hr = ::CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_ALL, IID_IShellWindows, (void**)&spShellWindows);
		if (spShellWindows) {
			long count = 0;
			spShellWindows->get_Count(&count);
			for (long i = 0; i < count; ++i) {
				CComVariant vIndex(i);
				CComPtr<IDispatch> spDisp;
				hr = spShellWindows->Item(vIndex, &spDisp);
				if (hr == S_OK) {
					CComPtr<IShellBrowser> spShellBrowser;
					hr = IUnknown_QueryService(spDisp, SID_STopLevelBrowser, IID_IShellBrowser, (void**)&spShellBrowser);
					if (SUCCEEDED(hr)) {
						LPITEMIDLIST pidl = GetCurIDList(spShellBrowser);
						if (pidl) {
							std::wstring folderPath = GetFullPathFromIDList(pidl);
							::ILFree(pidl);

							return folderPath;
						}
					}
				}
			}
		}
		return boost::none;
	};
	
	::CoInitialize(NULL);
	auto folderPath = funcGetExplorerFolder();
	::CoUninitialize();

	return folderPath;
}

/// ファイルダイアログの現在のフォルダを取得する
boost::optional<std::wstring>	GetDlgCurrentFolder(HWND hDlgWnd)
{
	IShellBrowser* shellBrowser = (IShellBrowser*)::SendMessage(hDlgWnd, WM_USER + 7, 0, 0);
	if (shellBrowser) {
		PIDLIST_ABSOLUTE pidl = GetCurIDList(shellBrowser);
		if (pidl) {
			std::wstring folderPath = GetFullPathFromIDList(pidl);
			::ILFree(pidl);
			return folderPath;
		}
	}

	// CDM_GETFOLDERPATH だとUNICODE関係で文字化けすることがあるため CDM_GETFOLDERIDLIST で
	int size = (int)SendMessage(hDlgWnd, CDM_GETFOLDERIDLIST, 0, NULL);
	if (size <= 0) {
		ATLASSERT(FALSE);
		return boost::none;
	}

	LPITEMIDLIST lpListBuff = (LPITEMIDLIST)CoTaskMemAlloc(size);
	if (lpListBuff == NULL) {
		ATLASSERT(FALSE);
		return boost::none;
	}

	LRESULT ret = SendMessage(hDlgWnd, CDM_GETFOLDERIDLIST, size, (LPARAM)lpListBuff);
	if (ret <= 0) {
		ATLASSERT(FALSE);
		CoTaskMemFree(lpListBuff);
		return boost::none;
	}

	std::wstring folderPath = GetFullPathFromIDList(lpListBuff);
	return folderPath;
}


/// ファイルダイアログの現在のフォルダを変更する
bool SetDlgCurrentFolder(HWND hDlgWnd, LPCWSTR folderPath)
{
	IShellBrowser* shellBrowser = (IShellBrowser*)::SendMessage(hDlgWnd, WM_USER + 7, 0, 0);
	if (shellBrowser) {
		LPITEMIDLIST pidlFolder = CreateIDListFromFullPath(folderPath);
		if (pidlFolder) {
			if (auto currentFolderPath = GetDlgCurrentFolder(hDlgWnd)) {
				if (::_wcsicmp(currentFolderPath->c_str(), folderPath) == 0) {
					::ILFree(pidlFolder);
					return true;	// 同じフォルダに移動しようとした
				}
			}
			HRESULT hr = shellBrowser->BrowseObject(pidlFolder, SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
			::ILFree(pidlFolder);
			if (SUCCEEDED(hr)) {
				return true;	// 移動に成功！
			}
		}
	}

	ATLASSERT(FALSE);
	return false;
}

// 設定に応じてダイアログの位置を調整
void AdjustWinPos( HWND hWnd )
{
	RECT Wndrc, Listrc, Deskrc;
	POINT Pos = {}, Size = {};
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
	case 0: //位置を指定しない
		flag |= SWP_NOMOVE;
		break;
	case 1: //位置を指定する
		Pos.x = g_Shared->DialogPos.x;
		Pos.y = g_Shared->DialogPos.y;
		break;
	case 2: //中央に
		Pos.x = ( Deskrc.right + Deskrc.left - Size.x ) / 2;
		Pos.y = ( Deskrc.bottom + Deskrc.top - Size.y ) / 2;
		break;
	}


	if( !( flag & SWP_NOSIZE) || !( flag & SWP_NOMOVE ) ) {
		SetWindowPos( hWnd, NULL, Pos.x, Pos.y, Size.x, Size.y, flag );
		TRACE("AdjustWinPos : x : %d  y : %d  幅 : %d  高さ : %d  NOSIZE : %d  NOMOVE : %d\n", Pos.x, Pos.y, Size.x, Size.y, flag & SWP_NOSIZE, flag & SWP_NOMOVE);
	}
}

// EnumChildWindowsで使う
BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam )
{
	char szClass[64];
	GetClassNameA( hwnd, szClass, 64 );

	// 右下の つまみ は無視
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

// 新しいダイアログでツールバーを作成
void CreateExToolBar(HWND hWnd)
{
	// ツールバーを作成
	HWND ToolBar = ::CreateToolbarEx(
		hWnd, //親ウィンドウのハンドル    
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TRANSPARENT | TBSTYLE_LIST |CCS_NODIVIDER,  //ウィンドウスタイル
		ID_TOOLBAR,  //ツールバーのコントロールＩＤ     
		0, //hBMInsやwBMIDで特定されたビットマップにはいっているボタンイメージの数
		NULL, //ビットマップリソースが入っているモジュールのインスタンスハンドル     
		0, //リソースＩＤ    
		NULL,//TBBUTTON構造体の配列のポインタ     
		0, //ツールバーのボタンの数    
		0, //ボタンの幅    
		0, //ボタンの高さ
		0, //ボタンイメージの幅
		0, //ボタンイメージの高さ
		sizeof(TBBUTTON) //TBBUTTON構造体の大きさ
		); 
	::SetWindowLongPtr(ToolBar, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	::SetWindowText(ToolBar, _T("拡張ツールバー"));

	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("ショートカット"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("最近使ったフォルダ"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("ツール"));
	::SendMessage(ToolBar, TB_ADDSTRINGA, 0, (LPARAM)("設定"));

	TBBUTTON tbb[] = {
		{I_IMAGENONE, ID_SHORTCUT	, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 0},
		{I_IMAGENONE, ID_HISTORY	, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 1},
		{I_IMAGENONE, ID_TOOL		, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 2},
		{I_IMAGENONE, ID_SETTEI		, TBSTATE_ENABLED, TBSTYLE_AUTOSIZE, {0}, 0, 3}
	};
	::SendMessage(ToolBar, TB_ADDBUTTONS, 4, (LPARAM)tbb);

	// ツールバーの位置を移動
	HWND DUI = FindChildWindowID(hWnd, "DUIViewWndClassName", 0);
	HWND WorkerW = FindChildWindowID(hWnd, "WorkerW", 0);
	RECT rcDUI, rcToolBar, rcWorker;
	::GetClientRect(DUI, &rcDUI);
	::GetClientRect(ToolBar, &rcToolBar);
	::GetClientRect(WorkerW, &rcWorker);
	// 下に移動
	::SetWindowPos(ToolBar, HWND_TOP, 0, rcWorker.bottom, 0, 0, SWP_NOSIZE);
	::SetWindowPos(DUI, NULL, 0, rcWorker.bottom + rcToolBar.bottom, rcDUI.right, rcDUI.bottom - rcToolBar.bottom, SWP_NOZORDER);
}

// ファイルダイアログの初期化
BOOL InitFileDialog( HWND hWnd )
{
	// サブクラス化
	WNDPROC DefWndProc = (WNDPROC)SetWindowLongX( hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc );
	SetPropA( hWnd, PROP_PROCEDURE, DefWndProc );

	if (GetAddressBarBread(hWnd)) {
		// 新しいファイルダイアログの場合
		CreateExToolBar(hWnd);

		ATLASSERT(g_LLMouseHook == NULL);
		g_LLMouseHook = ::SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, g_Inst, 0);
		ATLASSERT(g_LLMouseHook);

	} else {
		// 従来のダイアログの場合

		if( !g_Shared->bToolbar ) {
			// ツールバーに追加はしない

			// メニューをセット
			SetMenu(hWnd, LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU1 ) ) );

			// コントロールがはみ出さないように調整
			if( GetWindowLongX( hWnd, GWL_STYLE ) & WS_THICKFRAME ) {

			} else {
				RECT rc;
				GetWindowRect( hWnd, &rc );
				SetWindowPos( hWnd, NULL, 0, 0,
					rc.right - rc.left, rc.bottom - rc.top + GetSystemMetrics( SM_CYMENU ),
					SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );
			}

		} else {
			// ツールバーにボタンを追加
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
			ti.lpszText = _T("ファイルダイアログ拡張(ALT+M)");
			SendMessage( hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti );

			// ツールバーのサイズ調整
			SendMessage( hToolbar, TB_GETITEMRECT, btncnt - 1, (LPARAM)&rc );
			SetWindowPos( hToolbar, 0, 0, 0,
				rc.right, rc.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER );
		}
	}

	// 位置、サイズ調整
	AdjustWinPos( hWnd );

	// 表示を設定
	HWND hDefView = GetDefView( hWnd );
	if( g_Shared->ListStyle )
		SendMessage( hDefView, WM_COMMAND, g_Shared->ListStyle, 0 );
	// ソートする項目を設定
	if( g_Shared->ListSort )
		SendMessage( hDefView, WM_COMMAND, g_Shared->ListSort, 0 );

	g_Shared->cDialog++;

	if (g_Shared->bExplorerFolderOpenOnActive) {
		if (auto folderPath = GetExplorerFolderPath()) {
			SetDlgCurrentFolder(hWnd, folderPath->c_str());
		}
	}
	return TRUE;
}

// 現在開いているフォルダをショートカットに追加する
BOOL AppendShortcut( LPCTSTR pszPath, HWND hWnd )
{
	TCHAR szName[80];
	PathCompactPathEx( szName, pszPath, 64, 0 );
	if( g_Shared->shortcut.count < SHORTCUT_MAX )
	{
		lstrcpy( g_Shared->shortcut.data[g_Shared->shortcut.count].szName, szName );
		lstrcpy( g_Shared->shortcut.data[g_Shared->shortcut.count].szPath, pszPath );
		g_Shared->shortcut.count++;

		// iniにも書き込む
		TCHAR szKey[32];
		wsprintf( szKey, L"Name%d", g_Shared->shortcut.count - 1 );
		WritePrivateProfileString( L"Shortcut", szKey, szName, g_Shared->szIniPath );
		wsprintf( szKey, L"Path%d", g_Shared->shortcut.count - 1 );
		WritePrivateProfileString( L"Shortcut", szKey, pszPath, g_Shared->szIniPath );
	}
	else
	{
		wsprintf( szName, L"ショートカットは%d個までしか作成できません" , SHORTCUT_MAX );
		MessageBox( hWnd, szName, nullptr, 0 );
		return FALSE;
	}
	return TRUE;
}

// 履歴に追加
BOOL AppendHistory( LPCTSTR pszPath )
{
	// データを一つ後ろにずらす
	int i;
	for( i = 0; i < g_Shared->history.count; i++ )
		if( lstrcmp( g_Shared->history.data[i].szPath, pszPath ) == 0 )
			break;
	if( i == g_Shared->history.count && g_Shared->history.count < g_Shared->history.max )
		g_Shared->history.count++;
	if( i == g_Shared->history.max )
		i--;
	for( ; i > 0; i-- )
		lstrcpy( g_Shared->history.data[i].szPath, g_Shared->history.data[i - 1].szPath );

	// 履歴の先頭にに追加
	lstrcpy( g_Shared->history.data[0].szPath, pszPath );

	return TRUE;
}

// iniファイルからショートカットを読み込む
BOOL LoadShortcut( LPCTSTR szIni )
{
	TCHAR szKey[32];
	for( int i = 0; i < SHORTCUT_MAX; i++ )
	{
		wsprintf( szKey, L"Name%d", i );
		GetPrivateProfileString( L"Shortcut", szKey, L"none", g_Shared->shortcut.data[i].szName, MAX_PATH, szIni );
		if( lstrcmp( g_Shared->shortcut.data[i].szName, L"none" ) == 0 )
			break;
		wsprintf( szKey, L"Path%d", i );
		GetPrivateProfileString( L"Shortcut", szKey, L"", g_Shared->shortcut.data[i].szPath, MAX_PATH, szIni );
		g_Shared->shortcut.count++;
	}

	return TRUE;
}

// ini ファイルから履歴を読み込む
BOOL LoadHistory( LPCTSTR szIni )
{
	TCHAR szKey[32];
	for( int i = 0; i < HISTORY_MAX; i++ )
	{
		wsprintf( szKey, L"hist%d", i );
		GetPrivateProfileString( L"History", szKey, L"none", g_Shared->history.data[i].szPath, MAX_PATH, szIni );
		if( lstrcmp( g_Shared->history.data[i].szPath, L"none" ) == 0 )
			break;
		g_Shared->history.count++;
	}

	return TRUE;
}

// iniファイルからツールを読み込む
BOOL LoadTool( LPCTSTR szIni )
{
	TCHAR szKey[32];
	for( int i = 0; i < TOOL_MAX; i++ )
	{
		wsprintf( szKey, L"Name%d", i );
		GetPrivateProfileString( L"Tool", szKey, L"none", g_Shared->tool.data[i].szName, MAX_PATH, szIni );
		if( lstrcmp( g_Shared->tool.data[i].szName, L"none" ) == 0 )
			break;
		wsprintf( szKey, L"Path%d", i );
		GetPrivateProfileString( L"Tool", szKey, L"", g_Shared->tool.data[i].szPath, MAX_PATH, szIni );
		wsprintf( szKey, L"Param%d", i );
		GetPrivateProfileString( L"Tool", szKey, L"", g_Shared->tool.data[i].szParam, MAX_PATH, szIni );
		g_Shared->tool.count++;
	}

	return TRUE;
}

// ini ファイルに履歴を保存
BOOL SaveHistory( LPCTSTR szIni )
{
	WritePrivateProfileSection( L"History", L"\0\0", szIni );
	TCHAR szKey[32];
	for( int i = 0; i < g_Shared->history.count; i++ )
	{
		wsprintf( szKey, L"hist%d", i );
		WritePrivateProfileString( L"History", szKey, g_Shared->history.data[i].szPath, szIni );
	}

	return TRUE;
}

// メニューの項目を初期化

BOOL InitMenu( HMENU hMenu )
{
	HMENU hShortcutMenu = GetSubMenu( hMenu, 0 );
	HMENU hHistoryMenu = GetSubMenu( hMenu, 1 );
	HMENU hToolMenu = GetSubMenu( hMenu, 2 );
	UINT uID;

	// ショートカットのメニューを作る
	while( uID = GetMenuItemID( hShortcutMenu, 0 ),
		(ID_SHORTCUT_FIRST <= uID && uID < ID_SHORTCUT_FIRST + SHORTCUT_MAX) )
		DeleteMenu( hShortcutMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->shortcut.count; i++ )
		InsertMenu( hShortcutMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
		ID_SHORTCUT_FIRST + i, g_Shared->shortcut.data[i].szName );

	// 履歴のメニューを作る
	while( uID = GetMenuItemID( hHistoryMenu, 0 ),
		( ID_HISTORY_FIRST <= uID && uID < ID_HISTORY_FIRST + HISTORY_MAX ) )
		DeleteMenu( hHistoryMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->history.count; i++ )
	{
		TCHAR szBuff[MAX_PATH];
		PathCompactPathEx( szBuff, g_Shared->history.data[i].szPath, 64, 0);
		InsertMenu( hHistoryMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
			ID_HISTORY_FIRST + i, szBuff );
	}

	// ツールのメニューを作る
	while( uID = GetMenuItemID( hToolMenu, 0 ),
		(ID_TOOL_FIRST <= uID && uID < ID_TOOL_FIRST + TOOL_MAX) )
		DeleteMenu( hToolMenu, 0, MF_BYPOSITION );

	for( int i = 0; i < g_Shared->tool.count; i++ )
		InsertMenu( hToolMenu, i, MF_BYPOSITION | MF_STRING | MF_ENABLED,
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

LRESULT CALLBACK LowLevelMouseProc(
  int nCode,     // フックコード
  WPARAM wParam, // メッセージ識別子
  LPARAM lParam  // メッセージデータ
)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_MOUSEMOVE) {
			if (g_nowPopupIndex != -1) {
				LPMSLLHOOKSTRUCT pmsllhk = (LPMSLLHOOKSTRUCT)lParam;
				HWND hWndpt = ::WindowFromPoint(pmsllhk->pt);
				if (hWndpt == g_wndToolbar) {
					POINT pt = pmsllhk->pt;
					::ScreenToClient(g_wndToolbar, &pt);
					int nIndex = (int)::SendMessage(g_wndToolbar, TB_HITTEST, 0, (LPARAM)&pt);
					if (0 <= nIndex && nIndex < 3 && g_nowPopupIndex != nIndex) {
						TRACE("表示するメニューを切り替えます : %d\n", nIndex);

						HWND hMenuWnd = FindWindow(L"#32768", NULL);
						if (hMenuWnd) {
							::SendMessage(hMenuWnd, WM_CLOSE, 0, 0);
						}

						HWND hDlgWnd = ::GetParent(g_wndToolbar);						
						g_nowPopupIndex = nIndex;
						::PostMessage(hDlgWnd, WM_COMMAND, ID_SHORTCUT + nIndex, 1);
					}
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

	TRACE("PopupMenu index : %d\n", nIndex);

	g_nowPopupIndex = nIndex;
	TPMPARAMS tpm = { sizeof(TPMPARAMS) };
	SendMessage( hToolbar, TB_GETRECT, (WPARAM)iID, (LPARAM)&tpm.rcExclude);
	MapWindowPoints( hToolbar, HWND_DESKTOP, (LPPOINT)&tpm.rcExclude, 2 );

	HMENU hMenu = LoadMenu( g_Inst, MAKEINTRESOURCE( IDR_MENU2 ) );
	HMENU hSubMenu = GetSubMenu( hMenu, 0 );
	InitMenu( hSubMenu );
	HMENU hSubSubMenu = GetSubMenu( hSubMenu, nIndex );

	g_wndToolbar = hToolbar;

	::SendMessage(hToolbar, TB_PRESSBUTTON, iID, MAKELPARAM(1, 0));
	BOOL ret = TrackPopupMenuEx( hSubSubMenu, TPM_RETURNCMD, tpm.rcExclude.left, tpm.rcExclude.bottom, hWnd, &tpm );
	::SendMessage(hToolbar, TB_PRESSBUTTON, iID, MAKELPARAM(0, 0));

	g_wndToolbar = NULL;

	DestroyMenu( hMenu );
	if (ret != 0) {
		::SendMessage(hWnd, WM_COMMAND, ret, 0);
	}
	g_nowPopupIndex = -1;

	TRACE("PopupMenu End\n");
	return ret;
}


// --------------------------------------------------------
//    サブクラス化用のプロシージャ
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
				std::wstring path = (LPWSTR)CA2W(static_cast<LPCSTR>(pcds->lpData));
				SetDlgCurrentFolder(hWnd, path.c_str());
				return 1;
			}
			break;
		}

	case WM_ACTIVATE:
		{
			BOOL bActive = wParam & 0xFFFF;
			if (bActive && g_Shared->bExplorerFolderOpenOnActive) {
				if (auto folderPath = GetExplorerFolderPath()) {
					SetDlgCurrentFolder(hWnd, folderPath->c_str());
				}
			}
#if 0
			if (bActive && bFirstOpen && !(::GetAsyncKeyState(VK_CONTROL) < 0))
				ExplorerPathToFileDlg();
			bFirstOpen = false;
#endif
		}
		break;

	case WM_COMMAND:
		if( lParam == 0 ) // メニューから
		{
			WORD wID = LOWORD( wParam );
			switch( wID )
			{
			case ID_APPEND:
				{
					if(auto path = GetDlgCurrentFolder(hWnd) )
						AppendShortcut(path->c_str(), hWnd );
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
					SetDlgCurrentFolder( hWnd, g_Shared->shortcut.data[ wID - ID_SHORTCUT_FIRST ].szPath );
					return 0;
				}
				else if( ID_HISTORY_FIRST <= wID && wID < ID_HISTORY_FIRST + HISTORY_MAX )
				{
					SetDlgCurrentFolder( hWnd, g_Shared->history.data[ wID - ID_HISTORY_FIRST ].szPath );
					return 0;
				}
				else if( ID_TOOL_FIRST <= wID && wID < ID_TOOL_FIRST + TOOL_MAX )
				{
					std::wstring path = g_Shared->tool.data[wID - ID_TOOL_FIRST].szPath;
					if (path.find(L"%explorer_open%") != std::wstring::npos) {
						if (auto folderPath = GetExplorerFolderPath()) {
							SetDlgCurrentFolder(hWnd, folderPath->c_str());
						}
						return 0;
					}

					std::wstring szParam = g_Shared->tool.data[wID - ID_TOOL_FIRST].szParam;
					if( StrStr( szParam.c_str(), L"%1" ) != 0 )
					{ // "%1"を含んでいたら現在のフォルダのパスで置換する。
						if (auto path = GetDlgCurrentFolder(hWnd)) {
							boost::replace_all(szParam, L"%1", *path);
						} else {
							return 0;
						}
					}

					if( (int)ShellExecute( NULL, NULL, g_Shared->tool.data[wID - ID_TOOL_FIRST].szPath,
						szParam.c_str(), NULL, SW_SHOWDEFAULT ) <= 32 )
						MessageBoxA( hWnd, "実行できません。パスを確認してください。", "エラー", 0 );
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
					HWND ToolBar = FindWindowExA(hWnd, NULL, "ToolbarWindow32", "拡張ツールバー");
					PopupMenu(hWnd, ToolBar, wID);
					break;
				}
			case ID_SETTEI:
				SettingDialog( hWnd );
				return 0;
			case IDCANCEL: // キャンセルボタン
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
						// ツールバーのボタンがクリックされた
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
			// メニューを表示
			static int i = 0;
			if( i == 0 ) // メニューが2重に表示されないように
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
			HWND ToolBar = FindWindowExA(hWnd, NULL, "ToolbarWindow32", "拡張ツールバー");
			if (ToolBar != NULL) {
				// 新しいファイルダイアログの場合
				// ツールバーを表示するためにビューを下にずらす
				::SendMessage(hWnd, WM_SETREDRAW, (WPARAM)FALSE, 0);
				CallWindowProcX( DefProc, hWnd, message, wParam, lParam );

				HWND DUI = FindChildWindowID(hWnd, "DUIViewWndClassName", 0);
				HWND WorkerW = FindChildWindowID(hWnd, "WorkerW", 0);
				RECT rcDUI, rcToolBar, rcWorker;
				::GetClientRect(DUI, &rcDUI);
				::GetClientRect(ToolBar, &rcToolBar);
				::GetClientRect(WorkerW, &rcWorker);
				// 下に移動
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

		// 履歴に追加
		if( !GetPropA( hWnd, PROP_CANCEL) ) {
			// キャンセルで終了ではないので履歴を保存
			if(auto path = GetDlgCurrentFolder(hWnd) ) {
				AppendHistory(path->c_str());
				SaveHistory( g_Shared->szIniPath );
			}
		}

		// メニューを削除
		HMENU hMenu = GetMenu( hWnd );
		if( hMenu ) {
			SetMenu( hWnd, NULL );
			DestroyMenu( hMenu );
			//ウィンドウサイズを戻す
			//RECT rc;
			//GetWindowRect( hWnd, &rc );
			//SetWindowPos( hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top - GetSystemMetrics( SM_CYMENU ),
			// SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOZORDER );
		}

		//サブクラス化を解除
		SetWindowLongX( hWnd, GWLP_WNDPROC, (LONG_PTR)DefProc );

		if (g_LLMouseHook) {
			::UnhookWindowsHookEx(g_LLMouseHook);
			g_LLMouseHook = NULL;
		}

		g_Shared->cDialog--;

		RemovePropA( hWnd, PROP_PROCEDURE );
		RemovePropA( hWnd, PROP_CANCEL );

		break;
	}

	return CallWindowProcX( DefProc, hWnd, message, wParam, lParam );
}

BOOL IsFileDialog( HWND hWnd )
{
	TRACE("IsFileDialogを開始--------------\n");
	if( (((GetAddressBarBread(hWnd) || GetAddressBarTxt(hWnd)) && GetAddressBarBtn(hWnd) != NULL)
		|| (GetDefView( hWnd ) != NULL && GetNameEdit( hWnd ) != NULL)) ){
			TRACE("  ファイルダイアログでした。フックを開始します\n");
			return TRUE;
	} else {
		TRACE("  ファイルダイアログではなかった\n");
		return FALSE;
	}
}

// --------------------------------------------------------
//    フックプロシージャ
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
//    プラグインがロードされたときに呼ばれる
// --------------------------------------------------------

BOOL Init(void)
{
	int len = GetModuleFileName( g_Inst, g_Shared->szIniPath, MAX_PATH);

	g_Shared->szIniPath[len - 3] = L'i';
	g_Shared->szIniPath[len - 2] = L'n';
	g_Shared->szIniPath[len - 1] = L'i';

	TRACE("open");	// logファイル作成

	g_Shared->nDialogPos    = GetPrivateProfileInt(L"Setting", L"Pos", FALSE, g_Shared->szIniPath);
	g_Shared->DialogPos.x   = GetPrivateProfileInt(L"Setting", L"PosX", 100, g_Shared->szIniPath);
	g_Shared->DialogPos.y   = GetPrivateProfileInt(L"Setting", L"PosY", 100, g_Shared->szIniPath);
	g_Shared->bListSize     = GetPrivateProfileInt(L"Setting", L"Size", FALSE, g_Shared->szIniPath);
	g_Shared->ListSize.x    = GetPrivateProfileInt(L"Setting", L"SizeX", 640, g_Shared->szIniPath);
	g_Shared->ListSize.y    = GetPrivateProfileInt(L"Setting", L"SizeY", 480, g_Shared->szIniPath);
	g_Shared->ListStyle     = GetPrivateProfileInt(L"Setting", L"ListStyle", 0, g_Shared->szIniPath);
	g_Shared->ListSort      = GetPrivateProfileInt(L"Setting", L"ListSort", 0, g_Shared->szIniPath);
	g_Shared->bToolbar      = GetPrivateProfileInt(L"Setting", L"Toolbar", 0, g_Shared->szIniPath);
	g_Shared->bExplorerFolderOpenOnActive = GetPrivateProfileInt(L"Setting", L"ExplorerFolderOpenOnActive", 0, g_Shared->szIniPath);

	// 履歴
	g_Shared->history.count = 0;
	g_Shared->history.max = GetPrivateProfileInt( L"Setting", L"MaxHistory", 5, g_Shared->szIniPath);
	LoadHistory( g_Shared->szIniPath );

	// ショートカット
	g_Shared->shortcut.count = 0;
	LoadShortcut( g_Shared->szIniPath );

	// ツール
	g_Shared->tool.count = 0;
	LoadTool( g_Shared->szIniPath );

	g_hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_Inst, 0);
	if(!g_hHook) {
		TRACE(" SetWindowsHookExに失敗(%#x), inst(%#x)\n", GetLastError(), g_Inst);		
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

	::PostMessageA(HWND_BROADCAST, WM_NULL, 0, 0);	// すべてのウィンドウにフックを仕掛ける

	return TRUE;
}

// --------------------------------------------------------
//    プラグインがアンロードされたときに呼ばれる
// --------------------------------------------------------
void Unload(void)
{
	while (g_Shared->cDialog) {
		TCHAR buf[256] = _T("");
		wsprintf(buf, _T("TTBaseを終了する前にすべてのファイルダイアログを閉じてください\n\"いいえ\"で強制終了(表示ダイアログ数: %d)"), g_Shared->cDialog);
		int ret = MessageBox(NULL, buf,
			L"ファイルダイアログ拡張7", MB_YESNO | MB_ICONEXCLAMATION);
		if (ret == IDNO)
			g_Shared->cDialog = 0;
	}

	UnhookWindowsHookEx( g_hHook );
	g_hHook = NULL;

	// UnHookしたのでdllのunloadを促す
	::PostMessageA(HWND_BROADCAST, WM_NULL, 0, 0);
}

// --------------------------------------------------------
//    コマンド実行時に呼ばれる
// --------------------------------------------------------
BOOL Execute(int CmdId, HWND hWnd)
{
	return TRUE;
}

// --------------------------------------------------------
//    グローバルフックメッセージがやってくる
// --------------------------------------------------------
void Hook(UINT Msg, DWORD wParam, DWORD lParam)
{
}
