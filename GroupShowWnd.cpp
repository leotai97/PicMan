#include "pch.h"
#include "App.h"

void GroupShowWnd::Show(AWnd *parent, ImageList *il, std::vector<ImageParser *> const &images, HashTagSelectCtrl::enumFilterStyle htType)
{
 m_ItemsDeleted = false;
 m_ImageList = il;
 m_Original = images;
 m_ShowingFilter = false;  // start out not showing the filter
 m_FilterType = htType;
 ADialog::Show(IDD_BLANK, parent);
}

void GroupShowWnd::OnInitDialog()
{

 ADialog::OnInitDialog();

 m_TB.Create(this, 10);
 m_TB.AddItem(IDB_DETAILS, IDB_DETAILS, L"Switch List To Details");
 m_TB.AddItem(IDB_ICONS, IDB_ICONS, L"Switch List To Icons");
 m_TB.AddItem(IDB_EDIT, IDB_EDIT, L"Edit Picture");
 m_TB.AddItem(IDB_DELETE, IDB_DELETE, L"Move Selected Items To Recycle Bin (DEL)");
 m_TB.AddSeparator();
 m_TB.AddItem(IDB_SETTINGS,IDB_SETTINGS, L"Image Properites");
 m_TB.AddItem(IDB_HEART, IDB_HEART, L"Add Favorite HashTah (Ctrl+F)");
 m_TB.AddItem(IDB_HEARTBREAK, IDB_HEARTBREAK, L"Remove Favorite HashTah (Ctrl+U)");
 m_TB.AddItem(IDB_HASHTAG, IDB_HASHTAG, L"Show HashTag Filter");
 m_TB.AddItem(IDB_MAGNIFYSMALL, IDB_MAGNIFYSMALL, L"View Selected Picture");
 m_TB.AddItem(IDB_SHOWDUPS, IDB_SHOWDUPS, L"Select Duplicate Import Pictures");

 m_Split.Create(SPLIT_MAIN, this, SplitterWnd::SplitterOrientation::Vertical, 0.50);

 m_HTLV.Create(this, Rect(0,0,10,10), m_FilterType);
 m_HTLV.List.SetImageList(m_ImageList);
 m_HTLV.List.ViewStyle(ListView::DisplayStyle::LargeIcon); // after loading so icons are tiled correctly
 m_HTLV.List.SetOwner(this); // adds middle mouse picture viewing

 m_Pic.Create(this, Rect(20,0, 10, 10));

 m_Split.SetWindow1(&m_HTLV);
 m_Split.SetWindow2(&m_Pic); 

 m_SB.AddAutoPane(StatusBarPane::Content::Text);
 m_SB.Create(this);
 
 if (m_FilterType == HashTagSelectCtrl::enumFilterStyle::FilterGlobal)
   OnHashTag(); // show the filter at startup
 
 Reload(m_Original);
 m_HTLV.List.SetSort(ImageParser::SortChoices::FileName); 

 Maximize();
 OnSize();

}

bool GroupShowWnd::OnClose()
{
 if (m_HTLV.List.ItemsDeleted() == true)
   m_ItemsDeleted = true;

 return true;
}

void GroupShowWnd::Reload(std::vector<ImageParser *> const &list)
{

 m_HTLV.List.Clear();
 m_HTLV.List.ResetItemsDeleted();

 for(const auto &pxi : list)
  {
   if (pxi->Item() != nullptr)
     pxi->Item()->SetPictureHashTagString();
   m_HTLV.List.Insert(pxi);
  }
 m_HTLV.List.ResizeColumns();

 SetText(L"View All Pictures");

 m_HTLV.List.SetSort(ImageParser::SortChoices::FileName);
 m_Pic.SetItem(nullptr);
}

Rect GroupShowWnd::ClientRect()
{
 Size sz;
 RECT rct;
 Rect r;

 ::GetClientRect(m_hWnd, &rct);
 sz = Size(rct.right - rct.left, rct.bottom - rct.top);
 
 if (m_TB.Handle() == 0 || m_SB.Handle() == 0)
   r = ADialog::ClientRect();
 else
   r = Rect(0, m_TB.Height(), sz.Width, sz.Height - (m_TB.Height() + m_SB.Height()));

 return r;
}


