#include "pch.h"
#include "App.h"


GlobalSlideDlg::GlobalSlideDlg()
{
 m_Sort = 0;
 m_SortAscend = true;
}

GlobalSlideDlg::GlobalSlideDlg(std::map<int, FolderItem *> const &fldrs)
{
 m_Sort = 0;
 m_SortAscend = true;
}

void GlobalSlideDlg::OnInitDialog()
{
 ListItem item;
 double delay;
 String msg;
 int lastID;
 int files;

 m_List.Attach(this, IDC_GLOBAL_SLIDE_LIST);
 m_List.SetCheckBoxes(true);
 FolderOpenDlg::LoadHeaders(m_List);
 
 m_EditDelay.Attach(this, IDC_GLOBAL_SLIDE_DELAY);
 m_EditFileCount.Attach(this, IDC_GLOBAL_SLIDE_FILES);
 m_Globals.Attach(this, IDC_GLOBAL_SLIDE_HT);

 SetCheckState(IDC_GLOBAL_SLIDE_CHK_KEYS, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_CHK_KEYS", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_FOLDERS, App->GetSettingBool(L"IDC_GLOBAL_FOLDERS", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_SETS, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_SETS", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_FOLDER_SORT, App->GetSettingBool(L"IDC_GLOBAL_FOLDER_SORT", 1) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_FOLDER_RND, App->GetSettingBool(L"IDC_GLOBAL_FOLDER_RND", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_SETS_SORT, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_SETS_SORT", 1) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_SETS_RND, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_SETS_RND", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_PICS_SORT, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_PICS_SORT", 0) == 1);
 SetCheckState(IDC_GLOBAL_SLIDE_PICS_RND, App->GetSettingBool(L"IDC_GLOBAL_SLIDE_PICS_RND", 0) == 1);

 ShowHelp();

 lastID = App->GetSettingInt(L"SlideShowGlobalTag", -1);

 for (const auto &ht : App->GlobalHashTags)
  {
   item = ListItem(ht.second.Name(), ht.second.ID());
   m_Globals.AddItem(item);
  }

 if (lastID >=0)
  {
   if (App->GlobalHashTags.count(lastID) == 0)
     m_HT = App->GetFavoriteHashTag();
   else
    {
     m_Globals.SetCurrentItem(lastID);
     m_HT = App->GlobalHashTags[lastID];
    }
  }
 else
  {
   m_Globals.SetCurSel(0); // system is guarenteed to have at least the "Favorite" hash tag
   m_HT = App->GetFavoriteHashTag();
  }

 LoadItems();

 m_List.SetCheckBoxes(true);

 delay=App->GetSettingDbl(L"SlideShowDelay", 30.0);
 if (delay < 0.5)
   delay = 30.0;

 m_EditDelay.SetText(String::Double(delay,3,2));

 files = App->GetSettingDbl(L"SlideShowFileCount", 1);
 if (files <= 0)
   files = 1;

 m_EditFileCount.SetText(String::Decimal(files));

 ADialog::OnInitDialog();
}

void GlobalSlideDlg::Show()
{
 ADialog::Show(IDD_GLOBAL_SLIDE, Wnd);
}

bool GlobalSlideDlg::OnClose()
{
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_CHK_KEYS", GetCheckState(IDC_GLOBAL_SLIDE_CHK_KEYS));
 App->SaveSettingBool(L"IDC_GLOBAL_FOLDERS", GetCheckState(IDC_GLOBAL_SLIDE_FOLDERS));
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_SETS", GetCheckState(IDC_GLOBAL_SLIDE_SETS));
 App->SaveSettingBool(L"IDC_GLOBAL_FOLDER_SORT", GetCheckState(IDC_GLOBAL_SLIDE_FOLDER_SORT));
 App->SaveSettingBool(L"IDC_GLOBAL_FOLDER_RND", GetCheckState(IDC_GLOBAL_SLIDE_FOLDER_RND));
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_SETS_SORT",GetCheckState(IDC_GLOBAL_SLIDE_SETS_SORT));
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_SETS_RND", GetCheckState(IDC_GLOBAL_SLIDE_SETS_RND));
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_PICS_SORT", GetCheckState(IDC_GLOBAL_SLIDE_PICS_SORT)); 
 App->SaveSettingBool(L"IDC_GLOBAL_SLIDE_PICS_RND", GetCheckState(IDC_GLOBAL_SLIDE_PICS_RND));

 return true;
}

void GlobalSlideDlg::OnDropListChanged(int child, HWND hChild)
{
 int id;

 if (child != IDC_GLOBAL_SLIDE_HT) throw L"Only 1 combo on dialog";

 id = m_Globals.GetSelectedItem().Tag;

 if (App->GlobalHashTags.count(id) == 0) throw L"Global hashtag ID not found";

 m_HT = App->GlobalHashTags[id];

 App->SaveSettingInt(L"SlideShowGlobalTag", m_HT.ID());

 LoadItems();
}

void GlobalSlideDlg::LoadItems()
{
 String msg;
 int i;

 m_Folders.clear();

 for(const auto &f : Wnd->CurrentBase()->Folders)
  {
   f.second->GetFileList(m_HT.ID());
   if (f.second->FileList.size() > 0) 
      m_Folders.insert(std::pair<int, FolderItem *>(f.second->ID(), f.second));
  }

 m_List.Clear();

 FolderOpenDlg::LoadView(m_List, m_Folders);

 i=0;
 for(const auto &f : m_Folders)
  {
   m_List.SetItemText(i++, 1, String::Decimal((int)f.second->FileList.size()));  // number of items
  }

 if (m_Folders.size()==0)
  {
   if (m_HT.TagType() != HashTag::HashTagType::None )
    {
     msg = L"Warning, no folders found containing items found with favorite ";
     msg += m_HT.Name();
    }
   else
    {
     msg = L"Warning, no folders found";
    }
   App->Response(msg);
  }
}

bool GlobalSlideDlg::GetDelay()
{
 double f;
 bool g;

 g = String::TryDblParse(m_EditDelay.GetText(), &f);
   
 if (g == true)
  {
   if (f < 0.5)
     g = false;
  }
 if (g == false)
  {
   App->Response(L"Delay cannot be less than 1/2 second");
   return false;
  }

 App->SaveSettingDbl(L"SlideShowDelay", (double)f);

 m_Delay = f;
 return true;
}

bool GlobalSlideDlg::GetFileCount()
{
 int f;
 bool g;

 g = String::TryIntParse(m_EditFileCount.GetText(), &f);
   
 if (g == true)
  {
   if (f < 1)
     g = false;
  }
 if (g == false)
  {
   App->Response(L"File count cannot be less than 1");
   return false;
  }

 App->SaveSettingInt(L"SlideShowFileCount", f);

 m_Files = f;
 return true;
}

 
std::vector<FolderItem *> GlobalSlideDlg::GetFolders()
{
 Random rnd;

 FolderItem *item;
 std::vector<FolderItem *> fldrs;
 int tag, i;

 for(i = 0; i < m_List.Count(); i++)
  {
   if ( m_List.IsItemChecked(i) == true)
    {
     tag = m_List.GetItemParam(i);
     if (tag < 0) throw L"tag is bad";
     if (m_Folders.count(tag) == 0) throw L"folders doesn't contain id";
     item=m_Folders[tag];
     item->Random=rnd.Next(); // for random folders
     fldrs.push_back(item);
    }
  }

 if (fldrs.size()==0)
   App->Response(L"No folders for slide show are checked");
    
 return fldrs;
}

void GlobalSlideDlg::OnListViewColumnClick(int child, int col)
{
 if (m_Sort == col)
   m_SortAscend = !m_SortAscend;
 else
  {
   m_Sort = col;
   m_SortAscend = true;
  }                          
 m_List.Sort(m_Sort, m_SortAscend, FolderOpenDlg::CompareFolders); 
}

std::vector<String> GlobalSlideDlg::GetFileList()
{
 PSets sets;
 std::vector<FolderItem *> fldrs;
 std::vector<String> files;
 
 fldrs=GetFolders();

 if (fldrs.size()==0)
   return files;

 sets = PSets(fldrs);

 if (Sets() == true)
   files=sets.GetSets(FoldersRandom(), SetsRandom(), !Folders(), PicsRandom(), m_Files);
 else
   files=sets.GetItems(FoldersRandom(), PicsRandom(), !Folders());

 if (files.size()==0)
  {
   App->Response(L"No files found");
  }
 return files;
}

void GlobalSlideDlg::OnShow()
{
 std::vector<String> files;
 bool dark;

 if (GetDelay()==false)
   return;

 if (GetFileCount()==false)
   return;

 files = GetFileList();
 if (files.size() == 0)
   return;

 if (GetCheckState(IDC_GLOBAL_SLIDE_CHK_LIGHT) == true)
   dark = false;
 else
   dark = true;

 SlideDlg dlg(files, m_Delay, !KeyToStop(), dark);
 dlg.Show();

}

void GlobalSlideDlg::ShowHelp()
{
 String msg = L"If Folder Is Checked: \n\n";

 if (Sets() == true)
  {
   if (Folders() == true)  
    {
     if (FoldersRandom() == true) 
       msg += L"Sorted First By Random Folders\n";
     else 
       msg +=L"Sorted First By Sorted Folders\n";
    }
   if (SetsRandom() == true)
     msg += L"Showing Random Sets";
   else
     msg += L"Showing Sorted Sets";    
   if (PicsRandom() == true)
     msg += L" Random Pictures";
   else
     msg += L" Sorted Pictures";
  }
 else
  {
   if (Folders() == true)
    {
     if (FoldersRandom() == true) 
       msg += L"Sorted First By Random Folders\n";
     else 
       msg +=L"Sorted First By Sorted Folders\n";
     if (PicsRandom() == true)
       msg += L"Showing Random Pictures";
     else
       msg += L"Showing Sorted Pictures";
    }
   else
    {
     if (PicsRandom() == true)
       msg += L"Showing Totally Random Pictures";
     else
       msg += L"Showing Completely Sorted Pictures";
    }
  }
 SetItemText(IDC_GLOBAL_SLIDE_HELP, msg);
}

void GlobalSlideDlg::OnChooseShow()
{
 GlobalSlideChooseDlg dlg;
 DialogResult r;
 PSets sets, newSets;
 ListViewItem item;
 std::vector<FolderItem *> fldrs;
 std::vector<String> files;
 std::vector<String> final;
 std::vector<String> setNames;
 bool dark;

 if (GetDelay()==false)
   return;

 if (GetFileCount()==false)
   return;

 fldrs=GetFolders();

 if (fldrs.size()==0)
   return;

 sets=PSets(fldrs);

 if (sets.Sets.size()==0)
  {
   App->Response(L"There are no sets to choose from");
   return;
  }
 
 for(const auto &set : sets.Sets)
  {
   if (set.second.Items.size() >= m_Files)
    {
     item = ListViewItem(set.second.Folder->Folder());
     item.SubItems.push_back(set.second.SetName);
     item.SubItems.push_back(String::Decimal((int)set.second.Items.size()));
     dlg.Items.push_back(item);
    }
  }
 if (dlg.Items.size() > 0)
  {
   r=dlg.Show(Wnd);
   if (r != DialogResult::OK)
    {
     return;
    }
  }
 else
  {
   App->Response(L"No sets match the slideshow criteria");
   return;
  }

 if (dlg.CheckedItems.size()==0)
  {
   App->Response(L"No set names checked in list");
   return;
  }
 
 newSets=PSets();

 for(const auto &ps : sets.Sets)
  {
   for(const auto &lvi : dlg.CheckedItems)
    {
     if (lvi.SubItems[0] == ps.second.Folder->Folder() && lvi.SubItems[1] == ps.second.SetName)
      {
       newSets.Sets.insert(std::pair<std::wstring, PSet>(ps.second.Key, ps.second));
       break;
      }
    }
  }
 if (Sets() == true)
   files=newSets.GetSets(FoldersRandom(), SetsRandom(), !Folders(), PicsRandom(), m_Files);
 else
   files=newSets.GetItems(FoldersRandom(), PicsRandom(), !Folders() && PicsRandom());

 if (files.size()==0)
  {
   App->Response(L"No files found");
   return;
  }

 if (GetCheckState(IDC_GLOBAL_SLIDE_CHK_LIGHT) == true)
   dark = false;
 else
   dark = true;

 SlideDlg sdlg(files, m_Delay, !KeyToStop(), dark);
 sdlg.Show();
}

void GlobalSlideDlg::CheckAll(bool onoff)
{
 int i;

 for(i=0; i < m_List.Count(); i++)
  {
   m_List.SetItemChecked(i, onoff);
  }
}

//////////////////////////////////////////////////////////

PSets::PSets(std::vector<FolderItem *> fldrs)
{
 Random       rnd;
 FileInfo     info;
 std::wstring key;

 // find sets that contain more than one item

 for(const auto &fldr : fldrs)
  {
   for(const auto &item : fldr->FileList)
    {
     info=FileInfo(item);
     if (info.IsNumbered == true)
      {
       key=info.Dir.Chars();
       key += L"\\";
       key += info.FilePrefix.Chars();
       if (Sets.count(key)==false) 
         Sets.insert(std::pair<std::wstring, PSet>(key, PSet(fldr, key, rnd.Next(),info.FilePrefix)));
       Sets[key].Items.push_back(PItem(fldr, item, rnd.Next()));  // numbered item, gather all into one set
      }
     else
      {
       key=info.Dir.Chars();
       key += L"\\";
       key += info.FileNoExt.Chars();
       Sets.insert(std::pair<std::wstring, PSet>(key, PSet(fldr, key, rnd.Next(), info.FileNoExt)));
       Sets[key].Items.push_back(PItem(fldr, item, rnd.Next()));  // non numbered item, make a set for each non grouped item
      }
    }
  }
}

std::vector<String> PSets::GetItems(bool rndFldrs, bool rndItems, bool totally)
{
 std::vector<PItem> items;
 std::vector<String> list;
 PItemSorter::SortType sort; 

 for(const auto &ps : Sets)
  {
   for(const auto &pi : ps.second.Items)
    {
     items.push_back(pi);
    }
  }
 
 if (totally == true)
  {
   sort = PItemSorter::SortType::TotallyRandom;
  }
 else
  {
   if (rndFldrs)
    {
     if (rndItems)
       sort = PItemSorter::SortType::RandomFoldersRandomItems;
     else
       sort = PItemSorter::SortType::RandomFoldersSortedItems;
    }
   else
    {
     if (rndItems)
       sort=PItemSorter::SortType::SortedFoldersRandomItems;
     else
       sort=PItemSorter::SortType::SortedFoldersSortedItems;
    }
  }
 std::sort(items.begin(), items.end(), PItemSorter(sort));

 for(const auto &pi : items)
  {
   list.push_back(pi.Path);
  }

 return list;
}

std::vector<String> PSets::GetSets(bool rndFldrs, bool rndSets, bool totally, bool rndItems, int fileCount)
{
 std::vector<PSet> sets;
 std::vector<String> list;
 PSetSorter::SortType sort;

 for(const auto &ps : Sets)
  {
   sets.push_back(ps.second);
  }

 if (totally)
   sort=PSetSorter::SortType::TotallyRandom;
 else 
  {
   if (rndFldrs)
    {
     if (rndSets)
       sort=PSetSorter::SortType::RandomFoldersRandomSets;
     else
       sort=PSetSorter::SortType::RandomFoldersSortedSets;
    }
   else
    {
     if (rndSets)
       sort=PSetSorter::SortType::SortedFoldersRandomSets;
     else
       sort=PSetSorter::SortType::SortedFoldersSortedSets;
    }
  }
 
 std::sort(sets.begin(), sets.end(), PSetSorter(sort));

 for(auto &set : sets)
  {
   if (rndItems)
     std::sort(set.Items.begin(), set.Items.end(), PItemSorter(PItemSorter::SortType::TotallyRandom));
   if (set.Items.size() >= fileCount)
    {
     for(const auto &pi : set.Items)
       list.push_back(pi.Path);
    }
  }
 return list;
}

//////////////////////////////////////////////////////////////////

bool PSetSorter::operator ()(const PSet &sx, const PSet &sy)
{
 int c;

 switch(Sort)
  {
   case SortType::SortedFoldersSortedSets:
     {
      c=String::Compare(sx.Folder->Folder(), sy.Folder->Folder());
      if (c!=0)
        return c < 0;
      else
        return String::Compare(sx.SetName, sy.SetName) < 0;
     }
   case SortType::SortedFoldersRandomSets:
     {
      c=String::Compare(sx.Folder->Folder(), sy.Folder->Folder());
      if (c!=0)
        return c < 0;
      else
        return IntCompare(sx.Sort, sy.Sort);
     }
   case SortType::RandomFoldersSortedSets:
     {
      c=IntCompare(sx.Folder->Random, sy.Folder->Random);
      if (c!=0)
        return c<0;
      else
        return String::Compare(sx.SetName, sy.SetName) < 0;
     } 
   case SortType::RandomFoldersRandomSets:
     {
      c=IntCompare(sx.Folder->Random, sy.Folder->Random);
      if (c!=0)
        return c<0;
      else
        return IntCompare(sx.Sort, sy.Sort) < 0;
     } 
   case SortType::TotallyRandom:
     {
      return IntCompare(sx.Sort, sy.Sort) < 0;
     }
    default: throw L"switch error";
  }
}

bool PItemSorter::operator() (const PItem &ix, const PItem &iy)
{
 int c;

 switch(Sort)
  {
   case SortType::SortedFoldersSortedItems:
     {
      c=String::Compare(ix.Folder->Folder(), iy.Folder->Folder());
      if (c!=0)
        return c < 0;
      else
        return String::Compare(ix.Info.FullPath, iy.Info.FullPath) < 0;
     }
   case SortType::SortedFoldersRandomItems:
     {
      c=String::Compare(ix.Folder->Folder(), iy.Folder->Folder());
      if (c!=0)
        return c < 0;
      else
        return IntCompare(ix.Sort, iy.Sort) < 0;
     }
   case SortType::RandomFoldersSortedItems:
     {
      c=IntCompare(ix.Folder->Random, iy.Folder->Random);
      if (c!=0)
        return c < 0;
      else
        return String::Compare(ix.Info.FullPath, iy.Info.FullPath) < 0;
     } 
   case SortType::RandomFoldersRandomItems:
     {
      c=IntCompare(ix.Folder->Random, iy.Folder->Random);
      if (c!=0)
        return c < 0;
      else
        return IntCompare(ix.Sort, iy.Sort) < 0;
     } 
   case SortType::TotallyRandom:
     {
      return IntCompare(ix.Sort, iy.Sort) < 0;
     }
    default: throw L"switch error";
  }
}