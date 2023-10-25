#include "pch.h"
#include "App.h"

void GroupManageWnd::Show(AWnd *parent, ImageList *il, String const &groupName, std::vector<ImageParser *> const &images)
{
 m_Title = L"Manage Group \"";
 m_Title += groupName;
 m_Title += L"\"";
 m_ImageList = il;
 m_GroupName = groupName;
 m_Pictures = images;
 ADialog::Show(IDD_BLANK, parent);
}

void GroupManageWnd::OnInitDialog()
{
 ADialog::OnInitDialog();
 SetText(m_Title);

 m_TB.Create(this, 5);

 m_TB.AddItem(IDB_UP, IDB_UP, L"Move Picture Up");
 m_TB.AddItem(IDB_DOWN, IDB_DOWN, L"Move Picture Down");

 m_TB.AddSeparator();

 m_TB.AddItem(IDB_CUT, IDB_CUT, L"Cut Selected Pictures (Not To Clipboard)");
 m_TB.AddItem(IDB_MOVEBEFORE, IDB_MOVEBEFORE, L"Paste Cut Pictures Before Selection (Not From Clipboard)");
 m_TB.AddItem(IDB_MOVEAFTER, IDB_MOVEAFTER, L"Paste Cut Pictures After Selection (Not From Clipboard");
 m_TB.AddItem(IDB_CANCELMOVE, IDB_CANCELMOVE, L"Cancel Cut (Not From Clipboard)");

 m_TB.AddSeparator();

 m_TB.AddItem(IDB_DETAILS, IDB_DETAILS, L"Switch List To Details");
 m_TB.AddItem(IDB_ICONS, IDB_ICONS, L"Switch List To Icons");

 m_Split.Create(SPLIT_MAIN, this, SplitterWnd::SplitterOrientation::Vertical, 0.50);

 m_List.Create(this, LVS_REPORT, Rect(0,0,10,10));
 m_List.SetOwner(this);
 m_List.SetImageList(m_ImageList);
 m_List.SetOwner(this); // for middle mouse button picture viewing

 m_Pic.Create(this, Rect(20,0, 10, 10));

 m_Split.SetWindow1(&m_List);
 m_Split.SetWindow2(&m_Pic); 

 m_SB.AddAutoPane(StatusBarPane::Content::Text);
 m_SB.AddFixedPane(StatusBarPane::Content::Progress, 190);
 m_SB.CreateSB(this);

 Maximize();
 OnSize();

 EnableButtons();

 Reload(m_Pictures);
 m_List.SetSort(ImageParser::SortChoices::FileName, true);
 m_List.AutoSize(0, ListView::AutoSizes::Content); // need to show full path
}

void GroupManageWnd::EnableButtons()
{
 bool bEnable  = (m_listCut.size() > 0);

 m_TB.EnableButton(IDB_CUT, !bEnable);
 m_TB.EnableButton(IDB_MOVEBEFORE, bEnable);
 m_TB.EnableButton(IDB_MOVEAFTER, bEnable);
 m_TB.EnableButton(IDB_CANCELMOVE, bEnable);
}

void GroupManageWnd::Reload(std::vector<ImageParser *> const &list)
{
 m_Pic.SetItem(nullptr);
 m_List.Clear();
 for(const auto &pxi : list)
  {
   if (pxi->Item() != nullptr)
   m_List.Insert(pxi);
  }
}

Rect GroupManageWnd::ClientRect()
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


void GroupManageWnd::OnSize()
{
 Size cs;
 Rect r;

 if (m_TB.Handle() == 0) return; // not created yet

 cs = ClientSize();

 r = Rect(0,0, cs.Width, m_TB.Height());
 m_TB.SetRect(r);
 
 r = Rect(0, m_TB.Height(), cs.Width, cs.Height-(m_TB.Height() + m_SB.Height()));

 m_Split.CalcSizes(r); 

 m_SB.OnSize();
 
}

WMR GroupManageWnd::OnNotify(HWND hChild, int child, UINT code, LPARAM lParam)
{
 NMHDR *nm=(LPNMHDR)lParam;
 WMR ret=WMR::Zero;
 int ndx, id;

 ret = ADialog::OnNotify(hChild, child, code, lParam);

 switch(nm->code)
  {
   case LVN_ITEMCHANGING:
     ret = WMR::Zero;
     break;
   case LVN_ITEMCHANGED:
    {
     if (hChild == m_List.Handle() && m_List.NotifyIsOff()==false)
      {
       ndx = m_List.GetSelectedItem();
       if (ndx >= 0)
        {
         id = m_List.GetItemParam(ndx); 
         SetPictureViewer(App->Pictures[id]);
        }   
      }   
    } break;
  }
 return ret;
}

