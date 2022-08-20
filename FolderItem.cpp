#include "pch.h"
#include "app.h"

//////////////////////////////////////////////////////////////////////

FolderItem::FolderItem(String const &sName, BaseItem *bi)
{
 MySqlCommand cmd;
 MySqlDataReader *rs;

 m_ParentBase = bi;
 m_Folder = sName;
 
 cmd = MySqlCommand(L"INSERT INTO folder(dir_id, folder_name, folder_added) VALUES (@dir, @name, Now());", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@dir", m_ParentBase->ID()));
 cmd.Parameters.Add(new MySqlParameter(L"@name", sName));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 cmd = MySqlCommand(L"SELECT LAST_INSERT_ID();", DB->Con()); 
 rs = cmd.ExecuteReader();
 rs->Read();
 m_ID = rs->GetInt32(0);
 rs->Close();
 cmd.Dispose();

 m_Added = DateTime::Now();
 bi->Folders.insert(std::pair<int, FolderItem *>(m_ID, this));
}

FolderItem::FolderItem(int nID, String const &sName, BaseItem *baseItem, DateTime const &dtAdd, DateTime const &dtOpen)
{
 m_ID = nID;
 m_Folder = sName;
 m_ParentBase = baseItem;
 m_Added = dtAdd;
 m_LastOpened = dtOpen;
}

FolderItem::~FolderItem()
{
 if (Pictures.size()>0) throw L"FolderItem not closed";
}

String FolderItem::GetFolderDir()
{
 String dir;

 dir=m_ParentBase->DirPath();
 dir += L"\\";
 dir += m_Folder;

 return dir;
}

HashTag FolderItem::AddHashTag(String const &s)
{ 
 HashTag ht;
 MySqlDataReader *rs;

 for(const auto &it : FolderHashTags)
  {
   ht=it.second;
   if (s.ToLower() == ht.Name().ToLower()) 
     return HashTag(); // type set to None
  }

 // INSERT INTO hashtag_folder(name,folder_id) VALUES (@n,@f);  SELECT LAST_INSERT_ID();

 MySqlCommand cmd(L"CALL hashtag_folder_insert_proc(@n, @f);", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@n", s));
 cmd.Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd.ExecuteReader();
 if (rs->Read() == false) throw L"auto ident not read";
 ht = HashTag(rs->GetInt32(0), s, this, HashTag::HashTagType::FolderTag);
 rs->Close();
 cmd.Dispose();

 FolderHashTags.insert(std::pair<int, HashTag>(ht.ID(), ht));

 return ht;
}

bool FolderItem::UpdateFolderHashTag(HashTag &ht, String const &s)
{
 if (String::Compare(s,ht.Name()) == 0) 
   return false; // no change

 for(const auto &it : FolderHashTags)
  {
   if (it.second.ID() != ht.ID() && String::Compare(s, it.second.Name())==0) 
     return false; // duplication
  }

 MySqlCommand cmd(L"UPDATE hashtag_folder SET name=@s WHERE hashtag_id=@i;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@s", s));
 cmd.Parameters.Add(new MySqlParameter(L"@i", ht.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
 ht.SetName(s);

 return true;
}

void FolderItem::DropHashTag(HashTag *ht)
{
 PictureItem *p;

 MySqlCommand cmd1(L"DELETE FROM picture_folder_hashtag WHERE hashtag_id=@h;", DB->Con());
 cmd1.Parameters.Add(new MySqlParameter(L"@h", ht->ID()));
 cmd1.ExecuteNonQuery();
 cmd1.Dispose();

 MySqlCommand cmd2(L"DELETE FROM hashtag_folder WHERE hashtag_id=@h;", DB->Con());
 cmd2.Parameters.Add(new MySqlParameter(L"@h", ht->ID()));
 cmd2.ExecuteNonQuery();
 cmd2.Dispose();

 if (FolderHashTags.count(ht->ID()) > 0)
   FolderHashTags.erase(ht->ID());
 
 for(const auto &it : Pictures)
  {
   p=it.second; 
   if (p->PictureHashTags.count(ht->ID())>0) 
    {
     p->PictureHashTags.erase(ht->ID());
    }
  }
}


bool FolderItem::FolderHashTagInUse(HashTag const &fht)
{
 for(const auto &it : Pictures)
  {
   if (it.second->SearchPictureTags(fht.ID()) == true) return true;
  }
 return false;
}

void FolderItem::LoadInfo()
{
 MySqlDataReader *rs;

 MySqlCommand cmd(L"SELECT COUNT(*) FROM picture WHERE folder_id=@f;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd.ExecuteReader();
 rs->Read();
 m_PictureCount=rs->GetInt32(0);
 rs->Close();
 cmd.Dispose();

}




void FolderItem::ResetPictures()
{ 
 std::vector<String> sql;

 sql.push_back(L"DELETE FROM picture_folder_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@id);");
 sql.push_back(L"DELETE FROM picture_folder_subtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@id);");
 sql.push_back(L"DELETE FROM picture WHERE folder_id=@id;");
 sql.push_back(L"DELETE FROM picture_thumbnail WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@id);");

 for(const auto &q : sql)
  {
   MySqlCommand cmd(q, DB->Con());
   cmd.Parameters.Add(new MySqlParameter(L"@id", m_ID));
   cmd.ExecuteNonQuery();
   cmd.Dispose();
  }

 ClearPictures();
}
 

bool FolderItem::ContainsFile(String const &sName)
{
 std::wstring search;

 search = String::ToLower(sName);

 if (FileNames.count(search) > 0)
   return true;

 return false;
}

void FolderItem::Close()
{
 ClearPictures();
 ClearHashTags();
}

void FolderItem::ClearPictures()
{
 for(const auto &it : Pictures)
  {
   delete it.second;
  }
 Pictures.clear();
}

void FolderItem::ClearHashTags()
{
 FolderHashTags.clear();
}

String FolderItem::FolderPath()
{
 String dir;

 dir=m_ParentBase->DirPath();
 dir += L"\\";
 dir += m_Folder;

 return dir;
}

void FolderItem::BumpOpened()
{
 MySqlCommand cmd(L"UPDATE folder SET folder_opened=Now() WHERE folder_id=@f;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@f", m_ID));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
 m_LastOpened = DateTime::Now();
}

void FolderItem::ThumbThread(PictureItem *px, Bitmap *bmp)
{
 px->Picture()->SetItem(px, bmp);
 delete bmp;
}

void FolderItem::Load()
{
 ProgressBar pgb(Wnd->StatusBarCtrl(), 1);
 MySqlCommand *cmd;
 HashTag pht;
 PictureItem *pic;
 MySqlDataReader *rs;
 String s;
 Bitmap *img;
 int count;
 int id, h, st;
 std::vector<PictureItem *> dupPics;     // contains any file name that was duplicate, 1st row received is in LoadPics
 std::vector<Ints *> listDups;
 std::vector<Ints *> secondPass;
 std::vector<std::thread> threads;
 bool bMsg=true;
 
 ClearPictures();
 ClearHashTags();
 FileNames.clear();

 cmd=new MySqlCommand(L"SELECT COUNT(*) FROM picture WHERE folder_id=@folder;", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd->ExecuteReader();
 rs->Read();
 count = rs->GetInt32(0);
 rs->Close();
 delete cmd;

 cmd=new MySqlCommand(L"SELECT picture_id, file_name, picture_added, width, height, sub_dir FROM picture WHERE folder_id=@folder;", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   id=rs->GetInt32(0);
   if (rs->IsDBNull(5)==true)
     s=L"";
   else 
     s=rs->GetString(5);
   pic = new PictureItem(id, rs->GetString(1), rs->GetDateTime(2), this, rs->GetInt32(3), rs->GetInt32(4), s);
   s=pic->FullName().ToLower(); 
   if (FileNames.count(s)>0)
    {
     dupPics.push_back(pic);
    }
   else
    {
     Pictures.insert(std::pair<int, PictureItem *>(id, pic));
     FileNames.insert(std::pair<String, int>(s,id));  // file name, picture id
    }
  }
 rs->Close();
 delete cmd;
 
 for(const auto &dup : dupPics)
  {
   dup->Delete();  // a file name was somehow duplicated
   delete dup;
  }
 dupPics.clear();


 Wnd->StatusBarText(L"Getting Thumbnails From Database");
 pgb.Max(count);

 cmd = new MySqlCommand(L"SELECT picture_id, thumbnail FROM picture_thumbnail WHERE picture_id IN ( SELECT picture_id FROM picture WHERE folder_id = @folder);", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   id=rs->GetInt32(0);
   if (Pictures.count(id)>0)
    {
     pic = Pictures[id];
     pic->MissingThumbnailRow = false;
     if (rs->IsDBNull(1) == false)
      {
       img = rs->GetBitmap(1);
       if (img == nullptr)
        {
         img = GetNewThumb(pic->SubDir(),  pic->FileName(), bMsg);
         bMsg=false;
        }
      }
     else 
      {
       img = GetNewThumb(pic->SubDir(), pic->FileName(), bMsg);
       bMsg=false;
      }
     threads.push_back(std::thread(FolderItem::ThumbThread, pic, img));
    }
   pgb.Progress();
  }
 rs->Close();
 delete cmd;

 pgb.Max(threads.size());
 for (auto &thread : threads)
  {
   pgb.Progress();
   thread.join();
  }

 for(const auto &p : Pictures) // in case a picture is missing it's thumbnail row
  {
   pic = p.second;
   if (pic->MissingThumbnailRow==true)
    {
     img = GetNewThumb(pic->SubDir(),  pic->FileName(), false);  // this is an application error where a DB picture item is missing it's corresponding picture_thumb row
     pic->Picture()->SetItem(pic, img);
     App->Pictures.insert(std::pair<int, ImageParser *>(pic->Picture()->ID(), pic->Picture()));

     cmd = new MySqlCommand(L"INSERT INTO picture_thumbnail (picture_id, thumbnail) VALUES (@pid, @thumb);", DB->Con());
     cmd->Parameters.Add(new MySqlParameter(L"@pid", pic->ID()));
     cmd->Parameters.Add(new MySqlParameter(L"@thumb", img));
     cmd->ExecuteNonQuery();
     cmd->Dispose();     
    }
   App->Pictures.insert(std::pair<int, ImageParser *>(pic->Picture()->ID(), pic->Picture()));
  }

 cmd = new MySqlCommand(L"SELECT hashtag_id, name FROM hashtag_folder WHERE folder_id=@f;", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   id = rs->GetInt32(0);
   s = rs->GetString(1);
   FolderHashTags.insert(std::pair<int, HashTag>(id, HashTag(id, s, this, HashTag::HashTagType::FolderTag)));
  }
 rs->Close();
 delete cmd;

 cmd = new MySqlCommand(L"SELECT hashtag_id, picture_id FROM picture_global_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@f);", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   h = rs->GetInt32(0);
   id = rs->GetInt32(1);
   if (Pictures.count(id)==0)  throw L"hashtag contains picture id that wasn't in map";
   pic = Pictures[id];
   if (App->GlobalHashTags.count(h)==0) throw L"global hashtags doesn't contain h";
   pic->GlobalHashTags.insert(std::pair<int, HashTag>(h, App->GlobalHashTags[h]));
  } 
 rs->Close();
 delete cmd;

 // 1st level picture hashtags

 cmd = new MySqlCommand(L"SELECT hashtag_id, picture_id FROM picture_folder_hashtag WHERE hashtag_id IN (SELECT hashtag_id FROM hashtag_folder WHERE folder_id=@f);", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   h = rs->GetInt32(0);
   id = rs->GetInt32(1);
   if (Pictures.count(id) == 0) throw L"hash tag contains picture id not in map";
   pic = Pictures[id];
   if (pic->PictureHashTags.count(h) == false) 
     pic->PictureHashTags.insert(std::pair<int, HashTag>(h, HashTag(h, FolderHashTags[h].Name(), this, HashTag::HashTagType::PictureTag)));
   else
     listDups.push_back(new Ints(h, id, 0));
  }
 rs->Close();
 delete cmd;

 if (listDups.size() > 0)  // fix database, a coding mistake led to dups
      throw L"base picture hashtag dups";

// picture hashtags that are nested under 1st level picture hashtags
 
 cmd = new MySqlCommand(L"SELECT hashtag_id, picture_id, sub_hashtag_id FROM picture_folder_subtag WHERE hashtag_id IN (SELECT hashtag_id FROM hashtag_folder WHERE folder_id=@f);", DB->Con());
 cmd->Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd->ExecuteReader();
 while(rs->Read() == true)
  {
   h = rs->GetInt32(0);
   id = rs->GetInt32(1);
   st = rs->GetInt32(2);
   if (Pictures.count(id)==0) throw L"Picture not found";
   pic = Pictures[id];
   if (pic->PictureHashTags.count(h) > 0) 
    {
     if (FolderHashTags.count(st) > 0) 
      {
       if (pic->PictureHashTags[h].SubTags.count(st) == 0) 
         pic->PictureHashTags[h].SubTags.insert(std::pair<int, HashTag>(st, HashTag(FolderHashTags[st].ID(), FolderHashTags[st].Name(), this, HashTag::HashTagType::PictureTag)));
      }
     else
        secondPass.push_back(new Ints(h, id, st));
    }
  }
 rs->Close();
 delete cmd;

 for(const auto &sp : secondPass)
  {
   pic = Pictures[sp->PicID];
  // foreach(PictureHashTag pht in pic.PictureHashTags.Values)
   for(const auto &it : pic->PictureHashTags)
    {
     pht=it.second;
     if (pht.SubTags.count(sp->HashID))
       pht.SubTags.at(sp->HashID).SubTags.insert(std::pair<int, HashTag>(sp->SubID, HashTag(sp->SubID, FolderHashTags.at(sp->SubID).Name(), this, HashTag::HashTagType::PictureTag)));
    }
   delete sp;  // clean up instances of Ints
  }

 secondPass.clear();

    //  For Each dup In listDups
    //   MySqlCommand cmd(("DELETE FROM picture_folder_hashtag WHERE hashtag_id=@h AND picture_id=@p; INSERT INTO picture_folder_hashtag(hashtag_id,picture_id) VALUES (@h, @p);", DB)
    //   cmd.Parameters.Add(New MySqlParameter(L"@h", dup.HashID))
    //    cmd.Parameters.Add(New MySqlParameter(L"@p", dup.PicID))
    //    cmd.ExecuteNonQuery()
    //    cmd.Dispose()
    //  Next
}

Bitmap *FolderItem::GetNewThumb(String const &subdir, String const &fileName, bool bShowMessage)
{
 Bitmap *img;
 String sd;
 String ms;
 String fp;

 fp=FolderPath();

 sd = fp;

 if (subdir.Length()==0)
  {
   sd += L"\\";
   sd += fileName;
  }
 else
  {
   sd += L"\\";
   sd += subdir;
   sd += L"\\";
   sd += fileName;
  }
 if (bShowMessage == true)
  {
   ms = L"Failed to load thumb from DB. Use File > Reload Thumbnails to fix. ";
   ms += fp;
   ms += L"\\";
   ms += fileName;
   App->Response(ms);
  }
 img = ImageParser::GetOriginal(sd);
 if (img == nullptr) 
  {
   ms = L"File ";
   ms += sd;
   ms += L" is corrupt or doesn't exist";
   App->Response(ms);
  }
 return img;
}


void FolderItem::GetPictureList(int ghtID)
{
 std::map<int, PictureItem *> loadPics;
 PictureItem *pic;
 MySqlDataReader *rs;
 std::vector<std::thread> threads;
 String sFull, q;
 Bitmap *img;
 String sd;
 int id, hid;

 if (PictureList.size() != 0) throw L"MainWnd needs to empty list and delete items after use";

 MySqlCommand cmd1(L"SELECT picture_id, file_name, picture_added, width, height, sub_dir FROM picture WHERE folder_id=@folder AND picture_id IN (SELECT picture_id FROM picture_global_hashtag WHERE hashtag_id=@hid) ORDER BY file_name;", DB->Con());
 cmd1.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 cmd1.Parameters.Add(new MySqlParameter(L"@hid", ghtID));
 rs = cmd1.ExecuteReader();
 while(rs->Read() == true)
  {
   if (rs->IsDBNull(5)==true)
     sd=L"";
   else  
     sd=rs->GetString(5);
   pic = new PictureItem(rs->GetInt32(0), rs->GetString(1), rs->GetDateTime(2), this, rs->GetInt32(3), rs->GetInt32(4), sd);
   loadPics.insert(std::pair<int, PictureItem *>(pic->ID(), pic));
  }
 rs->Close();
 cmd1.Dispose();

 if (loadPics.size() == 0)
   return;  // none found

 q=L"SELECT picture_id, thumbnail FROM picture_thumbnail ";
 q+=L"WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@folder) ";
 q+=L"AND picture_id IN (SELECT picture_id FROM picture_global_hashtag WHERE hashtag_id=@hid);";
 MySqlCommand cmd2(q, DB->Con());
 cmd2.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 cmd2.Parameters.Add(new MySqlParameter(L"@hid", ghtID));
 rs = cmd2.ExecuteReader();
 while(rs->Read() == true)
  {
   id=rs->GetInt32(0);
   if (rs->IsDBNull(1) == false && loadPics.count(id)>0)
    {
     img=rs->GetBitmap(1);
     if (img != nullptr) // bad images might cause ShowGroupWnd to crash....
      {
       threads.push_back(std::thread(FolderItem::ThumbThread, loadPics[id], img));
      }
    }
  }
 rs->Close();
 cmd2.Dispose();

 for (auto &thread : threads)
  {
   thread.join();
  }

 MySqlCommand cmd4(L"SELECT hashtag_id, picture_id FROM picture_global_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@f);", DB->Con());
 cmd4.Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd4.ExecuteReader();
 while(rs->Read() == true)
  {
   hid = rs->GetInt32(0);
   id = rs->GetInt32(1);
   if (loadPics.count(id) > 0) 
    {
     pic = loadPics.at(id);
     #ifdef _DEBUG
     if (App->GlobalHashTags.count(hid)==0) throw L"global hashtags doesn't contain h";
     #endif
     pic->GlobalHashTags.insert(std::pair<int, HashTag>(hid, App->GlobalHashTags.at(hid)));
    }
  } 
 rs->Close();
 cmd4.Dispose();

 for(const auto &it : loadPics)
  {
   if (it.second->Picture()->Original() != nullptr)
     PictureList.push_back(it.second);
  }
}

void FolderItem::GetPictureListHavingGlobal() // pictures with any global tags
{
 std::map<int, PictureItem *> loadPics;
 std::vector<std::thread> threads;
 PictureItem *pic;
 MySqlDataReader *rs;
 String sFull, q;
 Bitmap *img;
 String sd;
 int id, hid;

 if (PictureList.size() != 0) throw L"MainWnd needs to empty list and delete items after use";

 MySqlCommand cmd1(L"SELECT picture_id, file_name, picture_added, width, height, sub_dir FROM picture WHERE folder_id=@folder AND picture_id IN (SELECT picture_id FROM picture_global_hashtag) ORDER BY file_name;", DB->Con());
 cmd1.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd1.ExecuteReader();
 while(rs->Read() == true)
  {
   if (rs->IsDBNull(5)==true)
     sd=L"";
   else  
     sd=rs->GetString(5);
   pic = new PictureItem(rs->GetInt32(0), rs->GetString(1), rs->GetDateTime(2), this, rs->GetInt32(3), rs->GetInt32(4), sd);
   loadPics.insert(std::pair<int, PictureItem *>(pic->ID(), pic));
  }
 rs->Close();
 cmd1.Dispose();

 if (loadPics.size() == 0)
   return;  // none found

 q=L"SELECT picture_id, thumbnail FROM picture_thumbnail ";
 q+=L"WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@folder) ";
 q+=L"AND picture_id IN (SELECT picture_id FROM picture_global_hashtag);";
 MySqlCommand cmd2(q, DB->Con());
 cmd2.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd2.ExecuteReader();
 while(rs->Read() == true)
  {
   id=rs->GetInt32(0);
   if (rs->IsDBNull(1) == false && loadPics.count(id)>0)
    {
     img=rs->GetBitmap(1);
     if (img != nullptr) // bad images might cause ShowGroupWnd to crash....
      {
       threads.push_back(std::thread(FolderItem::ThumbThread, loadPics[id], img));
      }
    }
  }
 rs->Close();
 cmd2.Dispose();

 for (auto &thread : threads)
  {
   thread.join();
  }

 MySqlCommand cmd4(L"SELECT hashtag_id, picture_id FROM picture_global_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@f);", DB->Con());
 cmd4.Parameters.Add(new MySqlParameter(L"@f", m_ID));
 rs = cmd4.ExecuteReader();
 while(rs->Read() == true)
  {
   hid = rs->GetInt32(0);
   id = rs->GetInt32(1);
   if (loadPics.count(id) > 0) 
    {
     pic = loadPics.at(id);
     #ifdef _DEBUG
     if (App->GlobalHashTags.count(hid)==0) throw L"global hashtags doesn't contain h";
     #endif
     pic->GlobalHashTags.insert(std::pair<int, HashTag>(hid, App->GlobalHashTags.at(hid)));
    }
  } 
 rs->Close();
 cmd4.Dispose();

 for(const auto &it : loadPics)
  {
   if (it.second->Picture()->Original() != nullptr)
     PictureList.push_back(it.second);
  }

}

void FolderItem::GetFileList()
{
 MySqlDataReader *rs;
 String file;
 String fp;
 FileList.clear();

 fp = FolderPath();

 MySqlCommand cmd(L"SELECT file_name,sub_dir FROM picture WHERE folder_id=@folder ORDER BY file_name;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd.ExecuteReader();
 while(rs->Read() == true)
  {
   file = fp;
   file + L"\\";
   if (rs->IsDBNull(1) == true)
    {
     file += rs->GetString(0);
    }
   else
    {
     file += rs->GetString(1);
     file += L"\\";
     file += rs->GetString(0);
    }
   FileList.push_back(file);
  }
 rs->Close();
 cmd.Dispose();
}

void FolderItem::GetFileList(int ghtID)
{
 MySqlDataReader *rs;
 String file;
 String fp;

 FileList.clear();
 fp = FolderPath();

 MySqlCommand cmd(L"SELECT file_name,sub_dir FROM picture WHERE folder_id=@folder AND picture_id IN (SELECT picture_id FROM picture_global_hashtag WHERE hashtag_id=@hid) ORDER BY file_name;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 cmd.Parameters.Add(new MySqlParameter(L"@hid", ghtID));
 rs = cmd.ExecuteReader();
 while(rs->Read() == true)
  {
   file = fp;
   file + L"\\";
   if (rs->IsDBNull(1) == true)
    {
     file += rs->GetString(0);
    }
   else
    {
     file += rs->GetString(1);
     file += L"\\";
     file += rs->GetString(0);
    }
   FileList.push_back(file);
  }
 rs->Close();
 cmd.Dispose();
}

void FolderItem::MapTagUse()
{
 PictureItem *pic;
 
 for(auto &fi : FolderHashTags)
   fi.second.SetInUse(false);  // reset all folder hashtags

 for(const auto &pi : Pictures)
  {
   pic=pi.second;
   for(auto pt : pic->PictureHashTags)
    {
     pt.second.SetInUse(true); // recursively set all used folder hashtags to "true"
    }
  }
}

PictureItem *FolderItem::FindPicture(ImageParser *p)
{
 PictureItem *pic;
 std::wstring search;
 int id;

 search = String::ToLower(p->FileName());
 
 if (FileNames.count(search)>0)
  {
   id = FileNames.at(search);
   if (Pictures.count(id) > 0)
    {
     pic=Pictures.at(id);
     return pic;
    }
  }
 return nullptr;
}

void FolderItem::ProcessPictures(std::vector<ImageParser *> const &pics)
{
 PictureItem *pic;
 std::vector<PictureItem *> append;
 bool bFound;
 String fp;

 fp = FolderPath();

 for(const auto &ip : pics)
  {
   if (ip->Item()==nullptr)
    {
     bFound = false;
     for(const auto &pi : Pictures)
      {
       pic=pi.second;
       if (String::Compare(ip->FileName(), pic->FileName()) == 0 && String::Compare(ip->Directory(), fp) == 0)
        {       
         ip->SetItem(pic, ip->Original());
         bFound = true;
         break;
        }
      }
     if (bFound == false)
      {
       PictureItem *pi2 = new PictureItem(this, ip);
       append.push_back(pi2);
      }
    }
  }
 for(const auto &pic : append)
  {
   Pictures.insert(std::pair<int, PictureItem *>(pic->ID(), pic));
  }
}

String FolderItem::FirstName()
{
 std::vector<String> parts;
 String ret;

 ret=L"";

 parts = m_Folder.Split(L',');
 if (parts.size() > 1)
   ret=parts[1].Trim();

 return ret;
}

String FolderItem::LastName()
{
 std::vector<String> parts;
 String ret;

 ret=L"";
 
 parts=m_Folder.Split(L',');
 if (parts.size() > 0)
   ret = parts[0].Trim();

 return ret;
}

void FolderItem::Drop()
{
 // physical directory not removed
 m_ParentBase->RemoveFolder(m_ID);
}

DateTime FolderItem::LastActivity()
{
 MySqlDataReader *rs;
 DateTime dt;

 MySqlCommand cmd(L"SELECT MAX(picture_added) FROM picture WHERE folder_id=@folder;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@folder", m_ID));
 rs = cmd.ExecuteReader();
 if (rs->Read() == true)
   dt = rs->GetDateTime(0);
 rs->Close();
 cmd.Dispose();

 return dt;
}

///////////////////////////////////////////////////////////////////////////

BaseItem::BaseItem(int nID, String const &sDirName)
{
 m_ID = nID;
 m_DirName = sDirName;
 m_DirPath = L"";
 m_DirType = DirectoryType::SingleLevel;
}

std::map<int, BaseItem *> BaseItem::LoadBases()
{
 std::map<int, BaseItem *> bases;
 MySqlDataReader *rs;
 BaseItem *item;
 String gs, s;

 MySqlCommand cmd(L"SELECT dir_id, dir FROM directory;", DB->Con());
 rs = cmd.ExecuteReader();
 while(rs->Read() == true)
  {
   item = new BaseItem(rs->GetInt32(0), rs->GetString(1));
    gs=L"Base Item ";
    gs+= App->ComputerName(); // add computer name because location of dir will be different
    gs+=L" - ";
    gs+=String::Decimal(item->ID());
   s = App->Options.GetSetting(gs);
   item->m_DirPath = s;
   bases.insert(std::pair<int, BaseItem *>(item->m_ID, item));
  }
 rs->Close();
 cmd.Dispose();

 return bases;
}

BaseItem *BaseItem::SaveNew(String const &sName)
{
 MySqlDataReader *rs;
 BaseItem *item;

 MySqlCommand cmd(L"INSERT INTO directory (dir) VALUES (@name);", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@name", sName));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 MySqlCommand cmd2(L"SELECT LAST_INSERT_ID();", DB->Con());
 rs = cmd2.ExecuteReader();
 rs->Read();
 item = new BaseItem(rs->GetInt32(0), sName);
 rs->Close();
 cmd2.Dispose();

 return item;
}

void BaseItem::Save(String const &sName, String const &sPath)
{
 String ss;

 MySqlCommand cmd(L"UPDATE directory SET dir=@name WHERE dir_id=@id;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@name", sName));
 cmd.Parameters.Add(new MySqlParameter(L"@id", m_ID));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 m_DirName = sName;

 ss=L"Base Item ";
 ss+=String::Decimal(m_ID);

 App->Options.SaveSetting(ss, sPath);
 m_DirPath = sPath;
}

bool BaseItem::GetDir()
{
 String s, ss;

 if (m_DirPath.Length() == 0) // || System.IO.Directory.Exists(s) == false) 
  {
   s = BrowsePath();
   if (s.Length()==0)
     return false;
   else
    {
     m_DirPath=s;
     ss=L"Base Item ";
     ss+= App->ComputerName();  // add computer name because paths to folders will be different
     ss+=L" - ";
     ss+=String::Decimal(m_ID);
     App->Options.SaveSetting(ss, s);
    }
  }
 else
  {
   if (::PathFileExists(m_DirPath.Chars())==false)
    {
     ss=L"Path '";
     ss+=m_DirPath;
     ss+=L"' not found. Do you want a chance to change it? If you can make the path available click cancel and do so now.";
     if (App->Question(ss, MB_OKCANCEL)==DialogResult::OK)
      {
       s=BrowsePath();
       if (s.Length()>0)
        {
         m_DirPath=s;
         ss=L"Base Item ";
         ss+= App->ComputerName();
         ss+=L" - ";
         ss+=String::Decimal(m_ID);
         App->Options.SaveSetting(ss, s);         
        }
      }
     else 
      {
       if (::PathFileExists(m_DirPath.Chars())==false)
        {
         ss=L"Warning. Path '";
         ss+=m_DirPath; 
         ss+=L"' is still not available.";
         App->Response(ss);
         return false;
        }
      }
    }
  }
 return true;
}

String BaseItem::BrowsePath()
{
 String s, sm;

 sm=L"Select Path For \"";
 sm += m_DirName;
 sm += L"\"";
 s = OpenFileDlg::ChooseFolder(Wnd->Handle(), sm);
 return s;
}

void BaseItem::Load()
{
 FolderItem *folder;
 MySqlDataReader *rs;
 DateTime dtOpen;

 ClearFolders();

 MySqlCommand cmd(L"SELECT folder_id, folder_name, folder_added, folder_opened FROM folder WHERE dir_id=@base;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@base", m_ID));
 rs = cmd.ExecuteReader();
 while(rs->Read() == true)
  {
   if (rs->IsDBNull(3) == false) 
     dtOpen = rs->GetDateTime(3); 
   else 
     dtOpen = DateTime::MinValue();
   folder = new FolderItem(rs->GetInt32(0), rs->GetString(1), this, rs->GetDateTime(2), dtOpen);
   Folders.insert(std::pair<int, FolderItem *>(folder->ID(), folder));
  }
 rs->Close();
 cmd.Dispose();

 for(const auto &it : Folders)
  {
   it.second->LoadInfo();
  }
}

void BaseItem::RemoveFolder(int id)
{
 String q;

 if (Folders.count(id) == 0) throw L"Folder not in map";

 q =  L"DELETE FROM hashtag_folder WHERE folder_id=@id; ";
 q += L"DELETE From folder WHERE folder_id=@id; ";
 q += L"DELETE From folder_site WHERE folder_id=@id; ";
 q += L"DELETE FROM picture_folder_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@id); ";
 q += L"DELETE FROM picture_global_hashtag WHERE picture_id IN (SELECT picture_id FROM picture WHERE folder_id=@id); ";
 q += L"DELETE FROM picture WHERE folder_id=@id; ";
 q += L"DELETE FROM folder_site WHERE folder_id=@id; ";

 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@id", id));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 delete Folders.at(id);
 Folders.erase(id);
}

void BaseItem::DBIntegrity()
{
 MySqlCommand cmd1(L"DELETE FROM picture_folder_hashtag WHERE picture_id NOT IN (SELECT picture_id FROM picture);", DB->Con());
 cmd1.ExecuteNonQuery();
 cmd1.Dispose();

 MySqlCommand cmd2(L"DELETE FROM picture_global_hashtag WHERE picture_id NOT IN (SELECT picture_id FROM picture);", DB->Con());
 cmd2.ExecuteNonQuery();
 cmd2.Dispose();
}

void BaseItem::ClearFolders()
{
 for (const auto &it : Folders)
  {
   it.second->Close();
   delete it.second;
  }
}