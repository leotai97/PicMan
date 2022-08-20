#include "pch.h"
#include "app.h"

MainWnd::MainWnd()
{
 m_ShowingFileGroups = true;
 m_ShowingNotInTree = true;
}

MainWnd::~MainWnd()
{
 m_Menu.DestroyMenu();
}

void MainWnd::Register(String const &wndcls)
{
 WNDCLASSEX wcex={0};

 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style          = CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc    = PopUpWnd::PopUpWndProc;
 wcex.cbClsExtra     = 0;
 wcex.cbWndExtra     = 0;
 wcex.hInstance      = App->Instance();
 wcex.hIcon          = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_PICMAN));
 wcex.hCursor        = LoadCursor(0, IDC_ARROW);
 wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
 wcex.lpszMenuName   = 0;
 wcex.lpszClassName  = wndcls.Chars();
 wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

 if (::RegisterClassEx(&wcex)==0)
   throw L"Register failed";

 App->AddCustomClass(wndcls);
}

bool MainWnd::Create(String const &wndcls, int nCmdShow)
{
 String s;
 int i;

 PopUpWnd::Create(wndcls, nCmdShow); //SW_MAXIMIZE);

 LoadMainMenu();

 m_PanelItems=Rect(5,5,200,100);
 m_PanelPics=Rect(208,5, 200, 100);
 m_SplitMain.Create(SPLIT_MAIN, this, SplitterWnd::SplitterOrientation::Vertical, 0.50);
 m_SplitMain.Show(); 

 m_TabList.Create(this, Rect(5, 5, 80, 100));

 m_ListPics.Create(&m_TabList, LVS_REPORT, Rect(5, 5, 60, 90));
 m_ListPics.SetOwner(this);
 m_TabList.AddTab(L"Runtime", &m_ListPics);
 m_ListPics.SetOwner(this);
 m_ListImport.Create(&m_TabList, LVS_REPORT, Rect(5, 5, 60, 90));
 m_ListImport.SetOwner(this);
 m_TabList.AddTab(App->Prose.Text(MAIN_IMPORTS), &m_ListImport);
 m_ListImport.Visible(false);
 m_ListImport.SetOwner(this);
 m_TabList.Show();
 
 m_TreeGroup.Create(this, Rect(0,0, 40, 100));
 m_TreeTags.Create(this, Rect(0,0,40,100));

 m_SplitItems.Create(SPLIT_ITEMS, this, SplitterWnd::SplitterOrientation::Vertical, 0.50);
 m_SplitItems.SetWindow1(&m_TabList);
 m_SplitItems.SetWindow2(nullptr); // will be sizing the tree and it's hashtag filter separately
 m_SplitItems.Show();
 m_TreeGroup.Show();

 m_PicList.Create(this, Rect(0,0,50,50));
 m_PicTree.Create(this, Rect(0,58,50,100));
 m_SplitPics.Create(SPLIT_PICS, this, SplitterWnd::SplitterOrientation::Horizontal, 0.50);
 m_SplitPics.SetWindow1(&m_PicList);
 m_SplitPics.SetWindow2(&m_PicTree);

 m_Status.AddAutoPane(StatusBarPane::Content::Text);
 m_Status.AddFixedPane(StatusBarPane::Content::Progress, 190);
 m_Status.Create(this);
 
 m_PicList.Show();
 m_PicTree.Show();
 m_SplitPics.Show();

 m_ImageThumbs.AddClient(&m_ListPics);
 m_ImageThumbs.AddClient(&m_ListImport);

 ::SetWindowPos(m_SplitMain.Handle(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
 ::SetWindowPos(m_SplitPics.Handle(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
 ::SetWindowPos(m_SplitItems.Handle(), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

 SizeChildren();

 i = App->GetSettingInt(L"RootDirectoryID");
 if (i > 0)
  {
   if (App->Bases.count(i) > 0)
    {
     m_CurrentBase = App->Bases.at(i);
     m_CurrentBase->Load();
     if (m_CurrentBase->GetDir() == false)
      {
       App->Response(App->Prose.Text(MAIN_BASE_OPEN_FAIL));
       m_CurrentBase = nullptr;
      }
    else
     {
      s =  L"Picture Manager --- ";
      s += m_CurrentBase->DirPath();
      ::SetWindowText(m_hWnd, s.Chars());
     }
   }
 }

 m_CurrentImportDir = App->Options.GetSetting(L"LastImport");
 if (m_CurrentImportDir.Length()>0) 
  {
   ReScanImports();
   ReloadImageList();
   RefreshImports();
  }
 LoadLastFolders();

 mnuViewSetGroupViewFileNames(); // initially view File Groups

 m_Status.SetText(0, App->Prose.Text(MAIN_GREETING));
 
 return true;
}

void MainWnd::LoadMainMenu()
{
 MenuItem *top;
 MenuItem *sub;

 // Menu Text imported from ProseData.dat

 m_Menu.CreateWindowMenu();

 top = m_Menu.SubMenu(ID_FILE);
 top->AddMenu(ID_FILE_SETBASEDIRECTORY);
 top->Separator();
 top->AddMenu(ID_FILE_NEWFOLDER);
 top->AddMenu(ID_FILE_OPENFOLDER);
 top->Separator();

 sub = top->SubMenu(ID_FILE_ACTIONS);
 sub->AddMenu(ID_FILE_ACTIONS_MAINTAINFOLDERHASHTAGS);
 sub->AddMenu(ID_FILE_ACTIONS_RELOADFOLDER);
 sub->AddMenu(ID_FILE_ACTIONS_RELOADFOLDERTHUMBNAILS);
 sub->AddMenu(ID_FILE_ACTIONS_RENUMBERFILESINFOLDER);
 sub->AddMenu(ID_FILE_ACTIONS_RENAMEFILESTOHASHTAGS);

 top->Separator();
 top->AddMenu(ID_FILE_CLOSEFOLDER);
 top->Separator();
 top->AddMenu(ID_FILE_SCANBASEFORNEWFOLDERS);
 top->AddMenu(ID_FILE_REFRESHIMPORTLIST);
 top->Separator();
 top->AddMenu(ID_FILE_EXIT);
 top->Separator();
 top->AddMenu(ID_FILE_P1);
 top->AddMenu(ID_FILE_P2);
 top->AddMenu(ID_FILE_P3);
 top->AddMenu(ID_FILE_P4);
 top->AddMenu(ID_FILE_P5);
 top->AddMenu(ID_FILE_P6);
 top->AddMenu(ID_FILE_P7);
 top->AddMenu(ID_FILE_P8);

 top = m_Menu.SubMenu(ID_VIEW);
 top->AddMenu(ID_VIEW_BASEDIRECTORYPROPERTIES);
 top->AddMenu(ID_VIEW_FOLDERPROPERTIES);
 
 sub = top->SubMenu(ID_VIEW_SORTLIST);

 sub->AddMenu(ID_VIEW_SORTLIST_NAME);
 sub->AddMenu(ID_VIEW_SORTLIST_NAME_NMBR);
 sub->AddMenu(ID_VIEW_SORTLIST_WIDTH);
 sub->AddMenu(ID_VIEW_SORTLIST_HEIGHT);
 sub->AddMenu(ID_VIEW_SORTLIST_WIDTHDIVHEIGHT);
 sub->AddMenu(ID_VIEW_SORTLIST_RGBCOLORS);
 sub->AddMenu(ID_VIEW_SORTLIST_BORDERCOLORAVERAGE);
 sub->AddMenu(ID_VIEW_SORTLIST_DATEADDED);
 sub->AddMenu(ID_VIEW_SORTLIST_SIZE);
 sub->AddMenu(ID_VIEW_SORTLIST_GHT);
 sub->AddMenu(ID_VIEW_SORTLIST_FHT);

 top->AddMenu(ID_VIEW_REFRESHLISTANDTREE);
 top->Separator();
 top->AddMenu(ID_VIEW_COLLAPSEALLGROUPS);
 top->AddMenu(ID_VIEW_EXPANDALLGROUPS);
 top->AddMenu(ID_VIEW_VIEWALLGROUPS);
 top->Separator();
 top->AddMenu(ID_VIEW_VIEWALLPICTURESINFOLDER);
 top->AddMenu(ID_VIEW_VIEWALLPICTURESANDIMPORTS);
 top->AddMenu(ID_VIEW_VIEWIMPORTS);
 top->AddMenu(ID_VIEW_VIEWALLDUPLICATES);
 top->AddMenu(ID_VIEW_VIEWBYGLOBALHASHTAG);
 top->Separator();
 top->AddMenu(ID_VIEW_VIEWSLIDESHOW);
 top->AddMenu(ID_VIEW_VIEWALLBYGLOBALHASHTAG);

 sub = top->SubMenu(ID_VIEW_SETLISTVIEW);
 
 sub->AddMenu(ID_VIEW_SETLISTVIEW_DETAILS, true, true, 0);
 sub->AddMenu(ID_VIEW_SETLISTVIEW_ICONS);
 sub->AddMenu(ID_VIEW_SETLISTVIEW_NOT_IN_TREE, true, true, 0);
 sub->AddMenu(ID_VIEW_SETLISTVIEW_SHOW_ALL);

 sub = top->SubMenu(ID_VIEW_SETGROUPVIEW);

 sub->AddMenu(ID_VIEW_SETGROUPVIEW_FILENAMES, true, true, 0);
 sub->AddMenu(ID_VIEW_SETGROUPVIEW_HASHTAGS);

 top = m_Menu.SubMenu(ID_TOOL);
 top->AddMenu(ID_TOOL_SETPHOTOEDITOR);
 top->AddMenu(ID_TOOL_DBCONNECTION);
 top->AddMenu(ID_TOOL_MAINTAINGLOBALHASHTAGS);
 top->AddMenu(ID_TOOL_SETIMPORTDIRECTORY);
 
 top = m_Menu.SubMenu(ID_HELP);
 top->AddMenu(ID_HELP_ABOUT);
 
 SetMenu(m_Menu);
}

Rect MainWnd::SplitterRect(HWND hSplit)
{
 if (hSplit == m_SplitMain.Handle()) return ClientRect();
 if (hSplit == m_SplitPics.Handle()) return m_PanelPics;
 if (hSplit == m_SplitItems.Handle()) return m_PanelItems;
 throw L"HWND not found"; 
}

Point MainWnd::SplitterCursor(HWND hSplit)
{
 Rect rct;
 POINT pt;

 ::GetCursorPos(&pt);
 ::ScreenToClient(m_hWnd, &pt);   // Splitter control calls this, it can't move itself
 rct=ClientRect();

 if (hSplit == m_SplitMain.Handle()) 
  {
   return Point(pt.x - rct.X, pt.y - rct.Y);
  }
 if (hSplit == m_SplitPics.Handle())
  {
   return Point(pt.x - m_PanelPics.X, pt.y - m_PanelPics.Y);
  }
 if (hSplit == m_SplitItems.Handle())
  {
   return Point(pt.x - m_PanelItems.X, pt.y - m_PanelItems.Y);
  }
 throw L"HWND not found"; 
}

void MainWnd::OnSplitterChanged(HWND hSplit, int splitContainer)
{
 switch(splitContainer)
  {
   case SPLIT_MAIN: 
     SizeChildren();
     break;
   case SPLIT_ITEMS:
     SizeTree();
     break;
  }
}

void MainWnd::OnSize()
{
 if (m_SplitMain.Handle()==0)
   return;

 if (Minimized() == true)
   return; // don't mess with sizing if minimized

 SizeChildren();
}

void MainWnd::SizeChildren()
{
 Rect r=ClientRect();

 m_SplitMain.CalcSizes(r); 

 m_PanelItems=m_SplitMain.GetRect1();
 m_SplitItems.CalcSizes(m_PanelItems);

 SizeTree();  // the tree and it's hash tag filter are sized below

 m_PanelPics=m_SplitMain.GetRect2();
 m_SplitPics.CalcSizes(m_PanelPics);

 m_Status.OnSize(r); 

 Refresh();
}

void MainWnd::SizeTree()
{
 Rect rp = m_SplitItems.GetRect2();
 Rect r;

 m_TreeGroup.SetRect(rp);
 m_TreeTags.SetRect(rp);

 if (m_ShowingFileGroups == true)
  {
   m_TreeTags.Visible(false);
   m_TreeGroup.Visible(true);
  }
 else
  {
   m_TreeTags.Visible(true);
   m_TreeGroup.Visible(false);
  }
}

bool MainWnd::OnClosing()
{
 return true;
}

void MainWnd::OnPaint()
{
 PAINTSTRUCT ps;
 RECT rct;
 HDC hDC;
 
 hDC = ::BeginPaint(m_hWnd, &ps);
  {
   ::GetClientRect(m_hWnd, &rct);
   ::FillRect(hDC, &rct, ::GetSysColorBrush(COLOR_WINDOW));
   ::FrameRect(hDC, &rct, ::GetSysColorBrush(COLOR_WINDOW));
  }
 ::EndPaint(m_hWnd, &ps);
}

WMR MainWnd::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 NMLISTVIEW *nlv; 
 NMHDR *nm=(LPNMHDR)lParam;
 WMR ret=WMR::Zero;
 int id;

 ret = PopUpWnd::OnNotify(hChild, child, code, lParam);

 if (hChild == m_TabList.Handle())
  {
   m_TabList.OnNotify(hChild, child, code, lParam);
  }

 switch(nm->code)
  {
   case LVN_COLUMNCLICK:
    {
     if (hChild == m_ListPics.Handle()) m_ListPics.OnNotify(hChild, child, code, lParam);
     if (hChild == m_ListImport.Handle()) m_ListImport.OnNotify(hChild, child, code, lParam);
    } break;
   case TCN_SELCHANGE:
    {
     m_TabList.OnTabChange();  // only have 1 tab control
    } break;
   case NM_DBLCLK:
    {
     if (hChild == m_ListPics.Handle())
       mnuPopUpListViewPicture();
     if (hChild == m_ListPics.Handle())
       mnuPopUpImportViewPicture();
     if (hChild == m_TreeGroup.Handle())
       mnuPopUpTreeGroupPicView();
     if (hChild == m_TreeTags.Tree.Handle())
       mnuPopUpTreeTagsItemView();
    } break;
   case LVN_ITEMCHANGED:
    {
     if (hChild == m_ListPics.Handle() && m_ListPics.NotifyIsOff() == false)
      {
       nlv = (NMLISTVIEW *)lParam;
       if (m_ListPics.IsItemSelected(nlv->iItem) == true)
        {
         id = m_ListPics.GetItemParam(nlv->iItem); 
         SetPictureViewer(&m_PicList, App->Pictures[id]);
        }
      }
     if (hChild == m_ListImport.Handle() && m_ListImport.NotifyIsOff() == false)
      {
       nlv = (NMLISTVIEW *)lParam;
       if (m_ListImport.IsItemSelected(nlv->iItem) == true)
        {
         id = m_ListImport.GetItemParam(nlv->iItem); 
         SetPictureViewer(&m_PicList, App->Pictures[id]);
        }
      }
    } break;
   case TVN_SELCHANGED:
    {
     if (hChild == m_TreeTags.Tree.Handle())
      {
       if (m_TreeTags.Tree.Clearing() == false)
         SetPictureViewer(&m_PicTree, m_TreeTags.Tree.SelectedPicture());
      }
     if (hChild == m_TreeGroup.Handle())
      {
       if (m_TreeGroup.Clearing() == false)
         SetPictureViewer(&m_PicTree, m_TreeGroup.SelectedPicture());
      }
    } break;
  }

 return ret;
}

WMR MainWnd::OnContextMenu(HWND hChild, Point const &pt)
{ 
 WMR ret = WMR::Zero;

 if (hChild == m_TabList.Handle())
  {
   if (m_TabList.GetCurrentTab() == 0)
     ShowMenuListPics(pt);
   else
     ShowMenuListImport(pt); 
   ret = WMR::One;
  }
 if (hChild == m_TreeTags.Tree.Handle())
  {
   ShowMenuTreeHashTags(pt);
   ret = WMR::One;
  }
 if (hChild == m_TreeGroup.Handle())
  {
   ShowMenuTreeGroup(pt);
   ret = WMR::One;
  }
 return ret;
}

void MainWnd::OnListViewMiddle(int id)
{
 ImageParser *px;

 if (App->Pictures.count(id) == 0) throw L"id not valid";

 px = App->Pictures[id];

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

// Context Menus

void MainWnd::ShowMenuListPics(Point const &pt)
{
 AMenu menu;
 MenuItem *sub;

 if (m_CurrentFolder == nullptr)
   return;

 menu.CreateContextMenu();

 menu.AddMenu(ID_POPUP_LIST_ADD_SELECTED_GROUP);
 menu.AddMenu(ID_POPUP_LIST_MOVE_NEW_GROUP);
 menu.AddMenu(ID_POPUP_LIST_REPLACE_FOLDER_HASHTAG);
 menu.AddMenu(ID_POPUP_LIST_ADD_FOLDER_HASHTAG);
 menu.AddMenu(ID_POPUP_LIST_ADD_TO_SELECTED_GROUP_HASHTAG);
 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_DELETE_SELECTED_FILES);
 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_SELECT_ALL);
 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_VIEW_SELECTED_ITEMS);
 menu.AddMenu(ID_POPUP_LIST_VIEW_PICTURE);
 
 sub = menu.SubMenu(ID_POPUP_LIST_SORTBY);
 
 PicListView::AddSortMenus(sub, m_ListPics.CurrentSort);

 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_EDIT);
 menu.AddMenu(ID_POPUP_LIST_PROPERTIES);

 if (m_ShowingFileGroups == true)
  {
   menu.SetEnabledState(ID_POPUP_LIST_REPLACE_FOLDER_HASHTAG, false);
   menu.SetEnabledState(ID_POPUP_LIST_ADD_FOLDER_HASHTAG, false);
   menu.SetEnabledState(ID_POPUP_LIST_ADD_TO_SELECTED_GROUP_HASHTAG, false);
  }
 else
  {
   menu.SetEnabledState(ID_POPUP_LIST_ADD_SELECTED_GROUP, false);
   menu.SetEnabledState(ID_POPUP_IMPORT_MOVE_NEW_GROUP, false);
  }
 
 menu.ShowContextMenu(this, pt);
}

void MainWnd::ShowMenuListImport(Point const &pt)
{
 AMenu menu;
 MenuItem *subMenu;

 if (m_ListImport.Count() == 0)
   return;

 menu.CreateContextMenu();

 menu.AddMenu(ID_POPUP_IMPORT_ADD_SELECTED_GROUP);
 menu.AddMenu(ID_POPUP_IMPORT_MOVE_NEW_GROUP);
 menu.AddMenu(ID_POPUP_IMPORT_MOVE_LIST);
 
 subMenu = menu.SubMenu(ID_POPUP_IMPORT_SORTBY);
 PicListView::AddSortMenus(subMenu, m_ListImport.CurrentSort); 

 menu.Separator();
 menu.AddMenu(ID_POPUP_IMPORT_DELETE_SELECTED);
 menu.Separator();
 menu.AddMenu(ID_POPUP_IMPORT_SELECT_ALL);
 menu.Separator();
 menu.AddMenu(ID_POPUP_IMPORT_PICVIEW);
 menu.AddMenu(ID_POPUP_IMPORT_PROPERTIES);

if (m_ShowingFileGroups == false)
 {
  menu.SetEnabledState(ID_POPUP_IMPORT_ADD_SELECTED_GROUP, false);
  menu.SetEnabledState(ID_POPUP_IMPORT_MOVE_NEW_GROUP, false);
 }

 menu.ShowContextMenu(this, pt);
 menu.DestroyMenu();
}

void MainWnd::ShowMenuTreeGroup(Point const &pt)
{
 AMenu menu;

 menu.CreateContextMenu();

 menu.AddMenu(ID_TREEGROUP_MANAGE_GROUP);
 menu.AddMenu(ID_TREEGROUP_VIEW_GROUP);
 menu.AddMenu(ID_TREEGROUP_FAVORITE);
 menu.AddMenu(ID_TREEGROUP_UNFAVORITE);
 menu.Separator();
 menu.AddMenu(ID_TREEGROUP_RENAME_GROUP);
 menu.AddMenu(ID_TREEGROUP_RENUMBER);
 menu.AddMenu(ID_TREEGROUP_MOVE_GROUP_BACK);
 menu.Separator();
 menu.AddMenu(ID_TREEGROUP_PIC_BACK_TO_LIST);
 menu.AddMenu(ID_TREEGROUP_PIC_MOVE_TO_ANOTHER);
 menu.Separator();
 menu.AddMenu(ID_TREEGROUP_DELETE_GROUP);
  menu.AddMenu(ID_TREEGROUP_PIC_DELETE);
 menu.Separator();
 menu.AddMenu(ID_TREEGROUP_COLLAPSE);
 menu.Separator();
 menu.AddMenu(ID_TREEGROUP_VIEW_PIC);
 menu.AddMenu(ID_TREEGROUP_PIC_PROPS);

 menu.ShowContextMenu(this, pt);
 menu.DestroyMenu();
}

void MainWnd::ShowMenuTreeHashTags(Point const &pt)
{
 AMenu menu;

 menu.CreateContextMenu();

 menu.AddMenu(ID_TREETAGS_FAVORITE);
 menu.AddMenu(ID_TREETAGS_UNFAVORITE);
 menu.Separator();
 menu.AddMenu(ID_TREETAGS_GROUP_TAGS_UPDATE);
 menu.AddMenu(ID_TREETAGS_GROUP_TAGS_REMOVE);
 menu.AddMenu(ID_TREETAGS_GROUP_VIEW);
 menu.AddMenu(ID_TREETAGS_GROUP_DELETE);
 menu.Separator();
 menu.AddMenu(ID_TREETAGS_ITEM_TAGS_UPDATE);
 menu.AddMenu(ID_TREETAGS_ITEM_TAGS_REMOVE);
 menu.AddMenu(ID_TREETAGS_ITEM_VIEW);
 menu.AddMenu(ID_TREETAGS_ITEM_PROPS);
 menu.AddMenu(ID_TREETAGS_ITEM_EDIT);
 menu.AddMenu(ID_TREETAGS_ITEM_DELETE);

 menu.ShowContextMenu(this, pt);
 menu.DestroyMenu();
}

WMR MainWnd::OnMenuOpening(HMENU hMenu, int pos, bool windowMenu)
{
 WMR ret = WMR::Zero;
 int id;

 id = m_Menu.GetHandle(hMenu);
 if (id < 0)
   return ret;

 switch(id)
  {
   case ID_FILE:         OnOpenFile();
   case ID_FILE_ACTIONS: OnOpenFileActions();
   case ID_VIEW:         OnOpenView();
  }

 return ret;
}

void MainWnd::OnOpenFile()
{
 m_Menu.SetEnabledState(ID_FILE_CLOSEFOLDER, m_CurrentFolder != nullptr);
}

void MainWnd::OnOpenFileActions()
{
 bool enabled;

 enabled = (m_CurrentFolder != nullptr);

 m_Menu.SetEnabledState(ID_FILE_ACTIONS_MAINTAINFOLDERHASHTAGS, enabled);
 m_Menu.SetEnabledState(ID_FILE_ACTIONS_RELOADFOLDER, enabled);
 m_Menu.SetEnabledState(ID_FILE_ACTIONS_RELOADFOLDERTHUMBNAILS, enabled);
 m_Menu.SetEnabledState(ID_FILE_ACTIONS_RENAMEFILESTOHASHTAGS, enabled);
 m_Menu.SetEnabledState(ID_FILE_ACTIONS_RENUMBERFILESINFOLDER, enabled);
}

void MainWnd::OnOpenView()
{
 bool base = (m_CurrentBase != nullptr);
 bool fldr = (m_CurrentFolder != nullptr);

 m_Menu.SetEnabledState(ID_VIEW_BASEDIRECTORYPROPERTIES, base);
 m_Menu.SetEnabledState(ID_VIEW_COLLAPSEALLGROUPS, fldr);
 m_Menu.SetEnabledState(ID_VIEW_EXPANDALLGROUPS, fldr);
 m_Menu.SetEnabledState(ID_VIEW_FOLDERPROPERTIES, fldr);
 m_Menu.SetEnabledState(ID_VIEW_REFRESHLISTANDTREE, fldr);

 m_Menu.SetEnabledState(ID_VIEW_VIEWALLGROUPS, fldr);
 m_Menu.SetEnabledState(ID_VIEW_VIEWALLPICTURESINFOLDER, fldr);
 m_Menu.SetEnabledState(ID_VIEW_VIEWBYGLOBALHASHTAG, fldr);
}

// Message Hander

WMR MainWnd::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 String stat;
 int ndx, id;

 switch(msg)
  {
   case WM_HTSC_FILTER_CHANGED:
    {
     if (m_ShowingFileGroups == false)
      {
       RefreshHashTagGroupTree();
       m_PicTree.SetItem(nullptr);
      } 
    } break;
   case WM_PICVIEW_HT_CHANGED: // picture item hashtags changed, refresh list and tree
    {
     if (m_ShowingFileGroups == false)
       RefreshBoth();
    } break;
   case WM_PICLIST_HT_CHANGED: 
    {
     if (m_PicList.Item() != nullptr) m_PicList.RefreshHashTags();
    } break;
   case WM_PICLIST_ITEMDELETED:
     {
      stat = String::Decimal((int)lParam);
      if (hWnd == m_ListImport.Handle())
        stat+= App->Prose.Text(MAIN_IMPORTS);
      stat += App->Prose.Text(MAIN_MOVE_RECYCLE);
      m_Status.SetText(0, stat);
      m_PicList.SetItem(nullptr);
     } break;
   case WM_LISTVIEW_MIDDLE:  // ListView middle mouse button clicked
    {
     ndx = (int)lParam;
     if ((HWND)wParam == m_ListPics.Handle())
      {
       id = m_ListPics.GetItemParam(ndx);
      }
     else
      {
      id = m_ListImport.GetItemParam(ndx);
      }
     OnListViewMiddle(id);
    } break;
   default: 
     return PopUpWnd::MessageHandler(hWnd, msg, wParam, lParam);
  }
 return WMR::Zero;
}

