#include "pch.h"
#include "App.h"

HashTagSelectCtrl::HashTagSelectCtrl()
{
 ControlMode=enumCtrlStyle::FilterCtrl;
 SelectStyle=enumSelectStyle::FolderTags;
 AutoSize=true;
 EditHashTag = nullptr;
}

HashTagSelectCtrl::~HashTagSelectCtrl()
{
 if (m_Format != nullptr)    delete m_Format;
 if (m_Pen != nullptr)       delete m_Pen;
 if (m_BG != nullptr)        delete m_BG;
 if (m_Black != nullptr)     delete m_Black;
 if (m_Lime != nullptr)      delete m_Lime;
 if (m_Green != nullptr)     delete m_Green;
 if (m_Turquoise != nullptr) delete m_Turquoise;
 if (m_Yellow != nullptr)    delete m_Yellow;
 if (m_Font != nullptr)      delete m_Font;  
}

void HashTagSelectCtrl::Create(AWnd *parent, enumCtrlStyle eCtrl, enumSelectStyle eSelectStyle)
{
 PanelWnd::Create(parent, Rect(0,0,0,0));
 Init();
 ControlMode=eCtrl;
 SelectStyle=eSelectStyle;
}

void HashTagSelectCtrl::Create(AWnd *parent, enumFilterStyle eFilterStyle)
{
 PanelWnd::Create(parent, Rect(0,0,0,0));
 Init();
 ControlMode = enumCtrlStyle::FilterCtrl;
 FilterStyle = eFilterStyle;
 ShowFilter = true;
}

void HashTagSelectCtrl::Init()
{
 MENUITEMINFO mi={0};

 m_Resizing = false;

 m_BG = new SolidBrush(Color(255, 255, 255, 255));
 m_Black = new SolidBrush(Color(255, 0, 0, 0));
 m_Lime = new SolidBrush(Color(255, 20, 255, 20));
 m_Green = new SolidBrush(Color(255, 0, 225, 0));
 m_Turquoise = new SolidBrush(Color(255, 0xAF, 0xEE, 0xEE)); 
 m_Yellow = new SolidBrush(Color(255, 255, 255, 0));

 m_Pen = new Pen(Color::Black, 1);

 m_Font = new Gdiplus::Font(L"Arial", 12, FontStyle::FontStyleRegular, Unit::UnitPixel,0);

 m_Format = new Gdiplus::StringFormat();
 m_Format->SetAlignment(StringAlignment::StringAlignmentNear);
 m_Format->SetFormatFlags(StringFormatFlagsNoWrap);
 m_Format->SetLineAlignment(StringAlignmentNear);

 Border(true);
}

void HashTagSelectCtrl::OnPaint(HDC hDC)
{
 std::vector<HashTag> listHT;
 String str;
 Rect rct;
 RectF brct, box;
 Brush *br;
 Color bg; 
 Size sz;
 INT chars, lines;
 int x,y,w,h;

 w = ClientSize().Width;
 h = ClientSize().Height;

 Bitmap *buffer = new Bitmap(w, h, PixelFormat24bppRGB); 
 Gdiplus::Graphics g(buffer);

 m_BG->GetColor(&bg);
 g.Clear(bg);

 DrawBorder(hDC);

 HashTagList.clear();
 if (h == 0) return;
 
 if ((ControlMode == enumCtrlStyle::SelectCtrl && SelectStyle != enumSelectStyle::FolderTags) || (ControlMode == enumCtrlStyle::FilterCtrl && FilterStyle != enumFilterStyle::FilterFolder))
  { 
   for (const auto &h : GlobalMap)
     listHT.push_back(h.second);
  }
 if ((ControlMode == enumCtrlStyle::FolderCtrl && SelectStyle != enumSelectStyle::GlobalTags) || (ControlMode == enumCtrlStyle::SelectCtrl && SelectStyle != enumSelectStyle::GlobalTags) || (ControlMode == enumCtrlStyle::FilterCtrl && FilterStyle != enumFilterStyle::FilterGlobal))
  {
   for(const auto &h : FolderMap)
     listHT.push_back(h.second);
  }
 if (ControlMode == enumCtrlStyle::PictureCtrl)
  {
   if (SelectStyle != enumSelectStyle::FolderTags) 
    {
     for (const auto &h : GlobalMap)
       listHT.push_back(h.second);
    }
   for(const auto &h : PictureMap)
     listHT.push_back(h.second);
  }
 if (ControlMode == enumCtrlStyle::DisplayCtrl) 
  {
   for (const auto &h : GlobalMap)
     listHT.push_back(h.second);
   for(const auto &h : PictureMap)
     listHT.push_back(h.second);
  }

 std::sort(listHT.begin(), listHT.end(), HashTagCustomSort);

 x = 2;
 y = 2;
 brct = RectF(0,0,w,h);

 for(auto &ht : listHT)
  {
   ht.ShowSelected = true;
   str = ht.ToString();
   
   g.MeasureString(str.Chars(), (INT)str.Length(), m_Font, brct, m_Format, &box,  &chars, &lines);

   if ((x + (int)box.Width + HorzGap) >= (w - (HorzGap / 2))) 
    {
     x = 2;
     y += (int)box.Height + VertGap; // have to draw this one on the next line
    }
   rct = Rect(x, y, (int)box.Width + (int)(HorzGap / 2), (int)box.Height + (int)(VertGap / 2));
   br = GetHashTagBrush(ht);
   g.FillRectangle(br, rct);
   g.DrawString(str.Chars(), str.Length(), m_Font, PointF(rct.X, rct.Y), m_Black);
   HashTagList.push_back(HashTagRect(ht, rct));
   x += (int)box.Width + HorzGap;
  }
 
 HBITMAP hBmp;
 buffer->GetHBITMAP(Color::White, &hBmp);

 HDC hMem = ::CreateCompatibleDC(hDC);
 SelectBitmap(hMem, hBmp);
 BitBlt(hDC, 0, 0, w, h, hMem, 0, 0, SRCCOPY);
 ::DeleteDC(hMem);
 ::DeleteObject(hBmp); 

 delete buffer;
}
 
