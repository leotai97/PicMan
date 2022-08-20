#pragma once
class DlgHelpAbout
{
 public:

 DlgHelpAbout();
~DlgHelpAbout();

 int Show();

 static INT_PTR CALLBACK DlgHelpAboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

};