// Menu Commands

WMR MainWnd::MenuHandler(int wmId)
{
 switch(wmId)
  {
   case ID_FILE_SETBASEDIRECTORY: mnuFileSetBaseDirectory(); break;
   case ID_FILE_NEWFOLDER:        mnuFileNewFolder();        break;
   case ID_FILE_OPENFOLDER:       mnuFileOpenFolder();       break;
   case ID_FILE_ACTIONS_MAINTAINFOLDERHASHTAGS: mnuFileActionsMaintainFolderHashtags(); break;
   case ID_FILE_ACTIONS_RELOADFOLDER:           mnuFileActionsReloadFolder(); break;
   case ID_FILE_ACTIONS_RELOADFOLDERTHUMBNAILS: mnuFileActionsReloadFolderThumbnails(); break;
   case ID_FILE_ACTIONS_RENUMBERFILESINFOLDER:  mnuFileActionsRenumberFilesInFolder(); break;
   case ID_FILE_ACTIONS_RENAMEFILESTOHASHTAGS:  mnuFileActionsRenameFilesToHashtags(); break;
   case ID_FILE_CLOSEFOLDER:           mnuFileCloseFolder(); break;
   case ID_FILE_SCANBASEFORNEWFOLDERS: mnuFileScanBaseForNewFolders(); break;
   case ID_FILE_REFRESHIMPORTLIST:     mnuFileRefreshImportList(); break;
   case ID_FILE_EXIT: mnuFileExit(); break;
   case ID_FILE_P1:   mnuFileP1(); break;
   case ID_FILE_P2:   mnuFileP2(); break;
   case ID_FILE_P3:   mnuFileP3(); break;
   case ID_FILE_P4:   mnuFileP4(); break;
   case ID_FILE_P5:   mnuFileP5(); break;
   case ID_FILE_P6:   mnuFileP6(); break;
   case ID_FILE_P7:   mnuFileP7(); break;
   case ID_FILE_P8:   mnuFileP8(); break;

   case ID_VIEW_BASEDIRECTORYPROPERTIES : mnuViewBaseDirectoryProperties(); break;
   case ID_VIEW_FOLDERPROPERTIES: mnuViewFolderProperties(); break;

   case ID_VIEW_SORTLIST_NAME:    mnuViewSortListName(); break;
   case ID_VIEW_SORTLIST_NAME_NMBR: mnuViewSortListNameNmbr(); break;
   case ID_VIEW_SORTLIST_WIDTH: mnuViewSortListWidth(); break;
   case ID_VIEW_SORTLIST_HEIGHT: mnuViewSortListHeight(); break;
   case ID_VIEW_SORTLIST_WIDTHDIVHEIGHT: mnuViewSortListWidthDivHeight(); break;
   case ID_VIEW_SORTLIST_RGBCOLORS: mnuViewSortListRGBColors(); break;
   case ID_VIEW_SORTLIST_BORDERCOLORAVERAGE: mnuViewSortListBorderColorAverage(); break;
   case ID_VIEW_SORTLIST_DATEADDED: mnuViewSortListDateAdded(); break;
   case ID_VIEW_SORTLIST_SIZE: mnuViewSortListFileSize(); break;
   case ID_VIEW_SORTLIST_GHT: mnuViewSortListGlobalHashTags(); break;
   case ID_VIEW_SORTLIST_FHT: mnuViewSortListFolderHashTags(); break;

   case ID_VIEW_REFRESHLISTANDTREE: mnuViewRefreshListAndTree(); break;
   case ID_VIEW_COLLAPSEALLGROUPS: mnuViewCollapseAllGroups(); break;
   case ID_VIEW_EXPANDALLGROUPS: mnuViewExpandAllGroups(); break;
   case ID_VIEW_VIEWALLGROUPS: mnuViewAllGroups(); break;
   case ID_VIEW_VIEWALLPICTURESINFOLDER: mnuViewAllPicturesInFolder(); break;
   case ID_VIEW_VIEWALLPICTURESANDIMPORTS: mnuViewAllPicturesAndImports(); break;
   case ID_VIEW_VIEWIMPORTS: mnuViewImports(); break;
   case ID_VIEW_VIEWALLDUPLICATES: mnuViewAllDuplicates(); break;
   case ID_VIEW_VIEWBYGLOBALHASHTAG: mnuViewFolderByGlobalHashtags(); break;
   case ID_VIEW_VIEWSLIDESHOW: mnuViewSlideShow(); break;
   case ID_VIEW_VIEWALLBYGLOBALHASHTAG: mnuViewAllByGlobalHashtag(); break;
   case ID_VIEW_SETLISTVIEW_ICONS: mnuViewSetListViewIcons(); break;
   case ID_VIEW_SETLISTVIEW_DETAILS: mnuViewSetListViewDetails(); break;
   case ID_VIEW_SETLISTVIEW_NOT_IN_TREE: mnuViewSetListViewNotInTree(); break;
   case ID_VIEW_SETLISTVIEW_SHOW_ALL: mnuViewSetListViewShowAll(); break;
   case ID_VIEW_SETGROUPVIEW_HASHTAGS: mnuViewSetGroupViewHashtags(); break;
   case ID_VIEW_SETGROUPVIEW_FILENAMES: mnuViewSetGroupViewFileNames(); break;

   case ID_TOOL_SETPHOTOEDITOR: mnuToolsSetPhotoEditor(); break;
   case ID_TOOL_DBCONNECTION:   mnuToolsDBConnection(); break;
   case ID_TOOL_MAINTAINGLOBALHASHTAGS: mnuToolsMaintainGlobalHashtags(); break;
   case ID_TOOL_SETIMPORTDIRECTORY: mnuToolsSetImportDirectory(); break;

   case IDM_ABOUT: OnHelpAbout(); break;

   case ID_SORT_BY_FILENAME:         mnuListSortByFileName(); break;
   case ID_SORT_BY_NAME_WITH_NUMBER: mnuListSortByNameNumber(); break;
   case ID_SORT_BY_WIDTH:            mnuListSortByWidth(); break;
   case ID_SORT_BY_HEIGHT:           mnuListSortByHeight(); break;
   case ID_SORT_BY_RATIO:            mnuListsSortByRatio(); break;
   case ID_SORT_BY_COLOR_RGB:        mnuListSortByColor(); break;
   case ID_SORT_BY_EDGE_AVERAGE:     mnuListSortByEdgeAvg(); break;
   case ID_SORT_BY_FILESIZE:         mnuListSortByFileSize(); break;
   case ID_SORT_BY_FILEDATE:         mnuListSortByFileDate(); break;
   case ID_SORT_BY_GLOBALTAGS:       mnuListSortByGlobalTags(); break;
   case ID_SORT_BY_FOLDERTAGS:       mnuListSortByFolderTags(); break;

   // context menus

   case ID_POPUP_LIST_ADD_SELECTED_GROUP:            mnuPopUpListAddToSelectedGroup(); break;
   case ID_POPUP_LIST_MOVE_NEW_GROUP:                mnuPopUpListMoveToNewGroup(); break;
   case ID_POPUP_LIST_REPLACE_FOLDER_HASHTAG:        mnuPopUpListReplaceFolderHashTag(); break;
   case ID_POPUP_LIST_ADD_FOLDER_HASHTAG:            mnuPopUpListAddFolderHashTag(); break;
   case ID_POPUP_LIST_ADD_TO_SELECTED_GROUP_HASHTAG: mnuPopUpListAddToSelectedGroupHashTag(); break;
   case ID_POPUP_LIST_DELETE_SELECTED_FILES:         mnuPopUpListDeleteSelectedFiles(); break;
   case ID_POPUP_LIST_SELECT_ALL:                    mnuPopUpListSelectAll(); break;
   case ID_POPUP_LIST_VIEW_SELECTED_ITEMS:           mnuPopUpListViewSelectedItems(); break;
   case ID_POPUP_LIST_VIEW_PICTURE:                  mnuPopUpListViewPicture(); break;
   case ID_POPUP_LIST_EDIT:                          mnuPopUpListPictureEdit(); break;
   case ID_POPUP_LIST_PROPERTIES:                    mnuPopUpListPictureProperties(); break;

   case ID_POPUP_IMPORT_ADD_SELECTED_GROUP:    mnuPopUpImportAddToSelectedGroup(); break;
   case ID_POPUP_IMPORT_MOVE_NEW_GROUP:        mnuPopUpImportMoveToNewGroup(); break;
   case ID_POPUP_IMPORT_MOVE_LIST:             mnuPopUpImportMoveToList(); break;
   case ID_POPUP_IMPORT_DELETE_SELECTED:       mnuPopUpImportDeleteSelectedFiles(); break;
   case ID_POPUP_IMPORT_SELECT_ALL:            mnuPopUpImportSelectAll(); break;
   case ID_POPUP_IMPORT_PICVIEW:               mnuPopUpImportViewPicture(); break;
   case ID_POPUP_IMPORT_PROPERTIES:            mnuPopUpImportPictureProperties(); break;

   case ID_TREEGROUP_MANAGE_GROUP:        mnuPopUpTreeGroupManage(); break;
   case ID_TREEGROUP_VIEW_GROUP:          mnuPopUpTreeGroupView(); break;
   case ID_TREEGROUP_FAVORITE:            mnuPopUpTreeGroupFavorite(); break;
   case ID_TREEGROUP_UNFAVORITE:          mnuPopUpTreeGroupUnFavorite(); break;
   case ID_TREEGROUP_RENAME_GROUP:        mnuPopUpTreeGroupRename(); break;
   case ID_TREEGROUP_RENUMBER:            mnuPopUpTreeGroupRenumber(); break;
   case ID_TREEGROUP_MOVE_GROUP_BACK:     mnuPopUpTreeGroupMoveBackToList(); break;
   case ID_TREEGROUP_VIEW_PIC:            mnuPopUpTreeGroupPicView(); break;
   case ID_TREEGROUP_PIC_BACK_TO_LIST:    mnuPopUpTreeGroupPicBackToList(); break;
   case ID_TREEGROUP_PIC_MOVE_TO_ANOTHER: mnuPopUpTreeGroupPicMoveToAnother(); break;
   case ID_TREEGROUP_DELETE_GROUP:        mnuPopUpTreeGroupDelete(); break;
   case ID_TREEGROUP_PIC_DELETE:          mnuPopUpTreeGroupPicDelete(); break;
   case ID_TREEGROUP_COLLAPSE:            mnuPopUpTreeGroupCollapse(); break;
   case ID_TREEGROUP_PIC_PROPS:           mnuPopUpTreeGroupPicProps(); break;

   case ID_TREETAGS_FAVORITE:             mnuPopUpTreeTagsFavorite(); break;
   case ID_TREETAGS_UNFAVORITE:           mnuPopUpTreeTagsUnFavorite(); break;
   case ID_TREETAGS_GROUP_TAGS_REMOVE:    mnuPopUpTreeTagsGroupTagsRemove(); break;
   case ID_TREETAGS_GROUP_TAGS_UPDATE:    mnuPopUpTreeTagsGroupTagsUpdate(); break;
   case ID_TREETAGS_GROUP_VIEW:           mnuPopUpTreeTagsGroupView(); break;
   case ID_TREETAGS_GROUP_DELETE:         mnuPopUpTreeTagsGroupDelete(); break;
   case ID_TREETAGS_ITEM_TAGS_REMOVE:     mnuPopUpTreeTagsItemTagsRemove(); break;
   case ID_TREETAGS_ITEM_TAGS_UPDATE:     mnuPopUpTreeTagsItemTagsUpdate(); break;
   case ID_TREETAGS_ITEM_DELETE:          mnuPopUpTreeTagsItemDelete(); break;
   case ID_TREETAGS_ITEM_VIEW:            mnuPopUpTreeTagsItemView(); break;
   case ID_TREETAGS_ITEM_PROPS:           mnuPopUpTreeTagsItemProps(); break;
   case ID_TREETAGS_ITEM_EDIT:            mnuPopUpTreeTagsItemEdit(); break;

   default: return WMR::Zero;
  }
 return WMR::One;
}

