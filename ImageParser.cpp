#include "pch.h"
#include "App.h"

PicColor::PicColor()
{
}

PicColor::PicColor(int ck, BYTE r, BYTE g, BYTE b, int count)
{
 m_Key=ck;
 m_R=r;
 m_G=g;
 m_B=b;
 m_Amount=count;
 m_Ratio = 0.0F;
}

int PicColor::CompareRGB(PicColor *p1, PicColor *p2)
{
 if (p1->R() > p2->R()) return 1;
 if (p1->R() < p2->R()) return -1;

 if (p1->G() > p2->G()) return 1;
 if (p1->G() < p2->G()) return -1;

 if (p1->B() > p2->B()) return 1;
 if (p1->B() < p2->B()) return -1;

 return 0;
}

bool PicColor::EqualRGB(PicColor *p1, PicColor *p2)
{
 if (p1->R() != p2->R()) return false;
 if (p1->G() != p2->G()) return false;
 if (p1->B() != p2->B()) return false;

 return true;
}


bool PicColor::PercentageRGB(PicColor *p1, PicColor *p2, float percentage)
{
 if (Utility::ComparePercentage(p1->R(), p2->R(), percentage) == false)
   return false;

 if (Utility::ComparePercentage(p1->G(), p2->G(), percentage) == false)
   return false;

 if (Utility::ComparePercentage(p1->B(), p2->B(), percentage) == false)
   return false;

 return true;
}

FileInfo::FileInfo()
{
 FileNumber=0;
 IsNumbered=false;
}

FileInfo::FileInfo(String const &path)
{
 String ws, wst;
 int i1, i2;
 int i3;

 FullPath = path;

 i1 = path.LastIndexOf(L'\\');
 if (i1>0)
  {
   Dir = path.Substring(0, i1);  
   Name = path.Substring(i1+1);
  }
 else
  {
   Dir=L"";           // no directory componenet
   Name=path;
  }
 
 ws = L"";

 i1 = Name.LastIndexOf(L'.');
 if (0 == i1) throw L"no extension?";
 
 FileNoExt = Name.Substring(0, i1);
 Extension = Name.Substring(i1+1);

 i2 = FileNoExt.LastIndexOf(L'_');

 FileNumber = 0;
 FilePrefix = L"";
 IsNumbered = false;

 if (i2 > 0)
  {
   if (i2 < FileNoExt.Length() - 1)
    {
     ws = FileNoExt.Substring(i2 + 1);
     if (String::TryIntParse(ws, &i3) == true)
      {
       FileNumber = i3;
       FilePrefix = FileNoExt.Substring(0, i2);
       IsNumbered = true;
      }
    }
  }  
}

////////////////////////////////////////////////////////////////////////////

ImageParser::ImageParser()
{
 m_Original=nullptr;
 m_Thumb=nullptr;
 m_Colors=nullptr;
 Counted = false;
}
 
ImageParser::ImageParser(String const &strDir, String const &strName, int id)
{
 Init(strDir, L"", strName, id);
}

ImageParser::ImageParser(String const &strDir, String const &sDir, String const &strName, int id)
{
 Init(strDir, sDir, strName, id);
}

ImageParser::~ImageParser()
{
 Dispose();
}

void ImageParser::Init(String const &strDir, String const &sDir, String const &strName, int id)
{
 m_ID = id;
 m_Directory = strDir;
 m_SubDir=sDir;
 m_FileName = strName;
 if (m_SubDir.Length()==0)
  {
   m_FullPath = m_Directory;
   m_FullPath += L"\\"; 
   m_FullPath +=m_FileName;
  }
 else
  {
   m_FullPath = m_Directory;
   m_FullPath += L"\\";
   m_FullPath +=m_SubDir;
   m_FullPath += L"\\"; 
   m_FullPath +=m_FileName;
  }
 m_IsNumbered = false;
 m_Good = false;
 m_Error = L"";
 m_Ready = false;
 ExtractFileInfo();
 GetFileDate();
 m_IsImport = false;
 Counted = false;
}

void ImageParser::InjectItem(PictureItem *item)
{
 m_Item=item;
}

