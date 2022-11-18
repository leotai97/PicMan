#pragma once

// controls specific to PicMan


class PicListView : public ListView
{
 public:
 
 PicListView();

 int ImageListIndex(ImageParser *px) { return m_ImageList->GetIndex(px->ID()); }

 virtual void Create(AWnd *parent, DWORD style, Rect const &r);
 static void AddSortMenus(MenuItem *subMenu, ImageParser::SortChoices currentSort);
 static void InsertSortMenu(MenuItem *subMenu, int pos, ImageParser::SortChoices item, ImageParser::SortChoices currentSort);

 void ResizeColumns();
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual void Clear();

 void Insert(ImageParser *px);
 void InsertAt(int row, ImageParser *px);
 void AddColumns();
 void DoSort();
 void SetSort(ImageParser::SortChoices sort);
 void SetSort(ImageParser::SortChoices sort, bool ascending);

 bool ItemsDeleted() { return m_ItemsDeleted; }
 void ResetItemsDeleted() { m_ItemsDeleted = false; }

 ImageParser *SelectedImage(); 

 ImageParser::SortChoices CurrentSort;
 bool                     IsAscending;

 static int CALLBACK CompareItems(LPARAM fldrIdA, LPARAM flderIdB, LPARAM nAscDesCol);

 std::vector<ImageParser *> SelectedPictures();

 void SetFavorite(bool onoff);

 protected:

 virtual WMR OnSubKeyDown(KeyEventArgs const &k);
 virtual WMR OnSubMouseDown(MouseEventArgs const &e);

 void OnDelete();

 bool m_ItemsDeleted;

};


class PicturesTree : public TreeView
{
 public:

 PicturesTree();

 ImageParser *SelectedPicture();

 void DoSort();
 void AddTree(std::map<String, TreeNode> &tree);

};

// ////////////////////////////////////////

class HashTagTreeView : public PanelWnd
{
 public:

 void Create(AWnd *parent, Rect const &r);

 virtual void OnSize();
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 void SizeTree();

 HashTagSelectCtrl Filter;
 PicturesTree      Tree;

};

// ///////////////////////////////////////

class HashTagListView : public PanelWnd
{
 public:

 void Create(AWnd *parent, Rect const &r, HashTagSelectCtrl::enumFilterStyle fStyle);

 virtual WMR OnContextMenu(HWND hChild, Point const &pt) { return m_Parent->OnContextMenu(hChild, pt); }
 virtual void OnSize();
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 void SizeList();

 HashTagSelectCtrl Filter;
 PicListView       List;
};

// ///////////////////////////////////////

class PicView : public PanelWnd
{
 public:
 PicView();
~PicView();

 virtual void Create(AWnd *wnd, Rect const &r);

 void SetItem(ImageParser *item);
 void RefreshHashTags();
 
 inline ImageParser *Item() { return m_Pic; }

 virtual void OnSize();
 virtual void OnPanelClick(HWND hWnd, MouseEventArgs const &e);

 protected:

 HashTagSelectCtrl m_HashTags;
 ImageWnd          m_Image;

 ImageParser *m_Pic;

};

/////////////////////////////////////////////////////

class HashTagEditCtrl : public PanelWnd
{
 public:

 void Create(AWnd *parent, AWnd *owner, int txtID, int btnId, HashTagSelectCtrl::enumSelectStyle);

 virtual void OnSize();
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 virtual WMR OnCommand(int child, HWND hChild);

 HashTagSelectCtrl HT;
 EditWnd           Edit;

 protected: 

 HashTagSelectCtrl::enumSelectStyle m_Style;

 AWnd *m_Owner;

 ButtonWnd m_Add;
};

// ///////////////////////////////

class SpreadSheetItem
{
 public:
 SpreadSheetItem()
 {
  Image = nullptr;
  Match = 0.0F;
  Selected = false;
  Moved = false;
  Deleted = false;
  ID = -1;
 }
 SpreadSheetItem(ImageParser *img)
 {
  Image = img;
  Match = 0.0F;
  Selected = false;
  Moved = false;
  Deleted = false;
  ID = img->ID();
 }
 
 ImageParser *Image;
 float        Match;
 bool         Selected;
 bool         Moved;
 bool         Deleted;
 int          ID;
};

class SpreadSheetRow
{
 public:
 SpreadSheetRow()
 {
  Image = nullptr;
  TotalMatches = 0.0F;
  Selected = false;
  Moved = false;
  Deleted = false;
  ID = -1;
 }
 SpreadSheetRow(ImageParser *img)
 {
  Image = img;
  TotalMatches = 0.0F;
  Selected = false;
  Moved = false;
  Deleted = false;
  ID = img->ID();
 }

 std::vector<SpreadSheetItem *>SelectedItems();
 void SelectAll();
 void SelectNone(); 

 ImageParser *Image;
 std::vector<SpreadSheetItem *> Matches;
 float TotalMatches;
 bool Selected;
 bool Moved;
 bool Deleted;
 int  ID;
};

class SpreadSheetDlg;

class SpreadSheet : public PanelWnd
{
 public:
 SpreadSheet();
~SpreadSheet();

