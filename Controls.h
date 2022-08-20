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