void ImageParser::ResetReady()
{
 m_Ready = false;
 m_Good = false;
 Counted = false;
}

void ImageParser::SetImport(bool val)
{
 m_IsImport=val;
}

void ImageParser::SetItem(PictureItem *item, Bitmap *img)
{
 m_Item = item;

 if (img->GetLastStatus() != Gdiplus::Status::Ok)
   throw L"invalid bitmap";

 m_Original=new Bitmap(img->GetWidth(), img->GetHeight(), PixelFormat24bppRGB);
  {
   Graphics gr(m_Original);
   gr.DrawImage(img, Point(0,0));  //  delete the PictureItem one after loading
  }
 m_Width = m_Item->Width();
 m_Height = m_Item->Height();
 m_FileLength = (long)(m_Width * m_Height);
 m_FileSize=FSizer(m_FileLength);
 m_Ratio = (double)m_Width / (double)m_Height;
 m_FileDate = item->DateAdded();

 ExtractColor();

 if (m_Thumb != nullptr)
   delete m_Thumb;

 m_Thumb=new Bitmap(ThumbSize, ThumbSize, PixelFormat24bppRGB); // Thumb is square with "window" border for ListView 
  {
   Graphics gr(m_Thumb);
   if (App->DarkMode() == true)
     gr.Clear(Application::GetSystemColor(COLOR_WINDOWTEXT));
   else
     gr.Clear(Application::GetSystemColor(COLOR_WINDOW));
   gr.DrawImage(m_Original, Point(0,0));
  }
 m_Ready = true;
}

void ImageParser::ProcessImageInternal(Bitmap  *img)
{
 int sw, sh;

 m_Width = img->GetWidth();
 m_Height = img->GetHeight();
 m_Ratio = (double)m_Width / (double)m_Height;
 m_FileLength = (long)(m_Width * m_Height);
 m_FileSize=FSizer(m_FileLength);

 if (m_Width > m_Height) 
  {
   sw = ThumbSize;
   sh = (int)((float)ThumbSize * (double)m_Height / (double)m_Width);
  }
 else
  {
   sh = ThumbSize;
   sw = (int)((float)ThumbSize * (double)m_Width / (double)m_Height);
  }

 m_Original = new Bitmap(sw, sh, PixelFormat24bppRGB);  // imgThumb is a smaller version of the original pic
  {
   Rect dest(0,0,sw,sh);
   Graphics gr(m_Original);
   gr.DrawImage(img, dest);
  } 

 ExtractColor();

 m_Thumb = new Bitmap(ThumbSize, ThumbSize, PixelFormat24bppRGB);  // Thumb is square with "window" border
  { 
   Graphics gr(m_Thumb);
   gr.Clear(Application::GetSystemColor(COLOR_WINDOW));
   gr.DrawImage(m_Original, Point(0,0));
  }
}

bool ImageParser::Swap(ImageParser *p) 
{
 FileMoveResult res;
 int nFile;
 String fnMe, fnWith;
 String sOrd, sMe, sWith;

 sOrd  = m_Directory;
 sOrd += L"\\";
 sOrd += m_FilePrefix;
 sOrd += L"_";
 sOrd += String::Decimal(m_FileNumber, 3);
 sOrd += L".ord";

 sMe  = m_Directory;
 sMe += L"\\";
 sMe += m_FilePrefix;
 sMe += L"_";
 sMe += String::Decimal(m_FileNumber, 3);
 sMe += L".jpg";

 sWith  = m_Directory;
 sWith += L"\\";
 sWith += m_FilePrefix;
 sWith += L"_";
 sWith += String::Decimal(p->m_FileNumber, 3);
 sWith += L".jpg";

 // move this to temp

 res = ImageParser::Mover(sWith, sOrd); // prepare swap with file temporary file
 if (false == res.Success)
  {
   App->Response(res.Message);
   return false;
  }

   // move arg to this

 res = ImageParser::Mover(sMe, sWith);  // move me to the swap with location 
 if (false == res.Success)
  {
   App->Response(res.Message);
   return false;
  }

    // move temp to arg

 res = ImageParser::Mover(sOrd, sMe);  // move the temporary to me
 if (false == res.Success) 
  {
   App->Response(res.Message);
   return false;
  }

 fnMe = m_FileName;
 fnWith = p->m_FileName;

 nFile = p->m_FileNumber;
 p->m_FileNumber = m_FileNumber;
 m_FileNumber = nFile;

 m_FullPath = sWith;
 p->m_FullPath = sMe;
 m_FileName = fnWith;
 p->m_FileName = fnMe;

 ExtractFileInfo();
 p->ExtractFileInfo();

 PathHasChanged();
 p->PathHasChanged();

 return true;
}