void GroupShowWnd::OnSize()
{
 Size cs;
 Rect r;

 if (m_TB.Handle() == 0) return; // not created yet

 cs = ClientSize();

 r = Rect(0,0, cs.Width, m_TB.Height());
 m_TB.SetRect(r);
 
 r = Rect(0, m_TB.Height(), cs.Width, cs.Height-(m_TB.Height() + m_SB.Height()));

 m_Split.CalcSizes(r); 

 m_SB.OnSize(Rect(0,0,cs.Width, cs.Height));
 
}

WMR GroupShowWnd::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 NMHDR *nm=(LPNMHDR)lParam;
 WMR ret=WMR::Zero;
 int ndx, id;

 ret = ADialog::OnNotify(hChild, child, code, lParam);

 switch(code)
  {
   case NM_DBLCLK:
    {
     if (hChild == m_HTLV.List.Handle())
       OnViewPicture();
    } break;
   case LVN_ITEMCHANGING:
     ret = WMR::Zero;
     break;
   case LVN_ITEMCHANGED:
    {
     if (hChild == m_HTLV.List.Handle() && m_HTLV.List.NotifyIsOff()==false)
      {
       ndx = m_HTLV.List.GetSelectedItem();
       if (ndx >= 0)
        {
         id = m_HTLV.List.GetItemParam(ndx); 
         SetPictureViewer(App->Pictures[id]);
        }
      }
    } break;
  }
 return ret;
}

WMR GroupShowWnd::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 String msg;
 int id, ndx;

 switch(message)
  {
   case WM_HTSC_FILTER_CHANGED: OnFilterChanged(); break;
   case WM_PICLIST_HT_CHANGED: 
    {
     if (m_Pic.Item() != nullptr) m_Pic.RefreshHashTags();
    } break;
   case WM_PICLIST_ITEMDELETED:
     {
      msg = String::Decimal((int)lParam);
      msg += L" Images Moved To Recycle Bin";
      m_SB.SetText(0, msg);
      m_Pic.SetItem(nullptr);
     } break;
   case WM_LISTVIEW_MIDDLE:  // ListView middle mouse button clicked
    {
     ndx = (int)lParam;
     if ((HWND)wParam == m_HTLV.List.Handle())
      {
       id = m_HTLV.List.GetItemParam(ndx);
       OnListViewMiddle(id);
      }
    } break;
   default: return ADialog::MessageHandler(hWnd, message, wParam, lParam);
  }
 return WMR::Zero;
}


WMR GroupShowWnd::OnContextMenu(HWND hChild, Point const &pt)
{
 AMenu menu;

 menu.CreateContextMenu();

 PicListView::AddSortMenus(&menu, m_HTLV.List.CurrentSort);
 menu.ShowContextMenu(this, pt);
 menu.DestroyMenu();

 return WMR::One;
}

