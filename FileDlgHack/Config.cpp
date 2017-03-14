
#include "stdafx.h"
#include "Config.h"
#include <Strsafe.h>
#include "FileDlgHack.h"


// iniファイルからショートカットを読み込む
BOOL LoadShortcut(LPCTSTR szIni)
{
	TCHAR szKey[32];
	for (int i = 0; i < SHORTCUT_MAX; i++)
	{
		wsprintf(szKey, L"Name%d", i);
		GetPrivateProfileString(L"Shortcut", szKey, L"none", g_Shared->shortcut.data[i].szName, MAX_PATH, szIni);
		if (lstrcmp(g_Shared->shortcut.data[i].szName, L"none") == 0)
			break;
		wsprintf(szKey, L"Path%d", i);
		GetPrivateProfileString(L"Shortcut", szKey, L"", g_Shared->shortcut.data[i].szPath, MAX_PATH, szIni);
		g_Shared->shortcut.count++;
	}

	return TRUE;
}

// ini ファイルから履歴を読み込む
BOOL LoadHistory(LPCTSTR szIni)
{
	TCHAR szKey[32];
	for (int i = 0; i < HISTORY_MAX; i++)
	{
		wsprintf(szKey, L"hist%d", i);
		GetPrivateProfileString(L"History", szKey, L"none", g_Shared->history.data[i].szPath, MAX_PATH, szIni);
		if (lstrcmp(g_Shared->history.data[i].szPath, L"none") == 0)
			break;
		g_Shared->history.count++;
	}

	return TRUE;
}

// iniファイルからツールを読み込む
BOOL LoadTool(LPCTSTR szIni)
{
	TCHAR szKey[32];
	for (int i = 0; i < TOOL_MAX; i++)
	{
		wsprintf(szKey, L"Name%d", i);
		GetPrivateProfileString(L"Tool", szKey, L"none", g_Shared->tool.data[i].szName, MAX_PATH, szIni);
		if (lstrcmp(g_Shared->tool.data[i].szName, L"none") == 0)
			break;
		wsprintf(szKey, L"Path%d", i);
		GetPrivateProfileString(L"Tool", szKey, L"", g_Shared->tool.data[i].szPath, MAX_PATH, szIni);
		wsprintf(szKey, L"Param%d", i);
		GetPrivateProfileString(L"Tool", szKey, L"", g_Shared->tool.data[i].szParam, MAX_PATH, szIni);
		g_Shared->tool.count++;
	}

	return TRUE;
}

// ini ファイルにショートカットを保存
void	SaveShortcut(LPCTSTR iniPath)
{
	WritePrivateProfileSection(L"Shortcut", L"\0\0", iniPath);
	TCHAR szKey[32];
	for (int i = 0; i < g_Shared->shortcut.count; i++)
	{
		wsprintf(szKey, L"Name%d", i);
		WritePrivateProfileString(L"Shortcut", szKey, g_Shared->shortcut.data[i].szName, iniPath);
		wsprintf(szKey, L"Path%d", i);
		WritePrivateProfileString(L"Shortcut", szKey, g_Shared->shortcut.data[i].szPath, iniPath);
	}
}

// ini ファイルに履歴を保存
BOOL SaveHistory(LPCTSTR szIni)
{
	WritePrivateProfileSection(L"History", L"\0\0", szIni);
	TCHAR szKey[32];
	for (int i = 0; i < g_Shared->history.count; i++)
	{
		wsprintf(szKey, L"hist%d", i);
		WritePrivateProfileString(L"History", szKey, g_Shared->history.data[i].szPath, szIni);
	}

	return TRUE;
}


BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int value, LPCTSTR lpFileName)
{
	TCHAR szbuff[32];
	wsprintf(szbuff, L"%d", value);
	return WritePrivateProfileString(lpAppName, lpKeyName, szbuff, lpFileName);
}

// 使用例:
// MutexLocker lock(m_hMutex);

class MutexLocker
{
public:
	MutexLocker(HANDLE hMutex) : m_hMutex(hMutex)
	{
		_ASSERT(m_hMutex);
		DWORD ret = ::WaitForSingleObject(m_hMutex, INFINITE);
		if (ret != WAIT_OBJECT_0) {
			throw std::runtime_error("WaitForSingleObject failed");
		}
	}

	~MutexLocker()
	{
		::ReleaseMutex(m_hMutex);
	}

private:
	HANDLE m_hMutex;
};

