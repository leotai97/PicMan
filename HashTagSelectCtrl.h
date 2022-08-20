#pragma once

class HashTagRect
{
 public:

 HashTagRect()
  {
   HT = HashTag(); // tag type none
  }

 HashTagRect(HashTag const &ht, Rect const &r)
 {
  HT = ht;
  R = r;
 }

 HashTag  HT;
 Rect     R;
};

#define HTSCCurrentId  101
#define HTSCEditId     102
#define HTSCDeleteId   103
#define HTSCCopyId     104

class HashTagSelectCtrl : public PanelWnd
{
 public:
 enum class enumCtrlStyle : int { 
    SelectCtrl,  // choose hashtags
    FilterCtrl,  //hi-light various hashtags
    FolderCtrl,  //selection of folderhashtags
    PictureCtrl, // build up sets of picturehashtags
    DisplayCtrl, // show hashtags
   };
 enum class enumSelectStyle : int { GlobalTags, FolderTags, GlobalFolderTags };
 enum class enumFilterStyle : int { FilterGlobal, FilterFolder, FilterGlobalFolder, NoFilter };
 
 HashTagSelectCtrl();
~HashTagSelectCtrl();

 void Create(AWnd *parent, enumCtrlStyle eCtrl, enumSelectStyle eSelectStyle);
 void Create(AWnd *parent, enumFilterStyle eFilterStyle);

 void ProcessTags(int nParentWidth);
 bool ShowItem(PictureItem *p);

 inline int GetHashTagHeight() { return m_HashTagHeight; }

 inline void AddGlobalTag(HashTag const &ht) { GlobalMap.insert(std::pair<int, HashTag>( ht.ID(), ht)); }
 inline void AddFolderTag(HashTag const &ht) { FolderMap.insert(std::pair<int, HashTag>( ht.ID(), ht)); }
 inline void AddPictureTag(HashTag const &ht) { PictureMap.insert(std::pair<int, HashTag>( ht.ID(), ht)); }

 std::map<int, HashTag>  GlobalMap;
 std::map<int, HashTag>  FolderMap;
 std::map<int, HashTag> PictureMap;
 std::vector<HashTagRect>    HashTagList;

 HashTag *SelectedHashTag;
 HashTag *EditHashTag;

 virtual void SetSize(Size const &sz);
 virtual void SetRect(Rect const &r);

 void RemoveFolderHashTag(HashTag *item);
 void RemovePictureHashTag(HashTag *item);
 void RemoveGlobalHashTag(HashTag *item);

 protected:

 void Init();

 void SortGlobalTags();
 void SortFolderTags();
 void SortPictureTags();

 virtual void OnPaint(HDC hDC);
 virtual WMR  OnCommand(int childID, HWND hWnd);
 virtual WMR  OnContextMenu(HWND hChild, Point const &pt);

 void ShowContextMenu(Point const &pt);

 void OnSize();

 virtual void OnMouseDown(MouseEventArgs const &m);

 bool ContainsPoint(Rect const &r, Point const &p);
 
 SolidBrush *GetHashTagBrush(HashTag const &ht);
 
 void OnMenuEdit(); // menu 
 void OnMenuCopy();
 void OnMenuDelete();

 enumSelectStyle SelectStyle;
 enumFilterStyle FilterStyle;
 enumCtrlStyle   ControlMode;

 public:

 bool ShowIfNotUsed;
 bool ShowFilter;
 bool AutoSize;

 protected:

 int m_HashTagHeight;
 
 Pen        *m_Pen;
 SolidBrush *m_BG;
 SolidBrush *m_Black;
 SolidBrush *m_Lime;
 SolidBrush *m_Green;
 SolidBrush *m_Turquoise;
 SolidBrush *m_Yellow;

 Gdiplus::StringFormat *m_Format;
 Gdiplus::Font  *m_Font;

 bool m_HasTags;
 bool m_Resizing;

 const int VertGap = 6;
 const int HorzGap = 8;

};

