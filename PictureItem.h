#pragma once

class ImageParser;

class PictureItem
{
 public:

 PictureItem(FolderItem *f, ImageParser *p);
 PictureItem(int id, String const &sFile, DateTime const &added, FolderItem *parent, int w, int h, String const &subDir);
~PictureItem();
 void SetOriginal(Bitmap *bmp);
 bool HasGlobalHashTag(HashTag const &ht);
 bool HasPictureHashTag(HashTag const &pt);
 bool SearchPictureTags(int id);
 bool AddGlobalHashTag(HashTag const &ht);
 bool AddPictureHashTag(HashTag &pht);
 bool RemoveGlobalHashTag(HashTag const &ght);
 bool RemovePictureHashTag(HashTag const &pht);
 void ReplacePictureHashTags(std::vector<HashTag> &tags);
 void AddPictureHashTags(std::vector<HashTag> &tags);
 void SetPictureHashTagString();
 void SaveNew();
 void Delete();
 void ReloadThumbnail();
 void UpdateThumbnail();

 String FullName();
 
 inline int ID() const { return m_ID; }
 inline ImageParser *Picture() const { return m_Picture; }
 inline int Width() const { return m_Width; }
 inline int Height() const { return m_Height; }
 inline String FileName() const { return m_FileName; }
 inline String SubDir() const { return m_SubDir; } 
 inline DateTime DateAdded() const { return m_DateAdded; }
 inline void SetFileName(String const &nn) { m_FileName=nn; }
 inline String PictureHashTagString() const { return m_PictureHashTagString; }
 inline FolderItem *ParentFolder() const { return m_ParentFolder; }

 std::map<int, HashTag> GlobalHashTags;
 std::map<int, HashTag> PictureHashTags;

 static void RemovePictureRowID(int id);
 static void UpdateFileName(int id, String const &name);

 bool MissingThumbnailRow;

 private:

 static void ChangeFile(ImageParser *p);

 int m_ID; 
 String m_FileName;
 String m_SubDir;
 String m_PictureHashTagString;
 
 ImageParser   *m_Picture;        // owned by App->Pictures
 FolderItem *m_ParentFolder;   // owned by BaseItem->Folders

 int m_Width;
 int m_Height;

 DateTime m_DateAdded;

};
