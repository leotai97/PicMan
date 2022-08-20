#include "pch.h"
#include "App.h"

Application::Application()
{
 m_hInst=0;
 Wnd=nullptr; 

 #ifdef _DEBUG

 String::Test();  // make sure the String class works correctly
 DateTime::Test();

 std::vector<BYTE> encrypted;
 String plain = L"test";
 String decrypted;

 encrypted = Utility::Encrypt(plain);
 decrypted = Utility::Decrypt(encrypted); 

 if (plain != decrypted)
   throw L"encryption decrypion failed";

 #endif
}

Application::~Application()
{
}

bool Application::Init(HINSTANCE hInst, int nCmdShow, String const &appName, String const &displayName)
{
 INITCOMMONCONTROLSEX cmcs={0};
 GdiplusStartupInput gdiplusStartupInput;
 wchar_t *name;
 DWORD szName;

 m_hInst=hInst;
 m_AppName=appName;
 m_AppDisplayName=displayName;

 m_AppLocalPath = OpenFileDlg::AppDataFolder();
 m_AppLocalPath += L"\\";
 m_AppLocalPath += m_AppName;
 if (Utility::DirectoryExists(m_AppLocalPath) == false)
  {
   if (Utility::DirectoryCreate(m_AppLocalPath, true) == false)
     return false;
  }

 ::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

 GetEncoder(L"image/png", &m_EncImagePNG);

 name=new wchar_t[MAX_COMPUTERNAME_LENGTH + 1];
 szName=MAX_COMPUTERNAME_LENGTH + 1;
 ::GetComputerName(name, &szName);
 m_ComputerName=name;
 delete [] name;
 
 RegisterClasses();

 cmcs.dwSize=sizeof(INITCOMMONCONTROLSEX);
 cmcs.dwICC=ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES;
 InitCommonControlsEx(&cmcs);

 m_DarkMode = Utility::IsDarkThemeActive();

 return true;
}

void Application::Shutdown()
{
 ::GdiplusShutdown(m_gdiplusToken);
 UnregisterClasses();
}

void Application::Response(String const &msg)
{
 HWND hWnd;

 if (Wnd==nullptr)
   hWnd=NULL;
 else
   hWnd=Wnd->Handle();

 ::MessageBox(hWnd, msg.Chars(), m_AppDisplayName.Chars(), MB_OK | MB_ICONINFORMATION);
}

DialogResult Application::Question(String const &msg, int btns)
{
 int retVal;
 HWND hWnd;

 if (Wnd==nullptr)
   hWnd=NULL;
 else
   hWnd=Wnd->Handle();

 retVal=::MessageBox(hWnd, msg.Chars(), m_AppDisplayName.Chars(), btns | MB_ICONQUESTION);

 return ADialog::ConvertReturn(retVal);
}

void Application::SetClipboardText(String const &val)
{
 HGLOBAL hglbCopy;
 wchar_t *lptstrCopy; 

 if (::OpenClipboard(Wnd->Handle()) == false)
   throw L"OpenClipboard failed";

 ::EmptyClipboard();

 hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (val.Length() + 1) * sizeof(wchar_t)); 
 if (hglbCopy == NULL) 
  { 
   ::CloseClipboard(); 
   throw L"Failed to alloc memory"; 
  } 

 // Lock the handle and copy the text to the buffer. 
 lptstrCopy = (wchar_t *) GlobalLock(hglbCopy); 
 if (lptstrCopy == nullptr)
   throw L"GlobalLock failed";

 ::memcpy(lptstrCopy, val.Chars(), (val.Length()+1) * sizeof(wchar_t)); 
 lptstrCopy[val.Length()+1]=(wchar_t)0; // null terminator
 ::GlobalUnlock(hglbCopy); 
 
 // Place the handle on the clipboard. 
 if (::SetClipboardData(CF_UNICODETEXT, hglbCopy) == 0)
   throw L"SetClipbardData failed";

 ::CloseClipboard();  
}

void Application::GetEncoder(String const &enc, CLSID *id)
{
 UINT  num = 0;          // number of image encoders
 UINT  size = 0;         // size of the image encoder array in bytes

 ImageCodecInfo* pImageCodecInfo = NULL;

 ::GetImageEncodersSize(&num, &size);
 if(size == 0)
   throw L"failed to get encoder";

 pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
 if(pImageCodecInfo == NULL)
   throw L"failed to allocate memory";

 GetImageEncoders(num, size, pImageCodecInfo);

 for(UINT j = 0; j < num; ++j)
  {
   if( wcscmp(pImageCodecInfo[j].MimeType, enc.Chars()) == 0 )
     {
      *id = pImageCodecInfo[j].Clsid;
       free(pImageCodecInfo);
       return;
      }    
   }
 free(pImageCodecInfo);
 throw L"failed to find encoder";
}

void Application::RegisterClasses()
{
 WNDCLASSEXW wcex={0};

 m_SplitterVertWndClass=L"SplitterVertWndClass";
 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style          = CS_HREDRAW | CS_VREDRAW ;
 wcex.lpfnWndProc    = SplitterWnd::SplitterWndProc;
 wcex.cbClsExtra     = 0;
 wcex.cbWndExtra     = 0;
 wcex.hInstance      = m_hInst;
 wcex.hIcon          = 0;
 wcex.hCursor        = LoadCursor(0, IDC_SIZEWE);
 wcex.hbrBackground  = ::GetSysColorBrush(COLOR_BTNFACE);
 wcex.lpszMenuName   = 0;
 wcex.lpszClassName  = m_SplitterVertWndClass.Chars();
 wcex.hIconSm        = 0;
 RegisterClassExW(&wcex);

 m_SplitterHorzWndClass=L"SplitterHorzWndClass";
 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style          = CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc    = SplitterWnd::SplitterWndProc;
 wcex.cbClsExtra     = 0;
 wcex.cbWndExtra     = 0;
 wcex.hInstance      = m_hInst;
 wcex.hIcon          = 0;
 wcex.hCursor        = LoadCursor(0, IDC_SIZENS);
 wcex.hbrBackground  = ::GetSysColorBrush(COLOR_BTNFACE);
 wcex.lpszMenuName   = 0;
 wcex.lpszClassName  = m_SplitterHorzWndClass.Chars();
 wcex.hIconSm        = 0;
 RegisterClassExW(&wcex);

 m_PanelWndClass=L"PanelWndClass";
 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style          = CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc    = PanelWnd::PanelWndProc;
 wcex.cbClsExtra     = 0;
 wcex.cbWndExtra     = 0;
 wcex.hInstance      = m_hInst;
 wcex.hIcon          = 0;
 wcex.hCursor        = LoadCursor(0, IDC_ARROW);
 wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
 wcex.lpszMenuName   = 0;
 wcex.lpszClassName  = m_PanelWndClass.Chars();
 wcex.hIconSm        = 0;
 RegisterClassExW(&wcex);

}

void Application::UnregisterClasses()
{
 ::UnregisterClass(m_SplitterVertWndClass.Chars(), m_hInst);
 ::UnregisterClass(m_SplitterHorzWndClass.Chars(), m_hInst);
 ::UnregisterClass(m_PanelWndClass.Chars(), m_hInst);
 for (const auto &cls : m_CustomClasses)
   ::UnregisterClass(cls.Chars(), m_hInst);
}

void Application::AddChildWnd(HWND hWnd, AWnd *child)
{
 if (m_ChildWindows.count(hWnd)>0)
   throw L"Child window HWND already exists";

 m_ChildWindows.insert(std::pair<HWND,AWnd *>(hWnd,child));
}

AWnd *Application::GetChildWnd(HWND hWnd)
{
 AWnd *wnd;

 if (m_ChildWindows.count(hWnd)==0)
   return nullptr;

 wnd = m_ChildWindows.at(hWnd);
 return wnd;
}

void Application::RemoveChildWnd(HWND hWnd)
{
 if (m_ChildWindows.count(hWnd)==0)
   throw L"HWND not found";

 m_ChildWindows.erase(hWnd);
}


// //////////////////////////////////////////////////////////

AWnd::AWnd()
{
 m_hWnd=0;
 m_Parent=nullptr;
 m_ChildID = 0;
 m_IsDestroyed = false;
 m_IsPopUpWnd = false;
 m_IsDialog = false;
}

AWnd::~AWnd()
{
}

String AWnd::GetText()
{
 String ret;
 WCHAR *txt;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 txt=new WCHAR[1000];
 ::GetWindowText(m_hWnd, txt, 1000);
 ret=txt;
 delete [] txt; 

 return ret;
}

void AWnd::SetText(String const &txt)
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 ::SetWindowText(m_hWnd, txt.Chars());
}

Rect AWnd::GetScreenRect(HWND hWnd)
{
 RECT wr;
 Rect rct;

 #ifdef _DEBUG
 if (hWnd == 0) throw L"HWND = 0";
 #endif


 ::GetWindowRect(hWnd, &wr);

 rct.X = wr.left;
 rct.Y = wr.top;

 rct.Width = wr.right - wr.left;
 rct.Height = wr.bottom - wr.top;

 return rct;
}

Rect AWnd::ClientRect(HWND hWnd)
{
 RECT wr;
 Rect rct;

 #ifdef _DEBUG
 if (hWnd == 0) throw L"HWND = 0";
 #endif

 ::GetClientRect(hWnd, &wr);

 rct.X = wr.left;
 rct.Y = wr.top;

 rct.Width = wr.right - wr.left;
 rct.Height = wr.bottom - wr.top;

 return rct;
}

Rect AWnd::GetRect()
{
 POINT pt;
 Rect r, rt;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif
 
 r=GetScreenRect(m_hWnd);
 if (m_Parent == nullptr)
   return r;

 pt.x = r.X;
 pt.y = r.Y;

 ::ScreenToClient(m_Parent->Handle(), &pt);

 rt = Rect(pt.x, pt.y, r.Width, r.Height); 

 return rt;
}

Size AWnd::GetSize()
{
 Rect r;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 r = GetScreenRect(m_hWnd);

 return Size(r.Width, r.Height);
 
}

HFONT AWnd::GetFont()
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 return GetWindowFont(m_hWnd);
}

Size AWnd::ClientSize()
{
 Size sz;
 RECT wr;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif
 
 ::GetClientRect(m_hWnd, &wr);
 sz=Size(wr.right-wr.left, wr.bottom - wr.top);

 return sz;
}

Rect AWnd::ClientRect()
{
 Rect r;
 RECT wr;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif
 
 ::GetClientRect(m_hWnd, &wr);
 r=Utility::GetRect(wr);

 return r;
}

void AWnd::SetRect(Rect const &r)
{
 Rect nr;

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 ::MoveWindow(m_hWnd, r.X, r.Y, r.Width, r.Height, TRUE);
 if (IsVisible()==true)
  {
   ::InvalidateRect(m_hWnd, nullptr, false);
   ::UpdateWindow(m_hWnd);
  }
}

void AWnd::SetPoint(Point const &p)
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 Size sz = ClientSize();
 ::MoveWindow(m_hWnd, p.X, p.Y, sz.Width, sz.Height, TRUE); 
 if (IsVisible()==true)
   ::UpdateWindow(m_hWnd);
}

void AWnd::SetSize(Size const &sz)
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 ::SetWindowPos(m_hWnd, 0, 0, 0, sz.Width, sz.Height, SWP_NOZORDER | SWP_NOMOVE);
}

void AWnd::Border(bool onoff)
{
 LONG lStyles = GetWindowLong(m_hWnd, GWL_STYLE);
 LONG lNew;

 if (onoff == true)
   lNew = lStyles | WS_BORDER;
 else
   lNew = lStyles & ~WS_BORDER;   

 SetWindowLong(m_hWnd, GWL_STYLE, lNew);
}