bool ImageParser::RenameBySeq(int nSeq)
{
 // If Me.FileNumber = nSeq Then Return True
  return RenameByFileToDir(String::Decimal(nSeq,4) + L".jpg", m_Directory);
}

bool ImageParser::RenameByPartAndSeq(String const  &strPart,int nSeq)
{
 String strFile;

 strFile = strPart;
 strFile += L"_";
 strFile += String::Decimal(nSeq, 4);
 strFile += L".jpg";
 return RenameByFileToDir(strFile, m_Directory);
}
 
bool ImageParser::RenameByPartAndSeqDash(String const &strPart,int nSeq)
{
 String strFile;

 if (strPart.Length() != 0)
  {
   strFile = strPart;
   strFile += L" - ";
   strFile += String::Decimal(nSeq, 4);
   strFile += L".jpg";
  }
 else
  {
   strFile = String::Decimal(nSeq, 4); 
   strFile += L".jpg";
  }
 return RenameByFileToDir(strFile, m_Directory);
}

bool ImageParser::RenameBySeqToOrd(int nSeq)
{
 String strFile;

 strFile = String::Decimal(nSeq, 4);
 strFile += L".ord";
 return RenameByFileToDir(strFile, m_Directory);
}

bool ImageParser::RenameByPartAndSeqToOrd(String const &strPart, int nSeq)
{
 String strFile;

 strFile = strPart;
 strFile += L"_";
 strFile += String::Decimal(nSeq, 4);
 strFile += L".ord";
 return RenameByFileToDir(strFile, m_Directory);
}


bool ImageParser::RenameByFileToDir(String const &strFile, String const &strNewDir)
{
 DialogResult r;
 String strNew, msg;
 bool   blnDone;
 String nl = L"\n\n";
 FileMoveResult res;

 strNew = strNewDir;
 strNew += L"\\";
 strNew += strFile;
 blnDone = false;
 while(false == blnDone)
  {
   res = Mover(m_FullPath, strNew);
   if (true == res.Success)
        blnDone = true;
   else
    {
     msg = L"Unable to rename file ";
     msg += m_FullPath;
     msg += L" to ";
     msg += strFile;
     msg += nl;
     msg += res.Message;
     msg += nl;
     msg += L"Do you want to try again?";
     r = App->Question(msg, MB_OKCANCEL);
     if (DialogResult::Cancel == r)
       return false;
    }
  }
 m_PrevFile = m_FileName;
 m_FullPath = strNew;
 m_FileName = strFile;
 m_Directory = strNewDir;
 ExtractFileInfo();
 PathHasChanged();
 return true;
}

bool ImageParser::Delete()
{
 wchar_t *file;
 SHFILEOPSTRUCT sfos = {0};
 BOOL ret=false;
 int i, k;

 if (Utility::FileExists(m_FullPath) == true)
  {
   file = new wchar_t[m_FullPath.Length()+2];
   k = 0;
   for(i=0;i<m_FullPath.Length(); i++)
    {
     file[k++]=m_FullPath[i];
    } 
   file[k++] = L'\0';
   file[k] = L'\0';

   sfos.hwnd = Wnd->Handle();
   sfos.wFunc = FO_DELETE;
   sfos.pFrom = file;
   sfos.pTo = NULL;
   sfos.fFlags = FOF_SILENT | FOF_ALLOWUNDO;

   if (SHFileOperation(&sfos) == 0) // move file to recycle bin
     ret = true;

   delete [] file;
  }
 else
   ret = true;

 if (m_Item != nullptr)
  {
   #ifdef _DEBUG
   if (m_Item->ParentFolder()->Pictures.count(m_Item->ID()) == 0) throw L"FolderItem doesn't contain PictureItem";
   #endif 
   m_Item->ParentFolder()->Pictures.erase(m_Item->ID());
   PictureItem::RemovePictureRowID(m_Item->ID());
  }

 return ret;
}

