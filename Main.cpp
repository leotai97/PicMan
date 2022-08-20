#include "pch.h"
#include "App.h"

Main::Main()
{
}

Main::~Main()
{
}

void Main::Register(String const &wndcls)
{
 WNDCLASSEX wcex;
 
 m_PopUpWndClass=wndcls;

 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style          = CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc    = PopUpWnd::PopUpWndProc;
 wcex.cbClsExtra     = 0;
 wcex.cbWndExtra     = 0;
 wcex.hInstance      = App->Instance();
 wcex.hIcon          = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_PICMAN));
 wcex.hCursor        = LoadCursor(0, IDC_ARROW);
 wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
 wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PICMAN);
 wcex.lpszClassName  = m_PopUpWndClass.Chars();
 wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

 if (::RegisterClassEx(&wcex)==0)
   throw L"Register failed";
}