bool AWnd::HasBorder()
{
 LONG lStyles = GetWindowLong(m_hWnd, GWL_STYLE);

 if (lStyles & WS_BORDER )
   return true;
 else
   return false;
}

void AWnd::Visible(bool val)
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 if (val==true)
  {
   ::ShowWindow(m_hWnd, SW_SHOW);
  }
 else
  {
   ::ShowWindow(m_hWnd, SW_HIDE);
  }
 ::UpdateWindow(m_hWnd);
}

bool AWnd::IsVisible()
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 return ::IsWindowVisible(m_hWnd);
}

void AWnd::Refresh()
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 ::InvalidateRect(m_hWnd, nullptr, false);
 ::UpdateWindow(m_hWnd);
}

void AWnd::SetFocus()
{
 ::SetFocus(m_hWnd);
}

void AWnd::Maximize()
{
 ::ShowWindow(m_hWnd, SW_MAXIMIZE);
}

bool AWnd::Minimized()
{
 LONG lStyles = GetWindowLong(m_hWnd, GWL_STYLE);

 if (lStyles & WS_MINIMIZE )
   return true;
 else
   return false;
}

bool AWnd::Maximized()
{
 LONG lStyles = GetWindowLong(m_hWnd, GWL_STYLE);

 if (lStyles & WS_MAXIMIZE )
   return true;
 else
   return false;
}

bool AWnd::Normal()
{
 LONG lStyles = GetWindowLong(m_hWnd, GWL_STYLE);

 if (lStyles & WS_MINIMIZE ) return false;
 if (lStyles & WS_MAXIMIZE ) return false;

 return true;
}

void AWnd::Show()
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 ::ShowWindow(m_hWnd, SW_SHOW);
 ::UpdateWindow(m_hWnd);
}

void AWnd::CreateWnd(AWnd *parent, HWND hWnd, String const &wcls, int childId)
{

 //if (hParent == 0)             LoginDlg appears before MainWnd
 //  throw L"Parent HWND = 0";

 if (hWnd == 0)
   throw L"HWND = 0"; 

 m_Class=wcls;
 m_Parent=parent;
 m_hWnd=hWnd;
 m_ChildID=childId;
 App->AddChildWnd(m_hWnd,this);

}

void AWnd::DestroyWnd()
{
 if (m_hWnd==0)
   throw L"HWND is 0";

 App->RemoveChildWnd(m_hWnd);
 
 m_hWnd=0;
 m_Parent=nullptr;
}

String AWnd::GetInfo()
{
 Rect r=GetRect();
 String info;

 info=L"HWND: ";
 info+=String::Hexadecimal((int)m_hWnd, 8);
 info+=L"X: ";
 info+=String::Decimal(r.X);
 info+=L"Y: ";
 info+=String::Decimal(r.Y);
 info+=L"W: ";
 info+=String::Decimal(r.Width);
 info+=L"H: ";
 info+=String::Decimal(r.Height);

 if (::IsWindowVisible(m_hWnd)==TRUE)
   info+= L"Visible";
 else
   info+= L"Not Visible";
 
 return info;
}

void AWnd::Close()
{

 #ifdef _DEBUG
 if (m_hWnd == 0) throw L"HWND = 0";
 #endif

 App->RemoveChildWnd(m_hWnd);
 ::DestroyWindow(m_hWnd);
 m_IsDestroyed = true;
}

void AWnd::Attach(ADialog *dlg, int childId)
{
 String cls;
 wchar_t *wcls;
 HWND hWnd;
 int c;

 hWnd = ::GetDlgItem(dlg->Handle(), childId);

 wcls=new wchar_t[1000]; 
 c = ::GetClassName(hWnd, wcls, 1000); 

 if (c == 0)
   throw L"couldn't get class name";

 cls=String(wcls);
 delete [] wcls; 

 CreateWnd(dlg, hWnd, cls, childId);
}

Point AWnd::SplitterCursor(HWND hSplit)
{
 Rect rct;
 POINT pt;

 if (m_Parent->IsPopUpWnd() == true || m_Parent->IsDialog()) // top level window?
  {
   ::GetCursorPos(&pt);           // monitor's cursor pos
   ::ScreenToClient(m_hWnd, &pt); // convert to window coords
   rct=ClientRect();              // convert to client area coords
   return Point(pt.x - rct.X, pt.y - rct.Y);
  }
 else
  {
   return m_Parent->SplitterCursor(hSplit); // recursive
  }
}

HWND AWnd::PopUpHandle()
{
 if (m_Parent == nullptr)
   return m_hWnd;
 else
   return m_Parent->PopUpHandle();
}

// /////////////////////////////////////////////
PopUpWnd::PopUpWnd()
{
}

PopUpWnd::~PopUpWnd()
{
}

bool PopUpWnd::Create(AWnd *parent,String const &className, int nCmdShow)
{
 String wcls, name;
 String s;
 HWND hParent;
 BOOL dmValue = App->DarkMode();

 wcls = className;
 name = App->AppDisplayName();
 m_IsPopUpWnd = true;

 if (parent == nullptr)
   hParent = 0;
 else
   hParent = parent->Handle();

 m_hWnd = ::CreateWindowW(wcls.Chars(), name.Chars(), WS_OVERLAPPEDWINDOW, 20, 20, 700, 450, hParent, nullptr, App->Instance(), nullptr);

 //m_hWnd = ::CreateWindowEx(xstyle, m_PopUpWndClass.Chars(), App->AppDisplayName().Chars(), style, 20, 20, 700, 450, nullptr, nullptr, App->Instance(), nullptr);
 if (m_hWnd==0)
  {
   String err = String::GetLastErrorMsg(GetLastError());
   throw err.Chars();
  }

 ::DwmSetWindowAttribute(m_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dmValue, sizeof(dmValue));

 CreateWnd(parent, m_hWnd, className, 0);

 m_hMenu = ::GetMenu(m_hWnd);
 
 ::ShowWindow(m_hWnd, nCmdShow);
 ::UpdateWindow(m_hWnd);

 return true;
}

int PopUpWnd::Loop(int acceleratorID)
{
 HACCEL hAccelTable = LoadAccelerators(App->Instance(), MAKEINTRESOURCE(acceleratorID));
 MSG msg;

 while (::GetMessage(&msg, nullptr, 0, 0))
  {
   if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
     ::TranslateMessage(&msg);
     ::DispatchMessage(&msg);
    }
  }
 return (int) msg.wParam;
}

int PopUpWnd::Loop()
{
 MSG msg;

 while (::GetMessage(&msg, nullptr, 0, 0))
  {
   ::TranslateMessage(&msg);
   ::DispatchMessage(&msg);
  }
 return (int) msg.wParam;
}

LRESULT CALLBACK PopUpWnd::PopUpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 PopUpWnd *wnd = (PopUpWnd *)App->GetChildWnd(hWnd);
 WMR ret = WMR::Default;
 int wmId;

 if (wnd == nullptr)
   return DefWindowProc(hWnd, message, wParam, lParam);

 switch (message)
  {
   case WM_COMMAND:
     wmId = LOWORD(wParam);
     ret = wnd->MenuHandler(wmId);
     break;
   case WM_DRAWITEM:
     wnd->OnDrawItem((int)wParam, (DRAWITEMSTRUCT *)lParam);
     ret = WMR::One;
     break;
   case WM_PAINT:
     wnd->OnPaint();
     ret = WMR::Zero;
     break;
   case WM_ERASEBKGND:
     ret = WMR::One;     // 1 says, I took care of erasing the background, even though I didn't
     break;
   case WM_DESTROY:
     ::PostQuitMessage(0);
     ret = WMR::Zero;
     break;
   case WM_SIZE:
     wnd->OnSize();
     ret = WMR::Zero;
     break;
   case WM_NOTIFY:
     ret = wnd->OnNotify(((LPNMHDR)lParam)->hwndFrom, ((LPNMHDR)lParam)->idFrom, ((LPNMHDR)lParam)->code, lParam);
     break;
   case WM_CONTEXTMENU:
     ret = wnd->OnContextMenu((HWND)wParam, Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
     break;
   case WM_SPLITWINDOWCHANGED:
     wnd->OnSplitterChanged((HWND)wParam, (int)lParam);
     ret = WMR::Zero;
     break;
   default:
     ret = wnd->MessageHandler(hWnd, message, wParam, lParam);
   }
 switch(ret)
  {
   case WMR::Default: return DefWindowProc(hWnd, message, wParam, lParam);
   case WMR::One:     return 1;
   case WMR::Zero:    return 0;
   default: throw L"unhandled case";
  }
}

void PopUpWnd::OnDrawItem(int child, DRAWITEMSTRUCT *dis)
{
 AWnd *wnd  = App->GetChildWnd(dis->hwndItem);
 if (wnd != nullptr)
   wnd->OnDrawItem(dis);
}

Point PopUpWnd::GetCursor()
{
 POINT cur;
 Point pt;

 ::GetCursorPos(&cur);
 pt.X = cur.x - pt.X;
 pt.Y = cur.y - pt.Y;
 
 return pt;
}

void PopUpWnd::ActivateWindow()
{
 ::SetActiveWindow(m_hWnd);
}

void PopUpWnd::SetMenuText(int menuID, String const &txt)
{
 MENUITEMINFO item={0};

 item.cbSize=sizeof(MENUITEMINFO);
 item.fMask = MIIM_STRING | MIIM_STATE | MIIM_FTYPE;
 item.fType = MFT_STRING;
 item.fState = MFS_ENABLED;
 item.dwTypeData=txt.Chars(); 
 item.cch = txt.Length()+1;

 if (::SetMenuItemInfo(m_hMenu, menuID, false, &item) == FALSE)
   throw L"Failed";
}

void PopUpWnd::SetMenuCheck(int menuID, bool checked)
{
 MENUITEMINFO item={0};

 item.cbSize=sizeof(MENUITEMINFO);
 item.fMask =  MIIM_STATE;
 if (checked == true)
   item.fState = MFS_CHECKED;
 else
   item.fState = MFS_UNCHECKED;
 
 if (::SetMenuItemInfo(m_hMenu, menuID, false, &item) == FALSE)
   throw L"Failed";
}
///////////////////////////////////////////////////

void SubClassWnd::SubClass()
{
 ::SetWindowSubclass(m_hWnd, SubClassWnd::SubClassProc, m_ChildID, (DWORD_PTR)this); 
}

LRESULT SubClassWnd::SubClassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
 SubClassWnd *wnd = (SubClassWnd *)dwRefData;
 
 switch(wnd->SubClassHandler(hWnd, msg, wParam, lParam))
  {
   case WMR::One:  return 1;
   case WMR::Zero: return 0;
   case WMR::Default: return DefSubclassProc(hWnd, msg, wParam, lParam);
   default: throw L"Unhandled case";
  }
}

WMR SubClassWnd::SubClassHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch(msg)
  {
   case WM_DESTROY:
     RemoveWindowSubclass(m_hWnd, SubClassWnd::SubClassProc, m_ChildID);
     return WMR::Zero; 
   case WM_KEYUP:
      return OnSubKeyUp(KeyEventArgs(wParam, lParam));
   case WM_KEYDOWN:
     return OnSubKeyDown(KeyEventArgs(wParam, lParam));
   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_RBUTTONDOWN:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     return OnSubMouseDown(e);
    } break;
   case WM_LBUTTONUP:
   case WM_MBUTTONUP:
   case WM_RBUTTONUP:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     return OnSubMouseUp(e);
    } break;
  }
 return WMR::Default;
}

///////////////////////////////////////////////////