FileMoveResult ImageParser::Mover(String const &f,String const &t)
{
 BOOL ret;
 String nl = L"\n";
 FileMoveResult res;

 if (String::Compare(f,t)==0)
  {
   res.Success = false;
   res.Message = L"From and To File names are identical -- "; 
   res.Message += f;
   return res;
  }

 if (Utility::FileExists(f) == false)
  {
   res.Success = false;
   res.Message = L"From file doesn't exist -- "; 
   res.Message += t;
   return res;
  }

 if (Utility::FileExists(t) == true)
  {
   res.Success = false;
   res.Message = L"To file already exists -- "; 
   res.Message += t;
   return res;
  }

 try
  {
   ret=::MoveFileW(f.Chars(), t.Chars());
  }
 catch(...)
  {
   res.Success = false;
   res.Message = L"System call ::MoveFileW(f,t) try catch fail -- "; 
   res.Message += String::GetLastErrorMsg(GetLastError());
   return res;
  }

 if (ret==FALSE)
  {
   res.Success = false;
   res.Message = String::GetLastErrorMsg(GetLastError());
   return res;
  }
 return res;
}

void ImageParser::ExtractFileInfo()
{
 FileInfo info(m_FileName);

 m_NoExtension=info.FileNoExt;
 m_FileNumber=info.FileNumber;
 m_FilePrefix=info.FilePrefix;
 m_IsNumbered=info.IsNumbered;
 m_Extension = info.Extension;
}


String ImageParser::FSizer(long size)
{
 String buff;
 int amt;
 

 if (size < 1024)
  { 
   buff=String::Decimal((int)size);
   return buff;
  }

 if (size < 1024000)
  { 
   amt=(int)size / 1024;
   buff=String::Decimal(amt);
   buff += L" KB";
   return buff;
  }

 if (size < 1024000000) 
  {
   amt=size / 1024000;
   buff = String::Decimal(amt);
   buff += L" MB";
   return buff;
  }

 amt=size / 1024000000;
 buff=String::Decimal(amt);
 buff += L" GB";
 return buff;
}

int ImageParser::CompareRGB(ImageParser *py)
{
 int c,i;
 int k;

 if (ColorIndex.size() <= py->ColorIndex.size()) 
   c = (int)ColorIndex.size(); 
 else 
   c = (int)py->ColorIndex.size();

 for (i = 0;i<c;i++)
  {
   k = PicColor::CompareRGB(m_Colors[i], py->m_Colors[i]);
   if (k != 0) return k;
   if (m_Colors[i]->Amount() > py->m_Colors[i]->Amount()) return 1;
   if (m_Colors[i]->Amount() < py->m_Colors[i]->Amount()) return -1;
  }
 return 0;
}

bool ImageParser::Equal(ImageParser *py)
{
 int i, c;

 if (m_Width != py->Width()) return false;
 if (m_Height != py->Height()) return false;
 if (ColorIndex.size() != py->ColorIndex.size()) return false;

 c = (int)ColorIndex.size();
 for (i=0;i<c;i++)
  {
   if (PicColor::EqualRGB(m_Colors[i], py->m_Colors[i]) == false)
     return false;
   if (m_Colors[i]->Amount() != py->m_Colors[i]->Amount()) 
     return false;
  }
 return true;
}

