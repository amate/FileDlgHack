// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

#include "FileDlgHack.h"

// --------------------------------------------------------
//    DLLMain
// --------------------------------------------------------
BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
  switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
      g_Inst = hInstance;
      // 共有メモリを作成
      g_hShared = CreateFileMapping( INVALID_HANDLE_VALUE  , NULL ,
                                     PAGE_READWRITE , 0 , sizeof( SHAREDDATA ) , FILEMAP_SHARED );
      g_Shared = (SHAREDDATA*)MapViewOfFile( g_hShared , FILE_MAP_WRITE , 0 , 0 , 0);
      break;

    case DLL_PROCESS_DETACH:
      // 共有メモリをクローズ
      UnmapViewOfFile( g_Shared );
      CloseHandle( g_hShared );
      break;
    }
  return TRUE;
}

