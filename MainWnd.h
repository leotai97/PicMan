#pragma once

#define SPLIT_MAIN  1
#define SPLIT_ITEMS 2
#define SPLIT_PICS  3

class MainWnd : public PopUpWnd
{
 public:
 
 MainWnd();
~MainWnd();

 static void Register(String const &wndcls);
 virtual bool Create(String const &wndcls, int nCmdShow);
 
 void LoadMainMenu();

 inline StatusBar *StatusBarCtrl() { return &m_Status; }
 void StatusBarText(String const &txt);

 inline BaseItem   *CurrentBase()   { return m_CurrentBase; }
 inline FolderItem *CurrentFolder() { return m_CurrentFolder; }

 virtual void OnSplitterChanged(HWND hSplitter, int SplitContainerID);
 virtual Rect SplitterRect(HWND hSplitter);
 virtual Point SplitterCursor(HWND hSplitter);
 virtual Rect GetRect();
 virtual Rect ClientRect();

 virtual bool OnClosing();
 virtual void OnPaint();
 virtual void OnSize();

 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual WMR OnMenuOpening(HMENU hMenu, int pos, bool bWindowMenu);
 virtual WMR MenuHandler(int wmId);
 virtual WMR OnContextMenu(HWND hChild, Point const &pt);
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
 void OnOpenFile();
 void OnOpenFileActions();
 void OnOpenView();

 void OnListViewMiddle(int id);

 void SetPictureViewer(PicView *view, ImageParser *img);
 
 ImageList *GetImageList() { return &m_ImageThumbs; }


 // Context Menus

 void ShowMenuListPics(Point const &pt);
 void ShowMenuListImport(Point const &pt);
 void ShowMenuTreeGroup(Point const &pt);
 void ShowMenuTreeHashTags(Point const &pt);

 // Menu Handlers

 void mnuFileSetBaseDirectory();
 void mnuFileNewFolder();
 void mnuFileOpenFolder();
 void mnuFileActionsMaintainFolderHashtags();
 void mnuFileActionsReloadFolder();
 void mnuFileActionsReloadFolderThumbnails();
 void mnuFileActionsRenumberFilesInFolder();
 void mnuFileActionsRenameFilesToHashtags();
 void mnuFileCloseFolder();
 void mnuFileScanBaseForNewFolders();
 void mnuFileRefreshImportList();
 void mnuFileExit();
 void mnuFileP1();
 void mnuFileP2();
 void mnuFileP3();
 void mnuFileP4();
 void mnuFileP5();
 void mnuFileP6();
 void mnuFileP7();
 void mnuFileP8();
 void OnFilePrevious(int ndx);

 void mnuViewBaseDirectoryProperties();
 void mnuViewFolderProperties();
 void mnuViewSortListName();
 void mnuViewSortListNameNmbr();
 void mnuViewSortListWidth();
 void mnuViewSortListHeight();
 void mnuViewSortListWidthDivHeight();
 void mnuViewSortListRGBColors();
 void mnuViewSortListBorderColorAverage();
 void mnuViewSortListDateAdded();
 void mnuViewSortListFileSize();
 void mnuViewSortListGlobalHashTags();
 void mnuViewSortListFolderHashTags();
 void mnuViewRefreshListAndTree();
 void mnuViewCollapseAllGroups();
 void mnuViewExpandAllGroups();
 void mnuViewAllGroups();
 void mnuViewAllPicturesInFolder();
 void mnuViewAllPicturesAndImports();
 void mnuViewImports();
 void mnuViewAllDuplicates();
 void mnuViewFolderByGlobalHashtags();
 void mnuViewSlideShow();
 void mnuViewAllByGlobalHashtag();
 void mnuViewSetListViewIcons();
 void mnuViewSetListViewDetails();
 void mnuViewSetListViewNotInTree();
 void mnuViewSetListViewShowAll();
 void mnuViewSetGroupViewHashtags();
 void mnuViewSetGroupViewFileNames();

 void mnuToolsSetPhotoEditor();
 void mnuToolsDBConnection();
 void mnuToolsMaintainGlobalHashtags();
 void mnuToolsSetImportDirectory();

 void OnHelpAbout();

 // m_PicList sorting

 void mnuListSortByFileName();
 void mnuListSortByNameNumber();
 void mnuListSortByWidth();
 void mnuListSortByHeight();
 void mnuListsSortByRatio();
 void mnuListSortByColor();
 void mnuListSortByEdgeAvg();
 void mnuListSortByFileSize();
 void mnuListSortByFileDate();
 void mnuListSortByGlobalTags();
 void mnuListSortByFolderTags();

 // context menus

 void mnuPopUpListAddToSelectedGroup();
 void mnuPopUpListMoveToNewGroup();
 void mnuPopUpListReplaceFolderHashTag();
 void mnuPopUpListAddFolderHashTag();
 void mnuPopUpListAddToSelectedGroupHashTag();
 void mnuPopUpListDeleteSelectedFiles();
 void mnuPopUpListSelectAll();
 void mnuPopUpListViewSelectedItems();
 void mnuPopUpListPictureEdit();
 void mnuPopUpListViewPicture();
 void mnuPopUpListPictureProperties();
 