float ImageParser::Percentage(ImageParser *py, float percentage)
{
 int i, c, cs1, cs2, tm;

 cs1 = (int)ColorIndex.size();
 cs2 = (int)py->ColorIndex.size();

 if (Utility::ComparePercentage(cs1, cs2, percentage) == false)
   return false;

 if (cs1 < cs2)
   c = cs1;
 else
   c = cs2;

 tm = 0;

 for (i=0;i<c;i++)
  {
   if ( PicColor::PercentageRGB(m_Colors[i], py->m_Colors[i], percentage) == true)
    {
     cs1 = m_Colors[i]->Amount();
     cs2 = py->m_Colors[i]->Amount();
     if (Utility::ComparePercentage(cs1, cs1, percentage) == true)
       tm++;
    }
  }

 return (float)tm/(float)c;
}

void ImageParser::Dispose() 
{
 if (nullptr != m_Thumb)
  {
   delete m_Thumb;
   m_Thumb = nullptr;
  }
 if (nullptr != m_Original)
  {
   delete m_Original;
   m_Original = nullptr;
  }
 ClearColorIndex();
}

void ImageParser::ClearColorIndex()
{
 if (ColorIndex.size() > 0)
  {
   for (const auto &it : ColorIndex)
     delete it.second;
   ColorIndex.clear();
   if (m_Colors != nullptr)
     delete [] m_Colors;
  }
}

void ImageParser::ReprocessImageProc(ImageParser *img)
{
 img->ReprocessImage();
}

void ImageParser::ReprocessImage()
{
 if (nullptr !=m_Thumb)
  {
   delete m_Thumb;
   m_Thumb = nullptr;
  }
 if (nullptr != m_Original)
  {
   delete m_Original;
   m_Original = nullptr;
  }
 ProcessImage();
}
 
void ImageParser::ProcessForCompare(Bitmap *bmp)  // special case used for picture comparison
{
 ProcessImageInternal(bmp);
}

void ImageParser::GetFileDate()
{
 WIN32_FILE_ATTRIBUTE_DATA data={0};
 GET_FILEEX_INFO_LEVELS info;
 SYSTEMTIME st{0};

 info = GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard;
 
 if (::GetFileAttributesExW(m_FullPath.Chars(), info, &data)==false)
   return;
 
 m_FileSize = FSizer(data.nFileSizeLow);
 if ( ::FileTimeToSystemTime(&data.ftLastWriteTime, &st) == FALSE)
   throw L"Failed to get system time from data";
 
 m_FileDate=DateTime::FromSystemTime(st);
}

void ImageParser::ProcessImage()
{
 Bitmap *img=nullptr;
 bool success;
 int sw, sh;


 if (nullptr != m_Thumb)
  {
   delete m_Thumb;
   m_Thumb = nullptr;
  }

 if (Utility::FileExists(m_FullPath) == false)
  {
   success = false;
   m_Error = L"File does not exist";
   m_Good = false;
   m_Ready = true;
   return;
  }

 try
  {
   img = Bitmap::FromFile(m_FullPath.Chars(), false);
   success=true;
  }
 catch(...)
  {
   success=false;
  } 
 if (nullptr == img || false == success)
  {
   m_Error = L"Failed to load bitmap";
   m_Good = false;
   m_Ready = true;
   return;
  }

  m_Width = img->GetWidth();
  m_Height = img->GetHeight();
  m_FileLength = (long)(m_Width * m_Height);
  m_Ratio = (double)m_Width / (double)m_Height;

  if (m_Width > m_Height)
   {
    sw = ThumbSize;
    sh = (int)(ThumbSize * m_Height / m_Width);
   }
  else
   {
    sh = ThumbSize;
    sw = (int)(ThumbSize * m_Width / m_Height);
   }

  m_Original = new Bitmap(sw, sh, PixelFormat24bppRGB); // imgThumb is a smaller version of the original pic
   {
    Rect dest(0,0,sw,sh);
    Graphics gr(m_Original);
    gr.DrawImage(img, dest);
   }

  ExtractColor();

  m_Thumb = new Bitmap(ThumbSize, ThumbSize, PixelFormat24bppRGB);
  {
   Graphics gr(m_Thumb);
   if (App->DarkMode() == true)
     gr.Clear(Application::GetSystemColor(COLOR_WINDOWTEXT));
   else
     gr.Clear(Application::GetSystemColor(COLOR_WINDOW));
   gr.DrawImage(m_Original, Point(0,0));
  }

 delete img;
 m_Good = true;
 m_Ready = true;
}

