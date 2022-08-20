#pragma once

class PictureItem;
class FolderItem;

class HashTag 
{
 public:

 // global hashtags exist in App->GlobalTags
 // folder hashtags exist under m_Folder->FolderTags
 // picture hashtags exist under a picture item, they share the id of the foldertag that describes them

 enum class HashTagType : int { None, GlobalTag, FolderTag, PictureTag };
 
 HashTag();
 HashTag(int id, String const &n, HashTagType t);
 HashTag(int id, String const &n, FolderItem *, HashTagType t);
~HashTag();

 bool operator == (const HashTag &c2) const; 
 bool operator != (const HashTag &c2) const;
 bool Equals(HashTag const &ht) const;

 bool Selected;
 bool ShowSelected;

 inline int ID() const { return m_ID; }
 inline String Name() const { return m_Name; }
 inline HashTagType TagType() const { return m_TagType; }
 inline bool InUse() const { return m_InUse; }
 inline FolderItem *Folder() const { return m_Folder; }

 static std::map<int, HashTag>LoadGlobalHashTags();
 static bool Match(HashTag &p1, HashTag &p2);
 static int Compare(HashTag const &h1, HashTag const &h2);
 static void Merge(std::map<int, HashTag> &target, std::map<int, HashTag> &source);

 void SetName(String const &s);
 String ToString() const;
 void SetInUse(bool b);
 
 bool SearchPictureHashTags(HashTag const &fht) const;
 void RemoveSubHashTags(PictureItem *item, HashTag const &parent) const;

 void InsertSubTags(PictureItem *p);

 std::map<int, HashTag> SubTags;

 protected:
 int m_ID;
 String m_Name;
 HashTagType m_TagType;

 FolderItem *m_Folder;
 HashTag    *m_FolderTag; // pointer to a picture hashtag's Folder Hash Tag
 
 bool m_InUse;

};

class HashTagList
{
 public:

 protected:

 void Remove(HashTag  const &ht)
  {
   std::vector<HashTag> final;
   for(const auto &lht : List)
    {
     if (lht.ID() != ht.ID())
       final.push_back(lht);
    }
   List=final;
  }

 int Find(HashTag const &ht)
  {
   int i;
   
   i = 0;
   for(const auto &lht : List)
    {
     if (lht.ID() == ht.ID())
       return i;
     i++;
    }
   return -1;
  }

 inline void Clear() { List.clear(); }
 inline void Add(HashTag const &ht) { List.push_back(ht); }
 inline bool Contains(HashTag const &pht) { return (Find(pht) >=0); }

 protected:

 std::vector<HashTag>List;

};



struct 
 {
  bool operator()(HashTag const &a, HashTag const &b) const 
   { 
    int k = HashTag::Compare(a, b);
    return k < 0;
   }
 } HashTagCustomSort;

