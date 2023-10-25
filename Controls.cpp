#include "pch.h"
#include "App.h"

// controls specific to PicMan

PicListView::PicListView()
{
 m_Owner = nullptr;
 m_ItemsDeleted = false;
}

void PicListView::Create(AWnd *parent, DWORD style, Rect const &r)
{
 DWORD xStyle;
 DWORD fStyle;

 xStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;

 fStyle = style | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS; 
 
 ListView::Create(parent, xStyle, fStyle, r);
 AddColumns();

 CurrentSort = ImageParser::SortChoices::FileName;
 IsAscending = true;
}

std::vector<ImageParser *> PicListView::SelectedPictures() 
{
 std::vector<ImageParser *> list;
 std::vector<int> indices;
 ImageParser *p;
 int id;

 if (SelectedItemsCount() == 0)
   return list;  // empty

 indices = GetSelectedIndices();
  
 for (const auto &ndx : indices)
  {
   id = GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"Item param not an ImageParser id";
   #endif 
   p = App->Pictures[id];
   list.push_back(p);
  }
 return list;
}

void PicListView::AddSortMenus(MenuItem *subMenu, ImageParser::SortChoices currentSort)
{
 InsertSortMenu(subMenu, ID_SORT_BY_FILENAME, ImageParser::SortChoices::FileName, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_NAME_WITH_NUMBER, ImageParser::SortChoices::NameWithNumber, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_WIDTH, ImageParser::SortChoices::Width, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_HEIGHT, ImageParser::SortChoices::Height, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_RATIO, ImageParser::SortChoices::Ratio, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_COLOR_RGB, ImageParser::SortChoices::ColorRGB, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_EDGE_AVERAGE, ImageParser::SortChoices::EdgeAverage, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_FILESIZE, ImageParser::SortChoices::FileSize, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_FILEDATE, ImageParser::SortChoices::FileDate, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_GLOBALTAGS, ImageParser::SortChoices::GlobalHashTags, currentSort);
 InsertSortMenu(subMenu, ID_SORT_BY_FOLDERTAGS, ImageParser::SortChoices::FolderHashTags, currentSort);
}


void PicListView::InsertSortMenu(MenuItem *subMenu, int id, ImageParser::SortChoices item, ImageParser::SortChoices currentSort)
{
 bool checked;

 checked = (item == currentSort); 
 subMenu->AddMenu(id, checked, true, 0);
} 

void PicListView::SetSort(ImageParser::SortChoices sort, bool ascending)
{
 CurrentSort = sort;
 IsAscending = ascending;
 DoSort();
}

void PicListView::SetSort(ImageParser::SortChoices sort) // toggle ascending / descending
{
 if (CurrentSort == sort)
   IsAscending = !IsAscending;
 else
  {
   CurrentSort = sort;
   IsAscending = true;
  }
 DoSort();
}


void PicListView::AddColumns() 
{
 AddColumn(UI_COLUMN_FILE, ListViewColumn::ColumnAlign::Left, 187);  // File
 AddColumn(UI_COLUMN_WIDTH, ListViewColumn::ColumnAlign::Right, 60);  // Width
 AddColumn(UI_COLUMN_HEIGHT, ListViewColumn::ColumnAlign::Right, 60);  // Height
 AddColumn(UI_COLUMN_RATIO, ListViewColumn::ColumnAlign::Right, 60);  // Ratio
 AddColumn(UI_COLUMN_SIZE, ListViewColumn::ColumnAlign::Right, 60);  // Size
 AddColumn(UI_COLUMN_COLORS, ListViewColumn::ColumnAlign::Right, 60);  // Colors
 AddColumn(UI_COLUMN_DATE, ListViewColumn::ColumnAlign::Left, 60);   // Date
}

void PicListView::ResizeColumns()
{
 int i;

 for(i = 1; i < m_ColumnCount; i++)  // not file name, since jpgs can have very large names
  {
   AutoSize(i,ListView::AutoSizes::Content);
  }
}

WMR PicListView::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 ImageParser::SortChoices eSort;
 int nCol;

 switch(code)
  {
   case LVN_COLUMNCLICK:
    {
     nCol = ((NMLISTVIEW *)lParam)->iSubItem;         
     switch(nCol)
      {
       case 0: eSort = ImageParser::SortChoices::FileName; break;
       case 1: eSort = ImageParser::SortChoices::Width;    break;
       case 2: eSort = ImageParser::SortChoices::Height;   break;
       case 3: eSort = ImageParser::SortChoices::Ratio;    break;
       case 4: eSort = ImageParser::SortChoices::FileSize; break;
       case 5: eSort = ImageParser::SortChoices::ColorRGB; break;
       case 6: eSort = ImageParser::SortChoices::FileDate; break;
       default: throw L"unhandled case";
      }
     if (eSort == CurrentSort)
       IsAscending = !IsAscending;
     else
      {
       IsAscending = true;
       CurrentSort = eSort;
      }  
     DoSort();
    } break;
   default: return ListView::OnNotify(hChild, child, code, lParam);
  }
 return WMR::Zero;
}

WMR PicListView::OnSubKeyDown(KeyEventArgs const &key)
{
 int ndx;

 switch(key.KeyCode)
  {
   case Keyboard::F:
    {
     if (key.Control==true) SetFavorite(true);
    } break;
   case Keyboard::U:
    {
     if (key.Control==true) SetFavorite(false);
    } break;
   case Keyboard::Delete:
     OnDelete(); break;
  }

 if (SelectedItemsCount()>0 && GetViewStyle()==DisplayStyle::LargeIcon)
  {
   ndx=GetSelectedItem(); // ndx will be >= 0 because we just checked SelectedItemsCount
   if (key.KeyCode==Keyboard::Right)
    {
     if (ndx<Count()-1)
      {
       SetSelected(ndx, false, true);
       SetSelected(ndx+1, true, false);
       EnsureVisible(ndx+1);
       return WMR::One;  // don't let base handle keys.right
      }
    }
   if (key.KeyCode==Keyboard::Left)
    {
     if (ndx>0)
      {
       SetSelected(ndx, false, true);
       SetSelected(ndx-1, true, false);
       EnsureVisible(ndx-1);
       return WMR::One;  // don't let base handle keys.right
      }
    }
  }

 return WMR::Default;
}

WMR PicListView::OnSubMouseDown(MouseEventArgs const &m)
{
 LVHITTESTINFO hti={0};
 int ndx;
 WMR ret = WMR::Default;

 if (m.Middle == false)
   return ret;

 if (m_Owner == nullptr)
   return ret;

 hti.pt.x = m.X;
 hti.pt.y = m.Y;
 ndx = ListView_HitTest(m_hWnd, &hti);
 if (ndx>=0)
  {
   ::SendMessage(m_Owner->Handle(), WM_LISTVIEW_MIDDLE, (WPARAM)m_hWnd, (LPARAM)ndx);
  }
 return ret;
}

