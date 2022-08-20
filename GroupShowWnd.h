#pragma once

class GroupShowWnd : public ADialog
{
 public:

 void Show(AWnd *parent, ImageList *il, std::vector<ImageParser *> const &imgs, HashTagSelectCtrl::enumFilterStyle);

 virtual void OnInitDialog();

 virtual Rect SplitterRect(HWND hWnd) { return ClientRect(); }
 
 bool ItemsDeleted() { return m_ItemsDeleted; }

 protected:

 virtual Rect ClientRect();
 virtual void OnSize();
 virtual bool OnClose();

 virtual WMR OnCommand(int menuID, HWND hItem);
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 virtual WMR OnContextMenu(HWND hChild, Point const &pt);
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 
 void OnEdit();
 void OnDelete();
 void OnProperties();
 void OnFavorite();
 void OnUnFavorite();
 void OnHashTag();
 void OnViewPicture();
 void OnFilterChanged();
 void OnSelectDupImports();
 

 void Reload(std::vector<ImageParser *> const &list);
 void OnListViewMiddle(int id);
 void SetPictureViewer(ImageParser *px); 

 SplitContainer m_Split;

 HashTagListView  m_HTLV;

 PicView      m_Pic;
 ToolBar      m_TB;
 StatusBar    m_SB;

 bool m_ItemsDeleted;
 bool m_ShowingFilter;
 int  m_ListCount;
 
 ImageList *m_ImageList;
 

 std::vector<ImageParser *> m_Original;

 HashTagSelectCtrl::enumFilterStyle m_FilterType;

};

