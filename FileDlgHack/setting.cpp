// setting.cpp

#include "stdafx.h"
#include <tchar.h>
#include <atlbase.h>
#include <atlconv.h>
#include "FileDlgHack.h"

struct DefviewCmd {
	TCHAR *szText; // 表示されるテキスト
	int param;    // WM_COMMANDのwParam
};

struct DefviewCmds {
	struct DefviewCmd style[7];
	struct DefviewCmd sort[6];
} ;

const DefviewCmds DefviewCmds[] = 
{
	{
		{//win98 わからないのでとりあえず2kと同じに
			{ _T("指定しない"),     0x0000 },
			{ _T("大きいアイコン"), 0x7029 },
			{ _T("小さいアイコン"), 0x702A },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前順"),         0x7230 },
			{ _T("種類順"),         0x7232 },
			{ _T("サイズ順"),       0x7231 },
			{ _T("日付順"),         0x7233 },
			{ NULL,             0x0000 }
		}
	},
	{//winME
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("大きいアイコン"), 0x7029 },
			{ _T("小さいアイコン"), 0x702A },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ _T("縮小版"),         0x7031 },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前順"),         0x7230 },
			{ _T("種類順"),         0x7232 },
			{ _T("サイズ順"),       0x7231 },
			{ _T("日付順"),         0x7233 },
			{ NULL,             0x0000 }
		}
	},
	{//win2k
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("大きいアイコン"), 0x7029 },
			{ _T("小さいアイコン"), 0x702A },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ _T("縮小版"),         0x7031 },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前順"),         0x7230 },
			{ _T("種類順"),         0x7232 },
			{ _T("サイズ順"),       0x7231 },
			{ _T("日付順"),         0x7233 },
			{ NULL,             0x0000 }
		}
	},
	{//winXP
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("縮小版"),         0x702D },
			{ _T("並べて表示"),     0x702E },
			{ _T("アイコン"),       0x7029 },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前"),           0x7602 },
			{ _T("サイズ"),         0x7603 },
			{ _T("種類"),           0x7604 },
			{ _T("更新日時"),       0x7605 },
			{ NULL,             0x0000 }
		}
	},
	{// winVt
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("縮小版"),         0x702D },
			{ _T("並べて表示"),     0x702E },
			{ _T("アイコン"),       0x7029 },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前"),           0x7602 },
			{ _T("サイズ"),         0x7603 },
			{ _T("種類"),           0x7604 },
			{ _T("更新日時"),       0x7605 },
			{ NULL,             0x0000 }
		}
	},
	{// win7
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("中アイコン"),		0x702D },
			{ _T("一覧"),           0x702B },
			{ _T("詳細"),           0x702C },
			{ _T("並べて表示"),     0x702E },
			{ _T("コンテンツ"),     0x7030 },
			{ NULL,             0x0000 }
		},
		{
			{ _T("指定しない"),     0x0000 },
			{ _T("名前"),           0x7602 },
			{ _T("サイズ"),         0x7603 },
			{ _T("種類"),           0x7604 },
			{ _T("更新日時"),       0x7605 },
			{ NULL,             0x0000 }
		}
	}
};

extern SHAREDDATA *g_Shared;
extern HINSTANCE  g_Inst;


BOOL WritePrivateProfileInt( LPCTSTR lpAppName, LPCTSTR lpKeyName, int value, LPCTSTR lpFileName )
{
	TCHAR szbuff[32];
	wsprintf( szbuff, L"%d", value );
	return WritePrivateProfileString( lpAppName, lpKeyName, szbuff, lpFileName );
}