WMR GroupShowWnd::OnCommand(int btn, HWND hItem)
{
 switch(btn)
  {
   case IDB_SHOWDUPS: 
    {
     OnSelectDupImports();
    } break;
   case IDB_DETAILS:                 m_HTLV.List.ViewStyle(ListView::DisplayStyle::Details); break;
   case IDB_ICONS:                   m_HTLV.List.ViewStyle(ListView::DisplayStyle::LargeIcon); break;
   case IDB_EDIT:                    OnEdit(); break;
   case IDB_DELETE:                  OnDelete(); break;
   case IDB_SETTINGS:                OnProperties(); break;
   case IDB_HEART:                   OnFavorite(); break;
   case IDB_HEARTBREAK:              OnUnFavorite(); break;
   case IDB_HASHTAG:                 OnHashTag(); break;
   case IDB_MAGNIFYSMALL:            OnViewPicture(); break;
   case ID_SORT_BY_FILENAME:         m_HTLV.List.SetSort(ImageParser::SortChoices::FileName); break;
   case ID_SORT_BY_NAME_WITH_NUMBER: m_HTLV.List.SetSort(ImageParser::SortChoices::NameWithNumber); break;
   case ID_SORT_BY_WIDTH:            m_HTLV.List.SetSort(ImageParser::SortChoices::Width); break;
   case ID_SORT_BY_HEIGHT:           m_HTLV.List.SetSort(ImageParser::SortChoices::Height); break;
   case ID_SORT_BY_RATIO:            m_HTLV.List.SetSort(ImageParser::SortChoices::Ratio); break;
   case ID_SORT_BY_COLOR_RGB:        m_HTLV.List.SetSort(ImageParser::SortChoices::ColorRGB); break;
   case ID_SORT_BY_EDGE_AVERAGE:     m_HTLV.List.SetSort(ImageParser::SortChoices::EdgeAverage); break;
   case ID_SORT_BY_FILESIZE:         m_HTLV.List.SetSort(ImageParser::SortChoices::FileSize); break;
   case ID_SORT_BY_FILEDATE:         m_HTLV.List.SetSort(ImageParser::SortChoices::FileDate); break;
   case ID_SORT_BY_GLOBALTAGS:       m_HTLV.List.SetSort(ImageParser::SortChoices::GlobalHashTags); break;
   case ID_SORT_BY_FOLDERTAGS:       m_HTLV.List.SetSort(ImageParser::SortChoices::FolderHashTags); break;
  }
 return WMR::One;
}

void GroupShowWnd::OnEdit()
{
 ImageParser *px;
 String msg;

 if (m_HTLV.List.SelectedItemsCount() == 0) 
  { 
   App->Response(L"Picture not selected."); 
   return; 
  }
 px = m_HTLV.List.SelectedImage();

 PicManApp::EditPicture(px->FullPath());

 msg = L"Editing ";
 msg += px->FileName();
 m_SB.SetText(0, msg);

}

void GroupShowWnd::OnDelete()
{
 ImageParser *px;
 String q;
 String nl = L"\n";
 std::vector<int> indices;
 std::vector<int> delIndices;
 int i, id;

 if (m_HTLV.List.SelectedItemsCount() == 0)
  {
   App->Response(L"No items selected in the list to delete");
   return;
  }
 
 q = L"Move To Recycle Bin:";
 indices = m_HTLV.List.GetSelectedIndices();

 i=0;
 for(const auto &ndx : indices)
  {
   if (i>12)
    {
     q += nl;
     q += L"(List Cut Short, Too Many Items)";
     break;
    }
   q += nl;
   q += m_HTLV.List.GetItemText(ndx,0);
   i++;
  }

 if (App->Question(q, MB_OKCANCEL) != DialogResult::OK) return;

 for(const auto &ndx : indices)
  {
   id = m_HTLV.List.GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_HTLV.List item param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Delete() == false)
    {
     q = L"Delete cancelled, unable to delete ";
     q += px->FileName();
     q += L". Any items before it have removed";
     break;
    }
   else
    {
     px->Dispose();
     delete px;
     App->Pictures.erase(id);
     delIndices.push_back(ndx);
     if (m_ItemsDeleted == false) 
       m_ItemsDeleted=true;
    }
  }

 m_HTLV.List.RemoveItems(delIndices);

 q = String::Decimal((int)delIndices.size());
 q += L" Images Moved To Recycle Bin";

 m_SB.SetText(0, q);

 m_Pic.SetItem(nullptr);
}

void GroupShowWnd::OnProperties()
{
 ImageParser *px = m_HTLV.List.SelectedImage();
 if (px != nullptr)
  {
   PicturePropertiesDlg dlg;
   dlg.Show(this, px);
  }
}

void GroupShowWnd::OnFavorite()
{
 m_HTLV.List.SetFavorite(true);
}

void GroupShowWnd::OnUnFavorite()
{
 m_HTLV.List.SetFavorite(false);
}

