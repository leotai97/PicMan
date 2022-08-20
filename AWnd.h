#pragma once

// Enable Visual Styles in Common Controls such as ListView

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WM_SPLITWINDOWCHANGED (WM_USER + 0x0001) // wParam is split window HWND, lParam is SplitContainer ID
#define WM_PANELWNDCLICKED    (WM_USER + 0x0002) // wParam is Panel hWnd, LPARAM is pointer to MouseEventArgs
#define WM_LISTVIEW_MIDDLE    (WM_USER + 0x0003) // wParam is ListView hWnd, LPARAM is item index

enum class DialogResult : int { OK, Cancel, Retry, Abort, Yes, No, None };

class ADialog;
class ImageList;
class AWnd;
class PopUpWnd;

class Application
{
 public:

 Application();
~Application();

 virtual bool Init(HINSTANCE hInst, int nCmdShow, String const &appName, String const &appDisplayName);
 virtual void Shutdown();

 void GetEncoder(String const &type, CLSID *id);
 inline CLSID EncoderPNG() const { return m_EncImagePNG; }
 
 inline String AppName() { return m_AppName; }
 inline String AppDisplayName() { return m_AppDisplayName; }
 inline String AppLocalPath()  { return m_AppLocalPath; }
 inline String ComputerName() { return m_ComputerName; }

 void Response(String const &msg);
 DialogResult Question(String const &msg, int btns);

 void SetClipboardText(String const &val);

 String SplitterVertWndClass() { return m_SplitterVertWndClass; }
 String SplitterHorzWndClass() { return m_SplitterHorzWndClass; }
 String PanelWndClass() { return m_PanelWndClass; }

 void  AddChildWnd(HWND hWnd, AWnd *child);
 AWnd *GetChildWnd(HWND hWnd);
 void  RemoveChildWnd(HWND hWnd);

 inline HINSTANCE Instance() { return m_hInst; }
 inline PopUpWnd *GetMain() { return m_Main; }
 inline bool DarkMode() { return m_DarkMode; }

 inline void AddCustomClass(String const &className) { m_CustomClasses.push_back(className); }
 
 protected:

 void RegisterClasses();
 void UnregisterClasses();

 std::map<HWND, AWnd *> m_ChildWindows;
 HINSTANCE m_hInst;

 ULONG_PTR m_gdiplusToken;
 
 CLSID m_EncImagePNG;
 
 String m_SplitterVertWndClass;
 String m_SplitterHorzWndClass;
 String m_PanelWndClass;

 String m_ComputerName;
 String m_AppDisplayName;
 String m_AppName;
 String m_AppLocalPath;  // user "appdata" path

 std::vector<String> m_CustomClasses;

 PopUpWnd *m_Main;
 
 bool m_DarkMode;
 
};

enum class WMR : int { One, Zero, Default }; // window msg proc returns

// //////////////////////////////////////////////////

class AWnd  // windows controls
{
 public:

 AWnd();
~AWnd();

 inline HWND Handle() const { return m_hWnd; }
 inline HWND GetParent() { return m_hWnd; }
 inline int  Child() { return m_ChildID; }
 inline bool IsPopUpWnd() { return m_IsPopUpWnd; }
 inline bool IsDialog() { return m_IsDialog; }
 HWND PopUpHandle();

 String GetText();
 void SetText(String const &text);
 virtual Rect GetRect();
 virtual Size GetSize();
 virtual HFONT GetFont();

 static Rect GetScreenRect(HWND hWnd);
 static Rect ClientRect(HWND hWnd);

 virtual Size ClientSize();
 virtual Rect ClientRect();

 // static Size GetScreenSize();

 int Top() { return GetRect().Y; }
 void SetTop(int y) { SetPoint( Point(Left(), y)); } 
 int Left() { return GetRect().X; }
 void SetLeft(int x) { SetPoint( Point(x, Top())); }

 int Width() { return GetSize().Width; }
 int Height() { return GetSize().Height; }

 void SetPoint(Point const &r);
 virtual void SetSize(Size const &sz);
 virtual void SetRect(Rect const &r);

 void Border(bool onoff);
 bool HasBorder();
 void Visible(bool vis);
 bool IsVisible();
 void Refresh();
 void SetFocus();

 void Maximize();

 bool Minimized();
 bool Maximized();
 bool Normal();
 