 void SetHSPos(int pos) { m_HSPos = pos; Refresh(); }
 void SetVSPos(int pos) { m_VSPos = pos; Refresh(); }
 void SetOwner(SpreadSheetDlg *owner) { m_Owner = owner; }
 Size ItemSize() { return m_ItemSize; }

 std::vector<SpreadSheetItem *> SelectedItems();

 void SelectNone();
 void SelectAll(ImageParser *row);
 bool Changed() { return m_Changed; }

 protected:
 
 virtual void OnMouseDown(MouseEventArgs const &m);
 virtual void OnMouseWheel(MouseEventArgs const &m);
 virtual WMR OnContextMenu(HWND hChild, Point const &pt);
 virtual WMR OnCommand(int id, HWND hChild);
 virtual void OnPaint(HDC hDC);

 void OnMatchNewGroup();        // selected matches moved to new group
 void OnMatchNewHashTag();      // hashtags assigned to selected matches
 void OnMatchExistingGroup();   // matches moved to existing group
 void OnMatchExistingHashTag(); // existing hashtag set assigned to matches
 void OnMatchRowGroup();        // matches added to row's group
 void OnMatchRowHashTag();      // row's hashtags assigned to selected matches
 
 void OnRowGroup();           // row added to existing group
 void OnRowHashTag();         // rows hashtags set to existing hashtag set
 void OnRowMatchGroup();      // row added to 1st match's group
 void OnRowMatchHashTag();    // rows hashtags set to 1st match

 void OnBothNewGroup();       // both row and selected matches to new group
 void OnBothAssignHashTags(); // assign hashtags to row and selected matches
 void OnProperties();
 void OnDelete();

 void ReplaceMatches(std::vector<SpreadSheetItem *> const &matches);
 void ReplaceSelectedRow();

 void mnuSortListName(); 
 void mnuSortListNameNmbr();
 void mnuSortListWidth();
 void mnuSortListHeight();
 void mnuSortListWidthDivHeight();
 void mnuSortListRGBColors();
 void mnuSortListBorderColorAverage();
 void mnuSortListDateAdded();
 void mnuSortListFileSize();
 void mnuSortListGlobalHashTags();
 void mnuSortListFolderHashTags();

 bool Delete(std::vector<ImageParser *> const &list);

 SpreadSheetDlg *m_Owner;

 int m_VSPos;
 int m_HSPos;

 bool m_RowImageSelected;  // if true only a row's image is selected, false and a match is selected
 int  m_SelectedRow;
 int  m_SelectedCol;
 
 Size m_ItemSize;

 HFONT  m_Font;
 HPEN   m_Pen;  // selection rectangle
 
 Gdiplus::Brush *m_Red;
 Gdiplus::Brush *m_White;
 Gdiplus::Brush *m_Green;

 ImageList m_ImageList;

 std::map<int, int> m_DeleteList;
 
 bool m_Changed; // mainwnd needs to refresh 
};

class SpreadSheetDlg : public ADialog
{
 public:

 SpreadSheetDlg(){};
~SpreadSheetDlg();

 void Show(AWnd *parent);

 std::vector<SpreadSheetRow *> Rows;
 SpreadSheet Sheet;

 void Sort();
 void SelectChange(int r, int c){};

 ScrollBar m_VS;  // vertical scroll bar
 ScrollBar m_HS;  // horizontal scroll bar

 protected: 

 void SizeChildren();

 virtual void OnInitDialog();
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

struct // sort item matches by match percentage
 {
  bool operator()(SpreadSheetItem *a, SpreadSheetItem *b) const { return a->Match > b->Match; } // high to low
 } SpreadSheetMatchSort;

struct // sort rows by total matches by match percentage
 {
  bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const { return a->TotalMatches > b->TotalMatches; } // high to low
 } SpreadSheetRowSort;

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::FileName)) < 0;
  }
} SpreadSheetRowSortFileName; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::NameWithNumber)) < 0;
  }
} SpreadSheetRowSortNameNmbr; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::Height)) < 0;
  }
} SpreadSheetRowSortHeight; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::Width)) < 0;
  }
} SpreadSheetRowSortWidth; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::Ratio)) < 0;
  }
} SpreadSheetRowSortRatio; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::FileDate)) < 0;
  }
} SpreadSheetRowSortFileDate; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::FileSize)) < 0;
  }
} SpreadSheetRowSortFileSize; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::ColorRGB)) < 0;
  }
} SpreadSheetRowSortColorRGB;

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::EdgeAverage)) < 0;
  }
} SpreadSheetRowSortEdgeAverage; 

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::GlobalHashTags)) < 0;
  }
} SpreadSheetRowSortGlobalHashTags;

struct // sort items by choice
{
 bool operator()(SpreadSheetRow *a, SpreadSheetRow *b) const
  {
   if (a->Image == nullptr) return true;
   if (b->Image == nullptr) return false;
   return PicListView::CompareItems(a->Image->ID(), b->Image->ID(), Utility::AscendDescendColumn(true, (int)ImageParser::SortChoices::FolderHashTags)) < 0;
  }
} SpreadSheetRowSortFolderHashTags;