void HashTagSelectCtrl::ProcessTags(int nParentWidth)
{
 HDC hDC;
 String str;
 std::vector<HashTag> htlist;
 INT chars, lines;
 RectF box, brct;
 int x, y, w, h;

 if ((ControlMode == enumCtrlStyle::SelectCtrl && SelectStyle != enumSelectStyle::FolderTags) || (ControlMode == enumCtrlStyle::FilterCtrl && FilterStyle != enumFilterStyle::FilterFolder))
  {
   for(const auto & h : GlobalMap)
     htlist.push_back(h.second);
  }
 if ((ControlMode == enumCtrlStyle::SelectCtrl && SelectStyle != enumSelectStyle::GlobalTags) || (ControlMode == enumCtrlStyle::FilterCtrl && FilterStyle != enumFilterStyle::FilterGlobal))
  {
   for(const auto & h : FolderMap)
     htlist.push_back(h.second);
  }
 if (ControlMode == enumCtrlStyle::FolderCtrl && SelectStyle != enumSelectStyle::GlobalTags)
  {
   for(const auto & h : FolderMap)
     htlist.push_back(h.second);
  }

 if (ControlMode == enumCtrlStyle::PictureCtrl)
  {
   if (SelectStyle != enumSelectStyle::FolderTags) 
    {
     for(const auto & h : GlobalMap)
       htlist.push_back(h.second);
    }
   for(const auto &h : PictureMap)
     htlist.push_back(h.second);
 }
if (ControlMode == enumCtrlStyle::DisplayCtrl)
 {
  for(const auto & h : GlobalMap)
    htlist.push_back(h.second);
  for(const auto &h : PictureMap)
    htlist.push_back(h.second);
 }

 std::sort(htlist.begin(), htlist.end(), HashTagCustomSort);

 if (htlist.size() == 0)
  {
   m_HasTags = false;
   m_HashTagHeight = 0;
   ::InvalidateRect(m_hWnd, nullptr, false);
   return;  
  }

 m_HasTags = true;

 hDC = ::CreateCompatibleDC(0);
 Gdiplus::Graphics g(hDC);
 
 x = 2;
 y = 2;
 w = m_Parent->ClientSize().Width; // need wide enough area to provide correct MeasureString
 h = m_Parent->ClientSize().Height;

 brct = RectF(0, 0, w, h);

 for(const auto &htl : htlist)
  {
   str = htl.ToString();

   g.MeasureString(str.Chars(), (INT)str.Length(), m_Font, brct, m_Format, &box,  &chars, &lines);

   if ((x + box.Width + HorzGap) >= (nParentWidth - (HorzGap / 2))) 
    {
     x = 2;
     y += box.Height + VertGap; // have to draw this one on the next line
    }
   x += box.Width + HorzGap;
  }

 ::DeleteDC(hDC);

 m_HashTagHeight = y + box.Height + VertGap * 2; // sz was set in the above foreach loop, wouldn't have been called if htlist was empty

 Refresh();
}
 