// windowsのバージョンを返す
int WinVersion()
{
	OSVERSIONINFO ov = { sizeof( OSVERSIONINFO ) };
	GetVersionEx( &ov );

	switch( ov.dwPlatformId )
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		if( ov.dwMinorVersion <= 10 )
			return WIN98;
		else
			return WINME;
	case VER_PLATFORM_WIN32_NT:
		if( ov.dwMajorVersion <= 4 ||
			ov.dwMajorVersion == 5 && ov.dwMinorVersion == 0 ) {
			return WIN2K;
		} else if( ov.dwMajorVersion == 5 && ov.dwMinorVersion >= 1 ) {
			return WINXP;
		} else if( ov.dwMajorVersion == 6 && ov.dwMinorVersion == 0 ) {
			return WINVT;
		} else if( ov.dwMajorVersion == 6 && ov.dwMinorVersion == 1 ) {
			return WIN7;
		}
	}	
	return WINXP;
}

////////////////////////////////////////////////////////////////////
//                             全般
////////////////////////////////////////////////////////////////////
BOOL CALLBACK PageProcGENERAL( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			//履歴数
			SetDlgItemInt( hWnd, IDC_EDIT2 , g_Shared->history.max, FALSE );
			SendDlgItemMessage( hWnd, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG( 100, 0 ) );

			//ツールバーにボタンを追加する
			if( g_Shared->bToolbar )
				SendDlgItemMessage( hWnd, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0);


			//ダイアログの位置
			SetDlgItemInt( hWnd, IDC_EDIT3 , g_Shared->DialogPos.x, FALSE );
			SetDlgItemInt( hWnd, IDC_EDIT4 , g_Shared->DialogPos.y, FALSE );
			SendDlgItemMessage( hWnd, IDC_SPIN4, UDM_SETRANGE, 0, MAKELONG( UD_MAXVAL / 2, UD_MINVAL / 2 ) );
			SendDlgItemMessage( hWnd, IDC_SPIN5, UDM_SETRANGE, 0, MAKELONG( UD_MAXVAL / 2, UD_MINVAL / 2 ) );

			switch( g_Shared->nDialogPos )
			{
			case 0: //指定しない
				CheckRadioButton( hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO3 );
				break;
			case 1: //位置を指定する
				CheckRadioButton( hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1 );
				break;
			case 2: //画面の中央
				CheckRadioButton( hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO2 );
				break;
			}

			if( g_Shared->nDialogPos == 0 || g_Shared->nDialogPos == 2 )
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT3 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT4 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN4 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN5 ), FALSE );
			}

			//リストビューの位置
			SetDlgItemInt( hWnd, IDC_EDIT5, g_Shared->ListSize.x, FALSE );
			SetDlgItemInt( hWnd, IDC_EDIT6, g_Shared->ListSize.y, FALSE );
			SendDlgItemMessage( hWnd, IDC_SPIN6, UDM_SETRANGE, 0, MAKELONG( UD_MAXVAL / 2, UD_MINVAL / 2 ) );
			SendDlgItemMessage( hWnd, IDC_SPIN7, UDM_SETRANGE, 0, MAKELONG( UD_MAXVAL / 2, UD_MINVAL / 2 ) );
			if( g_Shared->bListSize )
			{
				SendDlgItemMessage( hWnd, IDC_CHECK2, BM_SETCHECK, BST_CHECKED, 0 );
			}
			else
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT5 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT6 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN6 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN7 ), FALSE );
			}


			int ver = WinVersion();

			//表示
			for( int i = 0; DefviewCmds[ver].style[i].szText != NULL ; i++ )
			{
				SendDlgItemMessage( hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)DefviewCmds[ver].style[i].szText );
				SendDlgItemMessage( hWnd, IDC_COMBO1, CB_SETITEMDATA, i, DefviewCmds[ver].style[i].param );
				if( g_Shared->ListStyle == DefviewCmds[ver].style[i].param )
					SendDlgItemMessage( hWnd, IDC_COMBO1, CB_SETCURSEL, i, 0 );
			}

			//整列
			for( int i = 0; DefviewCmds[ver].sort[i].szText != NULL; i++ )
			{
				SendDlgItemMessage( hWnd, IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)DefviewCmds[ver].sort[i].szText );
				SendDlgItemMessage( hWnd, IDC_COMBO2, CB_SETITEMDATA, i, DefviewCmds[ver].sort[i].param );
				if( g_Shared->ListSort == DefviewCmds[ver].sort[i].param )
					SendDlgItemMessage( hWnd, IDC_COMBO2, CB_SETCURSEL, i, 0 );
			}

			if (g_Shared->bExplorerFolderOpenOnActive) {
				SendDlgItemMessage(hWnd, IDC_CHECK_EXPLORERFOLDEROPENONACTIVE, BM_SETCHECK, BST_CHECKED, 0);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch( HIWORD( wp ) )
		{
		case CBN_SELCHANGE:
		case EN_CHANGE:
		case BN_CLICKED:
			PropSheet_Changed( GetParent( hWnd ), hWnd );
			return TRUE;
		}

		switch( LOWORD( wp ) )
		{
		case IDC_BUTTON2: //ダイアログの位置
			{
				RECT rc = { 0 };
				GetWindowRect( GetParent( GetParent( hWnd ) ) , &rc );
				SetDlgItemInt( hWnd, IDC_EDIT3 , rc.left, FALSE );
				SetDlgItemInt( hWnd, IDC_EDIT4 , rc.top, FALSE );
			}
			return TRUE;
		case IDC_BUTTON3: //リストビューのサイズ
			{
				RECT rc = { 0 };
				GetWindowRect( FindWindowExA( GetParent( GetParent( hWnd ) ), NULL, "SHELLDLL_DefView", NULL ), &rc );
				SetDlgItemInt( hWnd, IDC_EDIT5, rc.right - rc.left, FALSE );
				SetDlgItemInt( hWnd, IDC_EDIT6, rc.bottom - rc.top, FALSE );
			}
			return TRUE;
		//case IDC_CHECK1:
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
			PropSheet_Changed( GetParent( hWnd ), hWnd );
			if( IsDlgButtonChecked( hWnd, IDC_RADIO1 ) == BST_CHECKED )
				//"位置を指定する"がチェックされている
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT3 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT4 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN4 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN5 ), TRUE );
			}
			else
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT3 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT4 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN4 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN5 ), FALSE );
			}
			return TRUE;
		case IDC_CHECK2:
			PropSheet_Changed( GetParent( hWnd ), hWnd );
			if( SendDlgItemMessage( hWnd, IDC_CHECK2, BM_GETCHECK , 0 , 0) == BST_CHECKED )
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT5 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT6 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN6 ), TRUE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN7 ), TRUE );
			}
			else
			{
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT5 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_EDIT6 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN6 ), FALSE );
				EnableWindow( GetDlgItem( hWnd, IDC_SPIN7 ), FALSE );
			}
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		if ( ( (LPNMHDR)lp)->code == PSN_APPLY ) // 適用 or OK
		{
			HWND hFileDlg = GetParent( GetParent( hWnd ) );
			HWND hView = FindWindowExA( hFileDlg, NULL, "SHELLDLL_DefView", NULL );

			//履歴数
			g_Shared->history.max = GetDlgItemInt( hWnd, IDC_EDIT2, NULL, FALSE );
			if( g_Shared->history.max > HISTORY_MAX )
				g_Shared->history.max = HISTORY_MAX;
			if( g_Shared->history.count > g_Shared->history.max )
				g_Shared->history.count = g_Shared->history.max;

			//ツールバーにボタンを追加する
			g_Shared->bToolbar = SendDlgItemMessage( hWnd, IDC_CHECK1, BM_GETCHECK , 0 , 0 ) != 0;

			//表示を設定
			g_Shared->ListStyle = (int)SendDlgItemMessage( hWnd, IDC_COMBO1, CB_GETITEMDATA,
					                    SendDlgItemMessage( hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0 ), 0 );
			if( g_Shared->ListStyle )
				SendMessage( hView, WM_COMMAND, g_Shared->ListStyle, 0 );

			//ソートする項目を設定
			int tmp = (int)SendDlgItemMessage( hWnd, IDC_COMBO2, CB_GETITEMDATA,
					                    SendDlgItemMessage( hWnd, IDC_COMBO2 , CB_GETCURSEL, 0, 0 ), 0 );
			if( tmp != g_Shared->ListSort ) // XPで降順になるのを防ぐために元の値と比較してから設定する
			{
				g_Shared->ListSort = tmp;
				if( g_Shared->ListSort )
					SendMessage( hView, WM_COMMAND, g_Shared->ListSort, 0 );
			}

			//位置、サイズ調整
			if( IsDlgButtonChecked( hWnd, IDC_RADIO1 ) == BST_CHECKED )
				g_Shared->nDialogPos = 1;
			if( IsDlgButtonChecked( hWnd, IDC_RADIO2 ) == BST_CHECKED )
				g_Shared->nDialogPos = 2;
			if( IsDlgButtonChecked( hWnd, IDC_RADIO3 ) == BST_CHECKED )
				g_Shared->nDialogPos = 0;

			g_Shared->DialogPos.x = GetDlgItemInt( hWnd, IDC_EDIT3, NULL, FALSE );
			g_Shared->DialogPos.y = GetDlgItemInt( hWnd, IDC_EDIT4, NULL, FALSE );
			g_Shared->bListSize   = SendDlgItemMessage( hWnd, IDC_CHECK2, BM_GETCHECK , 0 , 0 ) != 0;
			g_Shared->ListSize.x  = GetDlgItemInt( hWnd, IDC_EDIT5, NULL, FALSE );
			g_Shared->ListSize.y  = GetDlgItemInt( hWnd, IDC_EDIT6, NULL, FALSE );

			g_Shared->bExplorerFolderOpenOnActive = (IsDlgButtonChecked(hWnd, IDC_CHECK_EXPLORERFOLDEROPENONACTIVE) == BST_CHECKED);

			AdjustWinPos( hFileDlg );

			//設定ファイルに書き込み
			WritePrivateProfileInt( L"Setting", L"Pos"       , g_Shared->nDialogPos , g_Shared->szIniPath );
			WritePrivateProfileInt( L"Setting", L"PosX"      , g_Shared->DialogPos.x, g_Shared->szIniPath );
			WritePrivateProfileInt( L"Setting", L"PosY"      , g_Shared->DialogPos.y, g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"Size"      , g_Shared->bListSize  , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"SizeX"     , g_Shared->ListSize.x , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"SizeY"     , g_Shared->ListSize.y , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"ListStyle" , g_Shared->ListStyle  , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"ListSort"  , g_Shared->ListSort   , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"MaxHistory", g_Shared->history.max , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"Toolbar"   , g_Shared->bToolbar , g_Shared->szIniPath);
			WritePrivateProfileInt( L"Setting", L"ExplorerFolderOpenOnActive", g_Shared->bExplorerFolderOpenOnActive, g_Shared->szIniPath);

			return TRUE;
		}
		break;
	}
	return FALSE;
}