 virtual void SetImageList(ImageList *imgList){};

 void Show();

 void Close();

 String GetInfo();

 void Attach(ADialog *dlg, int childId);

 virtual WMR OnNotify(HWND hWnd, int child, UINT code, LPARAM lParam) { return WMR::Default; }
 virtual WMR MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return WMR::Default; }

 virtual void OnSplitterChanged(HWND hSplitter, int SplitContainerID){};
 virtual Rect SplitterRect(HWND hSplitter){ Rect r; return r;}
 virtual Point SplitterCursor(HWND hSplitter); // get SplitterWnd's parent cursor coords

 virtual WMR OnNotifyKeyDown(NMKEY *nkd){ return WMR::Default; }
 virtual WMR OnContextMenu(HWND hChild, Point const &pt) { return WMR::Default; }
 virtual void OnDrawItem(DRAWITEMSTRUCT *dis) {};

 protected:

 void CreateWnd(AWnd  *parent, HWND hWnd, String const &wcls, int childID);
 void DestroyWnd();

 HWND  m_hWnd;
 AWnd *m_Parent;

 String m_Class;

 int m_ChildID;

 bool m_IsDestroyed;
 bool m_IsPopUpWnd;
 bool m_IsDialog;
};

////////////////////////////////////////////////////

class PopUpWnd : public AWnd
{
 public:
 PopUpWnd();
~PopUpWnd();

 virtual bool Create(AWnd *parent, String const &className, int nCmdShow);
 virtual int Loop(int acceleratorID);
 virtual int Loop();
 Point GetCursor();
 void ActivateWindow();

 protected:

 void SetMenuText(int id, String const &txt);
 void SetMenuCheck(int id, bool checked);


 virtual bool OnClosing(){return true;}
 virtual void OnSize(){};
 virtual void OnPaint(){};
 virtual WMR  MenuHandler(int menuID){ return WMR::Default; }
 virtual void OnDrawItem(int childID, DRAWITEMSTRUCT *dis);
 virtual WMR  OnContextMenu(HWND hChild, Point const &pt) { return WMR::Default; }
 virtual WMR  OnCommand(int id, HWND hCtrl) { return WMR::Default; } // true if handled
 virtual WMR  MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return WMR::Default; }
 static LRESULT CALLBACK PopUpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 HMENU m_hMenu;
 
};

class SubClassWnd : public AWnd
{
 public:

 void SubClass();
 
 protected:

 virtual WMR OnSubKeyUp(KeyEventArgs const &k) { return WMR::Default; }
 virtual WMR OnSubKeyDown(KeyEventArgs const &k) { return WMR::Default; }
 virtual WMR OnSubMouseDown(MouseEventArgs const &m) { return WMR::Default; }
 virtual WMR OnSubMouseUp(MouseEventArgs const &m) { return WMR::Default; }

 virtual WMR SubClassHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};

///////////////////////////////////////////////////

class EditWnd : public AWnd
{
 public:

 virtual void Create(AWnd *parent, int child, Rect const &rct);
 
 void ShowPassword(bool yesno);
};

///////////////////////////////////////////////////

class ButtonWnd : public AWnd
{
 public:

 virtual void Create(AWnd *parent, String const &text, int child, Rect const &rct);
 void SetIcon(int iconResourceID, Size const &sz);

};


///////////////////////////////////////////////////

class PanelWnd : public AWnd
{
 public:
 PanelWnd();
~PanelWnd();

 virtual void Create(AWnd *parent, Rect const &rct);
 virtual Rect ClientRect();
 virtual Size ClientSize();
 void Clear(){};

 virtual HFONT GetFont() { return m_hFont; }

 static LRESULT CALLBACK PanelWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 protected:

 virtual void OnPaint(HDC hDC);
 virtual WMR  OnCommand(int id, HWND hCtrl) { return WMR::Default; } // true if handled
 virtual void OnSize(){};
 virtual void OnMouseDown(MouseEventArgs const &e){};
 virtual void OnPanelClick(HWND hWnd, MouseEventArgs const &e){};
 virtual void OnTimer(int timerID){};
 virtual WMR  OnNotify(HWND hChild, int child, UINT code, LPARAM lParam) { return WMR::Default; }
 virtual WMR  OnContextMenu(HWND hChild, Point const &pt) { return WMR::Default; }