Bitmap *ImageParser::GetOriginal(String const &file)
{
 Bitmap *img, *org;
 int w, h, sw, sh;

 try
  {
   img = new Bitmap(file.Chars(), false);
  }
 catch(...)
  {
   return nullptr; // caller must check for a null return
  } 

  w = img->GetWidth();
  h = img->GetHeight();

  if (w > h)
   {
    sw = ThumbSize;
    sh = (int)(ThumbSize * h / w);
   }
  else
   {
    sh = ThumbSize;
    sw = (int)(ThumbSize * w / h);
   }

  org = new Bitmap(sw, sh, PixelFormat24bppRGB); // imgThumb is a smaller version of the original pic
   {
    Rect dest(0,0,sw,sh);
    Graphics gr(org);
    gr.DrawImage(img, dest);
   }

  delete img;
  return org;
}

void ImageParser::ExtractColor()
{
 std::vector<PicColor *> arr;
 Rect rct;
 int r,g,b,k,w,h,t;
 long nColors, rba, gba, bba;
 PicColor *pc;
 int nEdgeColors;
 int ic;
 BitmapData data;
 int pixelSize;
 int dataWidth;

 rba=0;
 gba=0;
 bba=0;

 if (m_Original->GetLastStatus() != Gdiplus::Status::Ok)
   throw L"Invalid bitmap";

 w=m_Original->GetWidth();
 h=m_Original->GetHeight();
 t = w * h;

 #ifdef _DEBUG
 if (w < 1 || h < 1) throw L"width and or height < 1";
 #endif

 rct = Rect(0, 0, w, h);
 m_Original->LockBits(&rct, ImageLockModeRead, m_Original->GetPixelFormat(), &data);
 nEdgeColors = 0;
 nColors = 0;

 m_Monochrome = true;

 ClearColorIndex();  // clear old color data

 pixelSize = 3;      // PixelFormat24bppRGB

 
 register byte *bytes =(byte* )(void *)data.Scan0;
 dataWidth = w * pixelSize;

 register int y;
 for (y=0; y < h; y++)
  {
   register int x;
   for (x=0; x < dataWidth; x++ )
    {
     if (x % pixelSize == 0 || x ==0) // pointing to red?
      {
       r=bytes[0];
       g=bytes[1];
       b=bytes[2];
       k = ( r << 16 ) | ( g << 8 ) | b;
       pc = new PicColor(k, r, g, b, 1);
       if (0 == y  || y == h - 1 || 0 == x || x == w - 1) // edges
        {
         rba += r;
         gba += g;
         bba += b;
         nEdgeColors += 1;
        }
       if (ColorIndex.count(pc->Key()) == 0) 
         ColorIndex.insert(std::pair<int, PicColor *>(pc->Key(), pc)); 
       else 
         ColorIndex.at(pc->Key())->Accumulate();
       if (true == m_Monochrome)
        {
         if (r != g || r != b || g != b)
           m_Monochrome = false;  // nope, it's just another color picture
        }  
       nColors++; 
      }
     bytes++;
    }
  }

 m_Original->UnlockBits(&data);

 for (const auto &it : ColorIndex)
  {
   it.second->Finalize(t);
   arr.push_back(it.second);
  }
 std::sort(arr.begin(), arr.end(), ImageParserCustomSort);
 ic=(int)arr.size();
 m_Colors = new PicColor *[ic];
 register int i=0;
 for (const auto &ai : arr)
  {
   #ifdef _DEBUG
   if (i>=ic) throw L"buffer overrun";
   #endif 
   m_Colors[i++]=ai;
  }

 m_BGAvgRed = (int)(rba / nEdgeColors);
 m_BGAvgGreen = (int)(gba / nEdgeColors);
 m_BGAvgBlue = (int)(bba / nEdgeColors);
}

void ImageParser::PathHasChanged()
{
 if (m_Item != nullptr)
  {
   if (String::Compare(L"jpg", m_Extension) == 0)  // don't update if it's .ord used to re-number
     PictureItem::UpdateFileName(m_Item->ID(), m_FileName);
  }
}