/////////////////////////////////////////////////////
// Config

LPCWSTR kMutexName = L"FileDlgHackMutex";

Config::Config() : m_hMutex(NULL)
{
	m_hMutex = ::CreateMutex(nullptr, FALSE, kMutexName);
	_ASSERT(m_hMutex);
}

Config::~Config()
{
	::CloseHandle(m_hMutex);
}

Config& Config::GetInstance()
{
	static Config s_instance;
	return s_instance;
}

void	Config::LoadConfig()
{
	MutexLocker lock(m_hMutex);

	TCHAR iniPath[MAX_PATH] = _T("");
	int len = GetModuleFileName(g_Inst, iniPath, MAX_PATH);

	iniPath[len - 3] = L'i';
	iniPath[len - 2] = L'n';
	iniPath[len - 1] = L'i';

#ifdef _WIN64
	::StringCchCopy(g_Shared->szx64IniPath, MAX_PATH, iniPath);
#else
	::StringCchCopy(g_Shared->sz86IniPath, MAX_PATH, iniPath);
#endif

	TRACE("open");	// logファイル作成

	g_Shared->nDialogPos = GetPrivateProfileInt(L"Setting", L"Pos", FALSE, iniPath);
	g_Shared->DialogPos.x = GetPrivateProfileInt(L"Setting", L"PosX", 100, iniPath);
	g_Shared->DialogPos.y = GetPrivateProfileInt(L"Setting", L"PosY", 100, iniPath);
	g_Shared->bListSize = GetPrivateProfileInt(L"Setting", L"Size", FALSE, iniPath);
	g_Shared->ListSize.x = GetPrivateProfileInt(L"Setting", L"SizeX", 640, iniPath);
	g_Shared->ListSize.y = GetPrivateProfileInt(L"Setting", L"SizeY", 480, iniPath);
	g_Shared->ListStyle = GetPrivateProfileInt(L"Setting", L"ListStyle", 0, iniPath);
	g_Shared->ListSort = GetPrivateProfileInt(L"Setting", L"ListSort", 0, iniPath);
	g_Shared->bToolbar = GetPrivateProfileInt(L"Setting", L"Toolbar", 0, iniPath);
	g_Shared->bExplorerFolderOpenOnActive = GetPrivateProfileInt(L"Setting", L"ExplorerFolderOpenOnActive", 0, iniPath);

	// 履歴
	g_Shared->history.count = 0;
	g_Shared->history.max = GetPrivateProfileInt(L"Setting", L"MaxHistory", 5, iniPath);
	LoadHistory(iniPath);

	// ショートカット
	g_Shared->shortcut.count = 0;
	LoadShortcut(iniPath);

	// ツール
	g_Shared->tool.count = 0;
	LoadTool(iniPath);
}

//設定ファイルに書き込み
void	Config::SaveConfig(ConfigGeneral g)
{
	MutexLocker lock(m_hMutex);

	auto funcSaveSharedConfig = [](const std::wstring& iniPath) {
		WritePrivateProfileInt(L"Setting", L"Pos", g_Shared->nDialogPos, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"PosX", g_Shared->DialogPos.x, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"PosY", g_Shared->DialogPos.y, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"Size", g_Shared->bListSize, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"SizeX", g_Shared->ListSize.x, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"SizeY", g_Shared->ListSize.y, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"ListStyle", g_Shared->ListStyle, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"ListSort", g_Shared->ListSort, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"MaxHistory", g_Shared->history.max, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"Toolbar", g_Shared->bToolbar, iniPath.c_str());
		WritePrivateProfileInt(L"Setting", L"ExplorerFolderOpenOnActive", g_Shared->bExplorerFolderOpenOnActive, iniPath.c_str());
	};

	_ASSERT((g_Shared->sz86IniPath[0] != _T('\0')) || (g_Shared->szx64IniPath[0] != _T('\0')));
	if (g_Shared->sz86IniPath[0] != _T('\0')) {
		funcSaveSharedConfig(g_Shared->sz86IniPath);
	}
	if (g_Shared->szx64IniPath[0] != _T('\0')) {
		funcSaveSharedConfig(g_Shared->szx64IniPath);
	}
}