 void DrawBorder(HDC hDC);

 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 HFONT m_hFont;
 HPEN  m_hBorderPen;
};

// ///////////////////////////////////////////////

class ImageWnd : public PanelWnd
{
 public:

 ImageWnd();
~ImageWnd();

 virtual void OnPaint(HDC hDC);
 
 void SetImage(String const &file, bool refresh);

 protected:
 
 String  m_LastFile;
 String  m_Text;

 Size m_szBmp;

 HBITMAP m_hBmp;
};

// //////////////////////////////////////////////////////

class SplitContainer;

class SplitterWnd : public AWnd  
{
 public:
 enum class SplitterOrientation : int { Vertical, Horizontal };
 SplitterWnd();
~SplitterWnd();

 void Create(AWnd *Par, SplitterOrientation ori, double defaultPos, SplitContainer *container);

 virtual void OnPaint(HDC hDC);
 void OnCaptureMove(MouseEventArgs const &m);
 void OnCaptureUp(MouseEventArgs const &m);
 void OnCaptureDown(MouseEventArgs const &m);

 void CalcSizes(Rect const &parentSize);
 double CalcRatio(Rect const &rct);

 inline Rect Window1() { return m_Window1; }
 inline Rect Window2() { return m_Window2; }

 static LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

 private:

 int    m_SplitWidth;
 double m_Ratio;  // position of splitter

 SplitterOrientation m_Orientation;
 Point m_MousePos;
 bool  m_Capturing;

 Rect m_Window1;
 Rect m_Window2;

 HBRUSH m_hSolidBrush;
 HPEN   m_hNubPen;

 SplitContainer *m_Container;

};

class SplitContainer
{
 public:

 void Create(int id, AWnd *parent, SplitterWnd::SplitterOrientation orient, double defaultPos) 
  {
   m_SplitID=id;
   m_Parent=parent;
   m_Splitter=new SplitterWnd();
   m_Splitter->Create(parent, orient, defaultPos, this);
   m_Wnd1 = nullptr;
   m_Wnd2 = nullptr;
  }

 void SetWindow1(AWnd *wnd) { m_Wnd1 = wnd; }
 void SetWindow2(AWnd *wnd) { m_Wnd2 = wnd; } 

 virtual Rect  SplitterRect(HWND hWnd) { return m_Parent->SplitterRect(hWnd); } 
 virtual Point SplitterCursor(HWND hWnd) { return m_Parent->SplitterCursor(hWnd); }
 virtual WMR   MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return m_Parent->MessageHandler(hWnd, msg, wParam, lParam); }

 Rect GetRect1() { return m_Rect1; }
 Rect GetRect2() { return m_Rect2; }
 Size GetSize1() { return Size(m_Rect1.Width, m_Rect1.Height); }
 Size GetSize2() { return Size(m_Rect2.Width, m_Rect2.Height); }
 Point GetPoint1() { return Point(m_Rect1.X, m_Rect1.Y); }
 Point GetPoint2() { return Point(m_Rect2.X, m_Rect2.Y); }

 void CalcSizes(Rect const &r) 
  { 
   m_Base=r;
   m_Splitter->CalcSizes(r);
   m_Rect1 = m_Splitter->Window1();
   m_Rect2 = m_Splitter->Window2();
   if (m_Wnd1 != nullptr)
     m_Wnd1->SetRect(m_Rect1);
   if (m_Wnd2 != nullptr)
     m_Wnd2->SetRect(m_Rect2); 
  }

 void Show() 
  { 
   m_Splitter->Show(); 
  }

 void SplitterChanged()
  {
   bool bMsg=false;

   m_Rect1 = m_Splitter->Window1();
   m_Rect2 = m_Splitter->Window2();
   if (m_Wnd1 != nullptr)
     m_Wnd1->SetRect(m_Rect1);
   else
     bMsg = true;
   if (m_Wnd2 != nullptr)
     m_Wnd2->SetRect(m_Rect2); 
   else
     bMsg=true;

  if (bMsg == true)
    ::SendMessage(m_Parent->PopUpHandle(), WM_SPLITWINDOWCHANGED, (WPARAM)m_Splitter->Handle(), (LPARAM) m_SplitID);
  }


 SplitterWnd *Splitter() { return m_Splitter; }
 HWND  Handle() 
  { 
   if (m_Splitter == nullptr) return 0;
   return m_Splitter->Handle(); 
  }
 protected:

 Rect  m_Rect1;
 AWnd *m_Wnd1;

 Rect  m_Rect2;
 AWnd *m_Wnd2;

 Rect m_Base;

 AWnd *m_Parent;
 
 SplitterWnd *m_Splitter;
 int m_SplitID;

};