void HashTagSelectCtrl::OnSize() // Handles Me.Resize
{
 if (AutoSize == false)
   return;

 if (m_Resizing == true) return;

 m_Resizing = true;

 ProcessTags(ClientSize().Width);
 SetSize( Size(ClientSize().Width, m_HashTagHeight));
 
 m_Resizing = false;
}
 
WMR HashTagSelectCtrl::OnContextMenu(HWND hChild, Point const &pt)
{ 
 if (hChild == Handle())
  {
   ShowContextMenu(pt);
   return WMR::One;
  }
 return WMR::Zero;
}

void HashTagSelectCtrl::ShowContextMenu(Point const &pt)
{
 MENUITEMINFO mi={0};
 String txt;
 HMENU hMenu;
 UINT flags;

 if (EditHashTag == nullptr)
   return;

 if (EditHashTag->TagType() == HashTag::HashTagType::None)
   return;

 hMenu = ::CreatePopupMenu();

 mi.cbSize=sizeof(MENUITEMINFO);
 mi.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;

 txt = L"Edit \""; 
 txt += EditHashTag->Name();
 txt += L"\"";

 mi.wID = HTSCEditId;
 mi.fState = MFS_ENABLED;  // rest of the items are enabled
 mi.dwTypeData = (wchar_t *)txt.Chars(); 
 ::InsertMenuItem(hMenu, 1, true, &mi); 

 txt = L"Copy \""; 
 txt += EditHashTag->Name();
 txt += L"\" Name";

 mi.wID = HTSCCopyId;
 mi.dwTypeData = (wchar_t *)txt.Chars();
 ::InsertMenuItem(hMenu, 2, true, &mi); 

 txt = L"Drop HashTag \""; 
 txt += EditHashTag->Name();
 txt += L"\"";

 mi.wID = HTSCDeleteId;
 mi.dwTypeData = (wchar_t *)txt.Chars();
 ::InsertMenuItem(hMenu, 3, true, &mi); 

 flags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NOANIMATION;

 TrackPopupMenu(hMenu, flags, pt.X, pt.Y, 0, m_hWnd, nullptr);

 DestroyMenu(hMenu);
}


void HashTagSelectCtrl::OnMouseDown(MouseEventArgs const  &e) // Handles Me.MouseDown
{
 HashTag *ht=nullptr;


 for(const auto &htr: HashTagList)
  {
   if (ContainsPoint(htr.R, Point(e.X, e.Y)) == true) 
    {
     switch(htr.HT.TagType())
      {
       case HashTag::HashTagType::GlobalTag:  ht=&GlobalMap[htr.HT.ID()]; break;
       case HashTag::HashTagType::FolderTag:  ht=&FolderMap[htr.HT.ID()]; break;
       case HashTag::HashTagType::PictureTag: ht=&PictureMap[htr.HT.ID()]; break;
       default: throw L"Unhandled case";
      }
     EditHashTag = ht;
     if (ControlMode == enumCtrlStyle::FilterCtrl)
      {
       if (e.Left == true) 
        {
         ht->Selected = !ht->Selected;
         ::InvalidateRect(m_hWnd, nullptr, false);
         ::SendMessage(m_Parent->Handle(), WM_HTSC_FILTER_CHANGED, (WPARAM) m_hWnd, 0);
         }
        return;
       }
      else
       {
        if (e.Left == true)
         {
          SelectedHashTag = ht;
          ::SendMessage(m_Parent->Handle(), WM_HTSC_ITEMCLICK, (WPARAM)m_hWnd, (LPARAM)ht);
         }
        else
         {
          if (e.Right == true) 
           {
            SelectedHashTag = ht;
            ::SendMessage(m_Parent->Handle(), WM_HTSC_ITEMCONTEXT, (WPARAM)m_hWnd, (LPARAM)ht);
           }
        }
       return;
      }
    }
  }
}

bool HashTagSelectCtrl::ContainsPoint(Rect const &rct, Point const &pt)
{
 if (pt.X >= rct.X && pt.X <= rct.X + rct.Width && pt.Y >= rct.Y && pt.Y <= rct.Y + rct.Height)
   return true;
 else
   return false;
}