////////////////////////////////////////////////////////////////////
//                           ショートカット
////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EditShortcutDialogProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp );

BOOL CALLBACK PageProcSHORTCUT( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), FALSE );

			for( int i = 0; i < g_Shared->shortcut.count; i++ )
			{
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)g_Shared->shortcut.data[i].szName );
				SHORTCUTDATA::ShortcutItem *data = (SHORTCUTDATA::ShortcutItem *)HeapAlloc(
					GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( SHORTCUTDATA::ShortcutItem ) );
				if( data )
					*data = g_Shared->shortcut.data[i];
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, i, (LPARAM)data );
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch( LOWORD( wp ) )
		{
		case IDC_BUTTON1: //上へ
		case IDC_BUTTON2: //下へ
			{
				TCHAR szBuff[MAX_PATH];
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				if( LOWORD( wp ) == IDC_BUTTON1 && index == 0 ||
					LOWORD( wp ) == IDC_BUTTON2 && index == SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 ) - 1 )
					return TRUE;

				int Insertndx = index + ( LOWORD( wp ) == IDC_BUTTON1 ? -1 : 1 );
				
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETTEXT, index, (LPARAM)szBuff );
				LPARAM data = SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );

				Insertndx = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_INSERTSTRING, Insertndx, (LPARAM)szBuff );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, Insertndx, data );

				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETCURSEL, Insertndx, 0 );
				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);

				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON4: //編集
			{
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				LPARAM data = SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				if( data == NULL )
					return TRUE;
				int ret = (int)DialogBoxParam( g_Inst, MAKEINTRESOURCE( IDD_EDITSHORTCUT ),
					                       hWnd, EditShortcutDialogProc, data );

				if( ret == IDOK )
				{
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );
					index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_INSERTSTRING, index,
						(LPARAM)((SHORTCUTDATA::ShortcutItem*)data)->szName );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, index, data );
				}
				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON5: //削除
			{
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				SHORTCUTDATA::ShortcutItem *data = (SHORTCUTDATA::ShortcutItem*)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				if( data )
					HeapFree(GetProcessHeap(), 0, data );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );

				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);
				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON6: //新規作成
			{
				if( SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 ) >= SHORTCUT_MAX )
				{
					MessageBoxA( hWnd, "これ以上ショートカットを作成できません", "", 0 );
					return TRUE;
				}

				SHORTCUTDATA::ShortcutItem *data = (SHORTCUTDATA::ShortcutItem *)HeapAlloc(
					GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( SHORTCUTDATA::ShortcutItem ) );

				if( data == NULL )
					return TRUE;

				int ret = (int)DialogBoxParam( g_Inst, MAKEINTRESOURCE( IDD_EDITSHORTCUT ), hWnd, 
					                     EditShortcutDialogProc, (LPARAM)data );

				if( ret == IDOK )
				{
					int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)data->szName );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, index, (LPARAM)data );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETCURSEL, index, 0 );
					SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);
					PropSheet_Changed( GetParent( hWnd ), hWnd );
				}
				else
				{
					HeapFree(GetProcessHeap(), 0, data );
				}
			}
			return TRUE;
		case IDC_LIST1:
			switch( HIWORD( wp ) )
			{
			case LBN_SELCHANGE:
				{
					int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
					int count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 );
					if( index != LB_ERR )
					{
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), index == 0 ? FALSE : TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), index == count - 1 ? FALSE : TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), TRUE );
					}
					else
					{
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), FALSE );
					}
				}
				break;
			case LBN_DBLCLK:
				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_BUTTON4, 0 ), 0 );
				break;
			}
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		if ( ( (LPNMHDR)lp)->code == PSN_APPLY ) // 適用 or OK
		{
			g_Shared->shortcut.count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT , 0, 0 );
			for( int i = 0; i < g_Shared->shortcut.count; i++ )
				g_Shared->shortcut.data[i] = *(SHORTCUTDATA::ShortcutItem*)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, i, 0 );

			//セクションのキーをすべて削除してからiniに書き込む
			WritePrivateProfileSection( L"Shortcut", L"\0\0", g_Shared->szIniPath );
			TCHAR szKey[32];
			for( int i = 0; i < g_Shared->shortcut.count; i++ )
			{
				wsprintf( szKey, L"Name%d", i );
				WritePrivateProfileString( L"Shortcut", szKey, g_Shared->shortcut.data[i].szName, g_Shared->szIniPath );
				wsprintf( szKey, L"Path%d", i );
				WritePrivateProfileString( L"Shortcut", szKey, g_Shared->shortcut.data[i].szPath, g_Shared->szIniPath );
			}
		}
		return TRUE;

	case WM_DESTROY:
		{
			// 確保したメモリを開放
			int count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT , 0, 0 );
			for( int i = 0; i < count; i++ )
				HeapFree( GetProcessHeap(), 0, (LPVOID)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, i, 0 ) );
		}
		return TRUE;
	}

	return FALSE;
}

