#include "app.h"

DlgHelpAbout::DlgHelpAbout()
{
}

DlgHelpAbout::~DlgHelpAbout()
{
}

int DlgHelpAbout::Show()
{
 ::DialogBoxParam(App->Instance(), MAKEINTRESOURCE(IDD_ABOUTBOX), App->Wnd()->Handle(), DlgHelpAbout::DlgHelpAboutProc,123);
 return 0;
}

INT_PTR CALLBACK DlgHelpAbout::DlgHelpAboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 switch (message)
  {
   case WM_INITDIALOG:
     return (INT_PTR)TRUE;

   case WM_COMMAND:
     if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
       ::EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
     }
    break;
  }
 return (INT_PTR)FALSE;
}