//////////////////////////////////////////////////////////////////////////
  
bool ImageParserSorter::operator() (ImageParser *i1, ImageParser *i2)
{
 ImageParser *ix, *iy;
 int i, c, k;

 if (Ascending == true)
  {
   ix = i1;
   iy = i2;
  }
 else
  {
   ix = i2;
   iy = i1;
  } 

 switch(Sort)
  {
   case ImageParser::SortChoices::FileName:
    {
     return String::Compare(ix->FileName(), iy->FileName()) < 0;
    }
   case ImageParser::SortChoices::NameWithNumber:
    {
     if (ix->IsNumbered() == false || iy->IsNumbered() == false) // if either isn't numbered just compare names
       return String::Compare(ix->FileName(), iy->FileName()) < 0;
     c=String::Compare(ix->FilePrefix(), iy->FilePrefix());
     if (c != 0) return c < 0;
     return Utility::StdIntCompare(ix->FileNumber(), iy->FileNumber());
    }
   case ImageParser::SortChoices::Width:
    {
     return Utility::StdIntCompare(ix->Width(), iy->Width());
    }
   case ImageParser::SortChoices::Height:
    {
     return Utility::StdIntCompare(ix->Height(), iy->Height());
    }
   case ImageParser::SortChoices::Ratio:
    {
     if (ix->Ratio() < iy->Ratio())
       return true;
     else
       return false;
    }
   case ImageParser::SortChoices::ColorRGB:
    {
     if (ix->ColorIndex.size() <= iy->ColorIndex.size()) 
       c = (int)ix->ColorIndex.size(); 
     else 
       c = (int)iy->ColorIndex.size();
     for (i=0;i<c;i++)
      {
       k = PicColor::CompareRGB(ix->Colors()[i], iy->Colors()[i]);
       if (k != 0) return k < 0;
       if (ix->Colors()[i]->Ratio() < iy->Colors()[i]->Ratio()) 
         return true;
      }
     return false;
    }
   case ImageParser::SortChoices::EdgeAverage:
    {
     if (ix->BGAvgRed() < iy->BGAvgRed())     return true;
     if (ix->BGAvgGreen() < iy->BGAvgGreen()) return true;
     if (ix->BGAvgBlue() < iy->BGAvgBlue())   return true;
     return false;
    }
   case ImageParser::SortChoices::FileSize:
    {
     if (ix->FileLength() < iy->FileLength())
       return true;
     else
       return false;
    }
   case ImageParser::SortChoices::FileDate:
    {
     if (ix->FileDate() < iy->FileDate())
       return true;
     else
       return false;
    }
   case ImageParser::SortChoices::GlobalHashTags:
    {
     if (ix->Item() == nullptr && iy->Item() == nullptr) return false;
     if (ix->Item() != nullptr)
      {
       return false;
      }
     if (iy->Item() != nullptr)
      {
       if (ix->Item() == nullptr) return true;
       if (iy->Item()->GlobalHashTags.count(GlobalHashTagID)>0) return true;
      }
     return false;
    }
   case ImageParser::SortChoices::FolderHashTags:
    {
     if (ix->Item() == nullptr && iy->Item() == nullptr) return false;
     if (ix->Item() != nullptr)
      {
       if (iy->Item() == nullptr) return false;
       c=String::Compare(ix->Item()->PictureHashTagString(),iy->Item()->PictureHashTagString()); 
       if (c != 0) return c<0;
       return String::Compare(ix->Item()->FileName(), iy->Item()->FileName()) < 0;
      }
     if (iy->Item() != nullptr)
      {
       if (ix->Item() == nullptr) return true;
       c = String::Compare(ix->Item()->PictureHashTagString(), iy->Item()->PictureHashTagString()); 
       if (c != 0) return c < 0;
       return String::Compare(ix->Item()->FileName(),iy->Item()->FileName()) < 0;
      } 
     return false; // shouldn't get here
    }
   default: throw L"Unhandled Sort value";
  }
 throw L"error";
}