 void mnuPopUpImportAddToSelectedGroup();
 void mnuPopUpImportMoveToNewGroup();
 void mnuPopUpImportMoveToList();
 void mnuPopUpImportDeleteSelectedFiles();
 void mnuPopUpImportSelectAll();
 void mnuPopUpImportViewPicture();
 void mnuPopUpImportPictureProperties();

 void mnuPopUpTreeGroupManage();
 void mnuPopUpTreeGroupView();
 void mnuPopUpTreeGroupFavorite();
 void mnuPopUpTreeGroupUnFavorite();
 void mnuPopUpTreeGroupRename();
 void mnuPopUpTreeGroupRenumber();
 void mnuPopUpTreeGroupMoveBackToList();
 void mnuPopUpTreeGroupPicView();
 void mnuPopUpTreeGroupPicBackToList();
 void mnuPopUpTreeGroupPicMoveToAnother();
 void mnuPopUpTreeGroupDelete();
 void mnuPopUpTreeGroupPicDelete();
 void mnuPopUpTreeGroupCollapse();
 void mnuPopUpTreeGroupPicProps();

 void mnuPopUpTreeTagsFavorite();
 void mnuPopUpTreeTagsUnFavorite();
 void mnuPopUpTreeTagsGroupTagsUpdate();
 void mnuPopUpTreeTagsGroupTagsRemove();
 void mnuPopUpTreeTagsGroupView();
 void mnuPopUpTreeTagsGroupDelete();
 void mnuPopUpTreeTagsItemTagsUpdate();
 void mnuPopUpTreeTagsItemTagsRemove();
 void mnuPopUpTreeTagsItemView();
 void mnuPopUpTreeTagsItemProps();
 void mnuPopUpTreeTagsItemEdit();
 void mnuPopUpTreeTagsItemDelete();


 // Picture Management

 public:

 void SizeChildren();
 void SizeTree();

 bool ReScanDirectory();
 void OpenFolder(FolderItem *folder);
 void CloseFolder();
 void RefreshBoth();
 void ReScanImports();
 void RefreshImports();
 void RefreshFileGroupList();
 void RefreshFileGroupTree();
 void RefreshHashTagGroupList();
 void RefreshHashTagGroupTree();

 bool MoveToList(std::vector<ImageParser *> const &incoming, bool isImported);
 void MoveItemsToGroup(TreeNode &group, PicListView *ctrl, bool isImported);
 bool ReorderList(String const &strGroup, std::vector<ImageParser *> list, FolderItem *fldr, bool isImported);
 void SaveLastFolder(FolderItem *folder);
 void LoadLastFolders();
 void ReloadImageList();
 void ProcessImportDirectory(String const &strPath);


 std::vector<ImageParser *> ProcessFolder(FolderItem *folder);
 std::vector<ImageParser *> LoadImages(std::vector<ImageParser *> list);

 static void GrindPicture(ImageParser *px);

 // data

 protected:

 String       m_CurrentImportDir;
 bool         m_ShowingFileGroups;  // not hashtags
 bool         m_ShowingNotInTree;   // if a pic is not in group or hashtag tree then show in list

 BaseItem    *m_CurrentBase;
 FolderItem  *m_CurrentFolder;

 Rect         m_PanelItems;   // holds m_TabLists and either m_TreePics or m_TreeHashtags
 Rect         m_PanelPics;    // holds m_PicList and m_PicView

 SplitContainer m_SplitMain;  // splits m_PanelPics and m_PanelItems
 SplitContainer m_SplitPics;  // splits m_PicList and m_PicTree
 SplitContainer m_SplitItems; // splits m_TabLists and either m_TreePics or m_TreeHashtags

 TabWnd      m_TabList;      // holds m_ListPics and m_ListImports
 PicListView m_ListPics;     // list of unassigned pics not in either m_TreePics or m_TreeHashtags
 PicListView m_ListImport;   // list of pictures to import i.e. e:\pictures\incoming
 

 PicturesTree    m_TreeGroup;  // tree of pictures i.e. pic_001.jpg pic_002.jpg
 HashTagTreeView m_TreeTags;   // tree of pic hashtags with filter control
 
 PicView     m_PicList;      // display image of selected m_ListPics or m_ListImports
 PicView     m_PicTree;      // display image of selected  m_TreePics or m_TreeHashtag

 StatusBar   m_Status;   

 AMenu       m_Menu;    

 ImageList   m_ImageThumbs;

 std::vector<int> m_PreviousList;
 };

class HashTagSet
{
 public:
 HashTagSet(String const &s, ImageParser *px)
 {
  Tags = s;
  Pictures.push_back(px);
 }

 String Tags;
 std::vector<ImageParser *> Pictures;

};