void PicListView::OnDelete()
{
 ImageParser *px;
 String q;
 String nl = L"\n";
 std::vector<int> indices;
 std::vector<int> delIndices;
 int i, id;

 if (SelectedItemsCount() == 0)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 
 q = App->Prose.Text(COMMON_MOVE_TO_RECYCLE);

 indices = GetSelectedIndices();

 i=0;
 for(const auto &ndx : indices)
  {
   if (i>12)
    {
     q += nl;
     q += App->Prose.Text(COMMON_CUT_SHORT);
     break;
    }
   q += nl;
   q += GetItemText(ndx,0);
   i++;
  }

 if (App->Question(q, MB_OKCANCEL) != DialogResult::OK) return;

 for(const auto &ndx : indices)
  {
   id = GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_PicList item param not an ImageParser id";
   #endif 
   px = App->Pictures[id];
   if (px->Delete() == false)
    {
     App->Response(App->Prose.TextArgs(MAIN_DELETE_FAILED, px->FileName()));
     break;
    }
   else
    {
     px->Dispose();
     delete px;
     App->Pictures.erase(id);
     delIndices.push_back(ndx);
     if (m_ItemsDeleted == false) 
       m_ItemsDeleted = true;
    }
  }

 RemoveItems(delIndices);

 ::SendMessage(m_Owner->Handle(), WM_PICLIST_ITEMDELETED, (WPARAM)m_hWnd, delIndices.size()); 
}

void PicListView::SetFavorite(bool onoff)
{
 HashTag ht = App->GetFavoriteHashTag();
 ImageParser *px;
 std::vector<int>indices;
 int id;

 if (SelectedItemsCount() == 0) return;
 indices = GetSelectedIndices();
 for(const auto &ndx : indices)
  {
   id = GetItemParam(ndx); 
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"param of ListView not ImageParser id";
   #endif
   px = App->Pictures[id];
   if (px->Item() != nullptr)
    {
     if (px->Item()->HasGlobalHashTag(ht) == !onoff)
      {
       if (onoff == true)
         px->Item()->AddGlobalHashTag(ht);
       else
         px->Item()->RemoveGlobalHashTag(ht);
       ::SendMessage(m_Owner->Handle(), WM_PICLIST_HT_CHANGED, (WPARAM)Handle(), px->ID());
      }
    }
  }
}

void PicListView::Clear()
{
 ListView::Clear();
}

ImageParser *PicListView::SelectedImage()
{
 ImageParser *p;
 int ndx, id;
  
 ndx = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED);
 if (ndx >= 0)
  {
   id = GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"ListView param not ImageParser id";
   #endif
   p = App->Pictures[id];
  }
 else
  {
   p = nullptr;
  }
 return p;
}

void PicListView::Insert(ImageParser *px)
{
 int id, image;

 id = px->ID();
 image = m_ImageList->GetIndex(id);

 ListViewItem lvi(px->FileName(), id, image);

 lvi.SubItems.push_back(String::Decimal(px->Width()));
 lvi.SubItems.push_back(String::Decimal(px->Height()));
 lvi.SubItems.push_back(String::Double(px->Ratio()));
 lvi.SubItems.push_back(px->FileSize());
 lvi.SubItems.push_back(String::Decimal((int)px->ColorIndex.size()));
 lvi.SubItems.push_back(px->FileDate().ToString(DateTime::Format::MDYHMS));
 
 ListView::Insert(lvi);
}

void PicListView::InsertAt(int row, ImageParser *px)
{
 int id, image;

 id = px->ID();
 image = m_ImageList->GetIndex(id);

 ListViewItem lvi(px->FileName(), id, image);
 lvi.SubItems.push_back(px->FileName());
 lvi.SubItems.push_back(String::Decimal(px->Width()));
 lvi.SubItems.push_back(String::Decimal(px->Height()));
 lvi.SubItems.push_back(String::Double(px->Ratio()));
 lvi.SubItems.push_back(px->FileSize());
 lvi.SubItems.push_back(String::Decimal((int)px->ColorIndex.size()));
 lvi.SubItems.push_back(px->FileDate().ToString(DateTime::Format::MDYHMS));
 
 ListView::InsertAt(row, lvi);
}

void PicListView::DoSort()
{
 int c = (int)CurrentSort;
 
 ListView_SortItems(m_hWnd, &CompareItems, Utility::AscendDescendColumn(IsAscending, c));

}

int PicListView::CompareItems(LPARAM picA, LPARAM picB, LPARAM nAscDesCol)
{
 DateTime a1, a2;
 BaseItem *base = Wnd->CurrentBase();
 ImageParser *px, *py;
 bool AscDesc;
 ImageParser::SortChoices nCol;
 int k,i,c;

 if ((nAscDesCol & 0xFF00) > 0) AscDesc = true; else AscDesc=false;
 nCol = (ImageParser::SortChoices)(int)(nAscDesCol & 0x00FF);
 if (AscDesc==true)
  {
   px=App->Pictures.at(picA);
   py=App->Pictures.at(picB);
  }
 else
  {
   px=App->Pictures.at(picB);
   py=App->Pictures.at(picA);
  }

 switch(nCol)
  {
   case ImageParser::SortChoices::FileName:
     return String::Compare(px->FileName(), py->FileName());
   case ImageParser::SortChoices::NameWithNumber:
     if (px->IsNumbered() == false || py->IsNumbered() == false)
       return String::Compare(px->FileName(), py->FileName());
     c=String::Compare(px->FilePrefix(), py->FilePrefix());
     if (c != 0) return c;
     if (px->IsNumbered() == false || py->IsNumbered() == true) return 0;
     if (px->FileNumber() > py->FileNumber()) return 1;
     if (px->FileNumber() < py->FileNumber()) return -1;
     return 0;
   case ImageParser::SortChoices::Height:
     if (px->Height() > py->Height()) return 1;
     if (px->Height() < py->Height()) return -1;
     return 0;
   case ImageParser::SortChoices::Width:
     if (px->Width() > py->Width()) return 1;
     if (px->Width() < py->Width()) return -1;
     return 0;
   case ImageParser::SortChoices::Ratio:
     if (px->Ratio() > py->Ratio()) return 1;
     if (px->Ratio() < py->Ratio()) return -1;
     return 0;
   case ImageParser::SortChoices::FileDate:
     if (px->FileDate() > py->FileDate()) return 1;
     if (px->FileDate() < py->FileDate()) return -1;
     return 0;
   case ImageParser::SortChoices::FileSize:
     if (px->FileLength() > py->FileLength()) return 1;
     if (px->FileLength() < py->FileLength()) return -1;
     return 0;
   case ImageParser::SortChoices::ColorRGB:
     if (px->ColorIndex.size() <= py->ColorIndex.size()) c = (int)px->ColorIndex.size(); else c = (int)py->ColorIndex.size();
     for (i=0;i<c;i++)
      {
       k = PicColor::CompareRGB(px->Colors()[i], py->Colors()[i]);
       if (k != 0) return k;
       if (px->Colors()[i]->Amount() > py->Colors()[i]->Amount()) return 1;
       if (px->Colors()[i]->Amount() < py->Colors()[i]->Amount()) return -1;
      }
     return 0;
   case ImageParser::SortChoices::EdgeAverage:
     if (px->BGAvgRed() > py->BGAvgRed())     return 1;
     if (px->BGAvgRed() < py->BGAvgRed())     return -1;
     if (px->BGAvgGreen() > py->BGAvgGreen()) return 1;
     if (px->BGAvgGreen() < py->BGAvgGreen()) return -1;
     if (px->BGAvgBlue() > py->BGAvgBlue())   return 1;
     if (px->BGAvgBlue() < py->BGAvgBlue())   return -1;
     return 0;
   case ImageParser::SortChoices::GlobalHashTags:
     if (px->Item() == nullptr && py->Item() == nullptr) return 0;
     if (px->Item() != nullptr)
      {
       if (py->Item() == nullptr) return 1;
       if (px->Item()->GlobalHashTags.count(1)>0) return 1;
       return 0;
      }
     if (py->Item() != nullptr)
      {
       if (px->Item() == nullptr) return -1;
       if (py->Item()->GlobalHashTags.count(1)>0) return -1;
      }
     return 0;
   case ImageParser::SortChoices::FolderHashTags:
     if (px->Item() == nullptr && py->Item() == nullptr) return 0;
     if (px->Item() != nullptr)
      {
       if (py->Item() == nullptr) return 1;
       c=String::Compare(px->Item()->PictureHashTagString(),py->Item()->PictureHashTagString()); 
       if (c != 0) return c;
       return String::Compare(px->Item()->FileName(), py->Item()->FileName());
      }
     if (py->Item() != nullptr)
      {
       if (px->Item() == nullptr) return -1;
       c = String::Compare(px->Item()->PictureHashTagString(), py->Item()->PictureHashTagString()); 
       if (c != 0) return c;
       return String::Compare(px->Item()->FileName(),py->Item()->FileName());
      } 
     return 0;
   default: throw L"oops, unhandled switch";   
  }
 throw L"oops, no return";
}