// File Menu Handlers

void MainWnd::mnuFileSetBaseDirectory()
{
 BaseItemMgrDlg dlg(m_CurrentBase);
 String text;
 
 if (dlg.Show() == DialogResult::OK)
  {
   m_CurrentBase=dlg.GetBaseItem();
   App->SaveSettingInt(L"RootDirectoryID", m_CurrentBase->ID());
   text = L"Picture Manager --- ";
   text += m_CurrentBase->DirPath();
   ::SetWindowText(m_hWnd, text.Chars());  
    m_CurrentBase->Load();
   //  LoadLastFolders();
  }
}

void MainWnd::mnuFileNewFolder()
{
 DirDlg dlg(m_CurrentBase->DirPath(), DirDlg::enumWhichDir::NewFolder);
 DialogResult r;
 String msg, strDir, strFin;
 FolderItem *item;


 if (m_CurrentBase == nullptr)
  {
   App->Response(MAIN_SET_BASE);
   return;
  }

 if (Utility::DirectoryExists(m_CurrentBase->DirPath())==false)
  {
   App->Response(App->Prose.TextArgs(MAIN_BASE_FIX, m_CurrentBase->DirPath(), m_Menu.GetMenuTree(ID_VIEW_BASEDIRECTORYPROPERTIES)));
   return;
  }
    
 r = dlg.Show(this);
 if (r == DialogResult::OK)
   strDir = dlg.Directory();
 else
   return;

 strFin = m_CurrentBase->DirPath();
 strFin += L"\\";
 strFin += strDir;

 if (Utility::DirectoryExists(strFin)==false)
  {
   if (Utility::DirectoryCreate(strFin, true)==false)
    {
     return;
    }
  } 

 item = new FolderItem(strDir, m_CurrentBase);

 OpenFolder(item);
}

void MainWnd::mnuFileOpenFolder()
{
 FolderOpenDlg dlg;
 DialogResult r;
 String msg; 
   
 if (m_CurrentBase == nullptr)
  {
   App->Response(MAIN_SET_BASE);
   return;
  }

 if (Utility::DirectoryExists(m_CurrentBase->DirPath())==false)
  {
   App->Response(App->Prose.TextArgs(MAIN_BASE_FIX, m_CurrentBase->DirPath(), m_Menu.GetMenuTree(ID_VIEW_BASEDIRECTORYPROPERTIES)));
   return;
  }

 r = dlg.Show();
 if ( r == DialogResult::OK)
  {
   if (m_CurrentFolder != nullptr)
     CloseFolder();
   OpenFolder(dlg.CurrentFolder());
  }
}

void MainWnd::mnuFileActionsMaintainFolderHashtags()
{
 if (m_CurrentFolder == nullptr) { App->Response(MAIN_FOLDER_OPEN); return; }
  
 HashTagMaintDlg dlg(m_CurrentFolder);
 dlg.Show(this);
 if (m_ShowingFileGroups==false)
   RefreshBoth();
 
 m_PicList.RefreshHashTags();
 m_PicTree.RefreshHashTags();
}

void MainWnd::mnuFileActionsReloadFolder()
{
 std::vector<ImageParser *> list;
 WaitCursor wait;

 if (m_CurrentFolder == nullptr)
  {
   App->Response(MAIN_FOLDER_OPEN);
   return;
  }
 if (App->Question(MAIN_FOLDER_RELOAD, MB_OKCANCEL) != DialogResult::OK)
   return;

 m_ListPics.Clear();
 m_TreeGroup.Clear();
 m_TreeTags.Tree.Clear();
 m_ListImport.Clear();

 list = App->ClearAndGetImports(); // get list of imports and clear the dictionary
 App->ResetPictureID();            // fresh set of picture id's

 for (const auto &p : list)
  {
   p->SetID(App->NextPictureID());  // give import a new id
   App->Pictures.insert(std::pair<int, ImageParser *>(p->ID(), p)); // imports re-added to map
  }

 ReScanImports(); // load any new jpgs in incoming dir into App->Pictures

 m_CurrentFolder->ResetPictures();   // delete all the database instances

 list = ProcessFolder(m_CurrentFolder);  // rescan, get an imageparser, for all the jpgs in the folder
 if (list.size() == 0) 
   return;

 if (list.size() > 0)
  {
   for(const auto &p : list)
    {
     App->Pictures.insert(std::pair<int, ImageParser *>(p->ID(), p));
    }
   m_CurrentFolder->ProcessPictures(list); // add all the folder jpgs to the database
  }

 ReloadImageList();   // put all Thumbs in App->Pictures into ImageList
 RefreshImports();    // put all IsImport images in App->Pictures into m_ListImport    
 RefreshBoth();       // put non import items into m_ListPics and either m_TreePics or m_TreeHashTags

}

void MainWnd::mnuFileActionsReloadFolderThumbnails()
{
 std::vector<std::thread> threads;
 String msg;
 int i, msgs;
 ProgressBar pgb(&m_Status,1);
 WaitCursor wait;

 if (m_CurrentFolder == nullptr) 
  { 
   App->Response(MAIN_FOLDER_OPEN); 
   return; 
  }

 if (App->Question(App->Prose.TextArgs(MAIN_FOLDER_RELOAD_THUMB, m_CurrentFolder->Folder()), MB_OKCANCEL)!=DialogResult::OK)
   return;
 
 m_Status.SetText(0, L"Reloading Thumbnails");

 for(const auto &pi : m_CurrentFolder->Pictures)
   pi.second->Picture()->ResetReady();
  
 for(const auto &pi : m_CurrentFolder->Pictures)
  {
   threads.push_back(std::thread(ImageParser::ReprocessImageProc, pi.second->Picture()));
  }

 pgb.Max(m_CurrentFolder->Pictures.size());
 
 i = 0;
 for (auto &thread : threads)
  {
   pgb.Progress();
   thread.join();
  }

 pgb.Max(m_CurrentFolder->Pictures.size());
 msgs=0;
 i=0;
 for(const auto &pi : m_CurrentFolder->Pictures)
  {
   if (pi.second->Picture()->Good() == true)
     pi.second->UpdateThumbnail();
   else
    {
     if (msgs<5)
      {
       msg = pi.second->FileName();
       msg += App->Prose.Text(COMMON_FAILED_LOAD);
       msg += L"\n";
       msg += pi.second->Picture()->Error();
       App->Response(msg);
       msgs++;
      }
     else
      {
       App->Response(MAIN_PICTURES_FAILED_LOAD);
      }
    }
   pgb.Progress();
  }
}

void MainWnd::mnuFileActionsRenumberFilesInFolder()
{
 DialogResult res;
 String strPrefix;
 std::vector<ImageParser *> list;
 std::vector<String> listDir;
 ImageParser::SortChoices sort;
 bool bAsc;
 String msg;
 int i;

 if (m_CurrentFolder == nullptr)
  {
   App->Response(MAIN_FOLDER_OPEN);
   return;
  }

 if (Utility::DirectoryExists(m_CurrentFolder->FolderPath()) == false)
  {
   App->Response(App->Prose.TextArgs(MAIN_BASE_FIX, m_CurrentBase->DirPath(), m_Menu.GetMenuTree(ID_VIEW_BASEDIRECTORYPROPERTIES)));
   return;
  }

 listDir = Utility::GetFileNames(m_CurrentFolder->FolderPath(), L"*.ord");
 if (listDir.size() > 0)
  {
   msg = m_CurrentFolder->Folder();
   msg += L" ";
   msg += App->Prose.Text(MAIN_ORD_EXTENSION); // contains files with the .ord extension which is reserved for renaming files without name conflicts. The .ord files should be renamed to jpg to see if there are any wanted images.";
   App->Response(msg);
   return;
  }

 GroupNameDlg dlg(L"", true);
 res = dlg.Show(this, App->Prose.Text(MAIN_PREFIX_MESSAGE));
 strPrefix = dlg.GroupName();
 if (res != DialogResult::OK)
   return;
 
 FileSortDlg dlgSort;
 if (dlgSort.Show(this) == DialogResult::OK)
  {
   sort = dlgSort.SortChoice();
   bAsc = dlgSort.Ascending();
  }
 else
   return;

 for(const auto &p : App->Pictures)
  {
   if (p.second->IsImport()==false)
     list.push_back(p.second);
  }
 std::sort(list.begin(), list.end(), ImageParserSorter(sort, bAsc));

 i = 1;
 for(const auto &p : list)
  {
   p->RenameByPartAndSeqToOrd(strPrefix, i); // temporary rename to .ord  (order)
   i++;
  }
 i = 1;

 for(const auto &p : list)
  {
   p->RenameByPartAndSeqDash(strPrefix, i); // rename with dash instead of underline so they don't form a group.....
   i++;
  }

 RefreshBoth();
}

