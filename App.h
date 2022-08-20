#pragma once

#include <AWindowsLib.h>

#include "resource.h"

#include "ProseKeys.h"

#include "MySQLODBC.h"
#include "HashTag.h"
#include "PictureItem.h"
#include "FolderItem.h"
#include "ImageParser.h"
#include "HashTagSelectCtrl.h"
#include "Controls.h"
#include "Dialogs.h"
#include "GlobalSlideDlg.h"
#include "GroupShowWnd.h"
#include "GroupManageWnd.h"
#include "MainWnd.h"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif


// defined in AWnd.h  WM_SPLITWINDOWCHANGED (WM_USER + 0x0001) // wParam is split window HWND, lParam is SplitContainer ID

#define WM_HTSC_ITEMCLICK      (WM_USER + 0x0101) // wparam is HashTagSelectCtrl hWnd, lparam is HashTag * pointer 
#define WM_HTSC_ITEMCONTEXT    (WM_USER + 0x0102) // wparam is HashTagSelectCtrl hWnd, lparam is HashTag * pointer
#define WM_HTSC_FILTER_CHANGED (WM_USER + 0x0103) // wparam is HashTagSelectCtrl hWnd
#define WM_HTSC_NAME_CHANGED   (WM_USER + 0x0104) // wparam is HashTagSelectCtrl hWnd
#define WM_HTSC_LIST_CHANGED   (WM_USER + 0x0105) // wparam is HashTagSelectCtrl hWnd
#define WM_PICVIEW_HT_CHANGED  (WM_USER + 0x0106) // wparam is PicView's hWnd
#define WM_PICLIST_HT_CHANGED  (WM_USER + 0x0107) // wparam is PicListView hWnd, LPARAM is ImageParser ID
#define WM_PICLIST_ITEMDELETED (WM_USER + 0x0108) // wparam is PicListView hWnd, LPARAM is count of items deleted

class PicManApp : public Application
{
 public:

 PicManApp();
~PicManApp();

 virtual bool Init(HINSTANCE hInst, int nCmdShow, String const &app, String const &displayName);
 virtual void ShutDown(); 

 inline int NextPictureID() { return m_PictureID++; }
 inline void ResetPictureID() { m_PictureID=1; }

 void SaveSetting(String const &thing, String const &val);
 void SaveSettingInt(String const &thing, int val);
 void SaveSettingDbl(String const &thing, double val);
 void SaveSettingBool(String const &thing, bool val);

 String GetSetting(String const &thing);
 String GetSetting(String const &thing, String const &def);
 int GetSettingInt(String const &thing);
 int GetSettingInt(String const &thing, int def);
 bool GetSettingBool(String const &thing);
 bool GetSettingBool(String const &thing, bool def);
 double GetSettingDbl(String const &thing);
 double GetSettingDbl(String const &thing, double def);

 std::vector<ImageParser *> ClearAndGetImports();
 void ClearPictures();

 std::vector<HashTag> GetGlobalHashTags();
 bool UpdateGlobalHashTag(HashTag &ht, String const &s);
 void DropGlobalHashTag(HashTag const &ht);
 HashTag AddGlobalHashTag(String const &txt);
 HashTag GetFavoriteHashTag();

 static void EditPicture(String const &file);
 
 std::map<int, ImageParser *>    Pictures;
 std::map<int, HashTag>          GlobalHashTags;
 std::map<int, BaseItem *>       Bases;
 
 LocalOptions Options;

 private:

 int m_PictureID;


};

class MySqlConnection;

class DBCon
{
 public:
 DBCon();
~DBCon();

 inline MySqlConnection *Con() { return m_Connection; }
 inline String User() { return m_UserName; }

 static bool TestConnect(String const &cs);
 bool Connect(bool bSilent);

 private:

 String m_UserName;
 

 String m_ConnectionString;
 MySqlConnection *m_Connection;
}; 

extern PicManApp *App;
extern MainWnd   *Wnd;
extern DBCon     *DB;

class CredFileWrite
{
 public:
 
 CredFileWrite();
~CredFileWrite();

 bool Open(String const &path);
 bool Write(std::vector<BYTE> const &bytes);
 void Close();

 String Error() { return m_Error; }
 
 protected:

 String m_Error;
 HANDLE m_hFile;
};

class CredFileRead
{
 public:
 
 CredFileRead();
~CredFileRead();

 bool Open(String const &path);
 std::vector<BYTE> ReadBytes();
 void Close();

 String Error() { return m_Error; }

protected:

 String m_Error;
 HANDLE m_hFile;
};
