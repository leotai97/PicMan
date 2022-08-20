#pragma once

class BaseItem;

class Ints
{
 public:
 int PicID;
 int HashID;
 int SubID;

 Ints()
  {
   PicID=0;
   HashID=0;
   SubID=0;
  }

 Ints(int hid, int pid, int sid)
  {
   PicID = pid;
   HashID = hid;
   SubID = sid;
  }   
};

class FolderItem
{
 public:

 FolderItem(String const &sName, BaseItem *bi);
 FolderItem(int nID, String const &sName, BaseItem *bi, DateTime const &dtAdd, DateTime const &dtOpen);
~FolderItem();

 String GetFolderDir(); 
 HashTag AddHashTag(String const &s);
 bool UpdateFolderHashTag(HashTag &ht, String const &s);
 void DropHashTag(HashTag *ht); // ht is deleted
 bool FolderHashTagInUse(HashTag const &fht);
 void LoadInfo();
 void ResetPictures();
 void ClearPictures();
 void ClearHashTags();
 void Close(); // FolderItems remain under BaseItem life of the app
 bool ContainsFile(String const &sName);
 void BumpOpened();
 void Load();
 void GetPictureList(int ght);
 void GetPictureListHavingGlobal();
 void GetFileList();
 void GetFileList(int ghtID);
 void MapTagUse();
 void ProcessPictures(std::vector<ImageParser *> const &pics);
 void Drop();
 
 Bitmap *GetNewThumb(String const &subdir, String const &fileName, bool bShowMsg);
 PictureItem *FindPicture(ImageParser *ip);
 String FirstName();
 String LastName();
 DateTime LastActivity();

 inline int ID() { return m_ID; }
 inline UINT PictureCount(){ return m_PictureCount; }
 inline String Folder() { return m_Folder; }
 inline String FolderPath(); 
 inline BaseItem *ParentBase() { return m_ParentBase; }
 inline DateTime Added() { return m_Added; }
 inline DateTime LastAdded() { return m_LastAdded; }
 inline DateTime LastOpened() { return m_LastOpened; }
 
 std::map<String, int>        FileNames;  // file name, picture id
 std::map<int, PictureItem *> Pictures;         
 std::map<int, HashTag>       FolderHashTags;

 std::vector<PictureItem *> PictureList;  // special for View By Global Hashtag
 std::vector<String> FileList;

 static void ThumbThread(PictureItem *px, Bitmap *bmp);
 
 int Random;  // used to sort a random list of folders

 protected:

 int m_ID;
 UINT m_PictureCount;  // from LoadInfo

 String m_Folder;
 BaseItem *m_ParentBase;
 DateTime m_Added;
 DateTime m_LastAdded;
 DateTime m_LastOpened;
};

class BaseItem
{
 public:
 enum class DirectoryType : int // these different types control what the program does when walking the directories / folders under the base
  { 
    SingleLevel,  // single level means all dirs/folders are under the base and are single level 
    Mixed,        // mixed means all dirs/folders are under the base but can contain multiple levels, dirs under mixed can be marked as single so sub dirs will be ignored
    Disparate     // disparte means each dir/folder can be located anywhere and each must be added manually
  }; 

 BaseItem(int nID, String const &sDirName);

 int ID() { return m_ID; }
 inline String DirName() { return m_DirName; }
 inline String DirPath() { return m_DirPath; }

 std::map<int, FolderItem *> Folders;  

 static std::map<int, BaseItem *>LoadBases();
 static BaseItem *SaveNew(String const &name);
 static void DBIntegrity();

 void Save(String const &sName, String const &sPath);
 bool GetDir();
 void Load();
 void RemoveFolder(int id);
 void ClearFolders();

 protected:

 String BrowsePath(); 

 int m_ID;
 String m_DirName;
 String m_DirPath;
 
 DirectoryType m_DirType;
};