void MainWnd::mnuFileActionsRenameFilesToHashtags()
{
 std::map<String, HashTagSet> listUnique;
 std::map<String, int> dictPics;
 std::vector<ImageParser *> listMisc;
 std::vector<ImageParser *> listRenumber;
 std::vector<ImageParser *> listAll;
 String strHTGroup;
 String msg;
 std::vector<String> sort;
 bool blnFound;
 int i;

 if (m_CurrentFolder==nullptr) 
  { 
   App->Response(MAIN_FOLDER_OPEN); 
   return; 
   }

 for(const auto &px : App->Pictures)
  {
   if (px.second->IsImport() == false)
     listAll.push_back(px.second);
  }

 if (listAll.size()==0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 if (App->Question(App->Prose.TextArgs(MAIN_RENAME_TO_HASHTAGS, m_CurrentFolder->Folder()), MB_YESNO) != DialogResult::Yes) 
   return;

 std::sort(listAll.begin(), listAll.end(), ImageParserSorter(ImageParser::SortChoices::FileName, true));
 for(const auto &px : listAll)
  {
   if (px->Item()->PictureHashTags.size() == 0)
     listRenumber.push_back(px);
   else
    {
     sort.clear();
     i = 0;
     for(const auto &pht : px->Item()->PictureHashTags)
      {
       sort.push_back(pht.second.ToString());
       i++;
      }
     std::sort(sort.begin(), sort.end(), StringSort);
     strHTGroup.Clear();
     for (const auto &str : sort)
      {
       if (strHTGroup.Length()>0)
         strHTGroup += L",";
       strHTGroup += str;
      }
     blnFound = false;
     for(auto &hts : listUnique)
      {
       if (hts.second.Tags == strHTGroup) 
        {
         hts.second.Pictures.push_back(px);
         blnFound = true;
         break; // exit for
        }
      }
     if (blnFound == false)
      {
       listUnique.insert(std::pair<String, HashTagSet>(strHTGroup, HashTagSet(strHTGroup, px)));
      }
    }
  }
 if (listUnique.size() == 0)
  { 
   App->Response(MAIN_HASHTAGS_NOT_FOUND); 
   return; 
  }

 // change entire set to .ord to make sure there are no possible conflicts during renumbering

 for(const auto &hts : listUnique)
  {
   i = 1;
   for(const auto &px : hts.second.Pictures)
    {
     px->RenameByPartAndSeqToOrd(hts.second.Tags, i); // temporary rename to .ord  (order)
     i++;
    }
  }
 i = 1;
 for(const auto &px : listRenumber)
  {
   px->RenameBySeqToOrd(i);
   i++;
  }

 // change all ord back to jpg

 for(const auto &hts : listUnique)
  {
   i = 1;
   for(const auto &px : hts.second.Pictures)
    {
     px->RenameByPartAndSeq(hts.second.Tags, i); // rename back to .jpg
     i++;
    }
  }
 i = 1;
 for(const auto &px : listRenumber)
  {
   px->RenameBySeq(i);
   i++;
  }
 RefreshBoth();
}

void MainWnd::mnuFileCloseFolder()
{
 if (m_CurrentFolder != nullptr)
   CloseFolder();
}

void MainWnd::mnuFileScanBaseForNewFolders()
{
 ReScanDirectory();
}

void MainWnd::mnuFileRefreshImportList()
{
 std::vector<ImageParser *> keep;
 String msg;

 if (m_CurrentImportDir.Length() == 0)
  {
   App->Response(App->Prose.TextArgs(MAIN_IMPORT_SET, m_Menu.GetMenuTree(ID_TOOL_SETIMPORTDIRECTORY)));
   return;
  }
 if (Utility::DirectoryExists(m_CurrentImportDir) == false)
  {
   App->Response(App->Prose.TextArgs(MAIN_IMPORT_CHANGE, m_CurrentImportDir, m_Menu.GetMenuTree(ID_TOOL_SETIMPORTDIRECTORY)));
   return;
  }

 for(const auto &p : App->Pictures)
  {
   if (p.second->IsImport()==false)
    {
     keep.push_back(p.second);
    }
   else
    {
     p.second->Dispose();
     delete p.second;
    }
  }

 App->Pictures.clear();

 for(const auto &p : keep)
  {
   App->Pictures.insert(std::pair<int, ImageParser *>(p->ID(), p));
  }

 ProcessImportDirectory(m_CurrentImportDir);

 ReloadImageList();
 RefreshImports();
}

void MainWnd::mnuFileExit()
{
 Close();
}

void MainWnd::mnuFileP1()
{
 OnFilePrevious(0);
}
void MainWnd::mnuFileP2()
{
 OnFilePrevious(1);
}
void MainWnd::mnuFileP3()
{
 OnFilePrevious(2);
}
void MainWnd::mnuFileP4()
{
 OnFilePrevious(3);
}
void MainWnd::mnuFileP5()
{
 OnFilePrevious(4);
}
void MainWnd::mnuFileP6()
{
 OnFilePrevious(5);
}
void MainWnd::mnuFileP7()
{
 OnFilePrevious(6);
}
void MainWnd::mnuFileP8()
{
 OnFilePrevious(7);
}

void MainWnd::OnFilePrevious(int ndx)
{
 FolderItem *fldr;
 int id;

 if (m_PreviousList.size()>ndx && m_CurrentBase !=nullptr)
  {
   id = m_PreviousList[ndx];
   if (m_CurrentBase->Folders.count(id) > 0)
    {
     fldr=m_CurrentBase->Folders.at(id);
     OpenFolder(fldr);
    }
  }
}

// View Menu Handlers

void MainWnd::mnuViewBaseDirectoryProperties()
{
 BaseDirDlg dlg;
 DialogResult r;

 if (m_CurrentBase == nullptr)
  {
   App->Response(MAIN_SET_BASE);
   return;
  }

 r = dlg.Show(this, m_CurrentBase);
 if (r == DialogResult::OK)
  {
   m_CurrentBase->Save(dlg.BaseName(), dlg.BasePath());
  }
}

void MainWnd::mnuViewFolderProperties()
{
 String msg;

 if (m_CurrentFolder == nullptr)
  {
   App->Response(MAIN_FOLDER_OPEN);
   return;
  }

 msg = m_CurrentFolder->Folder();
 msg += L" ";
 msg += String::Decimal((int)m_CurrentFolder->Pictures.size());
 msg += L" jpgs";
 App->Response(msg);
}

void MainWnd::mnuViewSortListName()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileName);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileName);
}

void MainWnd::mnuViewSortListNameNmbr()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::NameWithNumber);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::NameWithNumber);
}

void MainWnd::mnuViewSortListWidth()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Width);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Width);
}
void MainWnd::mnuViewSortListHeight()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Height);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Height);
}
void MainWnd::mnuViewSortListWidthDivHeight()
{
if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Ratio);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Ratio);
}

void MainWnd::mnuViewSortListRGBColors()
{
 if (m_TabList.GetCurrentTab() == 0)
  m_ListPics.SetSort(ImageParser::SortChoices::ColorRGB);
 else
  m_ListImport.SetSort(ImageParser::SortChoices::ColorRGB);
}
void MainWnd::mnuViewSortListBorderColorAverage()
{
if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::EdgeAverage);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::EdgeAverage);
}
void MainWnd::mnuViewSortListDateAdded()
{
if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileDate);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileDate);
}

void MainWnd::mnuViewSortListFileSize()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileSize);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileSize);
}

void MainWnd::mnuViewSortListGlobalHashTags()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::GlobalHashTags);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::GlobalHashTags);
}

void MainWnd::mnuViewSortListFolderHashTags()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FolderHashTags);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FolderHashTags);
}

void MainWnd::mnuViewRefreshListAndTree()
{
 RefreshBoth();
}

void MainWnd::mnuViewCollapseAllGroups()
{
 if (m_ShowingFileGroups == true)
   m_TreeGroup.CollapseAll();
 else
   m_TreeTags.Tree.CollapseAll();
}

void MainWnd::mnuViewExpandAllGroups()
{
if (m_ShowingFileGroups == true)
   m_TreeGroup.ExpandAll();
 else
   m_TreeTags.Tree.ExpandAll();
}

void MainWnd::mnuViewAllGroups()
{
 GroupShowWnd wnd;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> rootNodes;
 std::vector<TreeNode> groupNodes;
 ImageParser *px; 

 if (m_ShowingFileGroups == true)
   rootNodes = m_TreeGroup.Nodes();
 else
   rootNodes = m_TreeTags.Tree.Nodes();

 for (const auto &root : rootNodes)
  {
   groupNodes = root.GetNodes();
   for (const auto &item : groupNodes)
    {
     #ifdef _DEBUG
     if (App->Pictures.count(item.Tag) > 0) throw L"Tag has to be ImageParser ID";   
     #endif
     px = App->Pictures[item.Tag];
     list.push_back(px);
    }
  }

 if (list.size() == 0)
  {
   App->Response(COMMON_NO_ITEMS_FOUND);
   return;
  }
    
 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::FilterFolder);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}

void MainWnd::mnuViewAllPicturesInFolder()
{
 GroupShowWnd wnd;
 std::vector<ImageParser *> list;

 for(const auto &px : App->Pictures)
  {
   if (px.second->IsImport() == false)
     list.push_back(px.second);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::FilterFolder);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}

void MainWnd::mnuViewAllPicturesAndImports()
{
GroupShowWnd wnd;
 std::vector<ImageParser *> list;

 for(const auto &px : App->Pictures)
  {
   list.push_back(px.second);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}

void MainWnd::mnuViewImports()
{
GroupShowWnd wnd;
 std::vector<ImageParser *> list;

 for(const auto &px : App->Pictures)
  {
   if (px.second->IsImport() == true)
     list.push_back(px.second);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
   RefreshImports();
}

void MainWnd::mnuViewAllDuplicates()
{
 GroupShowWnd wnd;
 std::map<int, ImageParser *> dups;
 std::vector<ImageParser *> listIn, listOut;
 int i;

 for(const auto &p : App->Pictures)
  {
   listIn.push_back(p.second);
  }

 i = 0;
 for(const auto &p : listIn)
  {
   for(const auto  &pc : listIn)
    {
     if (p->ID() != pc->ID())
      {
       if (p->Equal(pc) == true)
        {
         if (dups.count(p->ID()) == 0)  dups.insert(std::pair<int, ImageParser *>(p->ID(), p));
         if (dups.count(pc->ID()) == 0) dups.insert(std::pair<int, ImageParser *>(pc->ID(), pc));
        }
      }
    }
   i += 1;
  }
 if (dups.size() == 0) { App->Response(MAIN_DUPLICATES_NOT_FOUND); return; }

 for(const auto &p : dups)
   listOut.push_back(p.second);

 wnd.Show(this, GetImageList(), listOut, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
  {
   RefreshBoth();
   RefreshImports();
  }
}
void MainWnd::mnuViewFolderByGlobalHashtags()
{
 HashTagChooseDlg dlg;
 GroupShowWnd wnd;
 std::vector<ImageParser *> list;

 if (dlg.Show(this) != DialogResult::OK)
   return;

 for(const auto &px : App->Pictures)
  {
   if (px.second->IsImport() == false)
    {
     if (px.second->Item()->GlobalHashTags.count(dlg.SelectedHashTag().ID()) > 0)
       list.push_back(px.second);
    }
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}
void MainWnd::mnuViewSlideShow()
{
 GlobalSlideDlg dlg;

 dlg.Show();
}
void MainWnd::mnuViewAllByGlobalHashtag()
{
 WaitCursor *wait;
 ProgressBar pgb(&m_Status, 1);
 GroupShowWnd wnd;
 std::vector<ImageParser *> save;
 std::vector<ImageParser *> list;
 ImageList il;
 HashTag ht;
 String msg;
 int i;

 if (m_CurrentBase == nullptr)
  {
   App->Response(MAIN_SET_BASE);
   return;
  }

 HashTagChooseDlg dlg;
 if (dlg.Show(this) != DialogResult::OK)
   return;
  
 ht = dlg.SelectedHashTag();

 wait = new WaitCursor();
 m_Status.SetText(0, m_Menu.GetMenuTree(ID_VIEW_VIEWALLBYGLOBALHASHTAG));

 if (m_CurrentBase->Folders.size() == 0)
  {
   App->Response(COMMON_NO_ITEMS_FOUND);
   return;
  }

 pgb.Max(m_CurrentBase->Folders.size());

 i = 0;

 for(const auto &f : m_CurrentBase->Folders)
  {
   f.second->GetPictureList(ht.ID());
   for(const auto &pi : f.second->PictureList)
     list.push_back(pi->Picture());
   f.second->PictureList.clear();  // items will be deleted below
   pgb.Progress();
  }

 if (list.size() == 0)
  {
   delete wait;
   App->Response(COMMON_NO_ITEMS_FOUND);
   m_Status.SetText(0, L"");
   return;
  }

 for(const auto &px : App->Pictures)
   save.push_back(px.second);

 App->Pictures.clear(); // have to substitute the globals

 il.Create(ImageList::Style::Regular, 128, 128, (int)list.size());
 
 for (const auto &px : list)
  {
   App->Pictures.insert(std::pair<int, ImageParser *>(px->ID(), px));
   m_ImageThumbs.Add(px->Thumb(), px->ID());
  }

 delete wait;

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);

 wait = new WaitCursor();

 il.Destroy();

 for (const auto &px : list)
  {
   px->Dispose();
   delete px;
  } 

 App->Pictures.clear(); 

 for(const auto &px : save)
  {
   App->Pictures.insert(std::pair<int, ImageParser *>(px->ID(), px));
  }

 delete wait;
 m_Status.SetText(0, L"");
}

void MainWnd::mnuViewSetListViewIcons()
{
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_ICONS, true);
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_DETAILS, false);
 m_ListPics.ViewStyle(ListView::DisplayStyle::LargeIcon);
 m_ListImport.ViewStyle(ListView::DisplayStyle::LargeIcon);
}

void MainWnd::mnuViewSetListViewDetails()
{
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_ICONS, false);
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_DETAILS, true);
 m_ListPics.ViewStyle(ListView::DisplayStyle::Details);
 m_ListImport.ViewStyle(ListView::DisplayStyle::Details);
}

void MainWnd::mnuViewSetListViewNotInTree()
{
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_NOT_IN_TREE, true);
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_SHOW_ALL, false);
 m_ShowingNotInTree = true;
 RefreshBoth();
}

void MainWnd::mnuViewSetListViewShowAll()
{
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_NOT_IN_TREE, false);
 m_Menu.SetCheckState(ID_VIEW_SETLISTVIEW_SHOW_ALL, true);
 m_ShowingNotInTree = false;
 RefreshBoth();
}

void MainWnd::mnuViewSetGroupViewHashtags()
{
 m_ShowingFileGroups = false;
 m_Menu.SetCheckState(ID_VIEW_SETGROUPVIEW_HASHTAGS, true);
 m_Menu.SetCheckState(ID_VIEW_SETGROUPVIEW_FILENAMES, false);
 SetPictureViewer(&m_PicList, nullptr);
 SetPictureViewer(&m_PicTree, nullptr);
 RefreshBoth();
}

void MainWnd::mnuViewSetGroupViewFileNames()
{
 m_ShowingFileGroups = true;
 m_Menu.SetCheckState(ID_VIEW_SETGROUPVIEW_HASHTAGS, false);
 m_Menu.SetCheckState(ID_VIEW_SETGROUPVIEW_FILENAMES, true);
 SetPictureViewer(&m_PicList, nullptr);
 SetPictureViewer(&m_PicTree, nullptr);
 RefreshBoth();
}

// Tools Menu handlers

void MainWnd::mnuToolsSetPhotoEditor()
{
 String exe;

 exe = OpenFileDlg::ChoooseFile(Handle(), App->Instance(), App->Prose.Text(MAIN_EDITOR_EXE), OpenFileDlg::FileType::EXE);

 if (exe.Length() > 0)
      App->SaveSetting(L"ImageEditor", exe);

}

void MainWnd::mnuToolsDBConnection()
{
 if (DB->Connect(false) == false)
   Close();
}

void MainWnd::mnuToolsMaintainGlobalHashtags()
{
 HashTagMaintDlg dlg;

 dlg.Show(this);
}

void MainWnd::mnuToolsSetImportDirectory()
{
 String dir;

 dir= OpenFileDlg::ChooseFolder(m_hWnd, App->Prose.Text(MAIN_IMPORT_LOCATION));
 if (dir.Length()>0)
  {
   App->Options.SaveSetting(L"LastImport", dir);
   m_CurrentImportDir=dir;
   ReScanImports();
  }
}
void MainWnd::OnHelpAbout()
{
 HelpAboutDlg dlg;
 
 dlg.Show(this);
}

// PopUp Menu Handlers

void MainWnd::mnuListSortByFileName()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileName);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileName);
}
void MainWnd::mnuListSortByNameNumber()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::NameWithNumber);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::NameWithNumber);
}
void MainWnd::mnuListSortByWidth()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Width);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Width);
}
void MainWnd::mnuListSortByHeight()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Height);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Height);
}
void MainWnd::mnuListsSortByRatio()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::Ratio);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::Ratio);
}
void MainWnd::mnuListSortByColor()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::ColorRGB);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::ColorRGB);
}
void MainWnd::mnuListSortByEdgeAvg()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::EdgeAverage);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::EdgeAverage);
}
void MainWnd::mnuListSortByFileSize()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileSize);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileSize);
}
void MainWnd::mnuListSortByFileDate()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FileDate);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FileDate);
}
void MainWnd::mnuListSortByGlobalTags()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::GlobalHashTags);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::GlobalHashTags);
}
void MainWnd::mnuListSortByFolderTags()
{
 if (m_TabList.GetCurrentTab() == 0)
   m_ListPics.SetSort(ImageParser::SortChoices::FolderHashTags);
 else
   m_ListImport.SetSort(ImageParser::SortChoices::FolderHashTags);
}

