// PicMan.cpp : Defines the entry point for the application.
//
#include "pch.h"
#include "App.h"

PicManApp   *App;
MainWnd     *Wnd;
DBCon       *DB;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
 UNREFERENCED_PARAMETER(hPrevInstance);
 UNREFERENCED_PARAMETER(lpCmdLine);

 HRESULT hr;
 int ret=0;

 hr=::CoInitialize(NULL);
 if (FAILED(hr))
   return 0;

 App=new PicManApp();

 if (App->Init(hInstance, nCmdShow, L"PicMan", L"Picture Manager")==true)
  {
   ret = App->GetMain()->Loop(IDC_PICMAN);
   App->Shutdown();
  }

 ::CoUninitialize();
 return ret;
}