WMR GroupManageWnd::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 String msg;
 int id, ndx;

 switch(message)
  {
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
   case WM_LISTVIEW_MIDDLE:  // ListView middle mouse button clicked, view picture without affecting selected items
    {
     ndx = (int)lParam;
     if ((HWND)wParam == m_List.Handle())
      {
       id = m_List.GetItemParam(ndx);
       OnListViewMiddle(id);
      }
    } break;
   case WM_SIZE:
    {
     OnSize();
    } break;
   default: return ADialog::MessageHandler(hWnd, message, wParam, lParam);
  }
 return WMR::Zero;
}


WMR GroupManageWnd::OnContextMenu(HWND hChild, Point const &pt)
{
 AMenu menu;
 bool bEnable;

 menu.CreateContextMenu();
 
 menu.AddMenu(ID_GROUP_MANAGE_CUT);
 menu.Separator();
 menu.AddMenu(ID_GROUP_MANAGE_PASTE_BEFORE);
 menu.AddMenu(ID_GROUP_MANAGE_PASTE_AFTER);
 menu.Separator();
 menu.AddMenu(ID_GROUP_MANAGE_CANCEL);

 bEnable  = (m_listCut.size() > 0);

 if (bEnable == true)
   menu.SetEnabledState(ID_GROUP_MANAGE_CUT, false); // already have cut 

 if (bEnable == false)
  {
   menu.SetEnabledState(ID_GROUP_MANAGE_PASTE_BEFORE, false); // don't have cut 
   menu.SetEnabledState(ID_GROUP_MANAGE_PASTE_AFTER, false); // don't have cut 
   menu.SetEnabledState(ID_GROUP_MANAGE_CANCEL, false); // don't have cut 
 }

 menu.ShowContextMenu(this, pt);

 return WMR::One;
}

WMR GroupManageWnd::OnCommand(int btn, HWND hItem)
{
 switch(btn)
  {
   case IDB_DETAILS:
     m_List.ViewStyle(ListView::DisplayStyle::Details); 
     break;
   case IDB_ICONS:
     m_List.ViewStyle(ListView::DisplayStyle::LargeIcon); 
     break;
   case IDB_UP:         OnMoveUp(); break;
   case IDB_DOWN:       OnMoveDown(); break;
   case ID_GROUP_MANAGE_CUT:
   case IDB_CUT:        OnCut(); break;
   case ID_GROUP_MANAGE_PASTE_BEFORE:
   case IDB_MOVEBEFORE: OnMoveBefore(); break;
   case ID_GROUP_MANAGE_PASTE_AFTER:
   case IDB_MOVEAFTER:  OnMoveAfter(); break;
   case ID_GROUP_MANAGE_CANCEL:
   case IDB_CANCELMOVE: OnCancelMove(); break;
  }
 return WMR::One;
}

void GroupManageWnd::OnListViewMiddle(int id)
{
 ImageParser *px;

 if (App->Pictures.count(id) == 0) throw L"id not valid";

 px = App->Pictures[id];

 SlideDlg dlg(px->FullPath(), true);
 dlg.Show();
}

void GroupManageWnd::SetPictureViewer(ImageParser *pic)
{
 String msg;

 m_Pic.SetItem(pic);
 if (pic != nullptr)
  { 
   msg = pic->FileName();
   msg += L"   ";
   msg += String::Decimal(pic->Width());
   msg +=L" X ";
   msg += String::Decimal(pic->Height());
  }
 m_SB.SetText(0, msg);
}

bool GroupManageWnd::Reorder(std::vector<ImageParser *> const &list)
{
 ProgressBar pgb(&m_SB, 1);
 String f;
 int i;
 
 pgb.Max((int)list.size());
 i = 1;
 for(const auto &p : list)
  {
   f = m_GroupName;
   f += L"_";
   f += String::Decimal(i, 3);
   f += L".ord";
   if (p->RenameByFileToDir(f, p->Directory()) == false) return false; // straighten up the list
   pgb.Progress();
   i++;
  }

 pgb.Max((int)list.size());
 i = 1;
 for(const auto &p : list)
  {
   f = m_GroupName;
   f += L"_";
   f += String::Decimal(i, 3);
   f += L".jpg";
   if (p->RenameByFileToDir(f, p->Directory()) == false) return false; // back to jpg after straight
   pgb.Progress();
   i++;
  }

 return true;
}