void MainWnd::mnuPopUpListAddToSelectedGroup()
{
 TreeNode oGroup = m_TreeGroup.SelectedNode();

 if (oGroup.Handle()==0)
  {
   App->Response(MAIN_GROUP_NOT_SELECTED);
   return;
  }
 
 if (oGroup.Tag != 0)
  {
   App->Response(MAIN_GROUP_NOT_SELECTED);
   return;
  }

 if (m_ListPics.SelectedItemsCount() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 MoveItemsToGroup(oGroup, &m_ListPics, false);
}

void MainWnd::mnuPopUpListMoveToNewGroup()
{
 std::vector<ImageParser *> list;
 std::vector<int> indices;
 std::vector<TreeNode> nodes;
 DialogResult res;
 String strGroup=L"";
 String itemName;
 String msg;
 int tag;

 if (m_ListPics.SelectedItemsCount() < 1)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 indices = m_ListPics.GetSelectedIndices();

 itemName = m_ListPics.GetItemText(indices[0],0);
 GroupNameDlg dlg(itemName, false);
 res = dlg.Show(this, App->Prose.Text(MAIN_GROUP_NEW));
 if (res == DialogResult::OK)
   strGroup = dlg.GroupName();
 else
   return;

 nodes = m_TreeGroup.Nodes();
 for(const auto &ng : nodes)
  {
   if (String::Compare(ng.Text, strGroup) == 0)
    {
     App->Response(App->Prose.TextArgs(COMMON_EXISTS, strGroup));
     return;
    }
  }

 SetPictureViewer(&m_PicList, nullptr);
 SetPictureViewer(&m_PicTree, nullptr);

 for(const auto &index : indices)
  {
   tag = m_ListPics.GetItemParam(index);
   #ifdef _DEBUG
   if (App->Pictures.count(tag) == 0) throw L"Tag from ListView wasn't found in App->Pictures";
   #endif
   list.push_back(App->Pictures[tag]);
  }
 
 if (ReorderList(strGroup, list, m_CurrentFolder, false) == false)
  {
   App->Response(MAIN_RENAME_FILES_FAILED);
  }
 
 // move successful, do controls

 RefreshFileGroupList();

}

void MainWnd::mnuPopUpListReplaceFolderHashTag()
{
 std::vector<HashTag> list;
 std::vector<int> selected;
 HashTagSelectorDlg dlg;
 ImageParser *px;
 DialogResult dr;
 int id;

 if (m_CurrentFolder == nullptr) { App->Response(MAIN_FOLDER_OPEN); return; }
 if (m_ListPics.SelectedItemsCount() == 0) { App->Response(COMMON_NOTHING_SELECTED); return; }

 dr = dlg.Show(this, m_CurrentFolder, HashTagSelectorDlg::eMode::PictureTags); // get a fresh set of hashtags for each selected picture
 if (dr == DialogResult::OK)
  {
   for(const auto &pht1 : dlg.PictureHashTags)
     list.push_back(pht1);
  
  }
 if (dr != DialogResult::OK)
   return;
 
 if (list.size() == 0)
  {
   App->Response(MAIN_HASHTAGS_NOT_SELECTED);
   return;
  }

 selected = m_ListPics.GetSelectedIndices(); 

 for (const auto &ndx : selected)
  {
   id = m_ListPics.GetItemParam(ndx);

   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_ListPic item param wasn't a valid App->Picture ImageParser ID";
   #endif

   px = App->Pictures[id];

   #ifdef _DEBUG
   if (px->Item() == nullptr) throw L"ImageParser's PictureItem is null";
   #endif

   px->Item()->ReplacePictureHashTags(list);
   if (px->ID() == m_PicList.Item()->ID()) 
     m_PicList.RefreshHashTags();
  }

 RefreshBoth();
}

void MainWnd::mnuPopUpListAddFolderHashTag()
{
 std::vector<HashTag> list;
 std::vector<int> selected;
 HashTagSelectorDlg dlg;
 ImageParser *px;
 DialogResult dr;
 int id;

 if (m_CurrentFolder == nullptr) { App->Response(MAIN_FOLDER_OPEN); return; }
 if (m_ListPics.SelectedItemsCount() == 0) { App->Response(COMMON_NOTHING_SELECTED); return; }

 dr = dlg.Show(this, m_CurrentFolder, HashTagSelectorDlg::eMode::PictureTags); // get a fresh set of hashtags for each selected picture
 if (dr == DialogResult::OK)
  {
   for(const auto &pht1 : dlg.PictureHashTags)
     list.push_back(pht1);
  
  }
 if (dr != DialogResult::OK)
   return;
 
 if (list.size() == 0)
  {
   App->Response(MAIN_HASHTAGS_NOT_SELECTED);
   return;
  }

 selected = m_ListPics.GetSelectedIndices(); 

 for (const auto &ndx : selected)
  {
   id = m_ListPics.GetItemParam(ndx);

   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_ListPic item param wasn't a valid App->Picture ImageParser ID";
   #endif

   px = App->Pictures[id];

   #ifdef _DEBUG
   if (px->Item() == nullptr) throw L"ImageParser's PictureItem is null";
   #endif

   px->Item()->AddPictureHashTags(list);
   if (px->ID() == m_PicList.Item()->ID()) 
     m_PicList.RefreshHashTags();
  }

 RefreshBoth();
}

void MainWnd::mnuPopUpListAddToSelectedGroupHashTag()
{
 ImageParser *px;
 TreeNode gn;
 TreeNode ni;
 std::vector<HashTag> listHT;
 std::vector<TreeNode> nodes;
 std::vector<int> indices;
 int id;

 if (m_CurrentFolder==nullptr) {  App->Response(MAIN_FOLDER_OPEN); return; }
 if (m_ShowingFileGroups==true) { App->Response(MAIN_SWITCH_GROUP_VIEW); return; }
 if (m_TreeTags.Tree.IsNodeSelected() == false) { App->Response(MAIN_HASHTAGS_NOT_SELECTED); return; }
 if (m_ListPics.SelectedItemsCount() == 0) { App->Response(COMMON_NOTHING_SELECTED); return; }

 if (m_TreeTags.Tree.SelectedNode().Tag == 0) // root items have no Tag 
  {
   gn = m_TreeTags.Tree.SelectedNode();
   nodes = gn.GetNodes();
   if (nodes.size() == 0) throw L"a root node will always have sub items";
   ni = nodes[0];
   if (App->Pictures.count(ni.Tag) == 0) throw L"a child node tag will always contain the image id";
   px = App->Pictures[ni.Tag];
   for (const auto &pht : px->Item()->PictureHashTags)
     listHT.push_back(pht.second);
   }
 else
  {
   ni = m_TreeTags.Tree.SelectedNode();
   if (App->Pictures.count(ni.Tag) == 0) throw L"a child node tag will always contain the image id";
   px = App->Pictures[ni.Tag];
   for (const auto &pht : px->Item()->PictureHashTags)
     listHT.push_back(pht.second);
  }

 indices = m_ListPics.GetSelectedIndices();
 for(const auto &ndx: indices)
  {
   id = m_ListPics.GetItemParam(ndx);   
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"id of listview param not a valid ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Item() != nullptr) // not sure why since only import list items have no "Item()"
    {
     px->Item()->ReplacePictureHashTags(listHT);
     if (id == m_PicList.Item()->ID()) 
          m_PicList.RefreshHashTags();
     }
  }
 
 RefreshBoth();
}

void MainWnd::mnuPopUpListDeleteSelectedFiles()
{
 ImageParser *px;
 String q;
 String nl = L"\n";
 std::vector<int> indices;
 std::vector<int> delIndices;
 int i, id;

 if (m_ListPics.SelectedItemsCount() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 
 q = App->Prose.Text(COMMON_MOVE_TO_RECYCLE);
 q += L":";
 indices = m_ListPics.GetSelectedIndices();

 i=0;
 for(const auto &ndx : indices)
  {
   if (i>12)
    {
     q += nl;
     q += App->Prose.Text(COMMON_CUT_SHORT);
     break;
    }
   q += nl;
   q += m_ListPics.GetItemText(ndx,0);
   i++;
  }

 if (App->Question(q, MB_OKCANCEL) != DialogResult::OK) return;

 for(const auto &ndx : indices)
  {
   id = m_ListPics.GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_PicList item param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Delete() == false)
    {
     App->Response(App->Prose.TextArgs(MAIN_DELETE_FAILED, px->FileName()));
     break;
    }
   else
    {
     delIndices.push_back(ndx);
     px->Dispose();
     delete px;
     App->Pictures.erase(id);
    }
  }

 m_ListPics.RemoveItems(delIndices);

 m_Status.SetText(0, App->Prose.TextArgs(MAIN_MOVE_RECYCLE, String::Decimal((int)delIndices.size())));

 SetPictureViewer(&m_PicList, nullptr);
}

void MainWnd::mnuPopUpListSelectAll()
{
 m_ListPics.SelectAll();
}

void MainWnd::mnuPopUpListViewSelectedItems()
{
 GroupShowWnd wnd;
 std::vector<int> indices;
 std::vector<ImageParser *> list;
 int id;

 indices = m_ListPics.GetSelectedIndices();
 if (indices.size() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
    
 for (const auto &ndx : indices)
  {
   id = m_ListPics.GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"ListView param should be a valid ImageParser ID";
   #endif
   list.push_back(App->Pictures[id]);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}

void MainWnd::mnuPopUpListPictureEdit()
{
 ImageParser *px;
 String msg;

 if (m_ListPics.SelectedItemsCount() == 0) 
  { 
   App->Response(COMMON_NOTHING_SELECTED); 
   return; 
  }
 px = m_ListPics.SelectedImage();

 PicManApp::EditPicture(px->FullPath());

 msg = App->Prose.Text(ID_POPUP_LIST_EDIT);
 msg += L" ";
 msg += px->FileName();
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpListViewPicture()
{
 ImageParser *px;
 String msg;

 if (m_ListPics.SelectedItemsCount() == 0) 
  { 
   App->Response(COMMON_NOTHING_SELECTED); 
   return; 
  }
 px = m_ListPics.SelectedImage();

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

void MainWnd::mnuPopUpListPictureProperties()
{
 ImageParser *px;

 px = m_ListPics.SelectedImage();
 if (px != nullptr)
  {
   PicturePropertiesDlg dlg;
   dlg.Show(this, px);
  }
}

// import list context menu

void MainWnd::mnuPopUpImportAddToSelectedGroup()
{
 TreeNode oGroup;

 if (m_TreeGroup.IsNodeSelected() == false)
  {
   App->Response(MAIN_GROUP_NOT_SELECTED);
   return;
  }

 oGroup = m_TreeGroup.SelectedNode();

 if (oGroup.Tag == 0)
  {
   oGroup = oGroup.GetParent();
  }

 if (m_ListImport.SelectedItemsCount() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 if (m_CurrentFolder == nullptr)
  {
   App->Response(MAIN_FOLDER_OPEN);
   return;
  }
 MoveItemsToGroup(oGroup, &m_ListImport, true);
}

void MainWnd::mnuPopUpImportMoveToNewGroup()
{
 std::vector<ImageParser *> list;
 std::vector<int> indices;
 DialogResult res;
 ImageParser *px;
 String strGroup=L"";
 TreeNode nd;
 TreeNode gNode;
 ListViewItem item;
 int id;

 if (m_ListImport.SelectedItemsCount() < 1)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 if (m_CurrentFolder == nullptr)
  {
   App->Response(MAIN_FOLDER_OPEN);
   return;
  }

 indices = m_ListImport.GetSelectedIndices();
 id = m_ListImport.GetItemParam(indices[0]);
 #ifdef _DEBUG
 if (App->Pictures.count(id) == 0) throw L"m_ListImports param not a valid ImageParser id";
 #endif
 px = App->Pictures[id];

 GroupNameDlg dlg(px->FileName(), false);
 res = dlg.Show(this, App->Prose.Text(MAIN_GROUP_NEW));
 if (res == DialogResult::OK)
    strGroup = dlg.GroupName();
 else
   return;

 for(const auto &ng : m_TreeGroup.Nodes())
  {
   if (String::Compare(ng.Text, strGroup) == 0)
    {
     App->Response(App->Prose.TextArgs(MAIN_GROUP_EXISTS, strGroup));
     return;
    }
  }

 SetPictureViewer(&m_PicList, nullptr);

 for(const auto &ndx : indices)
  {
   id = m_ListImport.GetItemParam(indices[ndx]);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_ListImports param not a valid ImageParser id";
   #endif
   px = App->Pictures[id];
   list.push_back(px);
  }
  
 if (ReorderList(strGroup, list, m_CurrentFolder, true) == false) 
  {
   App->Response(App->Prose.TextArgs(MAIN_IMPORT_RENAME_FAILED, strGroup));
   return;
  }  

 // move successful, do controls

 m_ListImport.RemoveItems(indices);
 RefreshBoth();

 SetPictureViewer(&m_PicList, nullptr);

 m_Status.SetText(0, App->Prose.TextArgs(MAIN_IMPORT_RENAME_SUCCESS, strGroup));
}

void MainWnd::mnuPopUpImportMoveToList()
{
 std::vector<ImageParser *> tempList;
 std::vector<int> indices;
 ImageParser *px;
 int id;

 if (m_ListImport.SelectedItemsCount() == 0) 
  { 
   App->Response(COMMON_NOTHING_SELECTED); 
   return; 
  }
 if (m_CurrentFolder == nullptr) 
  { 
   App->Response(MAIN_FOLDER_OPEN); 
   return; 
  }

 indices = m_ListImport.GetSelectedIndices();
 for(const auto &ndx : indices)
  {
   id = m_ListImport.GetItemParam(indices[ndx]);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_ListImports param not a valid ImageParser id";
   #endif
   px = App->Pictures[id];
   tempList.push_back(px);
  }

 if (MoveToList(tempList, true) == true)
  {
   for(const auto &p : tempList)
    {
     m_ListPics.Insert(p);
    }
   m_CurrentFolder->ProcessPictures(tempList); // adds pictures to database
  }
 else
  {
   App->Response(App->Prose.TextArgs(COMMON_CANCELLED, App->Prose.Text(ID_POPUP_IMPORT_MOVE_LIST)));
   return;
  }

 m_ListImport.RemoveItems(indices);

 m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_POPUP_IMPORT_MOVE_LIST)));
}

void MainWnd::mnuPopUpImportDeleteSelectedFiles()
{
 std::vector<int> indices;
 std::vector<int> delIndices;
 ImageParser *px;
 String q;
 int i, id;

 if (m_ListImport.SelectedItemsCount() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 q = App->Prose.Text(COMMON_MOVE_TO_RECYCLE);
 q += L":";
 indices = m_ListImport.GetSelectedIndices();

 i=0;
 for(const auto &ndx : indices)
  {
   if (i>12)
    {
     q += L"\n";
     q += App->Prose.Text(COMMON_CUT_SHORT);
     break;
    }
   q += L"\n";
   q += m_ListImport.GetItemText(ndx,0);
   i++;
  }

 if (App->Question(q, MB_OKCANCEL) != DialogResult::OK) return;

 for(const auto &ndx : indices)
  {
   id = m_ListImport.GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_ListImport item param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Delete() == false)
    {
     App->Response(App->Prose.TextArgs(MAIN_DELETE_FAILED, px->FileName()));
     break;
    }
   else
    {
     px->Dispose();
     delete px;
     App->Pictures.erase(id);
     delIndices.push_back(ndx);
    }
  }

 m_ListImport.RemoveItems(delIndices);

  m_Status.SetText(0, App->Prose.TextArgs(MAIN_MOVE_RECYCLE, String::Decimal((int)delIndices.size())));

 m_PicList.SetItem(nullptr); 

}

void MainWnd::mnuPopUpImportSelectAll()
{
 m_ListImport.SelectAll();
}

void MainWnd::mnuPopUpImportViewPicture()
{
 ImageParser *px;

 px = m_ListImport.SelectedImage();
 if (px != nullptr)
  {
   SlideDlg dlg(px->FullPath(), true);
   dlg.Show();
  }
}


void MainWnd::mnuPopUpImportPictureProperties()
{
 ImageParser *px;

 px = m_ListImport.SelectedImage();
 if (px != nullptr)
  {
   PicturePropertiesDlg dlg;
   dlg.Show(this, px);
  }
}

// Tree Group context menu handlers

void MainWnd::mnuPopUpTreeGroupManage()
{
 GroupManageWnd wnd;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 ImageParser *px;
 TreeNode node;
 String msg, group;
 int i;

 if (m_TreeGroup.IsNodeSelected() == false)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 node = m_TreeGroup.SelectedNode();
 if (node.Tag != 0)
   node = node.GetParent();

 group = node.Text;

 i = 1;
 for(const auto &n : node.GetNodes())
  {
   #ifdef _DEBUG
   if (App->Pictures.count(n.Tag) == 0) throw L"m_TreeGroup node's Tag must be a valid ImageParser id";
   #endif
   px = App->Pictures[n.Tag];
   if (px->IsNumbered() == false) throw L"Group items should all be numbered";
   if (px->FileNumber() != i)
    {
     App->Response(App->Prose.TextArgs(MAIN_GROUP_MANAGE, px->FileName(), group, m_Menu.GetMenuTree(ID_FILE_ACTIONS_RENUMBERFILESINFOLDER)));
     return;
    }
   list.push_back(px);
   i++;
  }

 wnd.Show(this, GetImageList(), group, list);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
 else
   RefreshFileGroupTree();

 m_TreeGroup.SetFocus();

 nodes = m_TreeGroup.Nodes();
 for(const auto &n : nodes)
  {
   if (n.Text == group)
     m_TreeGroup.SetSelectedNode(n);
  }
}

void MainWnd::mnuPopUpTreeGroupView()
{
 ImageParser *px;
 GroupShowWnd wnd;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 int id;

 if (m_TreeGroup.IsNodeSelected() == false)
   return;

 group = m_TreeGroup.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeGroup node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   list.push_back(px);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted() == true)
   RefreshBoth();
}

void MainWnd::mnuPopUpTreeGroupFavorite()
{
 ImageParser *px;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 HashTag ht = App->GetFavoriteHashTag();
 ProgressBar pgb(&m_Status, 1);
 int id;

 if (m_TreeGroup.IsNodeSelected() == false)
   return;

 group = m_TreeGroup.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 pgb.Max(nodes.size());
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeGroup node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Item()->HasGlobalHashTag(ht) == false)
    {
     px->Item()->AddGlobalHashTag(ht);
    }
   pgb.Progress();
  }
 m_PicTree.RefreshHashTags();
}

