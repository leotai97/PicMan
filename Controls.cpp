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
   if (code == NM_RCLICK)
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