void EditWnd::Create(AWnd *parent, int childId, Rect const &r)
{
 String err;
 HWND hWnd;
 DWORD style;

 style=WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT;

 hWnd=::CreateWindow(L"EDIT", L"", style, r.X, r.Y, r.Width, r.Height, parent->Handle(), (HMENU)childId, App->Instance(), 0);
 if (hWnd==0)
  {
   err=String::GetLastErrorMsg(::GetLastError());
   throw L"create window failed";
  }
 CreateWnd(parent, hWnd, L"EDIT", childId);

 ::ShowWindow(hWnd, SW_SHOW);
}

void EditWnd::ShowPassword(bool yesno)
{
 if (yesno == false)
  {
   ::SendMessage(m_hWnd, EM_SETPASSWORDCHAR, L'*', 0);
  }
 else
  {
   ::SendMessage(m_hWnd, EM_SETPASSWORDCHAR, 0, 0);
  }
 Refresh();
}

////////////////////////////////////////////////////

void ButtonWnd::Create(AWnd *parent, String const &text, int childId, Rect const &r)
{
 String err;
 HWND hWnd;
 DWORD style;

 style=WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | BS_TEXT;

 hWnd=::CreateWindow(L"BUTTON", text.Chars(), style, r.X, r.Y, r.Width, r.Height, parent->Handle(), (HMENU)childId, App->Instance(), 0);
 if (hWnd==0)
  {
   err=String::GetLastErrorMsg(::GetLastError());
   throw L"create window failed";
  }
 CreateWnd(parent, hWnd, L"BUTTON", childId);

 ::SendMessage(hWnd, WM_SETFONT, (WPARAM)parent->GetFont(), MAKELPARAM(TRUE, 0));

 ::ShowWindow(hWnd, SW_SHOW);
}

void ButtonWnd::SetIcon(int iconID, Size const &sz)
{
 HICON hIcon = (HICON) ::LoadImage(App->Instance(), MAKEINTRESOURCE(iconID), IMAGE_ICON, sz.Width, sz.Height, 0);

 ::SendMessage(m_hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

 ::DeleteObject(hIcon); // clean up
}


////////////////////////////////////////////////////

PanelWnd::PanelWnd()
{
 m_hFont = 0;
 m_hBorderPen = 0;
}

PanelWnd::~PanelWnd()
{
 if (m_hFont != 0)
   ::DeleteObject(m_hFont);

 if (m_hBorderPen != 0)
   ::DeleteObject(m_hBorderPen);
}

void PanelWnd::Create(AWnd *parent, Rect const &r)
{
 NONCLIENTMETRICS metrics = {};
 String err;
 HWND hWnd;
 DWORD style;
 int childId=0;

 m_hBorderPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWFRAME));

 metrics.cbSize = sizeof(metrics);
 ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
 m_hFont = CreateFontIndirect(&metrics.lfCaptionFont);

 style=WS_CHILD | WS_VISIBLE | WS_BORDER;

 hWnd=::CreateWindow(App->PanelWndClass().Chars(), L"", style, r.X, r.Y, r.Width, r.Height, parent->Handle(), (HMENU)childId, App->Instance(), 0);
 if (hWnd==0)
  {
   err=String::GetLastErrorMsg(::GetLastError());
   throw L"create window failed";
  }

 CreateWnd(parent, hWnd, App->PanelWndClass(), childId);

 ::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));

 ::ShowWindow(hWnd, SW_SHOW);
}

Size PanelWnd::ClientSize()
{
 Size sz;

 sz.Width = AWnd::GetRect().Width;
 sz.Height = AWnd::GetRect().Height;

 if (true == HasBorder())
   return Size(sz.Width-2, sz.Height-2);
 else
   return sz;
}

Rect PanelWnd::ClientRect()
{
Size sz;

 sz.Width = AWnd::GetRect().Width;
 sz.Height = AWnd::GetRect().Height;

 if (true == HasBorder())
   return Rect(1, 1, sz.Width-2, sz.Height-2);
 else
   return Rect(0, 0, sz.Width, sz.Height);
}

void PanelWnd::OnPaint(HDC hDC)
{
 HBRUSH bg;
 Size sz;
 RECT rct;

 sz = ClientSize();
 ::SetRect(&rct, 0, 0, sz.Width, sz.Height);

 if (App->DarkMode() == true)
   bg = ::GetSysColorBrush(COLOR_WINDOWTEXT);
 else
   bg = ::GetSysColorBrush(COLOR_WINDOW);

 ::FillRect(hDC, &rct, bg);
 DrawBorder(hDC);
}

void PanelWnd::DrawBorder(HDC hDC)
{
 HPEN hOld;
 RECT rct;
 Size sz;

 return;  // don't have to draw it?

 ::GetWindowRect(m_hWnd, &rct);
 sz.Width = rct.right - rct.left;
 sz.Height = rct.bottom = rct.top;
  
 if (HasBorder() == true)
  {
   hOld = (HPEN)::SelectObject(hDC, m_hBorderPen);
   ::MoveToEx(hDC, 0, 0, NULL);
   ::LineTo(hDC, sz.Width-1, 0);           // across top left to right
   ::LineTo(hDC, sz.Width-1, sz.Height-1); // down top right to lower right
   ::LineTo(hDC, 0, sz.Height-1);          // across lower right to lower left
   ::LineTo(hDC, 0, 0);                    // up lower left to upper left
   ::SelectObject(hDC, hOld);
  }
}

LRESULT CALLBACK PanelWnd::PanelWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 PanelWnd *wnd=(PanelWnd *)App->GetChildWnd(hWnd);
 WMR ret;

 switch(message)
  {
   case WM_NCCREATE: return TRUE;
  }
 if (wnd == nullptr)
   return DefWindowProc(hWnd, message, wParam, lParam);

 ret = wnd->MessageHandler(hWnd, message, wParam, lParam);   
 switch(ret)
  {
   case WMR::Zero:    return 0;
   case WMR::One:     return 1;
   case WMR::Default: return DefWindowProc(hWnd, message, wParam, lParam);
   default: throw L"Unhandled case";
  }
}

WMR PanelWnd::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 WMR ret = WMR::Default;

 switch(message)
  {
   case WM_PAINT: 
    {
     PAINTSTRUCT ps;
     HDC hDC = ::BeginPaint(m_hWnd, &ps);
     OnPaint(hDC);
     ::EndPaint(m_hWnd, &ps);
     ret = WMR::Zero;
    } break;
   case WM_COMMAND:
    {
     int cmdID=LOWORD(wParam);
     HWND hCtrl=(HWND)lParam;
     ret = OnCommand(cmdID,hCtrl);
    } break;
   case WM_NOTIFY:
     ret = OnNotify(((LPNMHDR)lParam)->hwndFrom, ((LPNMHDR)lParam)->idFrom, ((LPNMHDR)lParam)->code, lParam);
     break;
   case WM_SIZE:
     OnSize();
     ret = WMR::Zero;
     break;
   case WM_ERASEBKGND:
     ret = WMR::One;
     break;
   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_RBUTTONDOWN:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     OnMouseDown(e);
     ::SendMessage(m_Parent->Handle(), WM_PANELWNDCLICKED, (WPARAM)m_hWnd, (LPARAM)&e);
     ret = WMR::Zero;
    } break;
   case WM_PANELWNDCLICKED:
      OnPanelClick((HWND)wParam, MouseEventArgs((MouseEventArgs *)lParam));
      ret = WMR::Zero;
      break;
   case WM_SETCURSOR:
      ::SetCursor(::LoadCursor(0, IDC_ARROW));
      ret = WMR::One;
      break;
   case WM_TIMER:
      OnTimer((int)wParam);
      WMR::Zero;
      break;
   case WM_CONTEXTMENU:
      ret = OnContextMenu((HWND)wParam, Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
      break;
   default:
      ret = WMR::Default;
  }
 return ret;
}
// /////////////////////////////////////////////////

ImageWnd::ImageWnd()
{
 m_LastFile.Clear();
 m_Text.Clear();
 m_hBmp = 0;
}

ImageWnd::~ImageWnd()
{
 if (m_hBmp != 0)
  ::DeleteObject(m_hBmp);
}

void ImageWnd::SetImage(String const &file, bool refresh)
{
 Bitmap *bmp;

 if (file == m_LastFile)
   return; // no change 

 if (file.Length()==0)
  {
   if (m_hBmp != 0)
    {
     ::DeleteObject(m_hBmp);
     m_hBmp = 0;
     if (refresh == true)
       Refresh();
     return;
    }
  }

 m_LastFile = file;

 if (m_hBmp != nullptr)
  {
   ::DeleteObject(m_hBmp);
   m_hBmp = 0;
  }

 try
  {
   bmp=Bitmap::FromFile(file.Chars(),0);
   m_Text = L"";
   bmp->GetHBITMAP(Color::White, &m_hBmp);
   m_szBmp.Width = bmp->GetWidth();
   m_szBmp.Height = bmp->GetHeight();
   delete bmp;
  }
 catch(...)
  {
   m_Text = L"Unable to load ";
   m_Text += file;
  }

 if (refresh == true)
   Refresh();
}