///////////////////////////////////////////////////////////

PicturesTree::PicturesTree()
{
}

ImageParser *PicturesTree::SelectedPicture()
{
 ImageParser *px = nullptr;
 std::vector<TreeNode> nodes;
 TreeNode node;
 int id;

 node = SelectedNode();
 if (node.Handle() != 0)
  {
   id = node.Tag;
   if (id == 0)  // if tag = 0 then it's a root node, get px from 1st child
    {
     nodes = node.GetNodes();
     if (nodes.size() == 0)
       throw L"root node should have at least 1 child node";
     node = nodes[0];
     id = node.Tag;
     if (id == 0)
       throw L"child node tag should not be 0";
     if (App->Pictures.count(id) == 0)
       throw L"child node tag not valid imageparser";
     px = App->Pictures[id]; 
    }
   else
    {
     if (App->Pictures.count(id) == 0)
       throw L"node tag not valid imageparser";
     px = App->Pictures[id];
    }
  }
 return px; 
}

void PicturesTree::AddTree(std::map<String, TreeNode> &tree)
{
 TreeNode group;

 for (const auto &it : tree)
  {
   group=InsertFirst(it.second.Text, 0);
   for (const auto &sn : it.second.Nodes)
    {
     group.InsertFirst(sn.Text, sn.Tag);
    }
  }
 DoSort();
 Refresh();
}

void PicturesTree::DoSort()
{
 std::vector<TreeNode> nodes;

 TreeView_SortChildren(m_hWnd, TVI_ROOT, TRUE);

 nodes = Nodes();
 for (const auto &node : nodes)
   TreeView_SortChildren(m_hWnd, node.Handle(), TRUE);

 
}

// //////////////////////////////////////////////////////////////

void HashTagTreeView::Create(AWnd *parent, Rect const &r)
{
 PanelWnd::Create(parent, r);

 Border(false);

 Filter.Create(this, HashTagSelectCtrl::enumFilterStyle::FilterFolder);
 Filter.Border(false);
 Tree.Create(this, Rect(0,0, 40, 100));

}

void HashTagTreeView::OnSize()
{
 PanelWnd::OnSize();
 SizeTree();
}

WMR HashTagTreeView::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch(msg)
  {
   case WM_CONTEXTMENU:
     {
      if (m_Parent->OnContextMenu((HWND)wParam, Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) == WMR::One)
        return WMR::One;
     } break;
   case WM_HTSC_FILTER_CHANGED: 
    {
     ::SendMessage(m_Parent->Handle(), msg, wParam, lParam);
    } break;
  }
 return PanelWnd::MessageHandler(hWnd, msg, wParam, lParam);
}

void HashTagTreeView::SizeTree()
{ 
 Rect r;
 Size sz;

 sz = ClientSize();

 Filter.ProcessTags(sz.Width);
 r = Rect(0, 0, sz.Width, Filter.GetHashTagHeight());
 Filter.SetRect(r);
 r = Rect(0, Filter.GetHashTagHeight(), sz.Width, sz.Height - Filter.GetHashTagHeight());
 Tree.SetRect(r);
 Tree.Refresh();
 Refresh();
}

WMR HashTagTreeView::OnNotify(HWND hChild, int child, UINT code, LPARAM param)
{
 if (hChild == Tree.Handle())
   return m_Parent->OnNotify(hChild, child, code, param);
 else
   return PanelWnd::OnNotify(hChild, child, code, param);
}

// //////////////////////////////////////////////////////////////

void HashTagListView::Create(AWnd *parent, Rect const &r, HashTagSelectCtrl::enumFilterStyle fStyle)
{
 PanelWnd::Create(parent, r);

 Border(false);

 Filter.Create(this, fStyle);
 Filter.Border(false);
 List.Create(this, LVS_REPORT, Rect(0,0, 40, 100));
 List.SetOwner(parent);
}

void HashTagListView::OnSize()
{
 PanelWnd::OnSize();
 SizeList();
}

WMR HashTagListView::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch(msg)
  {
   case WM_HTSC_FILTER_CHANGED: 
    {
     ::SendMessage(m_Parent->Handle(), msg, wParam, lParam);
    } break;
   case WM_CONTEXTMENU:
    {
     if (m_Parent->OnContextMenu((HWND)wParam, Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) == WMR::One)
       return WMR::One;
    } break; 
  }
 return PanelWnd::MessageHandler(hWnd, msg, wParam, lParam);
}

void HashTagListView::SizeList()
{ 
 Rect r;
 Size sz;

 sz = ClientSize();

 Filter.ProcessTags(sz.Width);
 r = Rect(0, 0, sz.Width, Filter.GetHashTagHeight());
 Filter.SetRect(r);
 r = Rect(0, Filter.GetHashTagHeight(), sz.Width, sz.Height - Filter.GetHashTagHeight());
 List.SetRect(r);
 List.Refresh();
 Refresh();
}

WMR HashTagListView::OnNotify(HWND hChild, int child, UINT code, LPARAM param)
{
 DWORD dwPos;
 Point pt, spt;

 if (hChild == List.Handle())
  {
   if (code == (UINT)NM_RCLICK)
    {
     dwPos = GetMessagePos();
     pt.X = GET_X_LPARAM(dwPos);
     pt.Y = GET_Y_LPARAM(dwPos);
     m_Parent->OnContextMenu(List.Handle(), pt);  // WM_CONTEXTMENU was only sent after a double right click, no idea why....
    }
   return m_Parent->OnNotify(hChild, child, code, param);
  }
 else
  {
   return PanelWnd::OnNotify(hChild, child, code, param);
  }
}


// //////////////////////////////////////////////////////////////

PicView::PicView()
{
 m_Pic=nullptr;
}

PicView::~PicView()
{
}

void PicView::Create(AWnd *parent, Rect const &r)
{
 PanelWnd::Create(parent, r);

 Border(true);

 m_Image.Create(this, Rect(0,0, 100, 60));
 m_Image.Show();
 m_HashTags.Create(this, HashTagSelectCtrl::enumCtrlStyle::DisplayCtrl, HashTagSelectCtrl::enumSelectStyle::GlobalFolderTags);
 m_HashTags.Show();
}

void PicView::SetItem(ImageParser *pic)
{
 if (m_Pic != nullptr && pic != nullptr)
  {
   if (m_Pic->ID() == pic->ID())
    return; // all set
  }

 if (pic == nullptr)
  {
   m_Pic = nullptr;
   m_Image.SetImage(L"", false);
  }
 else
  {
   m_Pic = pic;
   m_Image.SetImage(pic->FullPath(), false);
  }

 RefreshHashTags();
}

