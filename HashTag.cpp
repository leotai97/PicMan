#include "pch.h"
#include "app.h"

HashTag::HashTag()
{
 m_ID = 0;
 m_Name.Clear();
 m_TagType = HashTagType::None;
 Selected = false;
 ShowSelected = false;
 m_Folder=nullptr;
 m_FolderTag=nullptr;
 m_InUse = false;
}

HashTag::HashTag(int id, String const &n, HashTagType t)
{
 m_ID=id;
 m_Name=n;
 m_TagType=t;
 Selected=false;
 ShowSelected=false;
 m_Folder=nullptr;
 m_FolderTag=nullptr;
 m_InUse=false;
}

HashTag::HashTag(int id, String const &n, FolderItem *f, HashTagType t) : HashTag(id, n, t)
{
 m_Folder=f;
 if (t == HashTag::HashTagType::PictureTag)
   m_FolderTag = &f->FolderHashTags[id];
}

HashTag::~HashTag()
{
 SubTags.clear();
}

bool HashTag::operator == (HashTag const &c2) const
{
 if (m_ID != c2.ID())
   return false;
 if (m_TagType != c2.TagType())
   return false;

 return true;
}

bool HashTag::operator != (HashTag const &c2) const
{
 if (m_ID == c2.ID() && m_TagType != c2.TagType())
   return false;
 return true;
}

void HashTag::SetName(String const &s)
{
 m_Name=s;
}

bool HashTag::Equals(HashTag const &ht) const
{
 if (m_ID == ht.ID()) return true;
 return false; 
}


std::map<int, HashTag> HashTag::LoadGlobalHashTags()
{
 std::map<int, HashTag> tags;
 MySqlDataReader *rs;
 String s;
 int i;
  
 MySqlCommand cmd = MySqlCommand(L"SELECT hashtag_id, name FROM hashtag_global;", DB->Con());
 rs = cmd.ExecuteReader();
 while(rs->Read() == true)
  {
   i = rs->GetInt32(0);
   s = rs->GetString(1);
   tags.insert(std::pair<int, HashTag>(i, HashTag(i, s, HashTag::HashTagType::GlobalTag)));
  }
 rs->Close();
 cmd.Dispose();

 return tags;
}

bool HashTag::Match(HashTag &p1, HashTag &p2)
{
 bool bFound;

 if (p1.ID() != p2.ID()) return false;
 if (p1.TagType() != p2.TagType()) return false;
 if (p1.SubTags.size() != p2.SubTags.size()) return false;

 for(const auto &i1 : p1.SubTags)
  {
   bFound = false;
   for(const auto &i2 : p2.SubTags)
    {
     if (i1.second.ID() == i2.second.ID()) bFound = true;
    }
   if (bFound == false) return false;
  }
 return true;
}

bool HashTag::SearchPictureHashTags(HashTag const &fht) const
{
 if (m_ID == fht.ID() ) return true; // picture HashTags have same ID as their Folder HashTag counterpart

 for(const auto &i :SubTags)
  {
   if (i.second.SearchPictureHashTags(fht) == true) return true;
  }
 return false;
}

void HashTag::RemoveSubHashTags(PictureItem *item, HashTag const &parent) const // recursively remove sub tags
{
 String q;

 for(const auto &ht : SubTags)
  {
   ht.second.RemoveSubHashTags(item, *this); 
  }

 q = L"DELETE FROM picture_folder_subtag WHERE picture_id=@p AND hashtag_id=@h AND sub_hashtag_id=@st;";
 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@p", item->ID()));
 cmd.Parameters.Add(new MySqlParameter(L"@h", parent.ID()));
 cmd.Parameters.Add(new MySqlParameter(L"@st", ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
}

void HashTag::SetInUse(bool b)
{
 m_InUse = b;

 if (m_TagType != HashTag::HashTagType::PictureTag)
   return;

 m_FolderTag->SetInUse(b);
 for (auto &it : SubTags)
  {
   it.second.SetInUse(b);
  }
}

String HashTag::ToString() const
{
 std::vector<HashTag> list;
 String s;

 if (SubTags.size() == 0) 
  return m_Name;

 s = L"";
 for(const auto &it : SubTags)
  {
   list.push_back(it.second);
  }

 std::sort(list.begin(), list.end(), HashTagCustomSort); 

 for(const auto &sl : list)
  {
   if (s.Length() == 0)
    { 
     s = Name();
     s += L"(";
    }
   else
    {
     s += L", ";
    }
   s += sl.ToString();  // iterative
  }
 s += L")";

 return s;
}

void HashTag::InsertSubTags(PictureItem *p)
{
 
 // do inserts recursive

 for(auto &it : SubTags)
  {
   MySqlCommand cmd(L"INSERT INTO picture_folder_subtag (picture_id, hashtag_id, sub_hashtag_id) VALUES (@p, @h, @s);", DB->Con());
   cmd.Parameters.Add(new MySqlParameter(L"@p", p->ID()));
   cmd.Parameters.Add(new MySqlParameter(L"@h", m_ID));
   cmd.Parameters.Add(new MySqlParameter(L"@s", it.second.ID()));
   cmd.ExecuteNonQuery(true); // ignore error, duplicates will be rejected due to index
   cmd.Dispose();
   it.second.InsertSubTags(p);
  }
}

int HashTag::Compare(HashTag const &a, HashTag const &b)
{
 if (a.TagType() != b.TagType()) 
  {
   if (a.TagType() == HashTag::HashTagType::GlobalTag) return -1;
   if (b.TagType() == HashTag::HashTagType::GlobalTag) return 1;
   if (a.TagType() == HashTag::HashTagType::FolderTag) return -1;
   if (b.TagType() == HashTag::HashTagType::FolderTag) return 1;
   throw L"???";
  }
 return String::Compare(a.Name(), b.Name());
}

void HashTag::Merge(std::map<int, HashTag> &target, std::map<int, HashTag> &src)
{
 std::vector<int> del;

 for (const auto &t : target)
  {
   if (src.count(t.first) == 0)
     del.push_back(t.first);      
  }
 for(const auto &d : del)
  {
   target.erase(d);
  }
 for(const auto &s : src)
  {
   if (target.count(s.second.ID()) == 0)
     target.insert(s);
  }
}