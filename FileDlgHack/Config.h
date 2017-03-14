#pragma once

#include <string>

class Config
{
public:
	static Config& GetInstance();

	void	LoadConfig();

	enum ConfigGeneral { general };
	enum ConfigTool { tool };
	enum ConfigShortcut { shortcut };
	void	SaveConfig(ConfigGeneral g);
	void	SaveConfig(ConfigTool t);
	void	SaveConfig(ConfigShortcut s);


	// ショートカットに追加
	BOOL AppendShortcut(LPCTSTR pszPath, HWND hWnd);
	// 履歴に追加
	BOOL	AppendHistory(LPCTSTR pszPath);
	// 履歴をクリア
	BOOL	ClearHistory();

private:
	Config();
	~Config();

	HANDLE m_hMutex;

};