void MainWnd::mnuPopUpTreeGroupUnFavorite()
{
 ImageParser *px;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 HashTag ht = App->GetFavoriteHashTag();
 ProgressBar pgb(&m_Status, 1);
 int id;

 if (m_TreeGroup.IsNodeSelected() == false)
   return;

 group = m_TreeGroup.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 pgb.Max(nodes.size());
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeGroup node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Item()->HasGlobalHashTag(ht) == true)
    {
     px->Item()->RemoveGlobalHashTag(ht);
    }
   pgb.Progress();
  }
 m_PicTree.RefreshHashTags();
}

void MainWnd::mnuPopUpTreeGroupRename()
{
 TreeNode oGroup;
 std::vector<ImageParser *>list;
 std::vector<TreeNode> nodes;
 String strOld, strNew;
 String msg;

 if (m_TreeGroup.IsNodeSelected() == false) 
  {
   App->Response(COMMON_NOTHING_SELECTED);
  }
 oGroup = m_TreeGroup.SelectedNode();
 if (oGroup.Tag != 0)
  {
   oGroup = oGroup.GetParent();
  }

 strOld = oGroup.Text;

 GroupNameDlg dlg(oGroup.Text, false);
 if (dlg.Show(this, App->Prose.TextArgs(MAIN_GROUP_RENAME, strOld)) == DialogResult::OK)
  {
   strNew = dlg.GroupName();
   nodes = oGroup.GetNodes();
   for(const auto &n : nodes)
    {
     #ifdef _DEBUG
     if (App->Pictures.count(n.Tag) == 0) L"throw m_TreeGroup node tag not ImageParser id";
     #endif          
     list.push_back(App->Pictures[n.Tag]);
    }
   if (ReorderList(strNew, list, m_CurrentFolder, false) == false)
     App->Response(L"Rename group failed.");
   else
    {
     m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_TREEGROUP_RENAME_GROUP)));
     RefreshFileGroupTree();
    } 
  }
}

void MainWnd::mnuPopUpTreeGroupRenumber()
{
 ImageParser *px;
 TreeNode group;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 String msg; 
 int id;

 if (m_TreeGroup.IsNodeSelected() == false)
   return;

 group = m_TreeGroup.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 if (App->Question(App->Prose.TextArgs(MAIN_GROUP_RENUMBER, group.Text), MB_OKCANCEL) != DialogResult::OK)
   return;

 for(const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeGroup node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   list.push_back(px);
  } 

 if (ReorderList(group.Text, list, m_CurrentFolder, false) == false)
   App->Response(App->Prose.TextArgs(COMMON_FAILED, App->Prose.Text(ID_TREEGROUP_RENUMBER)));
 else
  {
   m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_TREEGROUP_RENUMBER)));
  }
 RefreshFileGroupTree();
}

void MainWnd::mnuPopUpTreeGroupMoveBackToList()
{
 std::vector<ImageParser *> tempList;
 ImageParser *px;
 TreeNode groupFrom;

 if (m_TreeGroup.IsNodeSelected() == false) { App->Response(COMMON_NOTHING_SELECTED); return; }

 groupFrom = m_TreeGroup.SelectedNode();
 if (groupFrom.Tag != 0)
  {
   groupFrom = groupFrom.GetParent();
  }

 for(const auto &n : groupFrom.GetNodes())
  {
   #ifdef _DEBUG
   if (App->Pictures.count(n.Tag) == 0) L"throw m_TreeGroup node tag not ImageParser id";
   #endif        
   px = App->Pictures[n.Tag];
   if (px == nullptr) throw L"null ImageParser ?";  
   tempList.push_back(px);
  }

 if (MoveToList(tempList, false) == true) 
  {
   RefreshBoth();
   m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_TREEGROUP_MOVE_GROUP_BACK)));
  }
}

void MainWnd::mnuPopUpTreeGroupPicView()
{
 std::vector<TreeNode> nodes;
 ImageParser *px;
 TreeNode node;

 if (m_TreeGroup.IsNodeSelected() == false) { App->Response(L"Nothing selected."); return; }

 node = m_TreeGroup.SelectedNode();
 if (node.Tag == 0)
  {
   nodes = node.GetNodes();
   node = nodes[0];
  }

 if (App->Pictures.count(node.Tag) == 0) throw L"m_TreeGroup item tag not imageparser id";
 px = App->Pictures[node.Tag];

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

void MainWnd::mnuPopUpTreeGroupPicBackToList()
{
 std::vector<ImageParser *> tempList;
 TreeNode nodeFrom;

 if (m_TreeGroup.IsNodeSelected() == false) { App->Response(L"Group not selected."); return; }

 nodeFrom = m_TreeGroup.SelectedNode();
 if (nodeFrom.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }

 if (App->Pictures.count(nodeFrom.Tag) == 0) L"throw m_TreeGroup node tag not ImageParser id";
 tempList.push_back(App->Pictures[nodeFrom.Tag]);


 if (MoveToList(tempList, false) == true) 
  {
   RefreshBoth();
   m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_TREEGROUP_PIC_BACK_TO_LIST)));
  }
}

void MainWnd::mnuPopUpTreeGroupPicMoveToAnother()
{
 GroupSelectDlg dlg;
 std::vector<ImageParser *> listPic;
 std::vector<TreeNode> nodes;
 std::vector<String> list;
 TreeNode nodeTo, nodeFrom, nodeFromGroup;

 if (m_TreeGroup.IsNodeSelected() == false)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 nodeFrom = m_TreeGroup.SelectedNode();
 if (nodeFrom.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }

 nodeFromGroup = nodeFrom.GetParent();

 nodes = m_TreeGroup.Nodes();
 for (const auto &n : nodes)
   list.push_back(n.Text);
 
 if (dlg.Show(this, list) == DialogResult::OK)
  {
   for(const auto &n : nodes)
    {
     if (n.Text == dlg.GroupName())
       nodeTo = n;
    }
   if (nodeTo.Handle() == 0) throw L"Didn't find selected node";

   for(const auto &n : nodeTo.GetNodes())
    {
     #ifdef _DEBUG
     if (App->Pictures.count(n.Tag) == 0) throw L"m_TreeGroup node tag should be id";
     #endif
     listPic.push_back(App->Pictures[n.Tag]);
    }

   if (App->Pictures.count(nodeFrom.Tag) == 0) throw L"m_TreeGroup node tag should be id";
   listPic.push_back(App->Pictures[nodeFrom.Tag]);

   if (ReorderList(nodeTo.Text, listPic, m_CurrentFolder, false) == false)
     App->Response(App->Prose.TextArgs(COMMON_FAILED, App->Prose.Text(ID_TREEGROUP_PIC_MOVE_TO_ANOTHER)));
   else
    {
     listPic.clear();
     for(const auto &n : nodeFromGroup.GetNodes()) 
      {
       if (n.Handle() != nodeFrom.Handle())  // get list of pics from original group
        {                                    // not including the moved one
         #ifdef _DEBUG
         if (App->Pictures.count(n.Tag) == 0) throw L"m_TreeGroup node tag should be id";
         #endif
         listPic.push_back(App->Pictures[n.Tag]);
        }
      }
     if (ReorderList(nodeFromGroup.Text, listPic, m_CurrentFolder, false) == false)
       App->Response(App->Prose.TextArgs(COMMON_FAILED, App->Prose.Text(ID_TREEGROUP_PIC_MOVE_TO_ANOTHER)));
     else
      {
       RefreshFileGroupTree();
       m_Status.SetText(0, App->Prose.TextArgs(COMMON_SUCCESS, App->Prose.Text(ID_TREEGROUP_PIC_MOVE_TO_ANOTHER)));
      }
    }
  }
}

void MainWnd::mnuPopUpTreeGroupDelete()
{
 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 TreeNode node;
 String msg;

 if (m_TreeGroup.IsNodeSelected() == false)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 node = m_TreeGroup.SelectedNode();
 if (node.Tag !=0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }

 msg = L"Move Entire Group '";
 msg += node.Text;
 msg += L"' to the Recycle Bin?"; 

 if (App->Question(msg, MB_OKCANCEL) != DialogResult::OK)
   return;

 nodes = node.GetNodes();
 for(const auto &n : nodes)
  {
   #ifdef _DEBUG
   if (App->Pictures.count(n.Tag) == 0) throw L"m_TreeGroup node tag should be id";
   #endif
   list.push_back(App->Pictures[n.Tag]);
  }
 
 for(const auto &px : list)
  {
   if (px->Delete() == false)
    {
     msg = L"Delete Failed On:\n\n";
     msg += px->FullPath();
     msg += L"\n\nPrevious items in the group were recycled...";
     App->Response(msg);
     return;
    }
   else
    {
     App->Pictures.erase(px->ID());
     px->Dispose();
     delete px;
    }
  }
 m_TreeGroup.RemoveItem(node);
 msg = L"Group ";
 msg += node.Text;
 msg += L" Moved To Recycle Bin";
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeGroupPicDelete()
{
 ImageParser *px;
 TreeNode item;
 String msg;

 if (m_TreeGroup.IsNodeSelected() == false) 
  { 
   App->Response(L"Select an item to delete"); 
   return; 
  }
 
 item = m_TreeGroup.SelectedNode();

 if (item.Tag == 0) 
  { 
   App->Response(L"Select an item level picture to delete"); 
   return; 
  }

 #ifdef _DEBUG
 if (App->Pictures.count(item.Tag) == 0) throw L"m_TreeGroup node param not an ImageParser id";
 #endif 
 px = App->Pictures[item.Tag];


 msg = L"Delete ";
 msg += px->FullPath();
 if (App->Question(msg, MB_OKCANCEL) != DialogResult::OK)
   return;

 if (px->Delete() == false)
  {
   msg = L"Unable to recycle ";
   msg += px->FullPath();
   App->Response(msg);
  }
  
 m_TreeGroup.RemoveItem(item);
 App->Pictures.erase(px->ID());
 px->Dispose();
 delete px;
  
 SetPictureViewer(&m_PicTree, nullptr);
}

void MainWnd::mnuPopUpTreeGroupCollapse()
{
 m_TreeGroup.CollapseAll();
}

void MainWnd::mnuPopUpTreeGroupPicProps()
{
 PicturePropertiesDlg dlg;
 ImageParser *px;
 std::vector<TreeNode> nodes;
 TreeNode node;

 if (m_TreeGroup.IsNodeSelected() == false)
   return;

 node = m_TreeGroup.SelectedNode();

 if (node.Tag == 0)
  {
   nodes = node.GetNodes();
   if (nodes.size() == 0) throw L"There should be child nodes...";
   node = nodes[0];
  }
 if (App->Pictures.count(node.Tag) == 0) throw L"node.Tag should have been an ImageParser id";
 px = App->Pictures[node.Tag];

 dlg.Show(this, px);

}

// HashTag Tree Popup Menu Handlers

void MainWnd::mnuPopUpTreeTagsFavorite()
{
 ImageParser *px;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 HashTag ht = App->GetFavoriteHashTag();
 ProgressBar pgb(&m_Status, 1);
 String msg;
 int id;

 if (m_TreeTags.Tree.IsNodeSelected() == false)
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 pgb.Max(nodes.size());
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeTag.Tree node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Item()->HasGlobalHashTag(ht) == false)
    {
     px->Item()->AddGlobalHashTag(ht);
    }
   pgb.Progress();
  }
 m_PicTree.RefreshHashTags();
 msg = L"HashTag Group '";
 msg += group.Text;
 msg += L"' Made Favorite";
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeTagsUnFavorite()
{
 ImageParser *px;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 HashTag ht = App->GetFavoriteHashTag();
 ProgressBar pgb(&m_Status, 1);
 String msg;
 int id;

 if (m_TreeTags.Tree.IsNodeSelected() == false)
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 pgb.Max(nodes.size());
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeTags.Tree node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Item()->HasGlobalHashTag(ht) == true)
    {
     px->Item()->RemoveGlobalHashTag(ht);
    }
   pgb.Progress();
  }
 m_PicTree.RefreshHashTags();
 msg = L"HashTag Group '";
 msg += group.Text;
 msg += L"' Favorite Removed";
 m_Status.SetText(0, msg);

}

void MainWnd::mnuPopUpTreeTagsGroupTagsUpdate()
{
 std::vector<TreeNode> nodes;
 TreeNode group;
 HashTagSelectorDlg dlg;
 ImageParser *px;
 DialogResult r;
 String msg;
   
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 px = App->Pictures[nodes[0].Tag];

 for(const auto &pht : px->Item()->PictureHashTags)
  {
   dlg.PictureHashTags.push_back(pht.second);
  }
 r = dlg.Show(this, m_CurrentFolder, px->Item());
 if (r == DialogResult::OK)
  {
   for(const auto &ni : nodes)
    {
     px = App->Pictures[ni.Tag];
     if (px->Item() != nullptr) 
      {
       px->Item()->ReplacePictureHashTags(dlg.PictureHashTags);
       if (px->ID() == m_PicTree.Item()->ID())
         m_PicTree.RefreshHashTags();
      }
    }   
   RefreshHashTagGroupTree();
   msg = L"HashTag Group '";
   msg += group.Text;
   msg += L"' HashTags Updated";
   m_Status.SetText(0, msg);
  }
}

void MainWnd::mnuPopUpTreeTagsGroupTagsRemove()
{
 std::vector<TreeNode> nodes;
 std::vector<HashTag> list;
 ImageParser *px;
 TreeNode group;
 String msg;
  
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 msg = L"Remove Group '";
 msg += group.Text;
 msg += L"' HashTags ? \n\nMoves Items Back To List";
 if (App->Question(msg, MB_OKCANCEL) != DialogResult::OK)
   return;

 nodes = group.GetNodes();

 for(const auto  &ni : nodes)
  {
   list.clear();
   px = App->Pictures[ni.Tag];
   for(const auto &pht : px->Item()->PictureHashTags)
     list.push_back(pht.second);
   for(const auto &pht : list)
     px->Item()->RemovePictureHashTag(pht);
  }
 RefreshBoth();
 msg = L"HashTag Group '";
 msg += group.Text;
 msg += L"' Moved Back To List";
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeTagsGroupView()
{
 ImageParser *px;
 GroupShowWnd wnd;
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 TreeNode group;
 int id;

 if (m_TreeTags.Tree.IsNodeSelected() == false)
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 nodes = group.GetNodes();
 for (const auto &n : nodes)
  {
   id = n.Tag;
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_TreeTags.Tree node param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   list.push_back(px);
  }

 wnd.Show(this, GetImageList(), list, HashTagSelectCtrl::enumFilterStyle::NoFilter);
 if (wnd.ItemsDeleted())
   RefreshBoth();
}

void MainWnd::mnuPopUpTreeTagsGroupDelete()
{
 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 TreeNode group;
 String msg;
  
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 group = m_TreeTags.Tree.SelectedNode();

 if (group.Tag != 0)
  {
   group = group.GetParent();
  }

 if (App->Question(App->Prose.TextArgs(MAIN_GROUP_DELETE, group.Text), MB_OKCANCEL) != DialogResult::OK)
   return;

 nodes = group.GetNodes();

 for(const auto &n : nodes)
  {
   #ifdef _DEBUG
   if (App->Pictures.count(n.Tag) == 0) throw L"m_TreeTags.Tree node tag should be id";
   #endif
   list.push_back(App->Pictures[n.Tag]);
  }
 
 for(const auto &px : list)
  {
   if (px->Delete() == false)
    {
     App->Response(App->Prose.TextArgs(MAIN_DELETE_FAILED, px->FullPath()));
     return;
    }
   else
    {
     App->Pictures.erase(px->ID());
     px->Dispose();
     delete px;
    }
  }
 m_TreeTags.Tree.RemoveItem(group);
 msg = App->Prose.Text(ID_TREEGROUP_DELETE_GROUP);
 msg += L" ";
 msg += App->Prose.TextArgs(COMMON_SUCCESS, group.Text);
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeTagsItemTagsUpdate()
{
 TreeNode node;
 HashTagSelectorDlg dlg;
 ImageParser *px;
 DialogResult r;
 String msg;
   
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }

 px = App->Pictures[node.Tag];

 for(const auto &pht : px->Item()->PictureHashTags)
  {
   dlg.PictureHashTags.push_back(pht.second);
  }
 r = dlg.Show(this, m_CurrentFolder, px->Item());
 if (r == DialogResult::OK)
  {
   if (px->Item() != nullptr) 
    {
     px->Item()->ReplacePictureHashTags(dlg.PictureHashTags);
     if (px->ID() == m_PicTree.Item()->ID())
       m_PicTree.RefreshHashTags();
    }   
   RefreshHashTagGroupTree();
   msg = App->Prose.Text(ID_TREETAGS_ITEM_TAGS_UPDATE);
   msg += L" ";
   msg += App->Prose.TextArgs(COMMON_SUCCESS, node.Text);
   m_Status.SetText(0, msg);
  }
}

void MainWnd::mnuPopUpTreeTagsItemTagsRemove()
{
 std::vector<HashTag> list;
 ImageParser *px;
 TreeNode node;
 String msg;
  
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }

 if (App->Question(App->Prose.TextArgs(MAIN_TREETAGS_ITEM_TO_LIST, node.Text), MB_OKCANCEL) != DialogResult::OK)
   return;

 px = App->Pictures[node.Tag];
 for(const auto &pht : px->Item()->PictureHashTags)
   list.push_back(pht.second);
 for(const auto &pht : list)
   px->Item()->RemovePictureHashTag(pht);

 RefreshBoth();

 msg = App->Prose.Text(ID_TREETAGS_ITEM_TAGS_REMOVE);
 msg += L" ";
 msg += App->Prose.TextArgs(COMMON_SUCCESS, node.Text);
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeTagsItemView()
{
 std::vector<TreeNode> nodes;
 TreeNode node;
 ImageParser *px;

 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   nodes = node.GetNodes();
   node = nodes[0];
  }
 px = App->Pictures[node.Tag];
 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

void MainWnd::mnuPopUpTreeTagsItemProps()
{
 TreeNode node;
 ImageParser *px;

 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }
 px = App->Pictures[node.Tag];

 PicturePropertiesDlg dlg;
 dlg.Show(this, px);
}