void PicView::OnSize() 
{
 RefreshHashTags();
}  

void PicView::RefreshHashTags()
{
 Size sz;
 Rect r;
 Rect cr;
 int hth;

 m_HashTags.GlobalMap.clear();
 m_HashTags.FolderMap.clear();
 m_HashTags.PictureMap.clear();

 if (m_Pic != nullptr)
  {
   if (m_Pic->Item() != nullptr)
    {
     for (const auto &ig : m_Pic->Item()->GlobalHashTags)
       m_HashTags.AddGlobalTag(ig.second);
     for (const auto &ip : m_Pic->Item()->PictureHashTags)
       m_HashTags.AddPictureTag(ip.second);
    }
  }

 sz=ClientSize();
 m_HashTags.ProcessTags(sz.Width);

 cr=ClientRect();
 hth = m_HashTags.GetHashTagHeight();
 
 r=Rect(cr.X, cr.Y, cr.Width, cr.Height-hth);
 m_Image.SetRect(r);
 
 r=Rect(cr.X, cr.Y + (cr.Height - hth), cr.Width, hth);
 m_HashTags.SetRect(r);

}

void PicView::OnPanelClick(HWND hWnd, MouseEventArgs const &e) // Handles Image.Click
{
 if (m_Pic == nullptr)
   return;

 if (m_Pic->Item() == nullptr)
   return;

 HashTagDlg dlg(m_Pic->Item());

 if (dlg.Show(this) == DialogResult::OK)
  {
   m_HashTags.GlobalMap.clear();
   for(const auto &ig : m_Pic->Item()->GlobalHashTags)
     m_HashTags.AddGlobalTag(ig.second);
   for(const auto &ip : m_Pic->Item()->PictureHashTags)
     m_HashTags.AddPictureTag(ip.second);
   RefreshHashTags();
   ::SendMessage(m_Parent->Handle(), WM_PICVIEW_HT_CHANGED, (WPARAM)m_hWnd, 0);
  }
}

//////////////////////////////////////////////////////////////////

void HashTagEditCtrl::Create(AWnd *parent, AWnd *owner, int editID, int btnID, HashTagSelectCtrl::enumSelectStyle htStyle)
{
 PanelWnd::Create(parent, Rect(0,0,10,10));

 m_Owner = owner;

 HT.Create(this, HashTagSelectCtrl::enumCtrlStyle::SelectCtrl, htStyle);
 HT.Border(false);
 HT.AutoSize = false;
 HT.Show();

 Edit.Create(this, editID, Rect(0,0,10,10));
 m_Add.Create(this, L"Add", btnID, Rect(0,0,10,10));
 m_Style = htStyle;
}

void HashTagEditCtrl::OnSize()
{
 Size sz;
 Rect r;
 const int ch = 20;  // height of edit and button ctrls
 const int g = 6;    // gap

 sz = ClientSize();

 r = Rect(g, g, sz.Width - (g + g), sz.Height - (g + g + ch + g));
 HT.SetRect(r);

 r = Rect(g, r.Y + r.Height + g, sz.Width - (g + g + 60 + g), ch);
 Edit.SetRect(r);
 
 r = Rect(r.X + r.Width + g, r.Y, 60, ch);
 m_Add.SetRect(r);
 
}

WMR HashTagEditCtrl::OnCommand(int child, HWND hChild)
{
 SendMessage(m_Owner->Handle(), WM_COMMAND, (WPARAM)child, (LPARAM)hChild);
 return WMR::One;
}

WMR HashTagEditCtrl::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch(msg)
  {
   case WM_HTSC_ITEMCLICK: 
   case WM_HTSC_ITEMCONTEXT:
   case WM_HTSC_FILTER_CHANGED:
   case WM_HTSC_NAME_CHANGED:
   case WM_HTSC_LIST_CHANGED:
     ::SendMessage(m_Owner->Handle(), msg, wParam, lParam);
     break;

   default: return PanelWnd::MessageHandler(hWnd, msg, wParam, lParam); 
  }
 return WMR::Zero;
}

///////////////////////////////////////////////////////

SpreadSheet::SpreadSheet()
{
 m_ItemSize = Size(ImageParser::ThumbSize + 2, ImageParser::ThumbSize + 2);

 m_Font = ::CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
 m_Pen = ::CreatePen(PS_SOLID, 2, ::GetSysColor(COLOR_MENUHILIGHT));

 m_SelectedRow = -1;
 m_SelectedCol = -1;
 m_RowImageSelected = false;
 m_Changed = false;

 m_Red = new Gdiplus::SolidBrush(Gdiplus::Color::Red);
 m_White = new Gdiplus::SolidBrush(Gdiplus::Color::White);
 m_Green = new Gdiplus::SolidBrush(Gdiplus::Color::Green);

 m_ImageList.Create(ImageList::Style::Regular, ImageParser::ThumbSize, ImageParser::ThumbSize, (int)App->Pictures.size());

 for(const auto &px : App->Pictures)
  {
   m_ImageList.Add(px.second->Thumb(), px.second->ID());
  }
}

SpreadSheet::~SpreadSheet()
{
 ::DeleteObject(m_Font);
 ::DeleteObject(m_Pen);

 delete m_Red;
 delete m_White;
 delete m_Green;

 m_ImageList.Destroy();
}

void SpreadSheet::OnMouseDown(MouseEventArgs const &m)
{
 bool rowSelected, shift, ctrl;
 int r, c, i;

 if (m.Right == true)
   return;

 c = m_HSPos + ( m.X / m_ItemSize.Width );
 r = m_VSPos + ( m.Y / m_ItemSize.Height );

 if (r < 0 || r >= m_Owner->Rows.size())
   return;  // outside of range

 if (c < 0)
   return;

 shift = ( ::GetKeyState(VK_SHIFT) & 0x8000 );   
 ctrl  = ( ::GetKeyState(VK_CONTROL) & 0x8000 );

 if (c == 0)
  {
   rowSelected = true; // 1st column is the row's image, 2nd and on are it's matches
   c = -1;
  }
 else
  {
   c--;
   if (c >= m_Owner->Rows[r]->Matches.size())
     return;  // beyond the last image
   rowSelected = false;
  }

 if (m.Middle == true)
  {
   if (rowSelected == true)
    {
     if (m_Owner->Rows[r]->Deleted == false)
      {
       ImageParser *ip = m_Owner->Rows[r]->Image;
       SlideDlg dlg(ip->FullPath(), true);
       dlg.Show();
      }
    }
   else
    {
     if (m_Owner->Rows[r]->Matches[c]->Deleted == false)
      {
       ImageParser *ip = m_Owner->Rows[r]->Matches[c]->Image;
       SlideDlg dlg(ip->FullPath(), true);
       dlg.Show();
      }
    }
   return;
  }

 // shift and ctrl not down, or selection is not in the same row then un-select last selection

 if ((shift == false && ctrl == false) || m_SelectedRow != r)
  {
   if (m_SelectedRow >=0)  // if row < 0 then nothing is selected
    {
     m_Owner->Rows[m_SelectedRow]->Selected = false;
     for(const auto &col : m_Owner->Rows[m_SelectedRow]->Matches)
       col->Selected = false;
    }
  }

 // shift is down select this and all items between last selection
 
 if (shift == true && rowSelected == false) // ctrl might be down, but I don't care
  {
   if (r == m_SelectedRow && m_Owner->Rows[r]->Deleted == false) // don't try to select items between different rows
    {
     if (c < m_SelectedCol)
      {
       for(i=c; i<m_SelectedCol; i++)
        {
         if (m_Owner->Rows[r]->Matches[i]->Deleted == false) // don't allow deleted images to be selected
          {
           if (m_DeleteList.count(m_Owner->Rows[r]->Matches[i]->ID) == 0)
             m_Owner->Rows[r]->Matches[i]->Selected=true; // ok, the item's id didn't show in in the deleted list
          }
        }
      }
     else
      {
       for(i=m_SelectedCol+1; i<=c; i++)
        {
         if (m_Owner->Rows[r]->Matches[i]->Deleted == false)
          {
           if (m_DeleteList.count(m_Owner->Rows[r]->Matches[i]->ID) == 0)
             m_Owner->Rows[r]->Matches[i]->Selected=true;
          }
        }
      }
    }
  }

 if (m_Owner->Rows[r]->Deleted == false && m_DeleteList.count(m_Owner->Rows[r]->ID)==0)  // if the row's image is deleted don't allow any selection on the row
  {
   m_RowImageSelected = rowSelected;
   m_SelectedRow = r;
   m_SelectedCol = c;
  }
 else
  {
   m_RowImageSelected = false;
   m_SelectedRow = -1;
   m_SelectedCol = -1;   
   return;
  }

 // select item

 if (m_RowImageSelected == true)
  {
   m_Owner->Rows[r]->Selected = true;
  }
 else
  {
   if (shift == false)
    {
     if (m_Owner->Rows[r]->Matches[c]->Deleted == false && m_DeleteList.count( m_Owner->Rows[r]->Matches[c]->ID ) == 0)
       m_Owner->Rows[r]->Matches[c]->Selected = true;
    }
  }
 m_Owner->SelectChange(r,c);

 Refresh();
}

