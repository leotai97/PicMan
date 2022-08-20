#include "pch.h"
#include "app.h"

PictureItem::PictureItem(FolderItem *f, ImageParser *p)
{
 m_ParentFolder = f;
 m_Picture = p;
 p->InjectItem(this);
 m_FileName = p->FileName();
 m_Width = p->Width();
 m_Height = p->Height();

 SaveNew();

 MissingThumbnailRow = false;
}

PictureItem::PictureItem(int nID, String const &sFile, DateTime const &added, FolderItem *parent,int  w, int h, String const &subdir)
{
 String sFullPath;

 m_ID = nID;
 m_FileName = sFile;
 m_DateAdded = added;
 m_ParentFolder = parent;
 m_Width = w;
 m_Height = h;
 m_SubDir = subdir;

 sFullPath=m_ParentFolder->FolderPath();

 m_Picture = new ImageParser(sFullPath, m_SubDir, m_FileName, App->NextPictureID());

 MissingThumbnailRow=true;
}

PictureItem::~PictureItem()
{
 // nothing to dispose
}

void PictureItem::SetOriginal(Bitmap *img)
{
 m_Picture->SetItem(this, img);
}

bool PictureItem::HasGlobalHashTag(HashTag const &ht)
{
 return GlobalHashTags.count(ht.ID()) > 0;
}

bool PictureItem::HasPictureHashTag(HashTag const &ht)
{

 #ifdef _DEBUG
 if (ht.TagType() != HashTag::HashTagType::FolderTag) throw L"supposed to be a Folder Tag";
 #endif

 return PictureHashTags.count(ht.ID()) > 0;
}

bool PictureItem::SearchPictureTags(int id)
{
 HashTag fht;

 fht = m_ParentFolder->FolderHashTags[id];
 for(const auto &it : PictureHashTags)
  {
   if (it.second.SearchPictureHashTags(fht) == true) return true;
  }
 return false;
}