void MainWnd::mnuPopUpTreeTagsItemEdit()
{
 TreeNode node;
 ImageParser *px;
 String msg;

 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
   return;
  }
 px = App->Pictures[node.Tag];

 PicManApp::EditPicture(px->FullPath());

 msg = App->Prose.Text(ID_TREETAGS_ITEM_EDIT);
 msg += L" ";
 msg += px->FileName();
 m_Status.SetText(0, msg);
}

void MainWnd::mnuPopUpTreeTagsItemDelete()
{
 ImageParser *px;
 TreeNode node;
 String msg;
  
 if (m_TreeTags.Tree.IsNodeSelected() == false) 
   return;

 node = m_TreeTags.Tree.SelectedNode();

 if (node.Tag == 0)
  {
   App->Response(MAIN_GROUP_NODE_ERROR);
  }

 if (App->Question(App->Prose.TextArgs(MAIN_DELETE_PICTURE, node.Text), MB_OKCANCEL) != DialogResult::OK)
   return;

 #ifdef _DEBUG
 if (App->Pictures.count(node.Tag) == 0) throw L"m_TreeTags.Tree node tag should be id";
 #endif
 px = App->Pictures[node.Tag];
 if (px->Delete() == false)
  {
   msg = App->Prose.Text(ID_TREETAGS_ITEM_DELETE);
   msg += L" ";
   msg += App->Prose.TextArgs(COMMON_FAILED, node.Text);
   App->Response(msg);
   return;
  }

 App->Pictures.erase(px->ID());
 px->Dispose();
 delete px;
  
 m_TreeTags.Tree.RemoveItem(node);

 msg = App->Prose.Text(ID_TREETAGS_ITEM_DELETE);
 msg += L" ";
 msg += App->Prose.TextArgs(COMMON_SUCCESS, node.Text);

 m_Status.SetText(0, msg);
}

// Picture Management

bool MainWnd::ReScanDirectory()
{
 std::vector<FolderItem *> deleted;
 std::vector<String> added;
 std::vector<String> listDirs;
 DialogResult r = DialogResult::Yes;
 String msg;
 bool bFound, bChanged;

 if (m_CurrentBase == nullptr)
  {
   App->Response(MAIN_SET_BASE);
   return false;
  }

 if (m_CurrentBase->DirPath().Length() == 0)
  {
   App->Response(MAIN_BASE_OPEN_FAIL);
   return false;
  }

 if (App->Question(App->Prose.TextArgs(MAIN_BASE_RESCAN, m_CurrentBase->DirPath()), MB_OKCANCEL) != DialogResult::OK)
   return false;

 listDirs = Utility::GetFolderNames(m_CurrentBase->DirPath());
 for( const auto &dir : listDirs)
  {
   bFound = false;
   for(const auto &ssi : m_CurrentBase->Folders)
    {
     if (String::Compare(ssi.second->Folder(), dir) == 0) bFound = true;
    }
   if (bFound == false)
    {
     r = App->Question(App->Prose.TextArgs(MAIN_BASE_ADD, dir), MB_YESNOCANCEL);
     if (r == DialogResult::Cancel) return false;
     if (r == DialogResult::Yes) added.push_back(dir);
    }
  }

 for(const auto &s : added)
  {
   FolderItem *ssi = new FolderItem(s, m_CurrentBase); // auto adds to m_CurrentBase
   ssi->LoadInfo();
  }
 
 bChanged = added.size() > 0;

 for(const auto &ssi : m_CurrentBase->Folders)
  {
   if (Utility::DirectoryExists(ssi.second->FolderPath()) == false) 
    {
     msg = L"Directory ";
     msg += ssi.second->FolderPath();
     msg += L" not found on disk. Remove it from the list?";
     r = App->Question(App->Prose.TextArgs(MAIN_BASE_REMOVE, ssi.second->FolderPath()), MB_YESNOCANCEL);
     switch(r)
      {
       case DialogResult::Yes:
         deleted.push_back(ssi.second);
         break; 
       case DialogResult::Cancel:
         return false;
      }
    }
  }

 for(const auto &ssi : deleted)
  {
   m_CurrentBase->RemoveFolder(ssi->ID());
  }

 return bChanged;
}

// Random Stuff

Rect MainWnd::GetRect()
{
 RECT r;
 Rect rr;

 ::GetWindowRect(m_hWnd, &r);
 rr.X=r.left;
 rr.Y=r.top;
 rr.Width=r.right - r.left;
 rr.Height=r.bottom - r.top; 
 
 return rr;
}

void MainWnd::OpenFolder(FolderItem *folder)
{
 std::vector<ImageParser *> list;
 DialogResult r;
 WaitCursor wait;
 String msg;
 int i;

 m_ListPics.Clear();
 m_TreeGroup.Clear();
 m_TreeTags.Tree.Clear();
 m_ListImport.Clear();

 SetPictureViewer(&m_PicList, nullptr);
 SetPictureViewer(&m_PicTree, nullptr);

 list = App->ClearAndGetImports(); // get list of imports and clear the dictionary
 App->ResetPictureID();            // fresh set of picture id's

 for (const auto &p : list)
  {
   p->SetID(App->NextPictureID());  // give import a new id
   App->Pictures.insert(std::pair<int, ImageParser *>(p->ID(), p)); // imports re-added to map
  }

 ReScanImports(); // load any new jpgs in incoming dir into App->Pictures

 m_CurrentFolder = folder;
 folder->BumpOpened();

 folder->Load();

 m_TabList.SetTabText(0, folder->Folder());

 list = ProcessFolder(folder);

 if (list.size() > 0)
  {
   for(const auto &p : list)
    {
     App->Pictures.insert(std::pair<int, ImageParser *>(p->ID(), p));
    }
   folder->ProcessPictures(list);
  }

 list.clear();
 i = 0;
 for(const auto &px : App->Pictures)
  {
   if (px.second->IsImport() == false)
    {
     if (Utility::FileExists(px.second->FullPath())==false)
      {
       list.push_back(px.second);
       if (i < 10)
        {
         if (msg.Length() == 0)
           msg = App->Prose.Text(COMMON_FILE_NOT_FOUND);
         msg += L"\n";
         msg += px.second->FullPath();
         i++;
        }
       else
        {
         msg += L"\n";
         msg += App->Prose.Text(COMMON_CUT_SHORT);
        }
      }
    }
  }
 if (i>0)
  {
   msg += L"\n\n";
   msg += App->Prose.Text(COMMON_REMOVE_ROWS);
   r = App->Question(msg, MB_YESNO);
   if (r == DialogResult::Yes)
    {
     for(const auto &px : list)
      {
       App->Pictures.erase(px->ID());
       if (px->Item() != nullptr)
        {
         px->Item()->Delete();
        }
      }
    }
  }

 ReloadImageList();   // put all Thumbs in App->Pictures into ImageList
 RefreshImports();    // put all IsImport images in App->Pictures into m_ListImport    
 RefreshBoth();       // put non import items into m_ListPics and either m_TreePics or m_TreeHashTags

 SaveLastFolder(folder);
 LoadLastFolders();
}

void MainWnd::CloseFolder()
{
 std::vector<ImageParser *> list;

 SetPictureViewer(&m_PicList, nullptr);
 SetPictureViewer(&m_PicTree, nullptr);

 m_ListPics.Clear();
 m_ListImport.Clear();
 m_TreeGroup.Clear();
 m_TreeTags.Tree.Clear();
 m_TreeTags.Filter.FolderMap.clear();
 SizeTree();

 list = App->ClearAndGetImports();
 App->ResetPictureID();
 for (const auto &px : list)
  {
   px->SetID(App->NextPictureID());  // give import a new id
   App->Pictures.insert(std::pair<int, ImageParser *>(px->ID(), px)); // imports re-added to map
  }

 ReScanImports(); // load any new jpgs in incoming dir into App->Pictures 

 m_CurrentFolder->Close();

 ReloadImageList(); // add imports back in
 m_TabList.SetTabText(0, App->Prose.Text(MAIN_FOLDER_NAME));
 m_Status.SetText(0, App->Prose.Text(MAIN_FOLDER_CLOSED));
 m_CurrentFolder = nullptr;
}

std::vector<ImageParser *> MainWnd::ProcessFolder(FolderItem *folder)
{
 std::vector<ImageParser *> listPictures;
 std::vector<ImageParser *> listFinal;
 std::vector<String> listDir;
 String sDir, msg, sFile;
 ImageParser *p;
 PictureItem *pi;

 sDir = folder->FolderPath();

 if (Utility::DirectoryExists(sDir) == false)
  {
   msg = App->Prose.TextArgs(MAIN_FOLDER_DIR_NOT_FOUND, m_CurrentBase->DirPath(), App->Prose.Text(ID_FILE_SCANBASEFORNEWFOLDERS));
   App->Response(msg);
   return listFinal;  // empty list
  }

 m_Status.SetText(0,App->Prose.TextArgs(MAIN_FOLDER_SCAN, sDir));

 listDir = Utility::GetFileNames(sDir, L"*.jpg");
 
 if (listDir.size() == 0)
   return listFinal; // empty

 for(const auto &file : listDir)
  {
   if (folder->ContainsFile(file) == false) // don't reprocess existing items
    {
     p=new ImageParser(sDir, file, App->NextPictureID());
     pi=folder->FindPicture(p);
     if (pi==nullptr)
       listPictures.push_back(p); // need to add file to database
    }
  }
 
 if (listPictures.size() > 0)
  {
   msg=L"Adding New ";
   msg+=String::Decimal((int)listPictures.size());
   msg+=L" Files From ";
   msg+=sDir;
   m_Status.SetText(0, msg);
   listFinal = LoadImages(listPictures);
  }
 msg = String::Decimal(listPictures.size());
 msg += L" new pictures added, ";
 msg += String::Decimal((int)folder->Pictures.size());
 msg += L" pictures total";
 m_Status.SetText(0, msg);

 return listFinal;
}
 
void MainWnd::ProcessImportDirectory(String const &strPath) 
{
 std::map<String, ImageParser *> existing;
 std::vector<ImageParser *> listTemp;
 std::vector<ImageParser *> listFinal;
 std::vector<String> listDir;
 ImageParser  *px, *py;
 String msg, sSearch;

 msg = L"Processing ";
 msg += strPath;
 m_Status.SetText(0, msg);

 if (strPath.Length() == 0) 
  {
   App->Response(App->Prose.TextArgs(MAIN_IMPORT_SET, App->Prose.Text(ID_TOOL_SETIMPORTDIRECTORY)));
   return; // empty list
  }

 if (Utility::DirectoryExists(strPath) ==false)
  {
   App->Response(App->Prose.TextArgs(MAIN_IMPORT_CHANGE, strPath, App->Prose.Text(ID_TOOL_SETIMPORTDIRECTORY)));
   return;
  } 

 for(const auto &it : App->Pictures)
  {
   if (it.second->IsImport() == true)
     existing.insert(std::pair<std::wstring, ImageParser *>(it.second->FileName().Chars(), it.second));
  }
 
 listDir = Utility::GetFileNames(strPath, L"*.jpg");
 if (listDir.size()==0)
   return;  // directory was empty
  
 for (const auto &file : listDir) 
  {
   px = new ImageParser(strPath, file, App->NextPictureID());
   px->SetImport(true);
   if (existing.count(px->FileName().Chars()) == 0)   // import file already processed?
     listTemp.push_back(px);                          // no, add to list
   else
    {
     py = existing.at(px->FileName().Chars());        // yes, check if file date is newer
     if (px->FileDate() > py->FileDate())
      {
       listTemp.push_back(px);                        // file date is newer, add to list
       App->Pictures.erase(py->ID());                 // remove older file from dictionary
       py->Dispose();                                 // dispose and delete
       delete py;
      }
    }
  }

 if (listTemp.size() == 0)
   return; // no changes in the imports directory

 m_Status.SetText(0, App->Prose.TextArgs(MAIN_FOLDER_SCAN, strPath));

 listFinal = LoadImages(listTemp);

 for (const auto &px : listFinal)
   App->Pictures.insert(std::pair<int, ImageParser *>(px->ID(), px));

}

void MainWnd::GrindPicture(ImageParser *px)
{
 px->ProcessImage();
}

std::vector<ImageParser *> MainWnd::LoadImages(std::vector<ImageParser *> list)
{
 std::vector<ImageParser *> finishedList;
 String msg;
 bool keepReporting = true;

 for(const auto &px : list)
  {
   px->ProcessImage();
   if (px->Good()==false)
    {
     if (keepReporting == true)
      {
       if (App->Question(App->Prose.TextArgs(COMMON_FAILED_QUESTION, px->FullPath(), px->Error()), MB_YESNO)!=DialogResult::Yes)
        {
         keepReporting = false;
        }
      }
     delete px;
    }
   else
    {
     finishedList.push_back(px);
    }
  }

 return finishedList;
}

void MainWnd::ReScanImports()
{
 ProcessImportDirectory(m_CurrentImportDir);
} 

void MainWnd::RefreshImports()
{
 m_ListImport.Clear();

 for (const auto &it : App->Pictures)
  {
   if (it.second->IsImport()==true)
   m_ListImport.Insert(it.second);
  }
 m_ListImport.ResizeColumns();
}

void MainWnd::RefreshBoth()
{
 if (m_ShowingFileGroups==true)
  {
   m_TreeGroup.Visible(true);
   m_TreeTags.Visible(false);
   RefreshFileGroupList();
  }
 else
  {
   m_TreeGroup.Visible(false);
   m_TreeTags.Visible(true);
   RefreshHashTagGroupList();
   RefreshHashTagGroupTree();
   m_TreeTags.SizeTree();
  }
}

// if the mode is File Groups, not HashTags