void ImageWnd::OnPaint(HDC hDC)
{
 HDC hMem;
 String strDisp;
 HBRUSH bg;
 RECT rct;
 Size sz;
 Rect r;
 double dRi, dRs;
 int nIW, nIH;
 int nSW, nSH;
 int nFX, nFY, nFW, nFH;

 if (App->DarkMode() == true)
   bg = ::GetSysColorBrush(COLOR_WINDOWTEXT);
 else
   bg = ::GetSysColorBrush(COLOR_WINDOW);

 r=AWnd::ClientRect();
 ::SetRect(&rct, 0, 0, r.Width, r.Height);
 ::FillRect(hDC, &rct, bg);

 if (m_Text.Length() > 0)
  {
   ::DrawText(hDC, m_Text.Chars(), m_Text.Length(), &rct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  }
 else
  {
   if (m_hBmp != 0)
    {
     nIW = m_szBmp.Width;
     nIH = m_szBmp.Height;
     nSW = r.Width;
     nSH = r.Height;

     dRi = (double)nIW / (double)nIH;
     dRs = (double)nSW / (double)nSH;

     if (dRi > dRs)
      {
       nFH = (int)(nSW * nIH / nIW);
       nFX = r.X;
       nFY = r.Y+(int)(nSH / 2) - (int)(nFH / 2);
       nFW = nSW;
      }
     else
      {
       nFW = (int)(nSH * dRi);
       nFY = r.Y;
       nFX = r.X+(int)(nSW / 2) - (int)(nFW / 2);
       nFH = nSH;
      }
    hMem=::CreateCompatibleDC(hDC);
    ::SelectObject(hMem, m_hBmp);
    ::SetStretchBltMode(hDC, COLORONCOLOR); //important! for smoothness
    ::StretchBlt(hDC, nFX, nFY, nFW, nFH, hMem, 0, 0, nIW, nIH, SRCCOPY);
    ::DeleteDC(hMem);
    }
  }
 DrawBorder(hDC);
}

// /////////////////////////////////////////////////

SplitterWnd::SplitterWnd() : AWnd()
{
 m_Window1=Rect();
 m_Window2=Rect();
 m_Capturing=false;
 m_SplitWidth=7;
 m_hSolidBrush=0;
 m_hNubPen=0;
}

SplitterWnd::~SplitterWnd()
{
 if (m_hSolidBrush != 0) 
   ::DeleteObject(m_hSolidBrush);
 if (m_hNubPen !=0 )     
   ::DeleteObject(m_hNubPen);
}

void SplitterWnd::Create(AWnd *parent, SplitterOrientation ori, double defaultPos, SplitContainer *container)
{
 String cls;
 HWND hWnd;
 Rect rct;
 DWORD style;
 int x,y,w,h;
 
 m_hSolidBrush=::CreateSolidBrush(RGB(0xB0, 0xC4, 0xDE)); // light steel blue

 m_Ratio=defaultPos;
 m_Orientation=ori;
 m_Container = container;
 m_Parent=parent;

 rct=parent->GetRect();

 if (ori==SplitterOrientation::Vertical)
  {
   m_hNubPen=::CreatePen(PS_SOLID, 3, RGB(0x46, 0x82, 0xB4)); // steel blue
   x=(int)((double)rct.Width * m_Ratio);
   y=0;
   w=m_SplitWidth;
   h=rct.Height;
   cls=App->SplitterVertWndClass();
  }
 else
  {
   m_hNubPen=::CreatePen(PS_SOLID, 2, RGB(0x46, 0x82, 0xB4));
   x=0;
   y=(int)((double)rct.Height * m_Ratio);
   w=rct.Width;
   h=m_SplitWidth;
   cls=App->SplitterHorzWndClass();
  }
 style = WS_CHILD | WS_VISIBLE;
 hWnd=::CreateWindow(cls.Chars(), L"SplitWnd", style, x, y, w, h, parent->Handle(), 0, App->Instance(), 0);
 CreateWnd(parent, hWnd, cls, 0);
::ShowWindow(hWnd, SW_SHOW);
}

void SplitterWnd::CalcSizes(Rect const &pr)
{
 Rect w1,w2;
 int x,y,w,h;

 if (m_Orientation==SplitterOrientation::Vertical) 
  {
   x=pr.X+(int)((double)pr.Width * m_Ratio);   // side by side
   y=pr.Y;
   w=m_SplitWidth;
   h=pr.Height;
   w1=Rect(pr.X, pr.Y, x, h);
   w2=Rect(x+m_SplitWidth, pr.Y, pr.Width-(x+m_SplitWidth), h);
  }
 else
  {
   x=pr.X;
   y=pr.Y+(int)((double)pr.Height * m_Ratio);  // stacked
   w=pr.Width;
   h=m_SplitWidth;
   w1=Rect(pr.X, pr.Y, w, y);
   w2=Rect(pr.X, y+m_SplitWidth, w, pr.Height-(y+m_SplitWidth));
  }
 if (true)  // validate sizes
  {
   SetRect(Rect(x,y,w,h));
   m_Window1=w1;
   m_Window2=w2;
  }
}

void SplitterWnd::OnPaint(HDC hDC)
{
 HGDIOBJ lo;
 Size sz;
 RECT rct;
 int c1, c2;

 sz = ClientSize();

 rct.left=0;
 rct.top=0;
 rct.right=sz.Width;
 rct.bottom=sz.Height;

 ::FillRect(hDC, &rct, m_hSolidBrush);

 if (m_Capturing==false)
  { 
   lo=::SelectObject(hDC, m_hNubPen);
   if (m_Orientation == SplitterOrientation::Vertical)
    {
     c1=sz.Height/2-6;
     c2=sz.Height/2+6;
     ::MoveToEx(hDC, 3, c1, nullptr);
     ::LineTo(hDC, 3, c2);
    }
   else
    {
     c1=sz.Width/2-6;
     c2=sz.Width/2+6;
     ::MoveToEx(hDC, c1, 4, nullptr);
     ::LineTo(hDC, c2, 4);
    }
   ::SelectObject(hDC, lo);
  }
}

double SplitterWnd::CalcRatio(Rect const &rct)
{
 double ratio;
 Point mp;
 int nx, ny;

 mp=m_Parent->SplitterCursor(m_hWnd);

 if (m_Orientation == SplitterOrientation::Vertical)
  {
   nx = m_MousePos.X + mp.X;         // mousepos is set to x of mouse in split 
   ratio = nx / (double)rct.Width; // vert is side by side, ratio width
  }
 else
  {
   ny = m_MousePos.Y + mp.Y;
   ratio = ny / (double)rct.Height; // horz is stacked, so ratio height
  }
 return ratio;
}

void SplitterWnd::OnCaptureDown(MouseEventArgs const &m)
{
 m_Capturing=true;
}

void SplitterWnd::OnCaptureMove(MouseEventArgs const &e)
{
 Point mp;
 Rect rct, swr;
 double r;

 if (m_Capturing==false)
   return;
 
 rct=m_Parent->SplitterRect(m_hWnd);
 r=CalcRatio(rct);
 if (m_Ratio != r)
  {
   m_Ratio=r;
   CalcSizes(rct);
  }
}

void SplitterWnd::OnCaptureUp(MouseEventArgs const &m)
{
 Rect rct;
 
 if (m_Capturing==true)
  {
   m_Capturing=false;
   rct=m_Parent->SplitterRect(m_hWnd);
   m_Ratio=CalcRatio(rct);
   CalcSizes(rct);
   m_Container->SplitterChanged();  
  }
}

LRESULT CALLBACK SplitterWnd::SplitterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 SplitterWnd *wnd=(SplitterWnd *)App->GetChildWnd(hWnd);
 WMR ret;

 switch(message)
  {
   case WM_NCCREATE: return TRUE;
  }
 if (wnd == nullptr)
   return DefWindowProc(hWnd, message, wParam, lParam);

 ret = wnd->MessageHandler(hWnd, message, wParam, lParam);   
 switch(ret)
  {
   case WMR::Zero:    return 0;
   case WMR::One:     return 1;
   case WMR::Default: return DefWindowProc(hWnd, message, wParam, lParam);
   default: throw L"Unhandled case";
  }
}

WMR SplitterWnd::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 HWND hCapture;
 WMR ret = WMR::Default;

 switch(message)
  { 
   case WM_PAINT:
    {
     PAINTSTRUCT ps;
     HDC hDC = ::BeginPaint(m_hWnd, &ps);
     OnPaint(hDC);
     ::EndPaint(m_hWnd, &ps);
     ret = WMR::Zero;
    } break;
   case WM_ERASEBKGND:
     ret = WMR::One;
     break;
   case WM_MOUSEMOVE:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     hCapture=::GetCapture(); 
     if (hCapture == m_hWnd)
      {
       OnCaptureMove(e);
      }
     ret = WMR::Zero;
    } break;
   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_RBUTTONDOWN:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     if (::GetCapture()==NULL)
      {
       m_MousePos=Point(e.X, e.Y);
       ::SetCapture(m_hWnd);
       OnCaptureDown(e);
      }
     ret = WMR::Zero;
    } break;
   case WM_LBUTTONUP:
   case WM_MBUTTONUP:
   case WM_RBUTTONUP:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     if (::GetCapture()!=NULL)
      {
       ::ReleaseCapture();
       OnCaptureUp(e);
      }
     ret = WMR::Zero;
    } break;
   case WM_SETCURSOR:
      if (m_Orientation==SplitterOrientation::Vertical)
        ::SetCursor(::LoadCursor(0, IDC_SIZEWE));
      else
        ::SetCursor(::LoadCursor(0, IDC_SIZENS));
      ret = WMR::One;
      break;
   default:
     ret = WMR::Default;
  }
 return ret;
}

///////////////////////////////////////////////////////

TabWnd::TabWnd()
{
 m_Tabs.clear();
}

TabWnd::~TabWnd()
{
}

void TabWnd::Create(AWnd *parent, Rect const &r)
{
 HWND hWnd;
 DWORD style;
 int childId=0;
 
 style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;
 hWnd = ::CreateWindow(WC_TABCONTROL, L"", style, r.X, r.Y, r.Width, r.Height, parent->Handle(), (HMENU)childId, App->Instance(), NULL); 
 CreateWnd(parent, hWnd, WC_TABCONTROL, childId);

 ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), true);
 ::ShowWindow(hWnd, SW_SHOW);
}

void TabWnd::AddTab(String const &tabName, AWnd *child)
{
 TCITEM tie={0};

 tie.mask = TCIF_TEXT; 
 tie.iImage = -1; 
 tie.pszText = (LPWSTR)tabName.Chars();

 TabCtrl_InsertItem(m_hWnd, (int)m_Tabs.size(), &tie);

 m_Tabs.push_back(child); 
}

void TabWnd::SetCurentTab(int tab)
{
 TabCtrl_SetCurSel(m_hWnd, tab);

}

int TabWnd::GetCurrentTab()
{
 return TabCtrl_GetCurSel(m_hWnd);
}

Size TabWnd::ClientSize()
{
 Size sz=AWnd::ClientSize();
 return sz;
}

Rect TabWnd::ClientRect()
{
 Rect rct=AWnd::ClientRect();
 return rct;
}

void TabWnd::SetSize(Size const &sz)
{
 RECT wr;
 Rect rct;

 AWnd::SetSize(sz);

 ::GetClientRect(m_hWnd, &wr); // get client area of tab control
 TabCtrl_AdjustRect(m_hWnd, FALSE, &wr); // convert client area of control to tab contents
 rct=Utility::GetRect(wr);
 for (const auto &tab : m_Tabs)
  {
   tab->SetRect(rct);
  }
}

Rect TabWnd::GetTabRect()
{
 RECT wr;
 Rect r;

 ::GetClientRect(m_hWnd, &wr); // get client area of tab control
 TabCtrl_AdjustRect(m_hWnd, FALSE, &wr); // convert client area of control to tab contents

 r.X = wr.left;
 r.Y = wr.top;
 r.Width = wr.right-wr.left;
 r.Height = wr.bottom-wr.top;

 return r;
}

void TabWnd::SetRect(Rect const &r)
{
 RECT wr;
 Rect rct;

 AWnd::SetRect(r);

 ::GetClientRect(m_hWnd, &wr); // get client area of tab control
 TabCtrl_AdjustRect(m_hWnd, FALSE, &wr); // convert client area of control to tab contents
 rct=Utility::GetRect(wr);
 for (const auto &tab : m_Tabs)
  {
   tab->SetRect(rct);
  }
}

void TabWnd::SetTabText(int tab, String const &txt)
{
 TCITEM tie;
 
 tie.mask = TCIF_TEXT; 
 tie.iImage = -1; 
 tie.pszText = (LPWSTR)txt.Chars();

 TabCtrl_SetItem(m_hWnd, tab, &tie);
}

WMR TabWnd::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 int cur, i;

 switch(code)
  {
   case TCN_SELCHANGING: return WMR::Zero; // allow tab to change
   case TCN_SELCHANGE: 
    {
     cur = TabCtrl_GetCurSel(m_hWnd);
     i = 0;
     for (const auto &tab : m_Tabs)
      {
       if (i == cur)
         tab->Visible(true);
       else
         tab->Visible(false);
       i++;
      }
    }
  }
 return WMR::Zero;
}

/////////////////////////////////////////////////////////////

ListViewItem::ListViewItem()
{
 SubItems.clear();
 Tag=-1;
 Image = -1;
}

ListViewItem::ListViewItem(String const &txt) : ListViewItem()
{
 SubItems.push_back(txt);
}

ListViewItem::ListViewItem(String const &txt, int param) : ListViewItem()
{
 SubItems.push_back(txt);
 Tag = param;
}

ListViewItem::ListViewItem(String const &txt, int param, int image) : ListViewItem()
{
 SubItems.push_back(txt);
 Tag = param;
 Image = image;
}

/////////////////////////////////////////////////////////////

ListView::ListView()
{
 m_ColumnCount=0;
 m_TurnOffNotify = false;
 m_Owner = nullptr;
}

ListView::~ListView()
{
}

