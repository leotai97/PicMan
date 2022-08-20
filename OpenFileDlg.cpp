
#include "pch.h"
#include <shobjidl.h> 
#include <Shlobj.h>     // this header has "FolderItem" defined so it's sequestered away 
#include <commdlg.h>
#include "Utility.h"
#include "OpenFileDlg.h"

String OpenFileDlg::ChooseFolder(HWND parent, String const &title)
{
 IFileOpenDialog *pDlg;
 DWORD dwOptions;
 HRESULT hr;
 String ret=L"";

 hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pDlg));
 if (SUCCEEDED(hr)==false)
   throw L"failed to create IFileOpenDialog";

 hr = pDlg->GetOptions(&dwOptions);
 if (SUCCEEDED(hr)==false)
   throw L"failed to get IFileOpenDialog options";
 
 hr = pDlg->SetOptions(dwOptions | FOS_PICKFOLDERS);
 if (SUCCEEDED(hr)==false)
   throw L"failed to set IFileOpenDialog options";
 
 hr = pDlg->SetTitle(title.Chars());
 if (SUCCEEDED(hr)==false)
   throw L"failed to set IFileOpenDialog title";

 hr = pDlg->Show(parent);
 if (SUCCEEDED(hr))
  {
   IShellItem *pItem;
   hr = pDlg->GetResult(&pItem);
   if (SUCCEEDED(hr))
    {
     PWSTR pszFilePath;
     hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
     if (SUCCEEDED(hr))
      {
       ret=String(pszFilePath);
       ::CoTaskMemFree(pszFilePath);
      }
     pItem->Release();
    }
  }
 pDlg->Release();

 return ret;
}


String OpenFileDlg::ChoooseFile(HWND hOwner, HINSTANCE hInst, String const &title, FileType fType)
{
 OPENFILENAME ofn={0};
 String name, exten, fName;
 wchar_t *filter;
 wchar_t *file;
 int i, fi;
 bool res;

 switch(fType)
  {
   case FileType::EXE:
    {
     name = L"EXE Files";
     exten = L"*.EXE";
    } break;
  }

 filter = new wchar_t[name.Length() + 1 + exten.Length() + 2];

 fi = 0;
 for (i = 0; i < name.Length(); i++)
   filter[fi++]= name[i];
 filter[fi++]=L'\0';
 for (i = 0; i < exten.Length(); i++)
   filter[fi++]=exten[i];
 filter[fi++]=L'\0';
 filter[fi]=L'\0';
 
 file = new wchar_t[1024];
 for (i=0; i<1024; i++)
   file[i]=L'\0';

 ofn.lStructSize = sizeof(OPENFILENAME);
 ofn.hwndOwner = hOwner;
 ofn.hInstance = hInst;
 ofn.lpstrFilter = filter;
 ofn.lpstrFile = file;
 ofn.nMaxFile = 1024;
 ofn.lpstrTitle = title.Chars();
 ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;


 res = GetOpenFileName(&ofn);
 if (res == true)
   fName = ofn.lpstrFile;
 
 delete [] filter;
 delete [] file;

 return fName;

}

String OpenFileDlg::AppDataFolder()
{
 String folder;
 wchar_t* localAppDataFolder;

 if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) != S_OK) 
   return folder;

 folder = localAppDataFolder;

 CoTaskMemFree(localAppDataFolder);

 return folder;  
}