bool PictureItem::AddGlobalHashTag(HashTag const &ht)
{
 if (GlobalHashTags.count(ht.ID())>0) return false;

 GlobalHashTags.insert(std::pair<int, HashTag>(ht.ID(), ht));

 MySqlCommand cmd(L"INSERT INTO picture_global_hashtag (picture_id, hashtag_id) VALUES (@p, @h);", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@p", m_ID));
 cmd.Parameters.Add(new MySqlParameter(L"@h", ht.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 return true;
}

bool PictureItem::AddPictureHashTag(HashTag &ht)
{
 if (PictureHashTags.count(ht.ID()) == 0)
  {
   MySqlCommand cmd(L"INSERT INTO picture_folder_hashtag (picture_id, hashtag_id) VALUES (@p, @h);", DB->Con());
   cmd.Parameters.Add(new MySqlParameter(L"@p", m_ID));
   cmd.Parameters.Add(new MySqlParameter(L"@h", ht.ID()));
   cmd.ExecuteNonQuery();
   cmd.Dispose();
   PictureHashTags.insert(std::pair<int, HashTag>(ht.ID(), ht));
  }

 ht.InsertSubTags(this); // might be changes to sub tags

 return true;
}
 
bool PictureItem::RemoveGlobalHashTag(HashTag const &ht)
{
 if (GlobalHashTags.count(ht.ID()) == 0) return false;
 
 GlobalHashTags.erase(ht.ID());

 MySqlCommand cmd(L"DELETE FROM picture_global_hashtag WHERE picture_id=@p AND hashtag_id=@h;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@p", m_ID));
 cmd.Parameters.Add(new MySqlParameter(L"@h", ht.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 return true;
}
 
bool PictureItem::RemovePictureHashTag(HashTag const &ht)
{
 String q;

 if (PictureHashTags.count(ht.ID()) == 0) return false;

 for(const auto &sht : ht.SubTags)
  {
   sht.second.RemoveSubHashTags(this, ht);
  }
 
 PictureHashTags.erase(ht.ID());

 q = L"DELETE FROM picture_folder_hashtag WHERE picture_id=@p AND hashtag_id=@h;";
 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@p", m_ID));
 cmd.Parameters.Add(new MySqlParameter(L"@h", ht.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 return true;
}
 
void PictureItem::ReplacePictureHashTags(std::vector<HashTag> &tags)
{
 std::vector<HashTag> temp;

 for(const auto &it : PictureHashTags)
  {
   temp.push_back(it.second);
  }

 for(const auto &t : temp)
   RemovePictureHashTag(t); // necessary because it removes the row from the database

 PictureHashTags.clear();

 for (auto &tag : tags)
   AddPictureHashTag(tag);
}

void PictureItem::AddPictureHashTags(std::vector<HashTag> &tags)
{
 std::vector<HashTag> temp;

 for (auto &tag : tags)
  {
   if (PictureHashTags.count(tag.ID()) == 0)
     AddPictureHashTag(tag);
  }
}

void PictureItem::SetPictureHashTagString()
{
 std::vector<std::wstring> listTags;
 std::wstringstream sa;
 String result;
 bool first;

 for(const auto &it : PictureHashTags)
  {
   listTags.push_back(it.second.ToString().Chars());
  }

 std::sort(listTags.begin(), listTags.end());   

 sa.clear();
 first = true;
 for(const auto &lt : listTags)
  {
   if (first == false) 
     sa << L", ";
   else
     first = false;
   sa << lt;
  }
 result = sa.str();
 if (result.Length() == 0)
   result=L"zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"; 

 m_PictureHashTagString = result;
}

void PictureItem::UpdateFileName(int id, String const &newName)
{
 MySqlCommand cmd(L"UPDATE picture SET file_name=@fn WHERE picture_id=@id;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@fn", newName));
 cmd.Parameters.Add(new MySqlParameter(L"@id", id));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
}

void PictureItem::RemovePictureRowID(int id)
{
 std::vector<String> sql;

 sql.push_back(L"DELETE FROM picture WHERE picture_id=@id;");
 sql.push_back(L"DELETE FROM picture_thumbnail WHERE picture_id=@id;");
 sql.push_back(L"DELETE FROM picture_global_hashtag WHERE picture_id=@id;");
 sql.push_back(L"DELETE FROM picture_folder_hashtag WHERE picture_id=@id;");
 sql.push_back(L"DELETE FROM picture_folder_subtag WHERE picture_id=@id;");

 for (const auto &q : sql)
  {
   MySqlCommand cmd(q, DB->Con());
   cmd.Parameters.Add(new MySqlParameter(L"@id", id));
   cmd.ExecuteNonQuery();
   cmd.Dispose();
  }
}

void PictureItem::SaveNew()
{
 MySqlDataReader *rs;
 String q; 

 // "INSERT INTO picture(folder_id, file_name, picture_added, width, height) VALUES (@fid,@name, NOW(), @w, @h); SELECT LAST_INSERT_ID();";

 q = L"CALL picture_insert_proc(@fid, @name, @w, @h);";

 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@fid", m_ParentFolder->ID()));
 cmd.Parameters.Add(new MySqlParameter(L"@name", m_FileName));
 cmd.Parameters.Add(new MySqlParameter(L"@w", m_Width));
 cmd.Parameters.Add(new MySqlParameter(L"@h", m_Height));
 rs = cmd.ExecuteReader();
 rs->Read();
 m_ID = rs->GetInt32(0);
 rs->Close();
 cmd.Dispose();

 q=L"INSERT INTO picture_thumbnail (picture_id, thumbnail) VALUES (@pid, @thumb);";
 MySqlCommand cmd2(q, DB->Con());
 cmd2.Parameters.Add(new MySqlParameter(L"@pid", m_ID));
 cmd2.Parameters.Add(new MySqlParameter(L"@thumb", m_Picture->Original()));
 cmd2.ExecuteNonQuery();
 cmd2.Dispose();
}

void PictureItem::ChangeFile(ImageParser *p)
{
 String fn;
 int id;

 if (p->Item() == nullptr)
   return;

 fn=p->FileName();
 id=p->Item()->ID();
 p->Item()->SetFileName(fn);
 
 MySqlCommand cmd(L"UPDATE picture SET file_name=@name WHERE picture_id=@id", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@name", fn));
 cmd.Parameters.Add(new MySqlParameter(L"@id", id));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
} 

void PictureItem::Delete()
{
 RemovePictureRowID(m_ID);
}

void PictureItem::ReloadThumbnail()
{
 m_Picture->ReprocessImage();
 UpdateThumbnail();
}

void PictureItem::UpdateThumbnail()
{
 String q;

 m_Width=m_Picture->Width();
 m_Height=m_Picture->Height();
 
 q = L"UPDATE picture SET width=@w, height=@h WHERE picture_id=@p;";
 MySqlCommand cmd1(q, DB->Con());
 cmd1.Parameters.Add(new MySqlParameter(L"@p", m_ID));
 cmd1.Parameters.Add(new MySqlParameter(L"@w", m_Width));
 cmd1.Parameters.Add(new MySqlParameter(L"@h", m_Height));
 cmd1.ExecuteNonQuery();
 cmd1.Dispose();


 q = L"UPDATE picture_thumbnail SET thumbnail=@thumb WHERE picture_id=@p;";
 MySqlCommand cmd2(q, DB->Con());
 cmd2.Parameters.Add(new MySqlParameter(L"@p", m_ID));
 cmd2.Parameters.Add(new MySqlParameter(L"@thumb", m_Picture->Original()));
 cmd2.ExecuteNonQuery();
 cmd2.Dispose();
}

String PictureItem::FullName()
{
 String fn;

 fn=L"";
 if (m_SubDir.Length()>0)
  {
   fn=m_SubDir;
   fn += L"\\";
  }
 fn+=m_FileName;

 return fn;
}