void SpreadSheet::OnMouseWheel(MouseEventArgs const &m)
{
 if (m.Delta > 0)
   m_VSPos = m_Owner->m_VS.LineUp(3);
 else
   m_VSPos = m_Owner->m_VS.LineDown(3);

 Refresh();
}

WMR SpreadSheet::OnContextMenu(HWND child, Point const &pt)
{
 AMenu menu;
 MenuItem *sub;

 menu.CreateContextMenu();      // menu entries
                                // if a row's matched items are selected

 if (m_RowImageSelected == false)
  {
   menu.AddMenu(ID_SPREADSHEET_MATCHES_NEWGROUP);      // new group
   menu.AddMenu(ID_SPREADSHEET_MATCHES_NEWHASHTAGS);    // assign hashtags
   menu.Separator();                                    // -------------------  
   menu.AddMenu(ID_SPREADSHEET_MATCHES_ADDTOGROUP);    // add to existing group
   menu.AddMenu(ID_SPREADSHEET_MATCHES_ADDHASHTAG);    // add to hashtag set
   menu.Separator();                                    // --------------             
   menu.AddMenu(ID_SPREADSHEET_MATCHES_ADDTOROWGROUP); // add matches to row's group
   menu.AddMenu(ID_SPREADSHEET_MATCHES_ADDROWHASHTAG); // add matches to row's hashtag set
   menu.Separator();
   menu.AddMenu(ID_SPREADSHEET_BOTH_NEWGROUP);          // add both selected row and matches
   menu.AddMenu(ID_SPREADSHEET_BOTH_NEWHASHTAGS);      
  }
 else
  {                                                     // if a row's thumb is selected
   menu.AddMenu(ID_SPREADSHEET_ROW_ADDGROUP);          // add row to existing group
   menu.AddMenu(ID_SPREADSHEET_ROW_ADDHASHTAG);        // add row to existing hash tag set
   menu.Separator();                                    // --------------
   menu.AddMenu(ID_SPREADSHEET_ROW_ADDMATCHGROUP);     // add row to group of the 1st match
   menu.AddMenu(ID_SPREADSHEET_ROW_ADDMATCHHASHTAG);   // add row to hashtag set of the 1st match
  }

 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_PROPERTIES);
 menu.Separator();
 menu.AddMenu(ID_POPUP_LIST_DELETE_SELECTED_FILES);

 menu.Separator();
 sub = menu.SubMenu(ID_VIEW_SORTLIST);

 sub->AddMenu(ID_VIEW_SORTLIST_NAME);
 sub->AddMenu(ID_VIEW_SORTLIST_NAME_NMBR);
 sub->AddMenu(ID_VIEW_SORTLIST_WIDTH);
 sub->AddMenu(ID_VIEW_SORTLIST_HEIGHT);
 sub->AddMenu(ID_VIEW_SORTLIST_WIDTHDIVHEIGHT);
 sub->AddMenu(ID_VIEW_SORTLIST_RGBCOLORS);
 sub->AddMenu(ID_VIEW_SORTLIST_BORDERCOLORAVERAGE);
 sub->AddMenu(ID_VIEW_SORTLIST_DATEADDED);
 sub->AddMenu(ID_VIEW_SORTLIST_SIZE);
 sub->AddMenu(ID_VIEW_SORTLIST_GHT);
 sub->AddMenu(ID_VIEW_SORTLIST_FHT);

 menu.ShowContextMenu(this, pt);
 return WMR::Zero;
}

WMR SpreadSheet::OnCommand(int id, HWND hChild)
{
 switch(id)
  {
   case ID_SPREADSHEET_MATCHES_NEWGROUP:      // new group
     OnMatchNewGroup();
     break;
   case ID_SPREADSHEET_MATCHES_NEWHASHTAGS:   // assign hashtags
     OnMatchNewHashTag();
     break;
   case ID_SPREADSHEET_MATCHES_ADDTOGROUP:    // add to existing group
     OnMatchExistingGroup();
     break;
   case ID_SPREADSHEET_MATCHES_ADDHASHTAG:    // add to hashtag set
     OnMatchExistingHashTag();
     break;
   case ID_SPREADSHEET_MATCHES_ADDTOROWGROUP: // add matches to row's group
     OnMatchRowGroup();
     break;
   case ID_SPREADSHEET_MATCHES_ADDROWHASHTAG: // add matches to row's hashtag set
     OnMatchRowHashTag();
     break;
   case ID_SPREADSHEET_ROW_ADDGROUP:          // add row to existing group
     OnRowGroup();
     break;
   case ID_SPREADSHEET_ROW_ADDHASHTAG:        // add row to existing hash tag set
     OnRowHashTag();
   case ID_SPREADSHEET_ROW_ADDMATCHGROUP:     // add row to group of the 1st match
     OnRowMatchGroup();
     break;
   case ID_SPREADSHEET_ROW_ADDMATCHHASHTAG:   // add row to hashtag set of the 1st match
     OnRowMatchHashTag();
     break;
   case ID_SPREADSHEET_BOTH_NEWGROUP:         // both row and matches moved to new group
     OnBothNewGroup();
     break;
   case ID_SPREADSHEET_BOTH_NEWHASHTAGS:      // both row and matches assigned new hashtag set
     OnBothAssignHashTags();
     break;
   case ID_POPUP_LIST_PROPERTIES:
     OnProperties();
     break;
   case ID_POPUP_LIST_DELETE_SELECTED_FILES:
     OnDelete();
     break;

   case ID_VIEW_SORTLIST_NAME:    
     mnuSortListName(); 
     break;
   case ID_VIEW_SORTLIST_NAME_NMBR: 
     mnuSortListNameNmbr(); 
     break;
   case ID_VIEW_SORTLIST_WIDTH: 
     mnuSortListWidth(); 
     break;
   case ID_VIEW_SORTLIST_HEIGHT: 
     mnuSortListHeight(); 
     break;
   case ID_VIEW_SORTLIST_WIDTHDIVHEIGHT: 
     mnuSortListWidthDivHeight(); 
     break;
   case ID_VIEW_SORTLIST_RGBCOLORS: 
     mnuSortListRGBColors(); 
     break;
   case ID_VIEW_SORTLIST_BORDERCOLORAVERAGE: 
     mnuSortListBorderColorAverage(); 
     break;
   case ID_VIEW_SORTLIST_DATEADDED: 
     mnuSortListDateAdded(); 
     break;
   case ID_VIEW_SORTLIST_SIZE: 
     mnuSortListFileSize(); 
     break;
   case ID_VIEW_SORTLIST_GHT: 
     mnuSortListGlobalHashTags(); 
     break;
   case ID_VIEW_SORTLIST_FHT: 
     mnuSortListFolderHashTags(); 
     break;
  }
 return WMR::Zero;
}