void GroupShowWnd::OnHashTag()
{
if (m_FilterType == HashTagSelectCtrl::enumFilterStyle::NoFilter)
  return;

if (m_ShowingFilter==true)
  {
   Reload(m_Original);               // it's a toggle, so now it's going to be off
   m_HTLV.Filter.FolderMap.clear();  // empty hashtags
   m_HTLV.Filter.GlobalMap.clear();
  }
 else 
  {
   switch(m_FilterType)
    {
     case HashTagSelectCtrl::enumFilterStyle::FilterFolder:
      {
       for(const auto &ht : Wnd->CurrentFolder()->FolderHashTags)
         m_HTLV.Filter.FolderMap.insert(std::pair<int, HashTag>(ht.second.ID(), ht.second));
      } break;
     case HashTagSelectCtrl::enumFilterStyle::FilterGlobal:
      {
       for(const auto &ht : App->GlobalHashTags)
         m_HTLV.Filter.GlobalMap.insert(std::pair<int, HashTag>(ht.second.ID(), ht.second));
      } break;
     case HashTagSelectCtrl::enumFilterStyle::FilterGlobalFolder:
      {
       for(const auto &ht : App->GlobalHashTags)
         m_HTLV.Filter.GlobalMap.insert(std::pair<int, HashTag>(ht.second.ID(), ht.second));
       for(const auto &ht : Wnd->CurrentFolder()->FolderHashTags)
         m_HTLV.Filter.FolderMap.insert(std::pair<int, HashTag>(ht.second.ID(), ht.second));
      } break;
    }
   m_HTLV.Filter.ProcessTags(m_HTLV.Width());
  }
 m_HTLV.OnSize();

 m_ShowingFilter=!m_ShowingFilter;
}

void GroupShowWnd::OnViewPicture()
{
 ImageParser *px;
 std::vector<int> indices;
 int id;

 if (m_HTLV.List.SelectedItemsCount()==0)
   return;

 indices = m_HTLV.List.GetSelectedIndices();

 id = m_HTLV.List.GetItemParam(indices[0]);

 if (App->Pictures.count(id) == 0) throw L"m_TreeGroup item tag not imageparser id";
 px = App->Pictures[id];

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
 m_HTLV.List.SetFocus();
}

void GroupShowWnd::OnFilterChanged()
{
 std::vector<ImageParser *> tempList;

 for(const auto &p : m_Original)
  {
   if (p->Item() != nullptr)
    {
     if (m_HTLV.Filter.HashTagList.size() == 0 || m_HTLV.Filter.ShowItem(p->Item()) == true)
       tempList.push_back(p);
    }
  }
 Reload(tempList);
 m_HTLV.List.SetSort(ImageParser::SortChoices::FileName, true);
}

void GroupShowWnd::OnSelectDupImports()
{
 std::vector<ImageParser *> imports;
 std::vector<ImageParser *> pics;
 std::vector<ImageParser *> dups;
 int i;

 for(const auto &p : App->Pictures)
  {
   if (p.second->IsImport() == true)
     imports.push_back(p.second);
   else
     pics.push_back(p.second);
  }

 for(const auto &p1 : pics)
  {
   for(const auto &p2 : imports)
    {
     if (p1->Equal(p2) == true)
       dups.push_back(p2);
    }
  }

 m_HTLV.List.SelectNone();

 for (i=0; i<m_HTLV.List.Count(); i++)
  {
   for(const auto &p : dups)
    {
     if (m_HTLV.List.GetItemParam(i) == p->ID())
       m_HTLV.List.SetSelected(i, true, true);
    }
  }
}

void GroupShowWnd::OnListViewMiddle(int id)
{
 ImageParser *px;

 if (App->Pictures.count(id) == 0) throw L"id not valid";

 px = App->Pictures[id];

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

void GroupShowWnd::SetPictureViewer(ImageParser *pic)
{
 String msg;

 m_Pic.SetItem(pic);
 if (pic != nullptr)
  { 
   msg =  String::Decimal(pic->Width());
   msg += L" x ";
   msg += String::Decimal(pic->Height());
   msg += L"  ";
   msg += pic->FullPath();
  }
 m_SB.SetText(0, msg);
}