// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>
#include <commctrl.h>
#include <CommDlg.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <ShellAPI.h>

// �W���b���C�u����
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

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B