void GroupManageWnd::OnMoveUp()
{
 int im;
 int pm, pi;

 ImageParser *mPic,*iPic;
 int i;

 if (m_List.SelectedItemsCount() == 0) { App->Response(L"Nothing selected"); return; }
 if (m_List.SelectedItemsCount() > 1) { App->Response(L"Move up and down only work on one item"); return; }


 im = m_List.GetSelectedItem(); 
 pm = m_List.GetItemParam(im);

 if (App->Pictures[m_List.GetItemParam(0)]->ID() == App->Pictures[pm]->ID())
  {
   App->Response(L"Selected item is at the top of the list"); 
   return; 
  }

 for (i = 0; i< m_List.Count(); i++)
  {
   pi = m_List.GetItemParam(i);
   if (App->Pictures[pi]->ID()  == App->Pictures[pm]->ID())
    {
     pi = m_List.GetItemParam(i - 1); // already determined that pm isn't at top of list
     mPic = App->Pictures[pm];
     iPic = App->Pictures[pi];
     mPic->Swap(iPic);
     m_List.RemoveItem(i);
     m_List.RemoveItem(i-1);
     m_List.InsertAt(i-1, mPic);
     m_List.InsertAt(i, iPic);
     m_List.SelectNone();
     m_List.SetSelectedItem(i-1);
     m_List.Refresh();
     return;
    }
  }
 throw L"Error Condition";
}

void GroupManageWnd::OnMoveDown()
{
 int im;
 int pm, pi;

 ImageParser *mPic,*iPic;
 int i;

 if (m_List.SelectedItemsCount() == 0) { App->Response(L"Nothing selected"); return; }
 if (m_List.SelectedItemsCount() > 1) { App->Response(L"Move up and down only work on one item"); return; }


 im = m_List.GetSelectedItem(); 
 pm = m_List.GetItemParam(im);

 if (App->Pictures[m_List.GetItemParam(m_List.Count()-1)]->ID() == App->Pictures[pm]->ID())
  {
   App->Response(L"Selected item is at the bottom of the list"); 
   return; 
  }

 for (i = 0; i< m_List.Count(); i++)
  {
   pi = m_List.GetItemParam(i);
   if (App->Pictures[pi]->ID()  == App->Pictures[pm]->ID())
    {
     pi = m_List.GetItemParam(i + 1); // already determined that pm isn't at bottom of list
     mPic = App->Pictures[pm];
     iPic = App->Pictures[pi];
     mPic->Swap(iPic);
     m_List.RemoveItem(i+1);
     m_List.RemoveItem(i);
     m_List.InsertAt(i, iPic);
     m_List.InsertAt(i+1, mPic);
     m_List.SelectNone();
     m_List.SetSelectedItem(i+1);
     m_List.Refresh();
     return;
    }
  }
 throw L"Error Condition";
}

void GroupManageWnd::OnCut()
{
 std::vector<int> indices;
 int id;

 if (m_List.SelectedItemsCount() == 0) { App->Response(L"No items are selected."); return; }

 indices = m_List.GetSelectedIndices();
 for(const auto &ndx : indices)
  {
   id = m_List.GetItemParam(ndx);
   #ifdef _DEBUG
   if (App->Pictures.count(id) == 0) throw L"m_List item param not ImageParser id";
   #endif
   m_listCut.push_back(App->Pictures[id]);
  }
 m_List.RemoveItems(indices);

 EnableButtons();
}

void GroupManageWnd::OnMoveBefore()
{
 WaitCursor wait(WaitCursor::WaitStyle::WaitLater);
 ImageParser  *target, *item;
 std::vector<ImageParser *> list;
 int i, id;

 if (m_List.SelectedItemsCount() == 0) { App->Response(L"Nothing selected"); return; }
 if (m_List.SelectedItemsCount() > 1)  { App->Response(L"Select a Single Item To Paste Before"); return; }

 wait.BeginWait();

 target = m_List.SelectedImage();

 for(i = 0; i < m_List.Count(); i++)
  {
   id = m_List.GetItemParam(i);
   item = App->Pictures[id];
   if (item->ID() == target->ID()) 
    {
     for(const auto &pc : m_listCut)
       list.push_back(pc); // move before
    }
   list.push_back(item);
  }
 if (Reorder(list) == false) return;

 Reload(list);
 m_listCut.clear();
 EnableButtons();

 wait.EndWait();
}

void GroupManageWnd::OnMoveAfter()
{
 WaitCursor wait(WaitCursor::WaitStyle::WaitLater);
 ImageParser  *target, *item;
 std::vector<ImageParser *> list;
 int i, id;

 if (m_List.SelectedItemsCount() == 0) { App->Response(L"Nothing selected"); return; }
 if (m_List.SelectedItemsCount() > 1)  { App->Response(L"Select a Single Item To Paste After"); return; }

 wait.BeginWait();

 target = m_List.SelectedImage();

 for(i = 0; i < m_List.Count(); i++)
  {
   id = m_List.GetItemParam(i);
   item = App->Pictures[id];
   list.push_back(item);
   if (item->ID() == target->ID()) 
    {
     for(const auto &pc : m_listCut)
       list.push_back(pc); // move before
    }
  }
 if (Reorder(list) == false) return;

 Reload(list);
 m_listCut.clear();
 EnableButtons();
 wait.EndWait();
}

void GroupManageWnd::OnCancelMove()
{
 Reload(m_Pictures);
 m_listCut.clear();
 EnableButtons();
}