void ListView::Create(AWnd *parent, DWORD xStyle, DWORD style, Rect const &r)
{
 DWORD fstyle, xstyle;
 HWND hWnd;
 int childId=0;

 fstyle= WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_SHAREIMAGELISTS;  // | LVS_REPORT | LVS_EDITLABELS,
 fstyle |= style;

 xstyle = LVS_EX_DOUBLEBUFFER;

 hWnd = ::CreateWindowEx(xstyle, WC_LISTVIEW, L"", fstyle, r.X, r.Y, r.Width, r.Height, parent->Handle(),  (HMENU)childId, App->Instance(), NULL); 
 CreateWnd(parent, hWnd, WC_LISTVIEW, childId);

 ListView_SetExtendedListViewStyleEx(hWnd, 0, xStyle);

 SubClass(); // sub class the ListView to deal with keydown issues

 if (App->DarkMode() == true)
  {
   ListView_SetBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOWTEXT));
   ListView_SetTextBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOWTEXT));
   ListView_SetTextColor(m_hWnd, ::GetSysColor(COLOR_WINDOW));
  }
 else
  {
   ListView_SetBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOW));
   ListView_SetTextBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOW));
   ListView_SetTextColor(m_hWnd, ::GetSysColor(COLOR_WINDOWTEXT));
  }
}

ListView::DisplayStyle ListView::GetViewStyle()
{
 DWORD s;

 s = ListView_GetView(m_hWnd);

 switch(s)
  {
   case LV_VIEW_ICON:      return DisplayStyle::LargeIcon;
   case LV_VIEW_SMALLICON: return DisplayStyle::SmallIcon;
   case LV_VIEW_DETAILS:   return DisplayStyle::Details;
   case LV_VIEW_LIST:      return DisplayStyle::List;
   case LV_VIEW_TILE:      return DisplayStyle::Tile;
  }
 throw L"Unknown style";
}

void ListView::ViewStyle(DisplayStyle style)
{
 LRESULT r;
 DWORD s;

 switch(style)
  {
   case DisplayStyle::LargeIcon: s=LV_VIEW_ICON; break;
   case DisplayStyle::SmallIcon: s=LV_VIEW_SMALLICON; break;
   case DisplayStyle::Details:   s=LV_VIEW_DETAILS; break;
   case DisplayStyle::List:      s=LV_VIEW_LIST; break;
   case DisplayStyle::Tile:      s=LV_VIEW_TILE; break;
  }

 r = ::SendMessage(m_hWnd, LVM_SETVIEW, s, 0);
 if (r != 1)
   throw L"Failed to set display style";
}

void ListView::FullRowSelect(bool onoff)
{
 DWORD s;

 s = ListView_GetExtendedListViewStyle(m_hWnd);

 if (onoff == true)
   s |= LVS_EX_FULLROWSELECT;
 else
   s &= ~LVS_EX_FULLROWSELECT;

 ListView_SetExtendedListViewStyle(m_hWnd, s);
}

void ListView::GridLines(bool onoff)
{
 DWORD s;

 s = ListView_GetExtendedListViewStyle(m_hWnd);

 if (onoff == true)
   s |= LVS_EX_GRIDLINES;
 else
   s &= ~LVS_EX_GRIDLINES;

 ListView_SetExtendedListViewStyle(m_hWnd, s);
}

void ListView::AddColumn(String const &text, ListViewColumn::ColumnAlign align, int len)
{
 LVCOLUMN col={0};

 col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_ORDER;
 col.cx = len;
 col.cchTextMax = text.Length()+1;
 col.pszText = text.Chars(); 
 col.iOrder = m_ColumnCount;

 ListView_InsertColumn(m_hWnd, m_ColumnCount, &col);

 m_ColumnCount++;
}

void ListView::Insert(ListViewItem const &lvi)
{
 InsertAt(Count(), lvi);
}

void ListView::InsertAt(int row, ListViewItem const &lvi)
{
 LVITEM item={0};
 int i;

 if (lvi.SubItems.size() == 0)
   throw L"ListViewItem subitems is empty";

 item.mask=LVIF_TEXT | LVIF_PARAM;

 if (lvi.Image >= 0)
  {
   item.mask = item.mask | LVIF_IMAGE;
   item.iImage = lvi.Image;
  }
 item.iItem = row;
 item.pszText=(LPWSTR)lvi.SubItems[0].Chars();
 item.lParam=lvi.Tag;
 ListView_InsertItem(m_hWnd, &item);

 i = 0;
 for (const auto &sub : lvi.SubItems)
  {
   ListView_SetItemText(m_hWnd, row, i, sub.Chars());
   i++;
  }
}

void ListView::RemoveItem(int index)
{
 ListView_DeleteItem(m_hWnd, index);
}

void ListView::RemoveItems(std::vector<int> &items)
{
 std::sort(items.begin(), items.end(), ListViewIndicesGreater());
 m_TurnOffNotify=true;
 for (const auto &i : items)
  {
   RemoveItem(i);  // remove items starting from last so indices don't get screwed up
  }
 m_TurnOffNotify = false;
}


void ListView::ColumnSize(int col, int sz)
{
 ListView_SetColumnWidth(m_hWnd, col, sz);
}

void ListView::AutoSize(int col, ListView::AutoSizes sz)
{
 if (sz == AutoSizes::Content)
   ListView_SetColumnWidth(m_hWnd, col, LVSCW_AUTOSIZE);
 else
   ListView_SetColumnWidth(m_hWnd, col, LVSCW_AUTOSIZE_USEHEADER);
}

void ListView::Clear()
{
 ListView_DeleteAllItems(m_hWnd);
}

void ListView::SetRect(Rect const &r)
{
 AWnd::SetRect(r);
 if (GetViewStyle() == DisplayStyle::LargeIcon)
  {
   ListView_Arrange(m_hWnd, LVA_DEFAULT);
  }
}

void ListView::SetImageList(ImageList *imgList)
{
 ListView_SetImageList(m_hWnd, imgList->Handle(), LVSIL_NORMAL);
 ListView_SetImageList(m_hWnd, imgList->Handle(), LVSIL_SMALL);
 m_ImageList=imgList;
}

void ListView::SetItemText(int row, int col, String const &txt)
{
 ListView_SetItemText(m_hWnd, row, col, txt.Chars());
}

void ListView::SetCheckBoxes(bool onff)
{
 if (onff == true)
   ListView_SetExtendedListViewStyleEx(m_hWnd, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES); 
 else
   ListView_SetExtendedListViewStyleEx(m_hWnd, LVS_EX_CHECKBOXES, 0); 
}

bool ListView::IsItemChecked(int item)
{
 return ListView_GetCheckState(m_hWnd, item);
}

bool ListView::IsItemSelected(int item)
{
 LRESULT r;

 r = ListView_GetItemState(m_hWnd, item, LVIS_SELECTED);
 if (r == LVIS_SELECTED)
   return true;
 else
   return false;
}


void ListView::SetItemChecked(int item, bool onoff)
{
 ListView_SetCheckState(m_hWnd, item, onoff);
}

int ListView::GetItemParam(int item)
{
 LVITEM lvi = {0};

 lvi.mask = LVIF_PARAM;
 lvi.iItem = item;

 if (ListView_GetItem(m_hWnd, &lvi) == TRUE)
   return (int)lvi.lParam;
 else
   return -1;
}

String ListView::GetItemText(int item, int subItem)
{
 String val;
 wchar_t *text;

 #ifdef _DEBUG

 if (item < 0 || item >= Count())
   throw L"item out of bounds";

 #endif

 text = new wchar_t[300];
 ListView_GetItemText(m_hWnd, item, subItem, text, 300);
 val = text;

 delete [] text;
 
 return val;
}

ListViewItem ListView::GetItem(int row)
{
 ListViewItem item;
 int i;
 String val;
 wchar_t *text;

 #ifdef _DEBUG
 if (row < 0 || row >= Count())
   throw L"item out of bounds";
 #endif

 text = new wchar_t[300];
 ListView_GetItemText(m_hWnd, row, 0, text, 300);
 item = ListViewItem(text);
 for (i=1; i<m_ColumnCount; i++)
  {
   ListView_GetItemText(m_hWnd, row, i, text, 300);
   item.SubItems.push_back(text);
  }
 delete [] text;

 item.Tag = GetItemParam(row);
 
 return item;
}

void ListView::SelectAll()
{
 int c,i;

 m_TurnOffNotify = true;

 c = ListView_GetItemCount(m_hWnd);

 for (i=0; i<c; i++)
  {
   ListView_SetItemState(m_hWnd, i, LVIS_SELECTED, LVIS_SELECTED);
  }

 m_TurnOffNotify = false;
}

void ListView::SetSelectedItem(int ndx)
{
 m_TurnOffNotify = true;

 ListView_SetItemState(m_hWnd, ndx, LVIS_SELECTED, LVIS_SELECTED);

 m_TurnOffNotify = false;
}

void ListView::SetSelected(int ndx, bool onoff, bool turnOffNotify)
{
 int state;

 m_TurnOffNotify = turnOffNotify;

 if (onoff == true)
   state = LVIS_SELECTED;
 else
   state = 0;

 ListView_SetItemState(m_hWnd, ndx, state, LVIS_SELECTED);

 m_TurnOffNotify = false;
}


void ListView::SelectNone()
{
 int c,i;

 m_TurnOffNotify = true;

 c = ListView_GetItemCount(m_hWnd);

 for (i=0; i<c; i++)
  {
   ListView_SetItemState(m_hWnd, i, 0, LVIS_SELECTED);
  }

 m_TurnOffNotify = false;
}

int ListView::Count()
{
 int c;
  
 c = ListView_GetItemCount(m_hWnd);

 return c;
}

void ListView::EnsureVisible(int ndx)
{
 ListView_EnsureVisible(m_hWnd, ndx, false);
}

int ListView::GetSelectedItem()
{
 int i;

 i = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED);
 
 return i;
}

std::vector<int> ListView::GetSelectedIndices()
{
 std::vector<int> list;
 bool loop = true;
 int i;

 if (ListView_GetSelectedCount(m_hWnd) == 0)
   return list;  // empty

 i = -1;
 while(loop)
  {
   i = ListView_GetNextItem(m_hWnd, i, LVNI_SELECTED);
   if (i>=0)
     list.push_back(i);
   else
     loop = false;
  }
 return list;
}

int ListView::SelectedItemsCount()
{
 return ListView_GetSelectedCount(m_hWnd);
}

void ListView::Sort(int col, bool bAscDesc, int (comparer)(LPARAM a, LPARAM b, LPARAM bAscDescCol))
{
 ListView_SortItems(m_hWnd, comparer, Utility::AscendDescendColumn(bAscDesc, col));
}

// ///////////////////////////////////////////////////////

TreeNode::TreeNode()
{
 m_Parent=nullptr;
 m_hWnd = 0;
 m_hItem = 0;
 m_hParent = 0;
 Text.Clear();
 Tag = 0;
}

TreeNode::TreeNode(TreeNode const &node)
{
 Text = node.Text;
 Tag = node.Tag;
 m_Parent = node.Parent();
 m_hWnd = node.ViewHandle();
 m_hItem = node.Handle();
}

TreeNode::TreeNode(String const &txt)
{
 Text = txt;
 Tag = 0;
 m_Parent = nullptr;
 m_hWnd = 0;
 m_hItem = 0;
 m_hParent = 0;
}

TreeNode::TreeNode(String const &txt, int tag)
{
 Text = txt;
 Tag = tag;
 m_Parent = nullptr;
 m_hWnd = 0;
 m_hItem = 0;
 m_hParent = 0;
}

