#pragma once

class GlobalSlideDlg : public ADialog
{
 public:
 GlobalSlideDlg();
 GlobalSlideDlg(std::map<int, FolderItem *> const &fldrs);

 void Show();

 protected:

 virtual void OnInitDialog();
 virtual void OnDropListChanged(int child, HWND hChild);
 virtual void OnListViewColumnClick(int child, int col);
 virtual bool OnClose();
 virtual WMR  OnCommand(int child, HWND hChild)
  {
   switch(child)
    {
     case IDC_GLOBAL_SLIDE_START:        OnShow();        break;
     case IDC_GLOBAL_SLIDE_CHOOSE_SETS:  OnChooseShow();  break;
     case IDC_GLOBAL_SLIDE_CHECK_ALL:    CheckAll(true);  break;
     case IDC_GLOBAL_SLIDE_CHECK_NONE:   CheckAll(false); break;
    }
   ShowHelp();
   return WMR::One;
  }

 void LoadItems();
 bool GetDelay();
 std::vector<FolderItem *>GetFolders();

 std::vector<String> GetFileList();
 void OnShow();
 void OnChooseShow();
 void ShowHelp();

 void CheckAll(bool onoff);

 bool KeyToStop()     { return GetCheckState(IDC_GLOBAL_SLIDE_CHK_KEYS); }
 bool Folders()       { return GetCheckState(IDC_GLOBAL_SLIDE_FOLDERS); }
 bool FoldersRandom() { return GetCheckState(IDC_GLOBAL_SLIDE_FOLDER_RND); }
 bool Sets()          { return GetCheckState(IDC_GLOBAL_SLIDE_SETS); }
 bool SetsRandom()    { return GetCheckState(IDC_GLOBAL_SLIDE_SETS_RND); }
 bool PicsRandom()    { return GetCheckState(IDC_GLOBAL_SLIDE_PICS_RND); }

 std::map<int, FolderItem *> m_Folders;
 HashTag m_HT;

 ListView  m_List;
 EditWnd   m_EditDelay;
 ADropList m_Globals;

 int     m_Sort;
 bool    m_SortAscend;
 HashTag m_GlobalHashTag;
 Random  m_Rnd;
 float   m_Delay;
 
};

class GlobalSlideChooseDlg : public ADialog
{
 public:

 DialogResult Show(AWnd *parent)
 {
  return ADialog::Show(IDD_GLOBAL_SLIDE_SETS, parent);
 }

 virtual void OnInitDialog()
  {
   listSets.Attach(this,IDC_GLOBAL_SLIDE_SETS_LIST);
   listSets.SetCheckBoxes(true);
   listSets.AddColumn(UI_COLUMN_FOLDER, ListViewColumn::ColumnAlign::Left, 20);
   listSets.AddColumn(UI_COLUMN_SET_NAME, ListViewColumn::ColumnAlign::Left, 20);
   listSets.AddColumn(UI_COLUMN_ITEMS, ListViewColumn::ColumnAlign::Right, 20);
   for(const auto &lvi : Items)
     listSets.Insert(lvi);
   listSets.AutoSize(0, ListView::AutoSizes::Content);
   listSets.AutoSize(1, ListView::AutoSizes::Content);
   listSets.AutoSize(2, ListView::AutoSizes::Header);

   ADialog::OnInitDialog();
  }

 virtual WMR OnCommand(int child, HWND hChild)
  {
   if (child == IDCANCEL) Close(DialogResult::Cancel);
   if (child == IDOK)
    {
     Items.clear();
     for(int i = 0; i< listSets.Count(); i++)
      {
       if (listSets.IsItemChecked(i) == true)
         CheckedItems.push_back(listSets.GetItem(i));
      }
     Close(DialogResult::OK);
    }
   return WMR::One;
  }

 std::vector<ListViewItem> Items;
 std::vector<ListViewItem> CheckedItems;
 ListView listSets;

};

class PItem
{
 public:
 PItem(FolderItem *fldr, String const &file, int s)
 {
  Folder = fldr;
  Path = file;
  int i = file.LastIndexOf('\\'); 
  Info=FileInfo(file.Substring(i+1));
  Sort = s;
 }

 FolderItem *Folder;
 FileInfo   Info;
 String     Path;
 int        Sort;
};

class PSet
{
 public:
 
 PSet()
  {
   Folder=nullptr;
   Sort = 0;
  }
 PSet(FolderItem *fldr, String const &key, int sort, String const &setName)
  {
   Folder=fldr;
   Key=key.Chars();
   SetName=setName;
   Sort=sort;
  }

 FolderItem   *Folder;
 std::wstring  Key;
 String        SetName;
 int           Sort;

 std::vector<PItem> Items;
};

class PSets
{
 public:
 PSets(){};
 PSets(std::vector<FolderItem *> fldrs);

 std::vector<String> GetItems(bool rndFldrs, bool rndItems, bool totally);
 std::vector<String> GetSets(bool rndFldrs, bool rndSets, bool totally, bool rndItems);

 std::map<String, PSet> Sets;
};


class PSetSorter
{
 public:

 enum class SortType : int { SortedFoldersSortedSets, SortedFoldersRandomSets, RandomFoldersSortedSets, RandomFoldersRandomSets, TotallyRandom };

 PSetSorter(SortType sort)
  {
   Sort = sort;
  }
 
 bool operator()(const PSet &a,const PSet &b); 

 private:

 int IntCompare(int x, int y)
  {
   if (x>y) return 1;
   if (x<y) return -1;
   return 0;
  }

 SortType Sort;
};

class PItemSorter 
{
 public:

 enum class SortType : int { SortedFoldersSortedItems, SortedFoldersRandomItems, RandomFoldersSortedItems, RandomFoldersRandomItems, TotallyRandom };

 PItemSorter(SortType sort) 
  {
   Sort = sort;
  }

 bool operator()(const PItem &a, const PItem &b); 

 private:

 int IntCompare(int x, int y)
  {
   if (x>y) return 1;
   if (x<y) return -1;
   return 0;
  }


 SortType Sort;

};