BOOL BrowseForFolder( HWND hWnd, LPTSTR pStr )
{
	BROWSEINFO bi = { 0 };
	TCHAR szBuff[MAX_PATH];

	bi.hwndOwner = hWnd;
	bi.pszDisplayName = szBuff;
	bi.lpszTitle = L"";
	bi.ulFlags = BIF_RETURNONLYFSDIRS ;

	LPITEMIDLIST pidl = SHBrowseForFolder( &bi );
	if( pidl != NULL )
	{
		SHGetPathFromIDList( pidl, pStr );
		CoTaskMemFree( pidl );			
	}
	else
		return FALSE;

	return TRUE;
}

INT_PTR CALLBACK EditShortcutDialogProc( HWND hWnd , UINT msg , WPARAM wp , LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			SHORTCUTDATA::ShortcutItem *data = (SHORTCUTDATA::ShortcutItem *)lp;
			SetWindowLongPtr( hWnd, GWLP_USERDATA, lp );
			SetDlgItemText( hWnd, IDC_EDIT1, data->szName );
			SetDlgItemText( hWnd, IDC_EDIT2, data->szPath );
		}
		return TRUE;
	case WM_COMMAND:
		switch( LOWORD( wp ) )
		{
		case IDOK:
			{
				SHORTCUTDATA::ShortcutItem * data = (SHORTCUTDATA::ShortcutItem*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
				GetDlgItemText( hWnd, IDC_EDIT1, data->szName, MAX_PATH );
				GetDlgItemText( hWnd, IDC_EDIT2, data->szPath, MAX_PATH );
			}
			EndDialog(hWnd , IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd , IDCANCEL);
			break;
		case IDC_BUTTON1:
			{
				HRESULT hRes = CoInitialize( NULL );
				if( hRes == E_OUTOFMEMORY || hRes == E_INVALIDARG || hRes == E_UNEXPECTED )
					return FALSE;
				SHORTCUTDATA::ShortcutItem *data = (SHORTCUTDATA::ShortcutItem*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
				BOOL res = BrowseForFolder( hWnd, data->szPath );
				CoUninitialize();

				if( res )
					SetDlgItemText( hWnd, IDC_EDIT2, data->szPath );
			}
			break;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog( hWnd, IDCANCEL );
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////
//                             ツール
////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK EditToolDialogProc( HWND hWnd , UINT msg , WPARAM wp , LPARAM lp );

BOOL CALLBACK PageProcTOOL( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), FALSE );

			for( int i = 0; i < g_Shared->tool.count; i++ )
			{
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)g_Shared->tool.data[i].szName );
				TOOLDATA::ToolItem *data = (TOOLDATA::ToolItem *)HeapAlloc(
					GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( TOOLDATA::ToolItem ) );
				if( data )
					*data = g_Shared->tool.data[i];
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, i, (LPARAM)data );
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch( LOWORD( wp ) )
		{
		case IDC_BUTTON1: //上へ
		case IDC_BUTTON2: //下へ
			{
				TCHAR szBuff[MAX_PATH];
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				if( LOWORD( wp ) == IDC_BUTTON1 && index == 0 ||
					LOWORD( wp ) == IDC_BUTTON2 && index == SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 ) - 1 )
					return TRUE;

				int Insertndx = index + ( LOWORD( wp ) == IDC_BUTTON1 ? -1 : 1 );
				
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETTEXT, index, (LPARAM)szBuff );
				LPARAM data = SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );

				Insertndx = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_INSERTSTRING, Insertndx, (LPARAM)szBuff );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, Insertndx, data );

				SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETCURSEL, Insertndx, 0 );
				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);

				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON4: //編集
			{
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				LPARAM data = SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				if( data == NULL )
					return TRUE;
				int ret = (int)DialogBoxParam( g_Inst, MAKEINTRESOURCE( IDD_EDITTOOL ), hWnd, EditToolDialogProc, data );

				if( ret == IDOK )
				{
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );
					index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_INSERTSTRING, index,
						(LPARAM)((TOOLDATA::ToolItem*)data)->szName );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, index, data );
				}
				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON5: //削除
			{
				int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
				if( index == LB_ERR )
					return TRUE;
				TOOLDATA::ToolItem *data = (TOOLDATA::ToolItem*)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, index, 0 );
				if( data )
					HeapFree(GetProcessHeap(), 0, data );
				SendDlgItemMessage( hWnd, IDC_LIST1, LB_DELETESTRING, index, 0 );

				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);
				PropSheet_Changed( GetParent( hWnd ), hWnd );
			}
			return TRUE;
		case IDC_BUTTON6: //新規作成
			{
				if( SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 ) >= TOOL_MAX )
				{
					MessageBoxA( hWnd, "これ以上ショートカットを作成できません", "", 0 );
					return TRUE;
				}

				TOOLDATA::ToolItem *data = (TOOLDATA::ToolItem*)HeapAlloc(
					GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( TOOLDATA::ToolItem ) );

				if( data == NULL )
					return TRUE;

				int ret = (int)DialogBoxParam( g_Inst, MAKEINTRESOURCE( IDD_EDITTOOL ),
					                      hWnd, EditToolDialogProc, (LPARAM)data );

				if( ret == IDOK )
				{
					int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)data->szName );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETITEMDATA, index, (LPARAM)data );
					SendDlgItemMessage( hWnd, IDC_LIST1, LB_SETCURSEL, index, 0 );
					SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_LIST1, LBN_SELCHANGE ), 0);
					PropSheet_Changed( GetParent( hWnd ), hWnd );
				}
				else
				{
					HeapFree(GetProcessHeap(), 0, data );
				}
			}
			return TRUE;
		case IDC_LIST1:
			switch( HIWORD( wp ) )
			{
			case LBN_SELCHANGE:
				{
					int index = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0 );
					int count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT, 0, 0 );
					if( index != LB_ERR )
					{
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), index == 0 ? FALSE : TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), index == count - 1 ? FALSE : TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), TRUE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), TRUE );
					}
					else
					{
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON1 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON2 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON4 ), FALSE );
						EnableWindow( GetDlgItem( hWnd, IDC_BUTTON5 ), FALSE );
					}
				}
				break;
			case LBN_DBLCLK:
				SendMessage( hWnd, WM_COMMAND, MAKEWPARAM( IDC_BUTTON4, 0 ), 0 );
				break;
			}
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		if ( ( (LPNMHDR)lp)->code == PSN_APPLY ) // 適用 or OK
		{
			g_Shared->tool.count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT , 0, 0 );
			for( int i = 0; i < g_Shared->tool.count; i++ )
				g_Shared->tool.data[i] = *(TOOLDATA::ToolItem*)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, i, 0 );

			//セクションのキーをすべて削除してからiniに書き込む
			WritePrivateProfileSection( L"Tool", L"\0\0", g_Shared->szIniPath );
			TCHAR szKey[32];
			for( int i = 0; i < g_Shared->tool.count; i++ )
			{
				wsprintf( szKey, L"Name%d", i );
				WritePrivateProfileString( L"Tool", szKey, g_Shared->tool.data[i].szName, g_Shared->szIniPath );
				wsprintf( szKey, L"Path%d", i );
				WritePrivateProfileString( L"Tool", szKey, g_Shared->tool.data[i].szPath, g_Shared->szIniPath );
				wsprintf( szKey, L"Param%d", i );
				WritePrivateProfileString( L"Tool", szKey, g_Shared->tool.data[i].szParam, g_Shared->szIniPath );
			}
		}
		return TRUE;

	case WM_DESTROY:
		{
			// 確保したメモリを開放
			int count = (int)SendDlgItemMessage( hWnd, IDC_LIST1, LB_GETCOUNT , 0, 0 );
			for( int i = 0; i < count; i++ )
				HeapFree( GetProcessHeap(), 0, (LPVOID)SendDlgItemMessage(
						hWnd, IDC_LIST1, LB_GETITEMDATA, i, 0 ) );
		}
		return TRUE;
	}

	return FALSE;
}


