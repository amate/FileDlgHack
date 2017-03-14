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


	// �V���[�g�J�b�g�ɒǉ�
	BOOL AppendShortcut(LPCTSTR pszPath, HWND hWnd);
	// �����ɒǉ�
	BOOL	AppendHistory(LPCTSTR pszPath);
	// �������N���A
	BOOL	ClearHistory();

private:
	Config();
	~Config();

	HANDLE m_hMutex;

};
