#pragma once

class PicColor
{
 public:
 PicColor();
 PicColor(int nColorKey, int pr, int pg, int pb, int nCount);
~PicColor(){};

 inline void Accumulate() { m_Amount++; }
 inline void Accumulate(int amt) { m_Amount+=amt; }

 static int CompareRGB(PicColor *p1, PicColor *p2);

 inline int Key() { return m_Key; }
 inline int Amount() { return m_Amount; }
 inline int R() { return m_R; }
 inline int G() { return m_G; }
 inline int B() { return m_B; }

 private:

 int m_Key;
 int m_Amount;
 int m_R;
 int m_G;
 int m_B;
};

class FileMoveResult
{
 public:
 FileMoveResult()
  {
   Success=true;
   Message=L"";
  }

 bool Success;
 String Message;
};

class FileInfo
{
 public:
 FileInfo();
 FileInfo(String const &fullPath); // can pass either full path or only file name

 String  FullPath;   // "E:\Pictures\file.jpg" 
 String  Dir;        // "E:\Pictures" if filename is "E:\Pictures\file.jpg"
 String  Name;       // file name with extension, "file.jpg" without directory
 String  Extension;  // "jpg" if filename is "file.jpg"
 String  FileNoExt;  // file name without extension
 String  FilePrefix; // "File" if filename is "File_001.jpg"
 
 int   FileNumber; // 1 if filename is "File_001.jpg"
 bool  IsNumbered; // true if filename is "File_001.jpg"

};

class ImageParser
{
 public:
 
 enum class SortChoices : int 
  { 
   FileName,  
   NameWithNumber,
   Width,
   Height,
   Ratio,     // width divided by height
   ColorRGB,
   EdgeAverage, 
   FileSize,
   FileDate,
   GlobalHashTags,
   FolderHashTags
  };

 ImageParser();
 ImageParser(String const &strDir, String const &strName, int id);
 ImageParser(String const &strDir, String const &strSubDir, String const &strName, int id);

~ImageParser(); 

 void InjectItem(PictureItem *item);
 void ResetReady();
 void SetImport(bool bImport);
 void SetItem(PictureItem *item, Bitmap *original);
 void ReprocessImage();
 void ProcessImage();
 void GetFileDate();
 void ProcessForCompare(Bitmap *bmp);
 bool Swap(ImageParser *p);
 bool RenameBySeq(int nSeq);
 bool RenameByPartAndSeq(String const &strPart, int nSeq);
 bool RenameByPartAndSeqDash(String const &strPath, int nSeq);
 bool RenameBySeqToOrd(int nSeq);
 bool RenameByPartAndSeqToOrd(String const &strPart, int nSeq);
 bool RenameByFileToDir(String const &strFile, String const &strNewDir);
 bool Delete();
 void Dispose();

 inline void SetID(int newID) { m_ID = newID; } // so imports don't have to be reloaded from scratch

 static Bitmap *GetOriginal(String const &file);
 static String FSizer(long sz);
 static void ReprocessImageProc(ImageParser *img);

 int CompareRGB(ImageParser *other);
 bool Equal(ImageParser *other);
 void PathHasChanged(); // update database

 private:

 void Init(String const &dir, String const &sdir, String const &file, int id);
 void ProcessImageInternal(Bitmap *bmp);
 void ExtractColor();
 void ExtractFileInfo();
 void ClearColorIndex();
 
 static FileMoveResult Mover(String const &from, String const &to);

 public:

 inline int ID() const { return m_ID; }
 inline int Width() const { return m_Width; }
 inline int Height() const { return m_Height; }
 inline int FileNumber() const { return m_FileNumber; }
 inline int BGAvgRed() const { return m_BGAvgRed; }
 inline int BGAvgGreen() const { return m_BGAvgGreen; }
 inline int BGAvgBlue() const { return m_BGAvgBlue; }

 inline long FileLength() const { return m_FileLength; }
 inline double Ratio() const { return m_Ratio; }

 inline PictureItem *Item() const { return m_Item; }