TreeNode::TreeNode(HWND hWnd, HTREEITEM hItem, TreeNode *parent)
{
 wchar_t *buff;
 TVITEM ti={0};

 m_hWnd = hWnd;
 m_hItem=hItem;
 m_Parent=parent;

 buff = new wchar_t[256];

 ti.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
 ti.hItem = hItem;
 ti.cchTextMax=255;
 ti.pszText=buff;

 TreeView_GetItem(hWnd, &ti);

 Text = buff;
 delete [] buff;
 
 Tag = (int)ti.lParam;
}

TreeNode TreeNode::GetParent()
{
 HTREEITEM hParent;

 hParent = TreeView_GetParent(m_hWnd, m_hItem);

 if (hParent == 0)
   return TreeNode(m_hWnd, TVI_ROOT, nullptr);
 else
   return TreeNode(m_hWnd, hParent, nullptr);
}

void TreeNode::ClearNodes()
{
 for (const auto &node : GetNodes())
   TreeView_DeleteItem(m_hWnd, node.Handle());
}

std::vector<TreeNode> TreeNode::GetNodes() const
{
 std::vector<TreeNode> nodes;
 HTREEITEM hItem;
 
 hItem = TreeView_GetChild(m_hWnd, m_hItem);
 if (hItem == 0)
   return nodes; // empty vector

 nodes.push_back(TreeNode(m_hWnd, hItem, nullptr));
 do
  {
   hItem = TreeView_GetNextSibling(m_hWnd, hItem);
   if (hItem != 0)
    {
     nodes.push_back(TreeNode(m_hWnd, hItem, nullptr));
    }
  }
 while(hItem != 0);
 
 return nodes;

}

TreeNode TreeNode::InsertLast(String const &txt, int tag)
{
 TVINSERTSTRUCT ti={0};
 HTREEITEM hItem;
 TreeNode item;

 ti.hParent= m_hItem;
 ti.hInsertAfter = TVI_LAST;
 ti.item.mask=TVIF_TEXT | TVIF_PARAM;
 ti.item.pszText=(LPWSTR)txt.Chars();
 ti.item.cchTextMax=txt.Length()+1;
 ti.item.lParam = tag;
 
 hItem = TreeView_InsertItem(m_hWnd, &ti);

 item = TreeNode(m_hWnd, hItem, this);
 item.Text=txt;
 item.Tag=tag;

 return item;
}

TreeNode TreeNode::InsertFirst(String const &txt, int tag)
{
 TVINSERTSTRUCT ti={0};
 HTREEITEM hItem;
 TreeNode item;

 ti.hParent= m_hItem;
 ti.hInsertAfter = TVI_FIRST;
 ti.item.mask=TVIF_TEXT | TVIF_PARAM;
 ti.item.pszText=(LPWSTR)txt.Chars();
 ti.item.cchTextMax=txt.Length()+1;
 ti.item.lParam = tag;
 
 hItem = TreeView_InsertItem(m_hWnd, &ti);

 item = TreeNode(m_hWnd, hItem, this);
 item.Text=txt;
 item.Tag=tag;

 return item;
}

// ///////////////////////////////////////////////////////

TreeView::TreeView()
{
 m_ImageList = nullptr;
 m_DoingClear = false;
}

TreeView::~TreeView()
{
}

void TreeView::Create(AWnd *parent, Rect const &r)
{
 DWORD fstyle, xstyle;
 HWND hWnd;
 int childId=0;

 fstyle= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS;
 xstyle = TVS_EX_DOUBLEBUFFER;

 hWnd = ::CreateWindowEx(xstyle, WC_TREEVIEW, L"", fstyle, r.X, r.Y, r.Width, r.Height, parent->Handle(),  (HMENU)childId, App->Instance(), NULL); 
 
 CreateWnd(parent, hWnd, WC_TREEVIEW, childId);


 if (App->DarkMode() == true)
  {
   TreeView_SetBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOWTEXT));
   TreeView_SetTextColor(m_hWnd, ::GetSysColor(COLOR_WINDOW));
  }
 else
  {
   TreeView_SetBkColor(m_hWnd, ::GetSysColor(COLOR_WINDOW));
   TreeView_SetTextColor(m_hWnd, ::GetSysColor(COLOR_WINDOWTEXT));
  }


::ShowWindow(hWnd, SW_SHOW);

}

TreeNode TreeView::InsertLast(String const &txt, int tag)
{
 TVINSERTSTRUCT ti={0};
 HTREEITEM hItem;
 TreeNode item;

 ti.hParent=TVI_ROOT;
 ti.hInsertAfter = TVI_LAST;  // lousy performance, has to check entire node list
 
 ti.item.mask=TVIF_TEXT | TVIF_PARAM;
 ti.item.pszText=(LPWSTR)txt.Chars();
 ti.item.cchTextMax=txt.Length()+1;
 ti.item.lParam = tag;

 hItem = TreeView_InsertItem(m_hWnd, &ti);

 item = TreeNode(m_hWnd, hItem, nullptr);
 item.Text = txt;
 item.Tag = tag;

 return item;
}

TreeNode TreeView::InsertFirst(String const &txt, int tag)
{
 TVINSERTSTRUCT ti={0};
 HTREEITEM hItem;
 TreeNode item;

 ti.hParent=TVI_ROOT;
 ti.hInsertAfter = TVI_FIRST; // much faster
 
 ti.item.mask=TVIF_TEXT | TVIF_PARAM;
 ti.item.pszText=(LPWSTR)txt.Chars();
 ti.item.cchTextMax=txt.Length()+1;
 ti.item.lParam = tag;

 hItem = TreeView_InsertItem(m_hWnd, &ti);

 item = TreeNode(m_hWnd, hItem, nullptr);
 item.Text = txt;
 item.Tag = tag;

 return item;
}

TreeNode TreeView::SelectedNode()
{
 TVITEMEX item={0};
 HTREEITEM hNode;
 TreeNode node;

 hNode = TreeView_GetSelection(m_hWnd);
 if (hNode == 0)
  {
   return TreeNode(); // empty uninitialized node handle will be 0
  }
 else
  {
   return TreeNode(m_hWnd, hNode, nullptr);
  }
}

bool TreeView::IsNodeSelected()
{
 TVITEMEX item={0};
 HTREEITEM hNode;

 hNode = TreeView_GetSelection(m_hWnd);
 if (hNode == 0)
   return false;
 else 
   return true;
}

void TreeView::SetSelectedNode(TreeNode const &node)
{
 HTREEITEM hNode;

 hNode = TreeView_GetSelection(m_hWnd);
 if (hNode != 0)
   TreeView_SetItemState(m_hWnd, hNode, 0, TVIS_SELECTED); // de-select any selected node

 TreeView_SetItemState(m_hWnd, node.Handle(), TVIS_SELECTED, TVIS_SELECTED);
}

void TreeView::EnsureVisible()
{
 HTREEITEM hNode;

 hNode = TreeView_GetSelection(m_hWnd);
 if (hNode == 0)
   return;
 else 
   TreeView_EnsureVisible(m_hWnd, hNode);
}

void TreeView::RemoveItem(TreeNode const &node)
{
 TreeView_DeleteItem(m_hWnd, node.Handle());
}

std::vector<TreeNode> TreeView::Nodes()
{
 std::vector<TreeNode> nodes;
 HTREEITEM hItem;
 
 hItem = TreeView_GetChild(m_hWnd, TVI_ROOT);
 if (hItem == 0)
   return nodes; // empty vector

 nodes.push_back(TreeNode(m_hWnd, hItem, nullptr));
 do
  {
   hItem = TreeView_GetNextSibling(m_hWnd, hItem);
   if (hItem != 0)
    {
     nodes.push_back(TreeNode(m_hWnd, hItem, nullptr));
    }
  }
 while(hItem != 0);
 
 return nodes;
}

void TreeView::Clear()
{
 m_DoingClear = true;
 TreeView_DeleteAllItems(m_hWnd);
 m_DoingClear = false;
 Refresh();
}

void TreeView::CollapseAll()
{
 std::vector<TreeNode> nodes = Nodes();

 for(const auto &node : nodes)
  {
   TreeView_Expand(m_hWnd, node.Handle(), TVE_COLLAPSE);
  }
}

void TreeView::ExpandAll()
{
 std::vector<TreeNode> nodes = Nodes();

 for(const auto &node : nodes)
  {
   TreeView_Expand(m_hWnd, node.Handle(), TVE_EXPAND);
  }
}

WMR TreeView::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 if (m_DoingClear == true)
   return WMR::Zero;  // don't handle notifications if the tree is being cleared

 switch(code)
  {
   case TVN_SELCHANGED: return OnItemChanged((NMTREEVIEW *)lParam);
  }
 return WMR::Zero;
}

void TreeView::SetImageList(ImageList *imgList)
{
 TreeView_SetImageList(m_hWnd, imgList->Handle(), TVSIL_NORMAL);
 m_ImageList=imgList;
}

/////////////////////////////////////////////////////////

ADialog::ADialog()
{
 m_Result=DialogResult::None;
 m_DialogID = 0;
 m_hShowParent = 0;
}

ADialog::~ADialog()
{
 if (m_hWnd != 0)
   App->RemoveChildWnd(m_hWnd);
}

DialogResult ADialog::Show(WORD dialogID, AWnd *parent)
{
 INT_PTR retVal;

 if (parent == nullptr)
   m_hShowParent = GetDesktopWindow();
 else
   m_hShowParent=parent->Handle();

 m_DialogID=dialogID;
 m_IsDialog = true;

 retVal = ::DialogBoxParam(App->Instance(), MAKEINTRESOURCE(dialogID), m_hShowParent, (DLGPROC)ADialog::DialogProc, (LPARAM)this); 
 return ConvertReturn(retVal);
} 

DialogResult ADialog::ConvertReturn(INT_PTR retVal)
{ 
 switch(retVal)
  {
   case IDOK: return DialogResult::OK;
   case IDCANCEL: return DialogResult::Cancel;
   case IDRETRY: return DialogResult::Retry;
   case IDABORT: return DialogResult::Abort;
   case IDYES: return DialogResult::Yes;
   case IDNO: return DialogResult::No;
   default: return DialogResult::None;
  }
}

void ADialog::Close()
{
 ADialog::Close(DialogResult::None);
}

void ADialog::Close(DialogResult result)
{
 int r;

 m_Result=result;
 switch(result) // pass the windows version of DialogResult to EndDialog...
  {
   case DialogResult::OK: r = IDOK; break;
   case DialogResult::Cancel: r = IDCANCEL; break;
   case DialogResult::Retry: r = IDRETRY; break;
   case DialogResult::Abort: r = IDABORT; break;
   case DialogResult::Yes: r = IDYES; break;
   case DialogResult::No: r = IDNO; break;
   default: r = 0;
  }
 ::EndDialog(m_hWnd, (INT_PTR)r);
}

INT_PTR CALLBACK ADialog::DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 ADialog *dlg=(ADialog *)App->GetChildWnd(hWnd);
 WMR ret;

 switch(message)
  {
   case WM_INITDIALOG:
    {
     dlg=(ADialog *)lParam;
     dlg->CreateWnd(dlg, hWnd, L"Dialog", dlg->m_DialogID);
     dlg->GetUnitRatio();
     dlg->OnInitDialog();
     return (INT_PTR)TRUE;
    }   
   default:
    {
     if (dlg == nullptr)
       return (INT_PTR)FALSE;
     else
      {
       ret = dlg->MessageHandler(hWnd, message, wParam, lParam);
       if (ret == WMR::One)
         return (INT_PTR)TRUE;
      }
    }
  }
 return (INT_PTR)FALSE;
}

void ADialog::OnInitDialog()
{
 Rect pr;
 Size sz;
 int x,y;

 pr = AWnd::GetScreenRect(m_hShowParent);
 sz = GetSize();
 x = pr.X + pr.Width/2 - sz.Width/2;
 y = pr.Y + pr.Height/2 - sz.Height/2;
 ::MoveWindow(m_hWnd, x, y, sz.Width, sz.Height, true);
}