void	Config::SaveConfig(ConfigTool t)
{
	MutexLocker lock(m_hMutex);

	auto funcSaveConfigTool = [](const std::wstring& iniPath) {
		//セクションのキーをすべて削除してからiniに書き込む
		WritePrivateProfileSection(L"Tool", L"\0\0", iniPath.c_str());
		TCHAR szKey[32];
		for (int i = 0; i < g_Shared->tool.count; i++)
		{
			wsprintf(szKey, L"Name%d", i);
			WritePrivateProfileString(L"Tool", szKey, g_Shared->tool.data[i].szName, iniPath.c_str());
			wsprintf(szKey, L"Path%d", i);
			WritePrivateProfileString(L"Tool", szKey, g_Shared->tool.data[i].szPath, iniPath.c_str());
			wsprintf(szKey, L"Param%d", i);
			WritePrivateProfileString(L"Tool", szKey, g_Shared->tool.data[i].szParam, iniPath.c_str());
		}
	};

	_ASSERT((g_Shared->sz86IniPath[0] != _T('\0')) || (g_Shared->szx64IniPath[0] != _T('\0')));
	if (g_Shared->sz86IniPath[0] != _T('\0')) {
		funcSaveConfigTool(g_Shared->sz86IniPath);
	}
	if (g_Shared->szx64IniPath[0] != _T('\0')) {
		funcSaveConfigTool(g_Shared->szx64IniPath);
	}
}

void	Config::SaveConfig(ConfigShortcut s)
{
	MutexLocker lock(m_hMutex);

	_ASSERT((g_Shared->sz86IniPath[0] != _T('\0')) || (g_Shared->szx64IniPath[0] != _T('\0')));
	if (g_Shared->sz86IniPath[0] != _T('\0')) {
		SaveShortcut(g_Shared->sz86IniPath);
	}
	if (g_Shared->szx64IniPath[0] != _T('\0')) {
		SaveShortcut(g_Shared->szx64IniPath);
	}
}

// ショートカットに追加
BOOL Config::AppendShortcut(LPCTSTR pszPath, HWND hWnd)
{
	MutexLocker lock(m_hMutex);

	TCHAR szName[80];
	PathCompactPathEx(szName, pszPath, 64, 0);
	if (g_Shared->shortcut.count < SHORTCUT_MAX) {
		lstrcpy(g_Shared->shortcut.data[g_Shared->shortcut.count].szName, szName);
		lstrcpy(g_Shared->shortcut.data[g_Shared->shortcut.count].szPath, pszPath);
		g_Shared->shortcut.count++;

	} else {
		wsprintf(szName, L"ショートカットは%d個までしか作成できません", SHORTCUT_MAX);
		MessageBox(hWnd, szName, nullptr, 0);
		return FALSE;
	}

	//  保存
	SaveConfig(shortcut);

	return TRUE;
}

// 履歴に追加
BOOL	Config::AppendHistory(LPCTSTR pszPath)
{
	MutexLocker lock(m_hMutex);

	// データを一つ後ろにずらす
	int i;
	for (i = 0; i < g_Shared->history.count; i++)
		if (lstrcmp(g_Shared->history.data[i].szPath, pszPath) == 0)
			break;
	if (i == g_Shared->history.count && g_Shared->history.count < g_Shared->history.max)
		g_Shared->history.count++;
	if (i == g_Shared->history.max)
		i--;
	for (; i > 0; i--)
		lstrcpy(g_Shared->history.data[i].szPath, g_Shared->history.data[i - 1].szPath);

	// 履歴の先頭にに追加
	lstrcpy(g_Shared->history.data[0].szPath, pszPath);

	// 保存
	_ASSERT((g_Shared->sz86IniPath[0] != _T('\0')) || (g_Shared->szx64IniPath[0] != _T('\0')));
	if (g_Shared->sz86IniPath[0] != _T('\0')) {
		SaveHistory(g_Shared->sz86IniPath);
	}
	if (g_Shared->szx64IniPath[0] != _T('\0')) {
		SaveHistory(g_Shared->szx64IniPath);
	}

	return TRUE;
}

// 履歴をクリア
BOOL	Config::ClearHistory()
{
	MutexLocker lock(m_hMutex);

	_ASSERT((g_Shared->sz86IniPath[0] != _T('\0')) || (g_Shared->szx64IniPath[0] != _T('\0')));
	if (g_Shared->sz86IniPath[0] != _T('\0')) {
		WritePrivateProfileSection(L"History", L"\0\0", g_Shared->sz86IniPath);
	}
	if (g_Shared->szx64IniPath[0] != _T('\0')) {
		WritePrivateProfileSection(L"History", L"\0\0", g_Shared->szx64IniPath);
	}
	return TRUE;
}