 inline String Directory() const { return m_Directory; }
 inline String SubDir() const { return m_SubDir; }
 inline String FullPath() const { return m_FullPath; }
 inline String FileName() const { return m_FileName; }
 inline String NoExtension() const { return m_NoExtension; }
 inline String FilePrefix() const { return m_FilePrefix; }
 inline String Error() const { return m_Error; }
 inline String FileSize() const { return m_FileSize; }
 inline String PrevFile() const { return m_PrevFile; }

 inline DateTime FileDate() const { return m_FileDate; }

 inline bool Ready() const { return m_Ready; }
 inline bool Good() const { return m_Good; }
 inline bool IsNumbered() const { return m_IsNumbered; }
 inline bool Monochrome() const { return m_Monochrome; }
 inline bool IsImport() const { return m_IsImport; }

 inline Bitmap *Original() const { return m_Original; }
 inline Bitmap *Thumb() const { return m_Thumb; }

 inline void SetFileNumber(int val) { m_FileNumber=val; }
 inline void SetFileName(String const &val) { m_FileName=val; }
 inline void SetFullPath(String const &val) { m_FullPath=val; }
 inline PicColor **Colors() { return m_Colors; }

 inline int BGAvgRed() { return m_BGAvgRed; }
 inline int BGAvgGreen() { return m_BGAvgGreen; }
 inline int BGAVgBlue() { return m_BGAvgBlue; }

 // members

  private:

 int m_ID;                   // unique instance identifier (not database id)
 int m_Width;                // actual image width
 int m_Height;               // actual image height
 int m_FileNumber;           // e.g. File_123.jpg would be 123
 int m_BGAvgRed;             // average of all red values of each RGB color calculated in "ExtractColor"
 int m_BGAvgGreen;
 int m_BGAvgBlue;

 long m_FileLength;          // width * height, not size on disk
 double m_Ratio;             // width divded by height
 
 PictureItem *m_Item;        // Pictureitem contains code to store image information on database, this parameter can be null
 
 String m_Directory;   // e:/dir/dir
 String m_SubDir;      // e:/dir/dir/subdir/
 String m_FullPath;    // e:/dir/dir/file.jpg
 String m_FileName;    // file.jpg
 String m_NoExtension; // "file"
 String m_FilePrefix;  // e.g. File_123.jpg would be "File_"
 String m_Error;       // last error msg
 String m_FileSize;    // for gui info, shows as 96K, 123MB, 1GB etc
 String m_PrevFile;    // previous file name after being renamed
 String m_Extension;   // jpg or ord
 DateTime m_FileDate;  // file's lastwritetime property

 bool m_Ready;       // class instance processing happens in utility threads, this flag signals when the extraction of this instance is finished
 bool m_Good;        // image loaded and parsed ok
 bool m_IsNumbered;  // e.g. true if file_123.jpg, false if "justafile.jpg"
 bool m_Monochrome;  // if r = g and g = b in every color of the jpg  
 bool m_IsImport;    // import list is a utility disk folder that new images can be dumped into and processed, checked for dups, added to folders, etc 

 Bitmap *m_Thumb;    // prepared for use in ImageList, "Original" is centered on a 128 by 128 bitmap   
 Bitmap *m_Original; // original dimension w / h ratio but shrunk down for storage in DB
 
 PicColor **m_Colors;  // dynamic PicColor[] array of colors, size is from m_ColorIndex.size()

 public:
 
 bool Counted;     // used to advance progress bar, once ready is determined this is set true to indicate it has been counted
 
 std::map<int, PicColor *> ColorIndex;  // map of PicColors indexed by color e.g. #FFFFFF white

};
 
struct // sort PicColor by amount
 {
  bool operator()(PicColor *a, PicColor *b) const { return a->Amount() > b->Amount(); } // high to low
 } ImageParserCustomSort;

class ImageParserSorter 
{
 public:
 
 ImageParserSorter(ImageParser::SortChoices eSort, bool bAscending) 
  {
   Sort = eSort;
   Ascending = bAscending;
   GlobalHashTagID = 0;
  }
 
 ImageParserSorter(int globalHashTagID, bool bAscending)
  {
   Sort = ImageParser::SortChoices::GlobalHashTags;
   GlobalHashTagID = globalHashTagID;
   Ascending = bAscending;
  }

 bool operator()(ImageParser *a, ImageParser *b); 

 private:

 int GlobalHashTagID;

 ImageParser::SortChoices Sort;
 bool                     Ascending;

};