WMR ADialog::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 HWND hCtrl;
 WMR ret = WMR::Default;
 int cmdID;
 int code;

 switch(msg)
  {
   case WM_COMMAND:
     cmdID=LOWORD(wParam);
     code=HIWORD(wParam);
     hCtrl=(HWND)lParam;
     switch(code)
      {
       case 0: 
         ret = OnCommand(cmdID, hCtrl);
         break;
       case CBN_SELCHANGE: 
         OnDropListChanged(cmdID, hCtrl);
         ret = WMR::One;
         break;
       }
     break;
   case WM_CLOSE:
     if (OnClose()==true)
      {
       Close(DialogResult::None); // closed from window "X"
       ret = WMR::Zero;
      }
     break;
   case WM_PAINT: 
     OnPaint();
     ret = WMR::Zero;
     break;
   case WM_NOTIFY:
     ret = OnNotify(((LPNMHDR)lParam)->hwndFrom, ((LPNMHDR)lParam)->idFrom, ((LPNMHDR)lParam)->code, lParam);
     break;
   case WM_MOUSEMOVE:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     OnMouseMove(e);
     ret = WMR::Zero;
    } break;
   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_RBUTTONDOWN:
    {
     MouseEventArgs e(m_hWnd, wParam, lParam, MouseEventArgs::DoubleClick::None,0);
     OnMouseDown(e);
     ret = WMR::Zero;
    } break;
   case WM_KEYDOWN:
    {
     KeyEventArgs k(wParam, lParam);
     OnKeyDown(k);
     hCtrl = GetFocus();
     if (hCtrl != 0)
      {
       OnKeyDown(GetDlgCtrlID(hCtrl), k);
      }
     ret = WMR::Zero;
    } break;
   case WM_TIMER:
      OnTimer((int)wParam);
      break;
   case WM_CONTEXTMENU:
     ret = OnContextMenu((HWND)wParam, Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
     break;
   default:
      ret = WMR::Default;
  }
 return ret;
}

String ADialog::GetItemText(int id)
{
 int sz=1000;
 WCHAR *buff=new WCHAR[sz];

 ::GetDlgItemText(m_hWnd, id, buff, sz);

 String ret=buff;
 delete [] buff;

 return ret;
}

void ADialog::SetItemText(int id, String const &txt)
{
 ::SetDlgItemText(m_hWnd, id, txt.Chars());
}

Rect ADialog::GetItemDlgRect(int child)
{
 RECT rct;
 Rect r;
 HWND hWnd;

 hWnd = ::GetDlgItem(m_hWnd, child);
 ::GetWindowRect(hWnd, &rct);

 r = Utility::GetRect(rct);

 return r;
}

void ADialog::SetItemVisible(int child, bool visible)
{
 HWND hWnd;

 hWnd = ::GetDlgItem(m_hWnd, child);
 if (hWnd == 0) throw L"Failed to get handle";

 if (visible==true)
  {
   ::ShowWindow(hWnd, SW_SHOW);
  }
 else
  {
   ::ShowWindow(hWnd, SW_HIDE);
  }
 
}

bool ADialog::GetCheckState(int id)
{
 UINT checked;

 checked = ::IsDlgButtonChecked(m_hWnd, id);

 if (checked == BST_CHECKED)
   return true;

 return false;
}

void ADialog::SetCheckState(int id, bool val)
{
 UINT v;

 if (val==true)
   v = BST_CHECKED;
 else
   v = BST_UNCHECKED;

 ::CheckDlgButton(m_hWnd, id, v);
}

void ADialog::SetItemFocus(int child)
{
 HWND hWnd;

 hWnd = GetDlgItem(m_hWnd, child);
 if (hWnd == 0)
   throw L"failed to get child HWND";

 PostMessage(m_hWnd, WM_NEXTDLGCTL, (WPARAM)hWnd, TRUE);
  
}

Rect ADialog::GetItemRect(int childId)
{
 POINT pt;
 RECT wr;
 Rect r;
 HWND hWnd;

 hWnd = ::GetDlgItem(m_hWnd, childId);
 if (hWnd == NULL)
   throw L"Invalid childID or m_hWnd might be 0";

 ::GetWindowRect(hWnd, &wr);

 pt.x = wr.left;
 pt.y = wr.top;
 
 ::ScreenToClient(m_hWnd, &pt);

 r.X = pt.x;
 r.Y = pt.y;
 r.Width = wr.right - wr.left;
 r.Height = wr.bottom - wr.top;

 return r;
}

void ADialog::GetUnitRatio()
{
 RECT r;

 r.left=0;
 r.top=0;
 r.right=1000;
 r.bottom=1000;
 ::MapDialogRect(m_hWnd, &r);
 
 m_UnitRatio = r.right / 1000.0;
}

WMR ADialog::OnNotify(HWND hWnd, int child, UINT code, LPARAM lParam)
{
 AWnd *wnd = App->GetChildWnd(hWnd);
 
 if (wnd != nullptr)
     wnd->OnNotify(hWnd, child, code, lParam);

 switch(code)
  {
   case LVN_ITEMCHANGED: OnListViewItemChanged(child, (NMLISTVIEW *)lParam); break;
   case LVN_COLUMNCLICK: OnListViewColumnClick(child, ((NMLISTVIEW *)lParam)->iSubItem); break;
   case NM_DBLCLK:       OnDoubleClick(child, (NMITEMACTIVATE *)lParam); break;
  }
 return WMR::One;
}
////////////////////////////////////////////////////////////////////////////

StatusBarPane::StatusBarPane(StatusBarPane::Content content, StatusBarPane::Style style)
{ 
 String str;
 SizeF szChr;

 m_Content=content;
 m_Style=style;
 m_Width=0;
 
 m_Value=0;
 m_Max=0;
 m_LastValue=0;
}

StatusBarPane::~StatusBarPane()
{
}

void StatusBarPane::SetMax(int m)
{
 if (m < 0) throw L"Max must be >= 0";
 m_Max = m;
 m_LastCharCount = 0;
 m_LastValue = 0;
 m_Value = 0;
}

//bool StatusBarPane::SetValue(int val)
//{
// double ratio;
// int v, chrs;
//
// if (val < 0 || val > m_Max) throw L"value out of range"; 
//
// v = (int)((double)val * m_ValueInc);
//
// if (v != m_LastValue)
//  {
//   m_LastValue = v;
//   ratio = (double)m_Value / (double)m_Max;
//   chrs = (double)(m_Width / m_CharWidth)*ratio;
//   
//   return true;
//  }
// return false;
//}
//
//void StatusBarPane::Paint(DRAWITEMSTRUCT *dis)
//{
// HPEN pen, old;
// RECT rct;
// Rect r = AWnd::ClientRect(dis->hwndItem);
//
// ::FillRect(dis->hDC, &dis->rcItem, ::GetSysColorBrush(COLOR_WINDOW));
//
// pen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWTEXT));
// old = SelectPen(dis->hDC, pen);
//
// ::MoveToEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, nullptr);
// ::LineTo(dis->hDC, dis->rcItem.right-3, dis->rcItem.top);
// ::LineTo(dis->hDC, dis->rcItem.right-3, dis->rcItem.bottom-2);
// ::LineTo(dis->hDC, dis->rcItem.left, dis->rcItem.bottom-2);
// ::LineTo(dis->hDC, dis->rcItem.left, dis->rcItem.top);
//
// SelectPen(dis->hDC, old);
// DeletePen(pen);
//}

// /////////////////////////////////////////////////////

StatusBar::StatusBar()
{
 m_hPen = 0;
}

StatusBar::~StatusBar()
{
 if (m_hPen != 0)
   DeletePen(m_hPen);
}

void StatusBar::Create(AWnd *parent)
{
 String prog;
 Rect rct;
 SizeF sz;
 DWORD fstyle;
 HWND hWnd;
 LPARAM lParam;
 WPARAM wParam;
 int childId = 0;
 int i;

 if (Panes.size() == 0) throw L"No panes added prior to creation";

 m_hPen = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOWTEXT));

 fstyle= WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP; 

 hWnd = CreateWindowEx(0, STATUSCLASSNAME, L"", fstyle, 0, 0, 0, 0, parent->Handle(), (HMENU)childId,App->Instance(), NULL);
 rct = parent->ClientRect();   // Get the coordinates of the parent window's client area.

 CreateWnd(parent, hWnd, STATUSCLASSNAME, childId);

 HFONT font  = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
 
 prog = L"\x25A0"; 
 sz = prog.MeasureString(font);
 m_CharWidth = sz.Width;

 SetWidths();

 i = 0;
 for (const auto &pane : Panes)
  {
   if (pane->GetContent() == StatusBarPane::Content::Text)
    {
     wParam = i;
     lParam = (LPARAM) L"Text Pane";
     ::SendMessage(m_hWnd, SB_SETTEXT, wParam, lParam); 
    }
   else
    {
     wParam = i | SBT_OWNERDRAW; 
     lParam = 0;
     ::SendMessage(m_hWnd, SB_SETTEXT, wParam, lParam); // posts a WM_DRAWITEM to parent
    }
   i++;
  }
::ShowWindow(hWnd, SW_SHOW);
}

void StatusBar::AddFixedPane(StatusBarPane::Content content, int width)
{
 StatusBarPane *item;

 item=new StatusBarPane(content, StatusBarPane::Style::Fixed);
 item->SetWidth(width);

 Panes.push_back(item);
}

void StatusBar::AddAutoPane(StatusBarPane::Content content)
{
 StatusBarPane *item;

 for (const auto &pane : Panes)
  {
   if (pane->GetStyle() == StatusBarPane::Style::AutoSize)
     throw L"Already have 1 auto size pane, only 1 can be autosize";
  }

 item=new StatusBarPane(content, StatusBarPane::Style::AutoSize);
 Panes.push_back(item);
 m_Panels++;
}

void StatusBar::SetWidths()
{
 int *Parts;
 int w;
 int ac;
 int i;
 int sz;

 w = ClientRect().Width;
 ac = 0;
 sz = (int)Panes.size();

 for(const auto &pane : Panes)
  {
   if ( pane->GetStyle()==StatusBarPane::Style::Fixed)
     ac+=pane->GetWidth();
  }
 for(const auto &pane : Panes)
  {
   if ( pane->GetStyle()==StatusBarPane::Style::AutoSize)
     pane->SetWidth(w-ac);
  }

 Parts = new int[sz];

 w = 0;
 i = 0;
 for (const auto &pane : Panes)
  { 
   w += pane->GetWidth();
   Parts[i++] = w; 
  }
 Parts[sz-1] = -1;  // last pane extends to the status bar border

 // Tell the status bar to create the window parts.
 ::SendMessage(m_hWnd, SB_SETPARTS, (WPARAM)sz, (LPARAM)Parts);
 delete [] Parts;

}

void StatusBar::SetText(int pane, String const &txt)
{
 WPARAM wParam;
 LPARAM lParam;

 if (pane >= Panes.size()) throw L"pane out of bounds";

 wParam = pane;
 lParam = (LPARAM)txt.Chars();
 ::SendMessage(m_hWnd, SB_SETTEXT, wParam, lParam); 
}

String StatusBar::GetText(int pane)
{
 String out;
 wchar_t *chrs;
 int len;

 if (pane < 0 || pane >= Panes.size()) throw L"out of bounds";

 len = ::SendMessage(m_hWnd, SB_GETTEXTLENGTH, pane, 0);
 if (len == 0)
   return out;

 chrs = new wchar_t[len+1];

 ::SendMessage(m_hWnd, SB_GETTEXT, pane, (LPARAM)chrs);

 out = chrs;
 delete [] chrs;
 
 return out;
}