std::vector<SpreadSheetItem *> SpreadSheet::SelectedItems()
{
 std::vector<SpreadSheetItem *> items;

 if (m_SelectedRow < 0)
   return items; // empty

 for (const auto &item : m_Owner->Rows[m_SelectedRow]->Matches)
  {
   if (item->Selected == true)
     items.push_back(item);
  }

 return items;
}

void SpreadSheet::OnMatchNewGroup()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 for(const auto &match : SelectedItems())
  {
   list.push_back(match->Image);
  }
 if (Wnd->MoveToNewGroup(list) == true)
  {
   ReplaceMatches(SelectedItems());
  }
}

void SpreadSheet::OnMatchNewHashTag()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 for(const auto &match : SelectedItems())
  {
   list.push_back(match->Image);
  }
 if (Wnd->ReplaceHashTags(list) == true)
  {
   ReplaceMatches(SelectedItems());
  }
}

void SpreadSheet::OnMatchExistingGroup()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 for(const auto &match : SelectedItems())
  {
   list.push_back(match->Image);
  }
 if (Wnd->MoveToExistingGroup(list) == true)
  {
   ReplaceMatches(SelectedItems());
  }
}

void SpreadSheet::OnMatchExistingHashTag()
{
 String txt;
 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;
 std::map<String, ImageParser *> unique;
 std::vector<HashTag> hashTags;

 // build list of unique hashtag sets and the 1st imaage that contains them

 for(const auto &px : App->Pictures)
  {
   if (px.second->Item() != nullptr)
    {
     if (px.second->Item()->PictureHashTags.size() > 0)
      {
       px.second->Item()->SetPictureHashTagString();
       txt = px.second->Item()->PictureHashTagString();
       if (txt.Length() > 0)
        {
         if (unique.count(txt) == 0)
          {
           unique.insert(std::pair<String, ImageParser *>(txt, px.second));
          }
        }
      }
    }
  }

 // get list of the 1st imageparser containing the hashtags

 for(const auto &val : unique)
  {
   list.push_back(val.second);
  }

 HashTagSetSelectDlg dlg;
 if (dlg.Show(m_Owner, list) != DialogResult::OK)
   return;

 hashTags = dlg.HashTags();

 for(const auto &match : SelectedItems())
  {
   match->Image->Item()->ReplacePictureHashTags(hashTags);
  }

 ReplaceMatches(SelectedItems());
}

void SpreadSheet::OnMatchRowGroup()
{
 ImageParser *px;
 String group;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 if (m_SelectedRow == -1)
   return;
 
 px = m_Owner->Rows[m_SelectedRow]->Image;

 if (px->IsNumbered() == false)
  {
   App->Response(L"Selected row not a group member");
   return;
  }

 group = px->FilePrefix();

 for(const auto &match : SelectedItems())
  {
   list.push_back(match->Image);
  }
 if (Wnd->MoveToExistingGroup(list, group) == true)
  {
   ReplaceMatches(SelectedItems());
  }
}

void SpreadSheet::OnMatchRowHashTag()
{
 ImageParser *px;

 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;
 std::map<String, ImageParser *> unique;
 std::vector<HashTag> hashTags;

 if (m_SelectedRow == -1)
   return;
 
 px = m_Owner->Rows[m_SelectedRow]->Image;

 if (px->IsImport() == true)
  {
   App->Response(L"Row's image is an import, it's not added to the folder yet");
   return;
  }  

 if (px->Item() == nullptr)
  {
   App->Response(L"Row's image hasn't been saved to the database yet, close and re-open folder");
   return;
  }

 if (px->Item()->PictureHashTags.size() == 0)
  {
   App->Response(L"Row's image does not have any folder hashtags assigned to it");
   return;
  }

 for(const auto &ht : px->Item()->PictureHashTags)
   hashTags.push_back(ht.second);

 for(const auto &match : SelectedItems())
  {
   match->Image->Item()->ReplacePictureHashTags(hashTags);
  }

 ReplaceMatches(SelectedItems());
}
 
void SpreadSheet::OnRowGroup()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;


 if (m_SelectedRow>=0)
  {
   list.push_back(m_Owner->Rows[m_SelectedRow]->Image);
  }
 if (Wnd->MoveToExistingGroup(list) == true)
  {
   ReplaceSelectedRow();
  }
}

void SpreadSheet::OnRowHashTag()
{
 ImageParser *pic;
 String txt;
 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;
 std::map<String, ImageParser *> unique;
 std::vector<HashTag> hashTags;

 // build list of unique hashtag sets and the 1st imaage that contains them

 if (m_SelectedRow<0)
  {
   App->Response(L"Row not selected");
   return;
  }

 pic = m_Owner->Rows[m_SelectedRow]->Image;
 if (pic->IsImport() == true)
  {
   App->Response(L"Row's image is still an import, it needs to be added to the folder");
   return;
  }

 for(const auto &px : App->Pictures)
  {
   if (px.second->Item() != nullptr)
    {
     if (px.second->Item()->PictureHashTags.size() > 0)
      {
       px.second->Item()->SetPictureHashTagString();
       txt = px.second->Item()->PictureHashTagString();
       if (txt.Length() > 0)
        {
         if (unique.count(txt) == 0)
          {
           unique.insert(std::pair<String, ImageParser *>(txt, px.second));
          }
        }
      }
    }
  }

 // get list of the 1st imageparser containing the hashtags

 for(const auto &val : unique)
  {
   list.push_back(val.second);
  }

 HashTagSetSelectDlg dlg;
 if (dlg.Show(m_Owner, list) != DialogResult::OK)
   return;

 hashTags = dlg.HashTags();

 pic->Item()->ReplacePictureHashTags(hashTags);
  
 ReplaceSelectedRow();
}

void SpreadSheet::OnRowMatchGroup()
{
 ImageParser *px;
 String group;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 if (m_SelectedRow == -1)
   return;
 
 px = m_Owner->Rows[m_SelectedRow]->Matches[0]->Image;

 if (px->IsNumbered() == false)
  {
   App->Response(L"Selected row's 1st match is not a group member");
   return;
  }

 group = px->FilePrefix();

 list.push_back(m_Owner->Rows[m_SelectedRow]->Image);
  
 if (Wnd->MoveToExistingGroup(list, group) == true)
  {
   ReplaceSelectedRow();
  }
}

