// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>
#include <commctrl.h>
#include <CommDlg.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <ShellAPI.h>

// 標準Ｃライブラリ
#include <stdlib.h>
#include <tchar.h>

// STL
#include <map>

#ifdef _DEBUG
#include <cstdarg>
void DebugTrace(const char *format, ...);
#define TRACE	DebugTrace
#else
#define TRACE
#endif

#include "resource.h"

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
