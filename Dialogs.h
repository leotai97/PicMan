#pragma once

class HelpAboutDlg : public ADialog
{
 public:

 void Show(AWnd *parent);

 protected:

 void OnInitDialog();

 virtual WMR OnCommand(int child, HWND hChild);
};

class LoginDlg : public ADialog
{
 public:
 LoginDlg();

 String Server;
 String Db;
 String User;
 String Password;

 bool RememberPassword;

 String Connection();
 void SaveSettings();

 DialogResult Show();

 protected:

 virtual void OnInitDialog();
 virtual WMR OnCommand(int child, HWND hChild);

 EditWnd m_Pwd;
};

class BaseItemMgrDlg : public ADialog
{
 public:
 BaseItemMgrDlg(BaseItem *current);

 BaseItem *GetBaseItem(){ return m_BaseItem; }

 DialogResult Show();

 
 protected:

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);
 virtual void OnListViewItemChanged(int child, NMLISTVIEW *plvn);

 void OnOK();
 void OnCancel();
 void OnAdd();
 void OnDrop();
 void OnEdit();

 void LoadBaseItems();

 BaseItem *m_BaseItem;
 ListView  lstBase;
};

class FolderOpenDlg : public ADialog
{
 public:

 FolderOpenDlg();

 inline FolderItem *CurrentFolder(){ return m_FolderItem; }

 DialogResult Show();

 static int CALLBACK CompareFolders(LPARAM fldrIdA, LPARAM flderIdB, LPARAM nAscDesCol);

 static void LoadHeaders(ListView &lstDir);
 static void LoadView(ListView &lstDir, std::map<int, FolderItem *> const &fldrs);
 
 protected:

 void OnOK();
 void OnCancel();
 void OnDrop();
 void OnScan();
 
 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND childhWnd);
 virtual void OnListViewColumnClick(int child, int col);
 virtual void OnListViewItemChanged(int child, NMLISTVIEW *plvn); 
 virtual void OnDoubleClick(int child, NMITEMACTIVATE *itemActivate);
 ListView lstDir;

 bool m_AscDesc;
 int  m_Column;

 FolderItem *m_FolderItem;

};

class HashTagEditDlg : public ADialog
{
 public:
 HashTagEditDlg();

 DialogResult Show(AWnd *parent);
 DialogResult Show(AWnd *parent, HashTag const &ht);

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChildWnd);

 HashTag EditTag;

 protected:

 void OnOK();

 ButtonWnd m_OK;
 ButtonWnd m_Cancel;

};

class HashTagDlg : public ADialog
{
 public:

 HashTagDlg(PictureItem *pItem); 

 DialogResult Show(AWnd *parent);

 void btnGlobal_Click();
 void btnFolder_Click();

 protected:

 void OnInitDialog();
 
 virtual WMR OnCommand(int child, HWND hChild);
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 void OnGlobalClicked(HashTag *ht);
 void OnFolderClicked(HashTag *ht);

 void OnSelectClicked(HashTag *ht);
 void OnSelectContext(HashTag *ht);

 void OnFolderContext(HashTag *ht);


 void OnOK();
 void OnCancel();

 TabWnd tabTags;

 HashTagEditCtrl pnlGlobal;
 HashTagEditCtrl pnlFolder;
 
 HashTagSelectCtrl SelectHashTagCtrl;

 PictureItem       *m_Picture;

};

class HashTagSubDlg : public ADialog
{
 public:

 HashTagSubDlg(HashTag *ht, FolderItem *fldr, AWnd *parent);

 std::vector<HashTag> PictureHashTags;

 DialogResult Show(AWnd *parent);

 protected:

 virtual void OnInitDialog();
 virtual WMR OnCommand(int id, HWND hCtrl);
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
 virtual void OnKeyDown(int child, KeyEventArgs const &k);

 void OnChoiceClicked(HashTag *ht);
 void OnSelectClicked(HashTag *ht);
 void OnSelectContext(HashTag *ht);
 void OnOK();
 void OnCancel();
 void OnAdd();


 HashTag        *EditTag;
 FolderItem     *Folder;

 HashTagSelectCtrl SubHashTagCtrl;
 HashTagSelectCtrl SelectHashTagCtrl;

 ButtonWnd m_OK;
 ButtonWnd m_Cancel;
 ButtonWnd m_Add;
 EditWnd   m_Text;

 AWnd *ParentWnd;

};

class SlideDlg : ADialog
{
 public:

 SlideDlg(String const &single, bool darkBackground);
 SlideDlg(std::vector<String> const &listImg, float fDelaySeconds, bool bMouseMove, bool darkBackground);
~SlideDlg();

 void Show();
 virtual void Close(DialogResult r); // true if ok to close, false to not close

 protected:

 void OnInitDialog();
 void PrepareImage(String const &file);

 virtual void OnMouseDown(MouseEventArgs const &m);
 virtual void OnMouseMove(MouseEventArgs const &m);
 virtual void OnKeyDown(KeyEventArgs const &k);
 virtual WMR  OnPaint();
 virtual void OnTimer(int timerID);

 Bitmap              *m_bmpNextImage;

 std::vector<String> m_listImages;

 int                 m_Tolerance;      // how far mouse can move to trigger cancel
 int                 m_nTotal;
 int                 m_nCurrent;

 String              m_strLastImage;

 float               m_Delay;          // "seconds" between pic