void SpreadSheet::OnRowMatchHashTag()
{
 ImageParser *px, *match;

 std::vector<TreeNode> nodes;
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;
 std::map<String, ImageParser *> unique;
 std::vector<HashTag> hashTags;

 if (m_SelectedRow == -1)
   return;
 
 px = m_Owner->Rows[m_SelectedRow]->Image;

 if (px->IsImport() == true)
  {
   App->Response(L"Row's image is an import, it's not added to the folder yet");
   return;
  }  

 if (px->Item() == nullptr)
  {
   App->Response(L"Row's image hasn't been saved to the database yet, close and re-open folder");
   return;
  }

 match = m_Owner->Rows[m_SelectedRow]->Matches[0]->Image;

 if (match->Item()->PictureHashTags.size() == 0)
  {
   App->Response(L"Row's 1st match image does not have any folder hashtags assigned to it");
   return;
  }

 for(const auto &ht : match->Item()->PictureHashTags)
   hashTags.push_back(ht.second);

  px->Item()->ReplacePictureHashTags(hashTags);
  
 ReplaceSelectedRow();
}

void SpreadSheet::OnBothNewGroup()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;


 if (m_SelectedRow>=0)
  {
   list.push_back(m_Owner->Rows[m_SelectedRow]->Image);
  }

 for(const auto &match : SelectedItems())
   list.push_back(match->Image);
 
 if (Wnd->MoveToNewGroup(list) == true)
  {
   ReplaceMatches(SelectedItems());
   ReplaceSelectedRow();
  }
}

void SpreadSheet::OnBothAssignHashTags()
{
 std::vector<ImageParser *> list;
 std::vector<SpreadSheetItem *> items;

 if (m_SelectedRow>=0)
  {
   list.push_back(m_Owner->Rows[m_SelectedRow]->Image);
  }

 for(const auto &match : SelectedItems())
   list.push_back(match->Image);
 
 HashTagSelectorDlg dlg;
 if (dlg.Show(m_Owner, Wnd->CurrentFolder(),HashTagSelectorDlg::eMode::PictureTags) != DialogResult::OK)
   return;

 for (const auto &px : SelectedItems())
   list.push_back(px->Image);
 
 for(const auto &px : list)
   px->Item()->ReplacePictureHashTags(dlg.PictureHashTags);

 ReplaceMatches(SelectedItems());  
 ReplaceSelectedRow();
}

void SpreadSheet::OnProperties()
{
 PicturePropertiesDlg dlg;
 ImageParser *px;

 if (m_SelectedRow < 0)
   return;

 if (m_RowImageSelected == true)
   px = m_Owner->Rows[m_SelectedRow]->Image;
 else
   px = m_Owner->Rows[m_SelectedRow]->Matches[m_SelectedCol]->Image;

 dlg.Show(this, px);
}

void SpreadSheet::OnDelete()
{
 std::vector<ImageParser *> list;
 std::vector<int> listIDs;
 ImageParser *px;
 String q;

 if (m_SelectedRow < 0)
   return;

 if (m_RowImageSelected == true)
  {
   px = m_Owner->Rows[m_SelectedRow]->Image;
   list.push_back(px);
   if (Delete(list) == true)
    {
     m_Owner->Rows[m_SelectedRow]->Deleted = true;
     m_Owner->Rows[m_SelectedRow]->Image = nullptr;
     m_Owner->Rows[m_SelectedRow]->Selected = false;
     m_SelectedRow = -1;
     m_SelectedCol = -1;
     m_RowImageSelected = false;
    }
  }
 else
  {
   for (const auto &match : SelectedItems())
    {
     px = match->Image;
     list.push_back(px);
    }
   if (Delete(list) == true)
    {
     for(const auto &match : SelectedItems())
      {
       match->Selected = false;
       match->Deleted = true;
       match->Image = nullptr;
       for(const auto &row : m_Owner->Rows)
        {
         if (row->ID == match->ID)
          {
           row->Deleted = true;  
           row->Image = nullptr;  // this is so the "row" item can't be selected and sort knows to skip it
          }
        }
      }
    }
  }
 Refresh();
}

bool SpreadSheet::Delete(std::vector<ImageParser *> const &list)
{
 Gdiplus::Bitmap *astrik;
 String q;
 int i, id;
 int x, y;

 q = App->Prose.Text(COMMON_MOVE_TO_RECYCLE);
 q += L":";

 i=0;
 for(const auto &p : list)
  {
   if (i>12)
    {
     q += L"\n";
     q += App->Prose.Text(COMMON_CUT_SHORT);
     break;
    }
   q += L"\n";
   q += p->FileName();
   i++;
  }

 if (App->Question(q, MB_OKCANCEL) != DialogResult::OK) return false;

 for(const auto &p : list)
  {
   if (p->Delete() == false)
    {
     App->Response(App->Prose.TextArgs(MAIN_DELETE_FAILED, p->FileName()));
     break;
    }
   else
    {
     id = p->ID();
     x = 2;
     y = 20;
     astrik = p->Thumb()->Clone(Rect(0,0, ImageParser::ThumbSize, ImageParser::ThumbSize), PixelFormat24bppRGB);
     Gdiplus::Graphics *g = Gdiplus::Graphics::FromImage(astrik);
     g->FillEllipse(m_White, Rect(x, y, 24, 24));
     g->FillEllipse(m_Red, Rect(x+2, y+2, 20, 20));
     m_ImageList.Replace(astrik, id);
     App->Pictures.erase(id);
     p->Dispose();
     delete p;
     m_DeleteList.insert(std::pair<int, int>(id, 1));
    }
  }

 m_Changed = true;
 return true;
}

void SpreadSheet::ReplaceMatches(std::vector<SpreadSheetItem *> const &matches)
{
 Gdiplus::Bitmap *astrik;
 float x,y;

 for(const auto &match : matches)
  {
   if (match->Moved == false)
    {
     x = 2;
     y = 20;
     astrik = match->Image->Thumb()->Clone(Rect(0,0, ImageParser::ThumbSize, ImageParser::ThumbSize), PixelFormat24bppRGB);
     Gdiplus::Graphics *g = Gdiplus::Graphics::FromImage(astrik);
     g->FillEllipse(m_White, Rect(x, y, 24, 24));
     g->FillEllipse(m_Green, Rect(x+2, y+2, 20, 20));
     m_ImageList.Replace(astrik, match->Image->ID());
     match->Moved = true;
    }
  }

 m_Changed = true;
 Refresh();
}

void SpreadSheet::ReplaceSelectedRow()
{
 std::vector<SpreadSheetRow *> eraser;
 SpreadSheetRow *row;
 Gdiplus::Bitmap *astrik;
 float x,y;

 if (m_SelectedRow < 0)
   throw L"Row not selected";
 
 row = m_Owner->Rows[m_SelectedRow];

 if (row->Moved == false)
  {
   x = 2;
   y = 20;
   astrik = row->Image->Thumb()->Clone(Rect(0,0, ImageParser::ThumbSize, ImageParser::ThumbSize), PixelFormat24bppRGB);
   Gdiplus::Graphics *g = Gdiplus::Graphics::FromImage(astrik);
   g->FillEllipse(m_White, Rect(x, y, 24, 24));
   g->FillEllipse(m_Green, Rect(x+2, y+2, 20, 20));
   m_ImageList.Replace(astrik, row->Image->ID());
   row->Moved = true;
  }

 m_Changed = true;
 Refresh();
}