// //////////////////////////////////////////////////

class TabWnd : public AWnd
{
 public:
 TabWnd();
~TabWnd();

 void Create(AWnd *parent, Rect const &rct);
 void AddTab(String const &name, AWnd *child);
 void SetCurentTab(int tab);
 int  GetCurrentTab();
 void SetTabText(int tab, String const &txt);

 Rect GetTabRect();

 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);

 virtual void SetSize(Size const &sz);
 virtual void SetRect(Rect const &r);
 virtual Rect ClientRect();
 virtual Size ClientSize();

 private:

 std::vector<AWnd *>m_Tabs;

};

// ////////////////////////////////////////////

class ListViewItem
{
 public:
 ListViewItem();
 ListViewItem(String const &text);
 ListViewItem(String const &text, int param);
 ListViewItem(String const &txt, int param, int image);
 
 String Text() { return SubItems[0]; }
 void SetText(String const &txt) { SubItems[0]=txt; }

 std::vector<String> SubItems;  

 int    Image; // ImageList index
 int    Tag;
};

class ListViewColumn
{
 public:
 enum class ColumnAlign : int { Left, Middle, Right };

 int    Position;
 int    Width;
 String Name;
 String Text;
 int    Tag;

 ColumnAlign Alignment;
};

class ListView : public SubClassWnd
{
 public:
 enum class DisplayStyle : int{ LargeIcon, SmallIcon, Details, List, Tile };
 enum class AutoSizes : int { Content, Header };
 ListView();
~ListView();

 virtual void Create(AWnd *parent, DWORD xStyle, DWORD style, Rect const &r);
 void SetOwner(AWnd *owner) { m_Owner = owner; }

 DisplayStyle GetViewStyle();
 void ViewStyle(DisplayStyle style);
 void FullRowSelect(bool onoff);
 void GridLines(bool onoff);

 void AddColumn(String const &text, ListViewColumn::ColumnAlign align, int len);
 void InsertAt(int row, ListViewItem const &item);
 void Insert(ListViewItem const &item);
 void RemoveItem(int index); 
 void RemoveItems(std::vector<int> &items);
 void ColumnSize(int col, int size);
 void AutoSize(int col, AutoSizes size); 
 void SetImageList(ImageList *imgList);
 void SetItemText(int row, int col, String const &txt);
 void SetCheckBoxes(bool onoff);
 bool IsItemChecked(int item);
 bool IsItemSelected(int item);
 void SetItemChecked(int item, bool onoff);
 int GetItemParam(int i);
 String GetItemText(int i, int subItem);
 void SelectAll();
 void SelectNone();

 inline bool NotifyIsOff() { return m_TurnOffNotify; }

 virtual void SetRect(Rect const &r);

 void EnsureVisible(int ndx); 
 int Count();

 int GetSelectedItem();
 void SetSelectedItem(int ndx);
 void SetSelected(int ndx, bool onoff, bool turnOffNotify);
 std::vector<int> GetSelectedIndices();
 int SelectedItemsCount();

 void Sort(int col, bool bAscDesc, int (comparer)(LPARAM a, LPARAM b, LPARAM bAscDescCol));

 virtual void Clear();

 ListViewItem  GetItem(int row);

 protected:

 bool       m_TurnOffNotify;
 ImageList *m_ImageList;
 int        m_ColumnCount;
 AWnd      *m_Owner;  // owner of window, used in OnMouseDown
 
};

struct ListViewIndicesGreater // sort descending
{
 bool operator()(int a, int b) const { return a > b; }
};

// //////////////////////////////////////////////

class TreeNode
{
 public:
 TreeNode();
 TreeNode(String const &txt);
 TreeNode(String const &txt, int tag);
 TreeNode(HWND hWnd, HTREEITEM hItem, TreeNode *parent);
 TreeNode(TreeNode const &node);

