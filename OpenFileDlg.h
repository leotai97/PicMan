#pragma once

class OpenFileDlg
{
 public:
 
 enum class FileType : int { EXE };

 static String ChooseFolder(HWND parent, String const &title);
 static String ChoooseFile(HWND hOwner, HINSTANCE hInst, String const &title, FileType fType);
 static String AppDataFolder();
  
};

