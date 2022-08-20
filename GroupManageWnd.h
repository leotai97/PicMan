#pragma once

class GroupManageWnd : public ADialog
{
 public:

 public:

 void Show(AWnd *parent, ImageList *il, String const &groupName, std::vector<ImageParser *> const &imgs);

 virtual Rect SplitterRect(HWND hWnd) { return ClientRect(); }

 bool ItemsDeleted() { return m_List.ItemsDeleted(); }

 protected:

 void OnInitDialog();

 virtual Rect ClientRect();
 virtual void OnSize();
 virtual WMR OnCommand(int menuID, HWND hChild);
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 virtual WMR OnContextMenu(HWND hChild, Point const &pt);
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 
 void OnMoveUp();
 void OnMoveDown();
 void OnCut();
 void OnMoveBefore();
 void OnMoveAfter();
 void OnCancelMove();
 
 void EnableButtons();
 void Reload(std::vector<ImageParser *> const &list);
 void OnListViewMiddle(int id);
 void SetPictureViewer(ImageParser *px); 
 bool Reorder(std::vector<ImageParser *> const &list);

 SplitContainer m_Split;

 PicListView  m_List;

 PicView      m_Pic;
 ToolBar      m_TB;
 StatusBar    m_SB;

 int  m_ListCount;

 std::vector<ImageParser *> m_Pictures;
 std::vector<ImageParser *> m_listCut;

 ImageList *m_ImageList;
 String m_GroupName;
 String m_Title;

};