 inline TreeNode *Parent() const { return m_Parent; }
 inline HTREEITEM Handle() const { return m_hItem; }
 inline HWND ViewHandle() const { return m_hWnd; }
 inline bool IsRoot() { return m_hParent == TVI_ROOT; }
 TreeNode GetParent();

 void ClearNodes();
 std::vector<TreeNode> GetNodes() const;
 TreeNode InsertFirst(String const &txt, int tag);
 TreeNode InsertLast(String const &txt, int tag);

 std::vector<TreeNode> Nodes; // for adding bulk nodes to tree

 String Text;
 int    Tag;

 protected:


 TreeNode *m_Parent;
 HWND m_hWnd;
 HTREEITEM m_hItem;
 HTREEITEM m_hParent;
 
};

class TreeView : public AWnd
{ 
 public:
 TreeView();
~TreeView();

 std::vector<TreeNode> Nodes();

 TreeNode InsertLast(String const &txt, int tag);
 TreeNode InsertFirst(String const &txt, int tag);

 virtual WMR  OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual void SetImageList(ImageList *imgList);
 virtual WMR OnItemChanged(NMTREEVIEW *tvs) { return WMR::Zero; }

 inline bool Clearing() { return m_DoingClear; }

 void Clear();

 void CollapseAll();
 void ExpandAll();

 void SetSelectedNode(TreeNode const &node); 
 bool IsNodeSelected();
 TreeNode SelectedNode();
 void EnsureVisible();
 void RemoveItem(TreeNode const &node);

 void Create(AWnd *parent, Rect const &r);

 protected:

 TreeNode m_SelectedNode;

 ImageList *m_ImageList;

 bool m_DoingClear;
};

// ////////////////////////////////////////////////////

class ADialog : public AWnd
{
 public:

 ADialog();
~ADialog();

 DialogResult Show(WORD dialogID, AWnd *parent);
 virtual void Close(DialogResult dr);
 virtual void Close();

 static DialogResult ConvertReturn(INT_PTR rv);

 //Rect PixelsToUnits(Rect const &r);
 //Point PixelsToUnits(Point const &r);
 //int PixelToUnit(int x);
 

 String GetItemText(int childId);

 void  SetItemText(int childId, String const &txt);
 Rect  GetItemDlgRect(int child);
 void  SetItemVisible(int childId, bool visible);
 bool  GetCheckState(int childId);
 void  SetCheckState(int childId, bool val);
 void  SetItemFocus(int childId);

 Rect GetItemRect(int childId);

 protected:

 virtual void OnInitDialog();
 virtual bool OnClose(){ return true; } // return true to close dialog

 static INT_PTR CALLBACK DialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam); 

 virtual WMR MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
 virtual WMR OnCommand(int id, HWND hCtrl) { return WMR::Zero; } // true if handled
 virtual WMR OnNotify(HWND hChild, int child, UINT code, LPARAM lParam);
 virtual WMR OnPaint(){ return WMR::Zero; }
 virtual void OnListViewItemChanged(int child, NMLISTVIEW *plvn){}
 virtual void OnListViewColumnClick(int child, int col){}
 virtual void OnDoubleClick(int child, NMITEMACTIVATE *pItemAct){};
 virtual void OnItemClick(HWND hWnd, MouseEventArgs const &m){};
 virtual void OnItemClick(int child, MouseEventArgs const &m){};
 virtual void OnMouseDown(MouseEventArgs const &m){};
 virtual void OnMouseMove(MouseEventArgs const &m){};
 virtual void OnKeyDown(KeyEventArgs const &k){};
 virtual void OnKeyDown(int child, KeyEventArgs const &k){};
 virtual void OnTimer(int timerID){};
 virtual void OnDropListChanged(int child, HWND hWnd){};

 void GetUnitRatio();

 DialogResult m_Result;
 INT_PTR      m_CustomResult;

 double m_UnitRatio;
 HWND   m_hShowParent; // used in OnInitDialog to center it
 int    m_DialogID;

};


// ////////////////////////////////////////////////

class StatusBarPane
{
 public:
 enum class Content : int { Text, Progress };
 enum class Style : int { Fixed, AutoSize };

 StatusBarPane(Content cont, Style style);
~StatusBarPane();

 inline Content GetContent() { return m_Content; }
 inline Style   GetStyle()   { return m_Style; }