HBRUSH hLime;
HBRUSH hTurquoise;
HBRUSH hYellow;

SolidBrush *HashTagSelectCtrl::GetHashTagBrush(HashTag const &ht)
{
 SolidBrush *br=nullptr;
  
 if (ControlMode == enumCtrlStyle::DisplayCtrl) 
  {
   if (ht.TagType() == HashTag::HashTagType::GlobalTag) 
     br = m_Lime;
   else
     br = m_Turquoise;
   return br;
  }

 if (ControlMode == enumCtrlStyle::SelectCtrl)
  {
   if (ht.TagType() == HashTag::HashTagType::GlobalTag)
     br = m_Lime;
   else
    {
     if (ShowIfNotUsed == true)
      {
       if (ht.InUse() == true)
         br = m_Turquoise;
       else
         br = m_Yellow;
       }
      else
        br = m_Turquoise;
     }
    return br;
   }

 if (ht.Selected == true && ShowFilter == true) 
  {
   if (FilterStyle != enumFilterStyle::FilterFolder && ht.TagType() == HashTag::HashTagType::GlobalTag)
     br = m_Green;
   else
     br = m_Lime; 
  }
 else
  {
   if (ShowIfNotUsed == true)
    {
     if (ht.InUse() == true)
       br = m_Turquoise;
     else
       br = m_Yellow;
     }
   else
     br = m_Turquoise;
   if (ht.TagType() == HashTag::HashTagType::GlobalTag) // this might overwrite br set above...
     br = m_Lime;
   else
     br = m_Turquoise;
  }
 return br;
}

bool HashTagSelectCtrl::ShowItem(PictureItem *p)
{
 int selectCount, matchCount;
 HashTag ht;

 if (ControlMode == enumCtrlStyle::SelectCtrl) throw L"this is supposed to called for filter function";

 selectCount = 0; // if no tags are selected then show 
 matchCount = 0;

 if (HashTagList.size() == 0) return false; // might need to change this to true because there is no list to limit by

 if (FilterStyle != enumFilterStyle::FilterGlobal)
  {
   for(const auto &htl : HashTagList)
    {
     switch(htl.HT.TagType())
      {
       case HashTag::HashTagType::GlobalTag: ht = GlobalMap[htl.HT.ID()]; break;
       case HashTag::HashTagType::FolderTag: ht = FolderMap[htl.HT.ID()]; break;
       case HashTag::HashTagType::PictureTag: ht = PictureMap[htl.HT.ID()]; break;
      }
     if (ht.Selected == true && ht.TagType() == HashTag::HashTagType::FolderTag)
      {
       selectCount += 1;
       if (p->SearchPictureTags(ht.ID()) == true) 
         matchCount += 1;
      }
    }
  }

 if (FilterStyle != enumFilterStyle::FilterFolder) 
  {
   for(const auto &htl : HashTagList)
    {
     switch(htl.HT.TagType())
      {
       case HashTag::HashTagType::GlobalTag: ht = GlobalMap[htl.HT.ID()]; break;
       case HashTag::HashTagType::FolderTag: ht = FolderMap[htl.HT.ID()]; break;
       case HashTag::HashTagType::PictureTag: ht = PictureMap[htl.HT.ID()]; break;
      }
     if (ht.Selected == true && ht.TagType() == HashTag::HashTagType::GlobalTag)
      {
       selectCount += 1;
       if (p->GlobalHashTags.count(ht.ID())>0)
         matchCount += 1;
      }
    }
  }

 return selectCount == matchCount;
}

void HashTagSelectCtrl::OnMenuEdit() // Handles mnuEdit.Click
{
 HashTagEditDlg dlg;

 if (EditHashTag->TagType() == HashTag::HashTagType::None)
  {
   App->Response(L"Hashtag not selected for edit");
   return;
  }

 if (EditHashTag->TagType() == HashTag::HashTagType::GlobalTag && String::Compare(EditHashTag->Name(), L"Favorite")==0)
  {
   App->Response(L"The global Favorite hashtag can't be changed, add a new global tag.");
   return;
 }

 if (dlg.Show(this) == DialogResult::OK)
  {
   ::InvalidateRect(m_hWnd, nullptr, false);
   ::SendMessage(m_Parent->Handle(), WM_HTSC_NAME_CHANGED, (WPARAM)m_hWnd, 0);
  }
}