int StatusBar::GetWidth(int pane)
{
 RECT rct;

 ::SendMessage(m_hWnd, SB_GETRECT, pane, (LPARAM)&rct);
 return rct.right - rct.left;
}

void StatusBar::OnSize(Rect const &r)
{
 LPARAM lParam;

 lParam = MAKELPARAM( r.Height, r.Width);
 
 ::SendMessage(m_hWnd, WM_SIZE, 0, lParam);

 SetWidths();
}

void StatusBar::ProgressMax(int pane, int max)
{
 StatusBarPane::Content content;

 if (pane>=Panes.size()) throw L"pane out of bounds";
 
 content = Panes[pane]->GetContent();
 if (content != StatusBarPane::Content::Progress) throw L"Not a progres bar";
 Panes[pane]->SetMax(max);
}

void StatusBar::Progress(int pane)
{
 int val;

 if (pane>=Panes.size()) throw L"pane out of bounds";
 if (Panes[pane]->GetContent() != StatusBarPane::Content::Progress) throw L"Not a progres bar"; 

 val = Panes[pane]->GetValue();
 if (val < Panes[pane]->GetMax())
   ProgressValue(pane, val+1); 
}

void StatusBar::ProgressValue(int pane, int val)
{
 String progress;
 WPARAM wParam;
 double vr, mc;
 int chrs, w, cw;


 if (pane>=Panes.size()) throw L"pane out of bounds";
 if (Panes[pane]->GetContent() != StatusBarPane::Content::Progress) throw L"Not a progres bar";
 if (val < 0 || val > Panes[pane]->GetMax()) throw L"value out of range"; 

 if (val != Panes[pane]->GetLastValue())
  {
   w = GetWidth(pane);
   cw = m_CharWidth;
   vr = (double)val / (double)Panes[pane]->GetMax();
   mc = (double)( w / cw ); // max number of progress chrs that will fit in pane
   chrs = (int)( mc * vr );
   if (Panes[pane]->GetLastCharCount() != chrs)
    {
     Panes[pane]->SetLastCharCount(chrs);
     wParam = pane | SBT_OWNERDRAW;
     ::SendMessage(m_hWnd, SB_SETTEXT, wParam, 0); 
    }
  }
 Panes[pane]->SetValue(val);
}

void StatusBar::OnDrawItem(DRAWITEMSTRUCT *dis)
{
 COLORREF oldColor;
 HPEN hOldPen;
 RECT rct;
 Rect r;
 int pane, lcc;
 String txt;

 pane = dis->itemID;
 if (pane>=Panes.size()) throw L"pane out of bounds"; 

 if (Panes[pane]->GetContent() == StatusBarPane::Content::Progress) 
  {
   lcc = Panes[pane]->GetLastCharCount();
   r = AWnd::ClientRect(dis->hwndItem);

   ::FillRect(dis->hDC, &dis->rcItem, ::GetSysColorBrush(COLOR_WINDOW));

   txt = String::Repeat(L"\x25A0", lcc);
   if (txt.Length() > 0)
    {
     oldColor = ::SetTextColor(dis->hDC, RGB(0,200,0));
     rct.left = dis->rcItem.left+2;
     rct.top = dis->rcItem.top+1;
     rct.right = dis->rcItem.right;
     rct.bottom = dis->rcItem.bottom;
     ::DrawText(dis->hDC, txt.Chars(), txt.Length(), &rct, DT_TOP | DT_LEFT | DT_SINGLELINE);
     ::SetTextColor(dis->hDC, oldColor);
    }
   hOldPen = SelectPen(dis->hDC, m_hPen);

   ::MoveToEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, nullptr);
   ::LineTo(dis->hDC, dis->rcItem.right-3, dis->rcItem.top);
   ::LineTo(dis->hDC, dis->rcItem.right-3, dis->rcItem.bottom-2);
   ::LineTo(dis->hDC, dis->rcItem.left, dis->rcItem.bottom-2);
   ::LineTo(dis->hDC, dis->rcItem.left, dis->rcItem.top);

   SelectPen(dis->hDC, hOldPen);
  }
}

ProgressBar::ProgressBar(StatusBar *bar, int pane)
{
 m_SB = bar;
 m_Pane = pane;
}

ProgressBar::~ProgressBar()
{
 m_SB->ProgressValue(m_Pane, 0);
}

void ProgressBar::Max(int val)
{
 m_SB->ProgressMax(m_Pane, val);
}

void ProgressBar::Progress()
{
 m_SB->Progress(m_Pane);
}


/////////////////////////////////////////////////////////

ImageList::ImageList()
{
 m_Handle=0;
 m_Width=0;
 m_Height=0;
}

ImageList::~ImageList()
{
 Destroy();
}

void ImageList::Create(ImageList::Style style, int width, int height, int initialSize)
{
 UINT flags;
 int grow;

 m_Width = width;
 m_Height = height;

 if ( style == ImageList::Style::Masked)
   flags = ILC_MASK | ILC_COLORDDB;
 else
   flags = ILC_COLOR24;

 grow = 1;

 if (m_Handle != 0)
   Destroy();

 m_Handle = ImageList_Create(width, height, flags, initialSize, grow);

 for (const auto &client : Clients)
  {
   if (client->Handle() != 0)
    {
     client->SetImageList(this);
    }
  }

 PictureMap.clear();
 m_ImageIndex=0;
}

void ImageList::Add(Bitmap *bmp, int pictureID)
{
 HBITMAP hBmp;

 bmp->GetHBITMAP(Color::White, &hBmp);
 ImageList_Add(m_Handle, hBmp , 0);
 PictureMap.insert(std::pair<int, int>(pictureID, m_ImageIndex++));
 ::DeleteObject(hBmp);
}

void ImageList::AddMasked(int bmpID, COLORREF mask)
{
 HBITMAP hBmp;
 
 hBmp=(HBITMAP)::LoadImage(App->Instance(), MAKEINTRESOURCE(bmpID), IMAGE_BITMAP, 0, 0, 0);
 
 ImageList_AddMasked(m_Handle, hBmp, mask);
  
 ::DeleteObject(hBmp);

}

HICON ImageList::GetIcon(int index)
{
 HICON hIcon;

 hIcon = ImageList_GetIcon(m_Handle, index, ILD_TRANSPARENT);

 return hIcon;
}

void ImageList::Destroy()
{
 if (m_Handle !=0)
  {
   ImageList_Destroy(m_Handle);
   m_Handle=0;
  }
}

int ImageList::GetIndex(int pictureID)
{
#ifdef _DEBUG
 if (PictureMap.count(pictureID) == 0)
   throw L"map didn't contain picture id";
#endif
 return PictureMap.at(pictureID);
}

/////////////////////////////////////////////////////////

void ToolBar::Create(AWnd *parent, int btns)
{
 DWORD style;

 style = WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS;

 m_hWnd = ::CreateWindow(TOOLBARCLASSNAME, L"", style, 0, 0, 100, 20, parent->Handle(), 0, App->Instance(), 0);
 if (m_hWnd == 0) throw L"Create failed";

 AWnd::CreateWnd(parent, m_hWnd, TOOLBARCLASSNAME, 0);

 m_Images.Create(ImageList::Style::Masked, 18, 18, btns);

 ::SendMessage(m_hWnd, TB_SETIMAGELIST, 0, (LPARAM)m_Images.Handle());

 m_Index = 0;
}

void ToolBar::AddItem(int bitmapID, int cmdID, String const &toolTip)
{
 TBBUTTON bs={0};

 m_Images.AddMasked(bitmapID, RGB(255,0,255)); // assum all buttons use magenta mask

 bs.iBitmap = m_Image;
 bs.fsStyle = BTNS_BUTTON;
 bs.fsState = TBSTATE_ENABLED;
 bs.idCommand = cmdID;
 bs.iString = (INT_PTR) toolTip.Chars();

 ::SendMessage(m_hWnd, TB_INSERTBUTTON, m_Index, (LPARAM)&bs);
 ::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, 0, 0);

 m_Index++; 
 m_Image++;

}

void ToolBar::AddSeparator()
{
 TBBUTTON bs={0};

 bs.iBitmap = 0;
 bs.fsStyle = BTNS_SEP;
 bs.fsState = 0;
 bs.idCommand = 0;
 bs.iString = 0;

 ::SendMessage(m_hWnd, TB_INSERTBUTTON, m_Index, (LPARAM)&bs);
 ::SendMessage(m_hWnd, TB_SETMAXTEXTROWS, 0, 0);

 m_Index++; 

}

void ToolBar::EnableButton(int id, bool enable)
{
 if (enable == true)
   ::SendMessage(m_hWnd, TB_ENABLEBUTTON, id, TRUE);
 else
   ::SendMessage(m_hWnd, TB_ENABLEBUTTON, id, FALSE);
}

////////////////////////////////////////////////////////////////////////////

bool ADropList::AddItem(String const &txt, int tag)
{
 for(const auto &li : Items)
  {
   if (li.Text == txt) return false;
  }

 Items.push_back(ListItem(txt, tag));
 ComboBox_AddString(m_hWnd, txt.Chars());

 return true;
}

bool ADropList::AddItem(ListItem const &item)
{
 int i;

 for(const auto &li : Items)
  {
   if (li.Text == item.Text) return false;
  }

 Items.push_back(item);
 i = ComboBox_AddString(m_hWnd, item.Text.Chars());
 if (i<0) throw L"Error";

 return true;
}

bool ADropList::SetCurrentItem(ListItem const &item)
{
 int i,s;

 i=0;
 s=-1;
 for (const auto &li : Items)
  {
   if (li.Text == item.Text)
    {
     s = i;
     break;
    }
   i++;
  }
 if (s >= 0)
   ComboBox_SetCurSel(m_hWnd, s);

 return s >= 0;
}

bool ADropList::SetCurrentItem(String const &txt)
{
 int i, s;
 
 s = -1;
 i = 0;
 for( const auto &li : Items)
  {
   if (li.Text == txt)
    {
     s = i;
     break;
    }
  }

 if (s >= 0)
  {
   if (ComboBox_SetCurSel(m_hWnd, s) == CB_ERR) throw L"SetCurSel failed";
  }

 return s >= 0;
}

bool ADropList::SetCurrentItem(int tag)
{
int i, s;
 
 s = -1;
 i = 0;
 for( const auto &li : Items)
  {
   if (li.Tag == tag)
    {
     s = i;
     break;
    }
   i++;
  }

 if (s >= 0)
  {
   if (ComboBox_SetCurSel(m_hWnd, s) == CB_ERR) throw L"SetCurSel failed";
  }

 return s >= 0;
}

bool ADropList::SetCurSel(int index)
{
 if ( ComboBox_SetCurSel(m_hWnd, index) == CB_ERR)
   return false;
 else
   return true;
}

void ADropList::Clear()
{
 ComboBox_ResetContent(m_hWnd);
 Items.clear();
}

ListItem ADropList::GetSelectedItem()
{
 int i;

 i = ComboBox_GetCurSel(m_hWnd);

 if (i == CB_ERR)
   return ListItem();
 else
  {
   if (i >= Items.size()) throw L"Out of bounds";
   return Items[i];
  }
}

bool ADropList::IsItemSelected()
{
 return ( ComboBox_GetCurSel(m_hWnd) != CB_ERR);
}

///////////////////////////////////////////////////////////////

WaitCursor::WaitCursor()
{
 m_CurrentCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
}

WaitCursor::~WaitCursor()
{
 ::SetCursor(m_CurrentCursor);
}