 inline void SetWidth(int w){ m_Width=w; }
 inline void SetLastCharCount(int c) { m_LastCharCount = c; }
 inline int GetWidth(){ return m_Width; }
 inline int GetLastValue() { return m_LastValue; }
 inline int GetValue() { return m_Value; }
 inline void SetValue(int v) { m_Value = v; }
 inline int GetLastCharCount() { return m_LastCharCount; }
 inline int GetMax() { return m_Max; }

 void SetMax(int max);
// bool SetValue(int v);

// void Paint(DRAWITEMSTRUCT *dis);

 protected:

 Content m_Content;
 Style   m_Style;

 int m_Width;

 int m_Value;           // progress bar value
 int m_Max;             // progress bar max
 int m_LastValue;       // progress bar last value
 int m_LastCharCount;   // calculated # of "#######"

};

// ///////////////////////////////////////////////

class StatusBar : public AWnd
{
 public:

 StatusBar();
~StatusBar();


 void Create(AWnd *parent); // add panes before calling Create
 void AddFixedPane(StatusBarPane::Content c, int width);
 void AddAutoPane(StatusBarPane::Content c);

 void SetText(int pane, String const &txt);
 String GetText(int pane);
 int GetWidth(int pane);

 void ProgressMax(int pane, int max);
 void ProgressValue(int pane, int val);
 void Progress(int pane);

 void OnSize(Rect const &r);  // parent size has changed, need to resize

 virtual void OnDrawItem(DRAWITEMSTRUCT *dis);

 protected:

 void SetWidths(); // set widths of panes

 std::vector<StatusBarPane *> Panes;
 
 int m_Panels; // count of panels passed to create
 int m_CharWidth;

 HPEN m_hPen;
 
};

class ProgressBar
{
 public:
 ProgressBar(StatusBar *bar, int pane);
~ProgressBar(); // reset progress to 0

 void Max(int val);
 void Max(UINT val)   { m_SB->ProgressMax(m_Pane, (int)val); }
 void Max(size_t val) { m_SB->ProgressMax(m_Pane, (int)val); }

 void Progress();

 protected:
 
 StatusBar *m_SB;
 int        m_Pane;
};

// ////////////////////////////////////////////

class ImageList
{
 public:
 
 enum class Style : int { Masked, Regular };

 ImageList();
~ImageList();

 inline HIMAGELIST Handle() const { return m_Handle; }

 void Create(Style style, int width, int height, int initalSize);
 void Add(Bitmap *bmp, int pictureID);
 void AddMasked(int bmpID, COLORREF mask); 
 void Destroy();
 void AddClient(AWnd *wnd) { Clients.push_back(wnd); }
 int GetIndex(int pictureID);
 HICON GetIcon(int index);

 protected:

 std::map<int, int> PictureMap;  // PictureID, ImageList Index
 std::vector<AWnd *> Clients;

 int m_ImageIndex;

 int m_Width;
 int m_Height;
 
 HIMAGELIST m_Handle;
};

////////////////////////////////////////////////

class ToolBar : public AWnd
{
 public:


 void Create(AWnd *parent, int btns);
 void AddItem(int bitmapID, int cmdID, String const &toolTip);

 void AddSeparator();
 void EnableButton(int id, bool enable);

 protected:

 int m_Index;
 int m_Image;

 ImageList m_Images;

};

///////////////////////////////////////////////

class ListItem
{
 public:
 ListItem() 
  { 
   Text.Clear();
   Tag = -1;
  }
 ListItem(String const &txt, int tag)
  {
   Text = txt;
   Tag = tag;
  }

 String Text;
 int    Tag;
};

class ADropList : public AWnd
{
 public:

 ADropList(){};
~ADropList(){};

 bool AddItem(String const &txt, int tag); // false if item exists
 bool AddItem(ListItem const &item);       // false if item exists
 bool SetCurrentItem(ListItem const &byItem);
 bool SetCurrentItem(String const &txt);
 bool SetCurrentItem(int tag);
 bool SetCurSel(int index);
 void Clear();
 int Count() { return (int)Items.size(); }
 ListItem GetSelectedItem();
 bool IsItemSelected(); 

 protected:
 
 std::vector<ListItem> Items;

};

////////////////////////////////////////////

class WaitCursor
{
 public:
 WaitCursor();
~WaitCursor();

 protected:

 HCURSOR m_CurrentCursor;
};