void HashTagSelectCtrl::OnMenuCopy() // Handles mnuCopy.Click
{
 if (EditHashTag->TagType() != HashTag::HashTagType::None)
  {
   App->SetClipboardText(EditHashTag->Name()); 
  }
}

void HashTagSelectCtrl::OnMenuDelete() // Handles mnuDelete.Click
{
 std::vector<HashTag> delList;
 String msg;

 if (EditHashTag->TagType() == HashTag::HashTagType::None)
  {
   App->Response(L"Hashtag not selected for delete");
   return;
  }

 if (EditHashTag->TagType() == HashTag::HashTagType::GlobalTag && String::Compare(EditHashTag->Name(),L"Favorite")==0) 
  {
   App->Response(L"The global Favorite hashtag can't be deleted.");
   return;
  }

 switch(EditHashTag->TagType())
  {
   case HashTag::HashTagType::FolderTag:
    {
     if (EditHashTag->Folder()->FolderHashTagInUse(*EditHashTag) == true)
      {
       msg = L"Folder hashtag ";
       msg += EditHashTag->ToString();
       msg += L" is being used. Delete Cancelled.";
       App->Response(msg);
       return;
      }
     msg = L"Delete hashtag \"";
     msg += EditHashTag->Name();
     msg += L"\" of folder ";
     msg += EditHashTag->Folder()->Folder();
     msg += L"?";
     if (App->Question(msg, MB_OKCANCEL) != DialogResult::OK)
       return;
     for(const auto &h : PictureMap)
      {
       if (h.second.ID() == EditHashTag->ID()) 
         delList.push_back(h.second);
      }
     for(const auto &dl : delList)
      {
       if (PictureMap.count(dl.ID())>0)
         PictureMap.erase(dl.ID());
      }
     EditHashTag->Folder()->DropHashTag(EditHashTag);  // fht is deconstructed
     RemoveFolderHashTag(EditHashTag);                 // have remove last because EditHashTag points to this
     Refresh();
     } break;
   case HashTag::HashTagType::GlobalTag:
    {
     msg=L"Delete global hashtag ";
     msg += EditHashTag->Name();
     msg += L" ?";
     if (App->Question(msg, MB_OKCANCEL) != DialogResult::OK)
       return;
     RemoveGlobalHashTag(EditHashTag);
     App->DropGlobalHashTag(*EditHashTag); // ght is deconstructed
     Refresh();
    } break;
    default: throw L"unhandled tag type";
   }

 ::SendMessage(m_Parent->Handle(), WM_HTSC_LIST_CHANGED, (WPARAM)m_hWnd, 0);
  ProcessTags(ClientSize().Width);
}

void HashTagSelectCtrl::RemoveFolderHashTag(HashTag *fht)
{
 if (FolderMap.count(fht->ID()) > 0)
   FolderMap.erase(fht->ID());
}

void HashTagSelectCtrl::RemovePictureHashTag(HashTag *pht)
{
 if (PictureMap.count(pht->ID())>0)
   PictureMap.erase(pht->ID());
}

void HashTagSelectCtrl::RemoveGlobalHashTag(HashTag *ght)
{
 if (GlobalMap.count(ght->ID())>0)
   GlobalMap.erase(ght->ID());
}

WMR HashTagSelectCtrl::OnCommand(int child, HWND hWnd)
{
 switch(child)
  {
   case HTSCEditId:   OnMenuEdit(); break;
   case HTSCDeleteId: OnMenuDelete(); break;
   case HTSCCopyId:   OnMenuCopy(); break;
  }
 return WMR::One;
}

void HashTagSelectCtrl::SetSize(Size const &s)
{
 if  (m_HasTags == true && s.Height == 0)
   throw L"strange";

// if (s.Height < 10) throw L"Strange";
// if (s.Width < 10) throw L"Strange";

// if (s.Height > 1000) throw L"Strange";
// if (s.Width > 1920) throw L"Strange";

 AWnd::SetSize(s);
}

void HashTagSelectCtrl::SetRect(Rect const &r)
{
// if (m_HasTags == true && r.Height==0)
//   throw L"strange";

// if (r.X < 0) throw 1;
// if (r.Y < 0) throw 1;
// if (r.X > 1000) throw 1;
// if (r.Y > 1200) throw 1;

// if (r.Width < 10) throw L"Strange";


 AWnd::SetRect(r);
}