void MainWnd::RefreshFileGroupList()
{
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 ImageParser *px;
 std::wstring str;
 std::map<String, int> dictPics;
 std::map<String, TreeNode> tree;
 
 m_ListPics.Clear();

 for(const auto &it : App->Pictures)
  {
   px=it.second;
   if (px->IsImport()==false) // don't want imports in the list
    {
     list.push_back(px);
     if (m_ShowingNotInTree == false)
       m_ListPics.Insert(px); // show all option, everything goes to the list 
    }
  }
 if (list.size() == 0) 
   return;

 std::sort(list.begin(), list.end(), ImageParserSorter(ImageParser::SortChoices::FileName, false));

 for(const auto &px : list)
  {
   if (px->IsNumbered()) 
    {
     str = px->FilePrefix().Chars();
     if (dictPics.count(str)>0) 
       dictPics.at(str) += 1;   // count the number of file prefix instances  "File_0001", "File_0002" would be 2
     else
       dictPics.insert(std::pair<std::wstring, int>(str, 1));
    }
  }

 m_TreeGroup.Clear();

 for(const auto &px : list)
  {
   str = px->FilePrefix().Chars();
   if (px->IsNumbered() == true && dictPics.at(str) > 1) 
    {
     if (tree.count(str) > 0)
      {
       tree.at(str).Nodes.push_back(TreeNode(px->FileName(),px->ID()));
      }
     else
      {
       tree.insert(std::pair<std::wstring, TreeNode>(str, TreeNode(str)));
       tree.at(str).Nodes.push_back(TreeNode(px->FileName(),px->ID()));
      }
    }
   else
    {
     if (m_ShowingNotInTree == true)
       m_ListPics.Insert(px); // wasn't numbered or there was only 1 numbered item
    }
  }
 m_TreeGroup.AddTree(tree);
 m_ListPics.DoSort();
 m_ListPics.ResizeColumns();
}

// Refresh Only m_TreeGroup Not m_ListPics

void MainWnd::RefreshFileGroupTree()
{
 std::vector<ImageParser *> list;
 std::vector<TreeNode> nodes;
 ImageParser *px;
 std::wstring str;
 std::map<String, int> dictPics;
 std::map<String, TreeNode> tree;
 
 for(const auto &it : App->Pictures)
  {
   px=it.second;
   if (px->IsImport()==false) // don't want imports in the list
    {
     list.push_back(px);
    }
  }
 if (list.size() == 0) 
   return;

 std::sort(list.begin(), list.end(), ImageParserSorter(ImageParser::SortChoices::FileName, false));

 for(const auto &px : list)
  {
   if (px->IsNumbered()) 
    {
     str = px->FilePrefix().Chars();
     if (dictPics.count(str)>0) 
       dictPics.at(str) += 1;   // count the number of file prefix instances  "File_0001", "File_0002" would be 2
     else
       dictPics.insert(std::pair<std::wstring, int>(str, 1));
    }
  }

 m_TreeGroup.Clear();

 for(const auto &px : list)
  {
   str = px->FilePrefix().Chars();
   if (px->IsNumbered() == true && dictPics.at(str) > 1) 
    {
     if (tree.count(str) > 0)
      {
       tree.at(str).Nodes.push_back(TreeNode(px->FileName(),px->ID()));
      }
     else
      {
       tree.insert(std::pair<std::wstring, TreeNode>(str, TreeNode(str)));
       tree.at(str).Nodes.push_back(TreeNode(px->FileName(),px->ID()));
      }
    } 
  }
 m_TreeGroup.AddTree(tree);
}

void MainWnd::RefreshHashTagGroupList()
{
 std::vector<int> indices;
 std::map<String, int> dictPics;
 int lastLVParamID;
 ImageParser *px;
 int i, lastNdx;

 lastLVParamID = -1;
 if (m_ListPics.SelectedItemsCount() > 0 )
  {
   indices = m_ListPics.GetSelectedIndices();
   lastLVParamID = m_ListPics.GetItemParam(indices[0]);
  }

 m_ListPics.Clear();

 if ( App->Pictures.size() == 0 )
   return;

 lastNdx = -1;
 i = 0;
 for (const auto &it : App->Pictures)
  {
   px=it.second;
   if (px->IsImport() == false)
    {
     if (m_ShowingNotInTree == false)
      {
       m_ListPics.Insert(px); // show all option, all pics go to list
       if (px->ID() == lastLVParamID)
         lastNdx = i;
       i++;
      }
     else
      {
       if (px->Item()->PictureHashTags.size() == 0)
        {
         m_ListPics.Insert(px);
         if (px->ID() == lastLVParamID)
           lastNdx = i;
         i++;
        }
      }
    }
  }

 m_ListPics.DoSort();
 m_ListPics.ResizeColumns();

 if (lastNdx >= 0)
   m_ListPics.EnsureVisible(lastNdx);
}
 
void MainWnd::RefreshHashTagGroupTree()
{
 ImageParser *px;
 String strHTGroup;
 std::vector<String> picTagSort;
 std::map<String, TreeNode> tree;
 std::vector<TreeNode> nodes;
 TreeNode lastTree;

 if (m_ShowingFileGroups == false)
  {
   if (m_TreeTags.Tree.IsNodeSelected() == true) 
    {
     if (m_TreeTags.Tree.SelectedNode().Tag == 0)
       lastTree = m_TreeTags.Tree.SelectedNode();
     else
       lastTree = m_TreeTags.Tree.SelectedNode().GetParent();
    }
  }

 m_TreeTags.Tree.Clear();

 if (m_CurrentFolder != nullptr)
   HashTag::Merge(m_TreeTags.Filter.FolderMap, m_CurrentFolder->FolderHashTags);

 m_TreeTags.SizeTree();
 
 for(const auto &ipx : App->Pictures)
  {
   px = ipx.second;
   if (px->IsImport() == false)
    {
     if (px->Item()->PictureHashTags.size() > 0)
      {
       if (m_TreeTags.Filter.ShowItem(px->Item()) == true)
        {
         picTagSort.clear();
         for(const auto &fi : px->Item()->PictureHashTags)
          {
           picTagSort.push_back(fi.second.ToString().Chars());
          }
         std::sort(picTagSort.begin(), picTagSort.end());
         strHTGroup=L"";
         for(const auto &ps : picTagSort) 
          {
           if (strHTGroup.Length() > 0) 
             strHTGroup+=L",";
           strHTGroup += ps;
          }
         if (tree.count(strHTGroup)>0)
          {
           tree.at(strHTGroup).Nodes.push_back(TreeNode(px->FileName(),px->ID()));
           }
         else
          {
           tree.insert(std::pair<String, TreeNode>(strHTGroup, TreeNode(strHTGroup)));
           tree.at(strHTGroup).Nodes.push_back(TreeNode(px->FileName(),px->ID())); // add the actual item to the tree
          }
        }
      }
    }
  }

 m_TreeTags.Tree.AddTree(tree);

 if (lastTree.Handle() != 0)
  {
   nodes = m_TreeTags.Tree.Nodes();
   for(const auto &nd : nodes)
    {
     if (nd.Text == lastTree.Text)
      {
       m_TreeTags.Tree.SetSelectedNode(nd);
       m_TreeTags.Tree.EnsureVisible();
       break; // exit for
      }
    }
  }
}

///////////////////////////////////////////////////////
//        U t i l i t y    F u n c t i o n s
///////////////////////////////////////////////////////

bool MainWnd::MoveToList(std::vector<ImageParser *> const &incoming, bool isImported)
{
 std::vector<ImageParser *> tempList;
 DialogResult dr;
 String strFile, newFile;
 String msg;
 int i, hi;

 if (incoming.size()==0)
   return false;

 strFile = L"";
 GroupNameDlg dlg(incoming[0]->NoExtension(), true); // blank allowed
 dr = dlg.Show(this, App->Prose.Text(MAIN_NEW_FILE_PREFIX));
 if (dr == DialogResult::OK) 
   strFile = dlg.GroupName();
 else
   return false;

 if (strFile.Length() > 0 ) 
   strFile += L" - ";

 for(const auto &pi : m_CurrentFolder->Pictures)
  {
   if (strFile.Length() > 0) 
    {
     if (pi.second->Picture()->FileName().Length() > strFile.Length())
      {
       if (String::Compare(pi.second->Picture()->FileName().Substring(0, strFile.Length()), strFile)==0)
         tempList.push_back(pi.second->Picture());
     }
    }
  else
   {
    if (String::TryIntParse(pi.second->Picture()->NoExtension(), &i) == true) // need to remove .jpg
      tempList.push_back(pi.second->Picture());
   }
 }
if (tempList.size() > 0) 
 {
  // obtain last file by sorting desc
  std::sort(tempList.begin(), tempList.end(), ImageParserSorter(ImageParser::SortChoices::FileName, false)); 
  if (strFile.Length() > 0)
   {
    if (String::TryIntParse(tempList[0]->FileName().Substring(strFile.Length()), &i) == false) throw L"failed to extract file number";
    i += 1; 
   }
  else
   {
    if (String::TryIntParse(tempList[0]->NoExtension(), &i) == false) throw L"expected 1st item in templist to be an integer";
    i += 1;
   }
 }
else
 {
  i = 1;
 }

 hi = i;

 for(const auto &item : incoming)
  {
   if (strFile.Length() > 0)
    {
     newFile = strFile;
     newFile += String::Decimal(i);
     newFile += L".jpg";
     if (item == nullptr)
       throw L"null item?";
     if (item->RenameByFileToDir(newFile, m_CurrentFolder->FolderPath()) == false)
      {
       App->Response(App->Prose.TextArgs(MAIN_FILE_MOVE_ERROR, item->FileName(), newFile));
       return false;
      }
    }
   else
    {
     newFile = String::Decimal(i);
     newFile += L".jpg";
     if (item->RenameByFileToDir(newFile, m_CurrentFolder->FolderPath()) == false) 
      {
       App->Response(App->Prose.TextArgs(MAIN_FILE_MOVE_ERROR, item->FileName(), newFile));
       return false;
      }
     if (isImported == true)
       item->SetImport(false);
    }
   i++;
  }
 return true;
}

void MainWnd::MoveItemsToGroup(TreeNode &group, PicListView *ctrl, bool isImported)
{
 std::vector<ImageParser *> list;
 std::vector<int> indices;
 ImageParser *p;
 TreeNode nd;
 
 for(const auto &ni : group.GetNodes())
  {
   if (App->Pictures.count(ni.Tag) == 0) throw L"TreeNode tag not a picture id";
   p = App->Pictures[ni.Tag];
   list.push_back(p);  // add existing items
  }

 for(const auto &item : ctrl->SelectedPictures())
   list.push_back(item); // add new items

 if (ReorderList(group.Text, list, m_CurrentFolder, isImported) == false)
  {
   App->Response(App->Prose.TextArgs(MAIN_GROUP_REORDER_FAILED, group.Text));
  }
 else
  {
   group.ClearNodes();
   for(const auto &px : list)
    {
     group.InsertLast(px->FileName(), px->ID());
    }
   indices = ctrl->GetSelectedIndices();
   ctrl->RemoveItems( indices );
   SetPictureViewer(&m_PicList, nullptr); // is both m_ListPics and m_ListImport
  }
} 

//  attempt to reorder a group, return false if it fails for any reason

bool MainWnd::ReorderList(String const &strGroup, std::vector<ImageParser *> list, FolderItem *folder, bool isImported)
{
 ProgressBar pgb(&m_Status, 1);
 String path;
 String fldr;
 int i;
 

 fldr = folder->FolderPath();

 pgb.Max(list.size());
 i = 1;
 for(const auto &p : list)
  {
   path = strGroup;
   path += L"_";
   path += String::Decimal(i, 3);
   path += L".ord";
   if (p->RenameByFileToDir(path, fldr) == false) return false; // straighten up the list
   pgb.Progress();
   i++;
  }

 pgb.Max(list.size());
 i = 1;
 for(const auto &p : list)
  {
   path = strGroup;
   path += L"_";
   path += String::Decimal(i, 3);
   path += L".jpg";
   if (p->RenameByFileToDir(path, fldr) == false) return false; // back to jpg after straightening
   if (isImported == true)
     p->SetImport(false);
   pgb.Progress();
   i++;
  }

 folder->ProcessPictures(list); // not sure why there would be new pictures in the list...
 return true;
}

void MainWnd::SaveLastFolder(FolderItem *fldr)
{
 std::vector<int> tempList;
 String str;
 bool blnReady;
 int i, id, c;

 id = fldr->ID();

 blnReady = false;

 for(const auto &prev : m_PreviousList)
  {
   if (prev == id) // already in the list, move it to the top
    {
     tempList.push_back(id);
     for(const auto &prev2 : m_PreviousList)
      {
       if (prev2 != id) 
         tempList.push_back(prev2);
     }
    m_PreviousList.clear();
    for(const auto &temp : tempList)
      m_PreviousList.push_back(temp);
    blnReady = true;
    break; // exit foreach
   }
 }

 if (blnReady == false)
  {
   tempList.push_back(id);
   c = 1;
   for(const auto &prev : m_PreviousList)
    {
     if (c < 8) tempList.push_back(prev);
     c += 1;
    }
   m_PreviousList.clear();
   for(const auto &prev2 : tempList)
     m_PreviousList.push_back(prev2);
  }    
 for (i=1;i<=8;i++)
  {
   str=L"Previous_Folder_";
   str+=String::Decimal(m_CurrentBase->ID());
   str+=L"_";
   str+=String::Decimal(i);
   App->SaveSettingInt(str, 0); // clear out any old entries
  }
 i = 1;
 for(const auto &prev : m_PreviousList)
  {
   str=L"Previous_Folder_";
   str+=String::Decimal(m_CurrentBase->ID());
   str+=L"_";
   str+=String::Decimal(i);
   App->SaveSettingInt(str, prev);
   i++;
  }
} 

void MainWnd::LoadLastFolders()
{
 FolderItem *fldr;
 String str;
 int i, id;

 if (m_CurrentBase == nullptr) return; // have to have a currentbase to operate

 m_PreviousList.clear();

 for(i=1;i<=8;i++)
  {
   str=L"Previous_Folder_";
   str+=String::Decimal(m_CurrentBase->ID());
   str+=L"_";
   str+=String::Decimal(i);
   id = App->GetSettingInt(str);
   if (id > 0)
    {
     if (m_CurrentBase->Folders.count(id)>0) 
       m_PreviousList.push_back(id);
    }
  }

 m_Menu.SetEnabledState(ID_FILE_P1, false);
 m_Menu.SetEnabledState(ID_FILE_P2, false);
 m_Menu.SetEnabledState(ID_FILE_P3, false);
 m_Menu.SetEnabledState(ID_FILE_P4, false);
 m_Menu.SetEnabledState(ID_FILE_P5, false);
 m_Menu.SetEnabledState(ID_FILE_P6, false);
 m_Menu.SetEnabledState(ID_FILE_P7, false);
 m_Menu.SetEnabledState(ID_FILE_P8, false);

 i=1;
 for(const auto &prev : m_PreviousList)
  {
   fldr=m_CurrentBase->Folders.at(prev);
   switch(i)
    {
     case 1: m_Menu.SetMenuText(ID_FILE_P1, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P1, true); break;
     case 2: m_Menu.SetMenuText(ID_FILE_P2, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P2, true); break;
     case 3: m_Menu.SetMenuText(ID_FILE_P3, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P3, true); break;
     case 4: m_Menu.SetMenuText(ID_FILE_P4, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P4, true); break;
     case 5: m_Menu.SetMenuText(ID_FILE_P5, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P5, true); break;
     case 6: m_Menu.SetMenuText(ID_FILE_P6, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P6, true); break;
     case 7: m_Menu.SetMenuText(ID_FILE_P7, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P7, true); break;
     case 8: m_Menu.SetMenuText(ID_FILE_P8, fldr->Folder()); m_Menu.SetEnabledState(ID_FILE_P8, true); break;
    }
   i++;
  }
}

void MainWnd::ReloadImageList()
{

 m_ImageThumbs.Destroy();
 m_ImageThumbs.Create(ImageList::Style::Regular, 128, 128, (int)App->Pictures.size());

 for (const auto &it : App->Pictures)
  {
   m_ImageThumbs.Add(it.second->Thumb(), it.second->ID());
  }
}

// Windows stuff

Rect MainWnd::ClientRect()
{
 RECT r;
 Rect rct;
 int h;

 if (m_Status.Handle() == 0) 
   return PopUpWnd::ClientRect(); 

 rct=m_Status.GetRect();
 h=rct.Height;

 ::GetClientRect(m_hWnd, &r);
 rct=Rect(0, 0, r.right-2, r.bottom-( 2 + h) ); 
  
 return rct;
}

void MainWnd::StatusBarText(String const &txt)
{
 if (m_Status.Handle() == 0) return;
 m_Status.SetText(0, txt);
}

void MainWnd::SetPictureViewer(PicView *ctrl, ImageParser *pic)
{
 String msg;

 ctrl->SetItem(pic);
 if (pic != nullptr)
  { 
   msg  = String::Decimal(pic->Width());
   msg += L" X ";
   msg += String::Decimal(pic->Height());
   msg += L"  ";
   msg += pic->FullPath();
  }
 m_Status.SetText(0, msg);
}