void SpreadSheet::OnPaint(HDC hOuter)
{
 HFONT  hOldFont;
 HBITMAP hMemBmp;
 HDC hDC;
 RECT rct;
 String txt;
 double match;
 int startRow, startCol;
 int ri, rc, ci, cc;
 int cx, cy;
 int bw, bh;
 int x, y;

 ::GetClientRect(m_hWnd, &rct);
 
 bw = rct.right - rct.left;
 bh = rct.bottom - rct.top;

 hDC = ::CreateCompatibleDC(hOuter);
 hMemBmp = ::CreateCompatibleBitmap(hOuter, bw, bh);
 SelectBitmap(hDC, hMemBmp);

 cx = ImageParser::ThumbSize;
 cy = ImageParser::ThumbSize;

 startRow = m_VSPos;
 startCol = m_HSPos;

 rc = startRow + (Height() / ImageParser::ThumbSize) + 1;
 cc = startCol + (Width() / ImageParser::ThumbSize) + 1;

 ::FillRect(hDC, &rct, ::GetSysColorBrush(COLOR_WINDOW));

 hOldFont = SelectFont(hDC, m_Font);
 
 SetBkColor(hDC, ::GetSysColor(COLOR_WINDOW));
 SetTextColor(hDC, ::GetSysColor(COLOR_WINDOWTEXT));

 y = 0;
 for (ri=startRow; ri<=rc; ri++)
  {
   if (ri < m_Owner->Rows.size())
    {
     m_ImageList.Paint(hDC, m_Owner->Rows[ri]->ID, 1, y+1);
     if (m_Owner->Rows[ri]->Selected == true)
      {
       DrawRectangle(hDC, m_Pen, 0, y, ImageParser::ThumbSize+2, ImageParser::ThumbSize+2);
      }
     x = ImageParser::ThumbSize+2;
     for (ci=startCol; ci<=cc; ci++)
      {
       if (ci < m_Owner->Rows[ri]->Matches.size())
        {
         m_ImageList.Paint(hDC, m_Owner->Rows[ri]->Matches[ci]->ID, x+1, y+1);
         match = (double)m_Owner->Rows[ri]->Matches[ci]->Match;
         txt = String::Double(match*100.0,3,0);
         txt += L"%";
         ::SetRect(&rct, x+4, y+4, x+200, y+200);
         ::DrawText(hDC, txt.Chars(), txt.Length(), &rct, DT_SINGLELINE | DT_LEFT | DT_TOP);
         ::SetRect(&rct, x+3, y+3, x+200, y+200);
         ::DrawText(hDC, txt.Chars(), txt.Length(), &rct, DT_SINGLELINE | DT_LEFT | DT_TOP);
         if (m_Owner->Rows[ri]->Matches[ci]->Selected == true)
          {
           DrawRectangle(hDC, m_Pen, x, y, ImageParser::ThumbSize, ImageParser::ThumbSize);
          }
         x += ImageParser::ThumbSize;
         x += 2;
        }
      }
     y += ImageParser::ThumbSize;
     y += 2;
    }
  }
 SelectFont(hDC, hOldFont);

 ::BitBlt(hOuter, 0, 0, bw, bh, hDC, 0, 0, SRCCOPY);

 ::DeleteDC(hDC);
 DeleteBitmap(hMemBmp);
}

void SpreadSheet::mnuSortListName()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortFileName);
 Refresh();
}
void SpreadSheet::mnuSortListNameNmbr()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortNameNmbr);
 Refresh();
}
void SpreadSheet::mnuSortListWidth()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortWidth);
 Refresh();
}
void SpreadSheet::mnuSortListHeight()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortHeight);
 Refresh();
}
void SpreadSheet::mnuSortListWidthDivHeight()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortRatio);
 Refresh();
}
void SpreadSheet::mnuSortListRGBColors()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortColorRGB);
 Refresh();
}
void SpreadSheet::mnuSortListBorderColorAverage()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortEdgeAverage);
 Refresh();
}
void SpreadSheet::mnuSortListDateAdded()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortFileDate);
 Refresh();
}
void SpreadSheet::mnuSortListFileSize()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortFileSize);
 Refresh();
}
void SpreadSheet::mnuSortListGlobalHashTags()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortGlobalHashTags);
 Refresh();
}
void SpreadSheet::mnuSortListFolderHashTags()
{
 std::sort(m_Owner->Rows.begin(), m_Owner->Rows.end(), SpreadSheetRowSortFolderHashTags);
 Refresh();
}


///////////////////////////////////////////////////////

SpreadSheetDlg::~SpreadSheetDlg()
{
 for(const auto &row : Rows)
  {
   for(const auto &match : row->Matches)
    {
     delete match;
    }
   row->Matches.clear();
   delete row;
  }
 Rows.clear();
}

void SpreadSheetDlg::Show(AWnd *parent)
{
 ADialog::Show(IDD_BLANK, parent);
}

void SpreadSheetDlg::OnInitDialog()
{
 Sheet.SetOwner(this);
 Sheet.Create(this, Rect(0,0,100,100));
 m_VS.Create(this, ScrollBar::Orientation::Vert);
 m_HS.Create(this, ScrollBar::Orientation::Horz);
 
 Maximize();
 SizeChildren();
 
 ADialog::OnInitDialog();
}

void SpreadSheetDlg::SizeChildren()
{
 Rect r=ClientRect();
 int size = m_VS.Width();
 int range;
 int page;
 int maxHS;
 
 m_VS.SetRect(Rect(r.Width-size, 0, m_VS.Width(), r.Height - size));
 m_HS.SetRect(Rect(0, r.Height-size, r.Width - size, m_HS.Height()));

 Sheet.SetRect(Rect(0, 0, r.Width-m_VS.Width(), r.Height-m_HS.Height()));

 range = Rows.size();
 m_VS.SetRange(0, range);
 page = Sheet.ClientSize().Height/Sheet.ItemSize().Height;
 m_VS.SetPageAmount(page);

 maxHS = 0;
 for(const auto &row : Rows)
  {
   if (row->Matches.size() > maxHS)
     maxHS = row->Matches.size();
  }
 range = Rows.size()  - m_HS.Width() / ImageParser::ThumbSize;
 m_HS.SetRange(0, maxHS);
 page = Sheet.ClientSize().Width/Sheet.ItemSize().Width;
 m_HS.SetPageAmount(page);

}

WMR SpreadSheetDlg::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 WMR ret = WMR::Default;
 int pos;

 switch(message)
  {
   case WM_HSCROLL:
    {
     pos = m_HS.GetPosition(wParam); 
     Sheet.SetHSPos(pos); 
      m_HS.SetPosition(wParam, pos); 
      ret = WMR::Zero;
     } break;
   case WM_VSCROLL: 
    {
     pos = m_VS.GetPosition(wParam);
     Sheet.SetVSPos(pos); 
     m_VS.SetPosition(wParam, pos); 
     ret = WMR::Zero;
    } break;
   case WM_SIZE:    SizeChildren(); ret = WMR::Zero; break;
   case WM_CLOSE:   Close(DialogResult::OK); ret = WMR::Zero; break;
  }
 return ret;
}

void SpreadSheetDlg::Sort()
{
 for(const auto &row : Rows)
  {
   std::sort(row->Matches.begin(), row->Matches.end(), SpreadSheetMatchSort);
  }
 std::sort(Rows.begin(), Rows.end(), SpreadSheetRowSort);
}