BOOL GetFileName( HWND hParent, LPTSTR lpBuf, int bufsize )
{
	OPENFILENAME ofn = { sizeof(OPENFILENAME) };
	ZeroMemory( lpBuf, bufsize * sizeof(TCHAR) );

	ofn.hwndOwner = hParent;
	ofn.lpstrFilter = L"All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = lpBuf;
	ofn.nMaxFile = bufsize;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;

	return GetOpenFileName(&ofn);
}

INT_PTR CALLBACK EditToolDialogProc( HWND hWnd , UINT msg , WPARAM wp , LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			TOOLDATA::ToolItem *data = (TOOLDATA::ToolItem *)lp;
			SetWindowLongPtr( hWnd, GWLP_USERDATA, lp );
			SetDlgItemText( hWnd, IDC_EDIT1, data->szName );
			SetDlgItemText( hWnd, IDC_EDIT2, data->szPath );
			SetDlgItemText( hWnd, IDC_EDIT3, data->szParam );
		}
		return TRUE;
	case WM_COMMAND:
		switch( LOWORD( wp ) )
		{
		case IDOK:
			{
				TOOLDATA::ToolItem * data = (TOOLDATA::ToolItem*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
				GetDlgItemText( hWnd, IDC_EDIT1, data->szName, MAX_PATH );
				GetDlgItemText( hWnd, IDC_EDIT2, data->szPath, MAX_PATH );
				GetDlgItemText( hWnd, IDC_EDIT3, data->szParam, MAX_PATH );
			}
			EndDialog(hWnd , IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd , IDCANCEL);
			break;
		case IDC_BUTTON1:
			{
				TOOLDATA::ToolItem *data = (TOOLDATA::ToolItem*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
				if( GetFileName( hWnd, data->szPath, MAX_PATH ) )
					SetDlgItemText( hWnd, IDC_EDIT2, data->szPath );
			}
			break;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog( hWnd, IDCANCEL );
		return TRUE;
	}
	return FALSE;
}


BOOL SettingDialog( HWND hWnd )
{
	const int nPage = 3;
	PROPSHEETPAGE psp = { 0 };
	PROPSHEETHEADER psh = { 0 };
	HPROPSHEETPAGE hPsp[nPage] = { 0 };

	InitCommonControls();

	psp.dwSize = sizeof (PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = g_Inst;
	
	psp.pszTemplate = MAKEINTRESOURCE( IDD_GENERAL );
	psp.pfnDlgProc = (DLGPROC)PageProcGENERAL;
	hPsp[0] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE( IDD_SHORTCUT );
	psp.pfnDlgProc = (DLGPROC)PageProcSHORTCUT;
	hPsp[1] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE( IDD_TOOL );
	psp.pfnDlgProc = (DLGPROC)PageProcTOOL;
	hPsp[2] = CreatePropertySheetPage(&psp);

	psh.dwSize = sizeof (PROPSHEETHEADER);
	psh.dwFlags = PSH_DEFAULT;
	psh.hwndParent = hWnd;
	psh.pszCaption = _T("Setting");
	psh.nPages = nPage;
	psh.phpage = hPsp;
	PropertySheet(&psh);


	return TRUE;
}