 bool                m_blnPause=false; // current state paused or not
 bool                m_MouseMove;      // if mouse moves close slide show otherwise must use keyboard
 bool                m_Single;         // only 1 image
 bool                m_DarkBackground; // if true background is black

 HFONT               m_hFont;
 HBITMAP             m_hBitmap;
 HBRUSH              m_hBG;

 int m_BW;
 int m_BH;

 const int ID_TIMER_1 = 101;
 const int ID_TIMER_2 = 102;
};

class PicturePropertiesDlg : public ADialog
{
 public:

 void Show(AWnd *parent, ImageParser *pic);

 protected:
 
 void OnInitDialog();
 virtual WMR OnCommand(int child, HWND hChildWnd);

 ImageParser *m_Pic;
};

class DirDlg : public ADialog
{
 public:
 enum class enumWhichDir : int
  {
   WorkingDir,
   ScreenSaver,
   NewFolder,
   GenericFolder
  };

 DirDlg(String const &dir, enumWhichDir eType);

 DialogResult Show(AWnd *parent);

 String Directory() { return m_strDirOut; } // full path
 
 protected:

 virtual void OnInitDialog();
 virtual WMR OnCommand(int id, HWND hCtrl);
 
 void OnOK();
 void OnCancel();
 void OnDir();

 enumWhichDir m_eType;

 String m_strDir;
 String m_strDirOut;
 String m_Title;

};

class HashTagMaintDlg : public ADialog
{
 public:

 HashTagMaintDlg();
 HashTagMaintDlg(FolderItem *fldr);

 void Show(AWnd *parent);

 private:

 void ItemClicked(HashTag *ht);
 void OnAdd();

 virtual void OnInitDialog();
 virtual WMR OnCommand(int id, HWND hCtrl);
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
 
 protected:

 FolderItem *m_Folder;
 
 HashTag *m_SelectedTag;
 
 HashTagSelectCtrl m_hashCtrl;
 
 int m_HTWidth;
 int m_HTHeight;


};

class GroupNameDlg : public ADialog
{
 public:

 GroupNameDlg(String const &suggestion, bool blankOK);

 DialogResult Show(AWnd *parent, String const &title);

 String GroupName() { return m_GroupName; }

 protected:

 void OnOK();

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);

 bool   m_blnBlankOK;
 String m_strSuggest;
 String  m_GroupName;
 String  m_Title;
};

class FileSortDlg : public ADialog
{
 public:

 DialogResult Show(AWnd *parent);

 ImageParser::SortChoices SortChoice() { return m_Sort; }
 bool Ascending() { return m_Ascending; }

 protected:
 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hWnd);
 void OnOK();
 
 ImageParser::SortChoices m_Sort;
 bool                     m_Ascending;
};

class BaseDirDlg : public ADialog
{
 public:

 DialogResult Show(AWnd *parent, BaseItem *item);
 
 String BaseName() { return m_Name; }
 String BasePath() { return m_Dir; }

 protected:

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);

 void OnOK();
 void OnBrowse();

 String m_Name;
 String m_Dir;
 
 BaseItem *m_Item;

};

class HashTagChooseDlg : public ADialog
{
 public:

 DialogResult Show(AWnd *parent); // global tags
 DialogResult Show(AWnd *parent, FolderItem *folder); // folder tags

 HashTag SelectedHashTag() { return m_SelectedTag; }

 protected:

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);
 virtual void OnListViewItemChanged(int child, NMLISTVIEW *plvn);
 virtual void OnListViewColumnClick(int child, int col){};
 virtual void OnDoubleClick(int child, NMITEMACTIVATE *pItemAct);

 ListView m_List;

 HashTag::HashTagType m_HashTagType;

 HashTag m_SelectedTag;
};
 

class HashTagSelectorDlg : public ADialog
{
 public:
 enum class eMode : int { PictureTags, FolderTags };

 HashTagSelectorDlg();

 DialogResult Show(AWnd *parent, FolderItem *fldr, eMode mode);
 DialogResult Show(AWnd *parent, FolderItem *fldr, PictureItem *pic);

 std::vector<HashTag> PictureHashTags;
 std::vector<HashTag> FolderHashTags;
 PictureItem         *Item() { return m_Item; }


 protected:

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);
 virtual void OnKeyDown(int child, KeyEventArgs const &k);
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 void OnFolderClicked(HashTag *ht);
 void OnSelectClicked(HashTag *ht);
 void OnSelectContext(HashTag *ht);
 void OnHashTagNameChange();


 void OnOK();
 void OnAdd();

 void FinishNew(FolderItem *fldr);

 HashTagSelectCtrl FolderHashTagCtrl;
 HashTagSelectCtrl SelectHashTagCtrl;
 FolderItem       *m_Folder;

 PictureItem *m_Item;

 eMode m_Mode;

 ButtonWnd btnOK;
 EditWnd   txtFolder;


};

class GroupSelectDlg : public ADialog
{
 public:

 DialogResult Show(AWnd *parent, std::vector<String> const &list);

 String GroupName() { return m_GroupName; }

 protected:

 virtual void OnInitDialog();
 virtual WMR  OnCommand(int child, HWND hChild);
 
 void OnOK();

 ListView m_List;
 String m_GroupName;
 std::vector<String> m_Groups;

};

class LanguageDlg : public ADialog
{
 public:

 DialogResult Show();
 ProseLanguage Language() { return m_Lang; }

 protected:

 virtual void OnInitDialog();
 WMR OnCommand(int child, HWND hChild);
 
 ADropList m_List;

 ProseLanguage m_Lang;
 
};