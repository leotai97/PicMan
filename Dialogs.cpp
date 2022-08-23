#include "pch.h"
#include "App.h"

void HelpAboutDlg::Show(AWnd *parent)
{
 ADialog::Show(IDD_ABOUTBOX, parent);
}

void HelpAboutDlg::OnInitDialog()
{
 HWND hWnd;
 HICON hIcon;
 HRESULT  hr;

 hWnd = GetDlgItem(m_hWnd, IDC_STATIC_IMG);

 hr = ::LoadIconMetric(App->Instance(), MAKEINTRESOURCE(IDI_PICMAN) , LIM_LARGE, &hIcon);
 if (FAILED(hr) == false)
  { 
   ::SendMessage(hWnd, STM_SETICON, (WPARAM)hIcon, 0); 
   DestroyIcon(hIcon);
  }
}

WMR HelpAboutDlg::OnCommand(int childID, HWND hCtrl)
{
 if (childID == IDOK)
   Close(DialogResult::OK);
   
 return WMR::Zero;
}

//////////////////////////////////////////////////////////
LoginDlg::LoginDlg()
{
 std::vector<BYTE> bytes;
 String path, msg;
 CredFileRead file;

 path = App->AppLocalPath();
 path += L"\\PicMan.lpf";

 Server.Clear();
 Db.Clear();
 User.Clear();
 Password.Clear();
 RememberPassword = false;

 if (Utility::FileExists(path) == false)
   return;
 
 if (file.Open(path) == true)
  {
   Server = Utility::Decrypt(file.ReadBytes());
   Db = Utility::Decrypt(file.ReadBytes());
   User = Utility::Decrypt(file.ReadBytes());
   bytes = file.ReadBytes();  // "1" or "0"
   RememberPassword = ( bytes[0] == 1);
   if (RememberPassword == true)
     Password = Utility::Decrypt(file.ReadBytes());
   file.Close();
  }
 else
  {
   msg = App->Prose.Text(DLG_DB_PROP_FILE_FAIL);
   msg += L" ";
   msg += path;
   msg += L"\n";
   msg += file.Error();
   App->Response(msg);
  }
}

void LoginDlg::SaveSettings()
{
 String path, msg;
 std::vector<BYTE> bytes;
 CredFileWrite file;

 path = App->AppLocalPath();
 path += L"\\PicMan.lpf";

 if (file.Open(path) == false)
  {
   msg = App->Prose.Text(DLG_DB_PROP_FILE_FAIL_2);
   msg += path;
   msg += L"\n";
   msg += file.Error();
   App->Response(msg);
  }
 else
  {
   file.Write(Utility::Encrypt(Server));
   file.Write(Utility::Encrypt(Db));
   file.Write(Utility::Encrypt(User));
   if (RememberPassword == true)
    {
     bytes.push_back(1);
     file.Write(bytes);
     file.Write(Utility::Encrypt(Password));
     }
   else
    {
     bytes.push_back(0);
     file.Write(bytes);
    }
   file.Close();
  }
}

void LoginDlg::OnInitDialog()
{
 ADialog::OnInitDialog();
 SetItemText(IDC_LOGIN_EDIT_SERVER, Server);
 SetItemText(IDC_LOGIN_EDIT_DB, Db);
 SetItemText(IDC_LOGIN_EDIT_USER, User);
 SetItemText(IDC_LOGIN_EDIT_PWD, Password);
 SetCheckState(IDC_LOGIN_CHECK_REMEMBER, RememberPassword);

 SetItemText(IDC_LOGIN_LABEL_INFO, L"");
 
 m_Pwd.Attach(this, IDC_LOGIN_EDIT_PWD);
 m_Pwd.ShowPassword(false);

 if (Server.Length() > 0)
   SetItemFocus(IDC_LOGIN_EDIT_PWD);

}

DialogResult LoginDlg::Show()
{
 return ADialog::Show(IDD_LOGIN, Wnd);
}

WMR LoginDlg::OnCommand(int child, HWND hChild)
{
 WMR ret = WMR::Default;

 switch(child)
  {
   case IDCANCEL:
    {
     Close(DialogResult::Cancel);
     ret = WMR::Zero;
    } break;
   case IDC_LOGIN_CHECK_SHOWPWD:
    {
     bool yesno = GetCheckState(IDC_LOGIN_CHECK_SHOWPWD);
     m_Pwd.ShowPassword(yesno);
     ret = WMR::One;
    } break;
   case IDC_LOGIN_CHECK_REMEMBER:
    {
     bool yesno = GetCheckState(IDC_LOGIN_CHECK_REMEMBER);
     if (yesno == true)
       SetItemText(IDC_LOGIN_LABEL_INFO, App->Prose.Text(DLG_DB_CRED_STORED));
     else
       SetItemText(IDC_LOGIN_LABEL_INFO, L"");
     ret = WMR::One;
    } break;
   case IDOK:
    {
     Server=GetItemText(IDC_LOGIN_EDIT_SERVER);
     Db=GetItemText(IDC_LOGIN_EDIT_DB);
     User=GetItemText(IDC_LOGIN_EDIT_USER);
     Password=GetItemText(IDC_LOGIN_EDIT_PWD);
     RememberPassword=GetCheckState(IDC_LOGIN_CHECK_REMEMBER);
     if (Server.Length()>0 && Db.Length()>0 && User.Length()>0 && Password.Length()>0)
      {
       Close(DialogResult::OK);
       ret = WMR::Zero;
      }
     else
      {
       SetItemText(IDC_LOGIN_LABEL_INFO, App->Prose.Text(DLG_DB_VALIDATE));
       ret = WMR::One;
      }
    } break;
  }
 return ret;
}

String LoginDlg::Connection()
{
 String con = MySqlConnection::ConnectionString(Server, Db, User, Password);
 return con;
}

/////////////////////////////////////////////////////////////////////////
BaseItemMgrDlg::BaseItemMgrDlg(BaseItem *current)
{
 m_BaseItem=current;
}


DialogResult BaseItemMgrDlg::Show()
{
 return ADialog::Show(IDD_BASEITEM_MGR, Wnd);
}

void BaseItemMgrDlg::OnInitDialog()
{
 lstBase.Attach(this, IDC_BASEITEM_MGR_LIST);
 lstBase.FullRowSelect(true);
 lstBase.GridLines(true);

 lstBase.AddColumn(UI_COLUMN_BASE_NAME, ListViewColumn::ColumnAlign::Left, 245);
 lstBase.AddColumn(UI_COLUMN_DIRECTORY, ListViewColumn::ColumnAlign::Left, 252); 

 LoadBaseItems();
 ADialog::OnInitDialog();
}

WMR BaseItemMgrDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDC_BASEITEM_MGR_OK:   OnOK();     break;
   case IDCANCEL:              OnCancel(); break;
   case IDC_BASEITEM_MGR_ADD:  OnAdd();    break;
   case IDC_BASEITEM_MGR_DROP: OnDrop();   break;
   case IDC_BASEITEM_MGR_EDIT: OnEdit();   break;
   default: return WMR::Zero;
  }
 return WMR::One;
}

void BaseItemMgrDlg::OnListViewItemChanged(int child, NMLISTVIEW *lvn)
{
 if (child == IDC_BASEITEM_MGR_LIST)
  {
   if (( lvn->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
     m_BaseItem=App->Bases.at(lvn->lParam);
  }
}

void BaseItemMgrDlg::LoadBaseItems()
{
 ListViewItem lv;

 for(const auto &it : App->Bases)
  {
   lv=ListViewItem(it.second->DirName());
   lv.SubItems.push_back(it.second->DirPath());
   lv.Tag=it.second->ID();
   lstBase.Insert(lv);
  }
}

void BaseItemMgrDlg::OnOK()
{
 if (m_BaseItem == nullptr)
   App->Response(COMMON_NOTHING_SELECTED);
 else
  {
   if (m_BaseItem->GetDir() == true)
     Close(DialogResult::OK);  
  }
}

void BaseItemMgrDlg::OnCancel()
{
 Close(DialogResult::Cancel);
}

void BaseItemMgrDlg::OnAdd()
{
 App->Response(L"todo");
}

void BaseItemMgrDlg::OnDrop()
{
 App->Response(L"todo");
}

void BaseItemMgrDlg::OnEdit()
{
 App->Response(L"todo");
}

/////////////////////////////////////////////////////////////////////////

FolderOpenDlg::FolderOpenDlg()
{
 m_FolderItem=nullptr;
}

DialogResult FolderOpenDlg::Show()
{
 return ADialog::Show(IDD_FOLDER_OPEN, Wnd);
}

void FolderOpenDlg::OnInitDialog()
{
 ADialog::OnInitDialog(); 
 String txt;
 
 lstDir.Attach(this, IDC_FOLDER_OPEN_LIST);
 LoadHeaders(lstDir);

 SetText(App->Prose.TextArgs(DLG_FOLDER_UNDER, Wnd->CurrentBase()->DirPath()));

 m_Column = App->GetSettingInt(L"FolderDialogLastSort",0);
 m_AscDesc = App->GetSettingInt(L"FolderDialogLastSortDirection",1) != 0;

 LoadView(lstDir, Wnd->CurrentBase()->Folders);

 ListView_SortItems(lstDir.Handle(), &CompareFolders, Utility::AscendDescendColumn(m_AscDesc, m_Column));
}

void FolderOpenDlg::LoadHeaders(ListView &lstDir)
{
 lstDir.FullRowSelect(true);
 lstDir.GridLines(true);

 lstDir.AddColumn(UI_COLUMN_DIRECTORY, ListViewColumn::ColumnAlign::Left, 138);
 lstDir.AddColumn(UI_COLUMN_COUNT, ListViewColumn::ColumnAlign::Right, 60); 
 lstDir.AddColumn(UI_COLUMN_OPENED, ListViewColumn::ColumnAlign::Left, 60);
 lstDir.AddColumn(UI_COLUMN_LAST_ACTIVITY, ListViewColumn::ColumnAlign::Left, 174);
 lstDir.AddColumn(UI_COLUMN_ADDED, ListViewColumn::ColumnAlign::Left, 190);
}

WMR FolderOpenDlg::OnCommand(int child, HWND hChild)
{
 WMR ret = WMR::One;

 switch(child)
  {
   case IDC_FOLDER_OPEN_OK:   OnOK();     break;
   case IDCANCEL:             OnCancel(); break;
   case IDC_FOLDER_OPEN_DROP: OnDrop();   break;
   case IDC_FOLDER_OPEN_SCAN: OnScan();   break;
   default: ret = WMR::Zero;
  }
 return ret;
}

void FolderOpenDlg::OnOK()
{
 if (m_FolderItem == nullptr)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }
 Close(DialogResult::OK);
}

void FolderOpenDlg::OnCancel()
{
 Close(DialogResult::None);
}

void FolderOpenDlg::OnDrop()
{
 String txt; 

 if (m_FolderItem == nullptr)
  {
   App->Response(COMMON_NOTHING_SELECTED);
   return;
  }

 txt = App->Prose.TextArgs(DLG_FOLDER_DELETE, m_FolderItem->Folder(), String::Decimal((int)m_FolderItem->Pictures.size()));

 if (App->Question(txt, MB_YESNO) != DialogResult::Yes)
   return;

 m_FolderItem->Drop();
 lstDir.Clear();
 LoadView(lstDir, Wnd->CurrentBase()->Folders);
 ListView_SortItems(lstDir.Handle(), &CompareFolders, Utility::AscendDescendColumn(m_AscDesc, m_Column));
}

void FolderOpenDlg::OnScan()
{
 Wnd->ReScanDirectory();
 lstDir.Clear();
 LoadView(lstDir, Wnd->CurrentBase()->Folders);
 ListView_SortItems(lstDir.Handle(), &CompareFolders, Utility::AscendDescendColumn(m_AscDesc, m_Column));
}

void FolderOpenDlg::LoadView(ListView &lstDir, std::map<int, FolderItem *> const &fldrs)
{
 ListViewItem lv;
 FolderItem *f;
 DateTime dt;
 String s;
 
 for (const auto &it : fldrs)
  {
   f=it.second;

   lv=ListViewItem(f->Folder());
   lv.SubItems.push_back(String::Decimal((int)f->PictureCount()));
   if (f->LastOpened().IsMinValue()) 
     s = App->Prose.Text(COMMON_NEVER);
   else
     s = f->LastOpened().ToString(DateTime::Format::YMD);
   lv.SubItems.push_back(s);

   dt=f->LastActivity();
   if (dt.IsMinValue())
     lv.SubItems.push_back(App->Prose.Text(COMMON_NEVER));
   else
     lv.SubItems.push_back(dt.ToString(DateTime::Format::YMD));  
 
   lv.SubItems.push_back(f->Added().ToString(DateTime::Format::YMD));
 
   lv.Tag = f->ID();
 
   lstDir.Insert(lv);
  }
 
 lstDir.AutoSize(0, ListView::AutoSizes::Content);
 lstDir.AutoSize(1, ListView::AutoSizes::Header);
 lstDir.AutoSize(2, ListView::AutoSizes::Content);
 lstDir.AutoSize(3, ListView::AutoSizes::Content);
 lstDir.AutoSize(4, ListView::AutoSizes::Content);
}

void FolderOpenDlg::OnListViewColumnClick(int child, int col)
{
 int i;

 if (child != IDC_FOLDER_OPEN_LIST) throw L"not the list?";
 
 if (m_Column==col)
   m_AscDesc = !m_AscDesc;
 else
  {
   m_Column = col;
   m_AscDesc = true;
  }
 lstDir.Sort(m_Column, m_AscDesc, &CompareFolders);

 App->SaveSettingInt(L"FolderDialogLastSort", m_Column);
 if (m_AscDesc == true) i = 1; else i = 0;
 App->SaveSettingInt(L"FolderDialogLastSortDirection", i);
}

void FolderOpenDlg::OnListViewItemChanged(int child, NMLISTVIEW *plvn)
{
 if (( plvn->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
   m_FolderItem=Wnd->CurrentBase()->Folders.at(plvn->lParam); 
}

void FolderOpenDlg::OnDoubleClick(int child, NMITEMACTIVATE *item)
{
 if (m_FolderItem != nullptr)
  {
   Close(DialogResult::OK);
  }
}

int FolderOpenDlg::CompareFolders(LPARAM fldrIdA, LPARAM fldrIdB, LPARAM nAscDesCol)
{
 DateTime a1, a2;
 BaseItem *base = Wnd->CurrentBase();
 FolderItem *f1, *f2;
 bool AscDesc;
 int nCol;

 if ((nAscDesCol & 0xFF00) > 0) AscDesc = true; else AscDesc=false;
 nCol = (int)(nAscDesCol & 0x00FF);
 if (AscDesc==true)
  {
   f1=base->Folders.at(fldrIdA);
   f2=base->Folders.at(fldrIdB);
  }
 else
  {
   f1=base->Folders.at(fldrIdB);
   f2=base->Folders.at(fldrIdA);
  }

 switch(nCol)
  {
   case 0: 
    {
     return String::Compare(f1->Folder(), f2->Folder());
    } 
   case 1: 
    {
     if (f1->PictureCount() > f2->PictureCount()) return 1;
     if (f1->PictureCount() < f2->PictureCount()) return -1;
     return 0;   
    } 
   case 2: 
    {
     if (f1->LastOpened() > f2->LastOpened()) return 1;
     if (f2->LastOpened() < f2->LastOpened()) return -1;
     return 0;
    } 
   case 3: 
    {
     a1=f1->LastActivity();
     a2=f2->LastActivity();
     if (a1 > a2) return 1;
     if (a1 < a2) return -1;
     return 0;
    } 
   case 4: 
    {
     if (f1->Added() > f2->Added()) return 1;
     if (f1->Added() < f2->Added()) return -1;
    } break;
   default: throw L"bad column";
  }
 throw L"error";
}
/////////////////////////////////////////////////////////////////

HashTagEditDlg::HashTagEditDlg()
{
 EditTag = HashTag();
}

DialogResult HashTagEditDlg::Show(AWnd *parent)
{
 return ADialog::Show(IDD_HASHTAG_EDIT, parent);
}

DialogResult HashTagEditDlg::Show(AWnd *parent, HashTag const &ht)
{
 EditTag = ht;
 return ADialog::Show(IDD_HASHTAG_EDIT, parent);
}


void HashTagEditDlg::OnInitDialog()
{
 ADialog::OnInitDialog();

 m_OK.Attach(this, IDOK);
 m_Cancel.Attach(this, IDCANCEL);

 m_OK.SetIcon(IDI_CHECK, Size(16, 16));
 m_Cancel.SetIcon(IDI_CANCEL,Size(16,16));

 if (EditTag.ID() > 0)
   SetItemText(IDC_HASHTAG_EDITBOX, EditTag.Name());
}

WMR HashTagEditDlg::OnCommand(int child, HWND hChild)
{
 WMR ret = WMR::One;

 switch(child)
  {
   case IDCANCEL:
     Close(DialogResult::Cancel);
     break;
   case IDOK:
     OnOK();
     break;
   default: 
     ret = WMR::Zero;
  }
 return ret;
}

void HashTagEditDlg::OnOK()
{
 HashTag fht;
 HashTag ght;
 String txtName;
 String msg;

 txtName = GetItemText(IDC_HASHTAG_EDITBOX).Trim();

 if (txtName.Length() == 0) 
  {
   App->Response(DLG_HASHTAG_BLANK); 
   return;
  }

 if (txtName == EditTag.Name())
  {
   App->Response(DLG_HASHTAG_SAME);
   return;
  }

 switch(EditTag.TagType())
  {
   case HashTag::HashTagType::FolderTag:
    {
     fht = EditTag;
     if (fht.Folder()->UpdateFolderHashTag(fht, txtName) == false) 
      {
       App->Response(App->Prose.TextArgs(DLG_HASHTAG_EXISTS, txtName, fht.Folder()->Folder()));
       return;
      }
     Close(DialogResult::OK);
    } break;
   case HashTag::HashTagType::GlobalTag:
    {
     ght = EditTag;
     if (App->UpdateGlobalHashTag(ght, txtName) == false)
      {
       App->Response(DLG_HASHTAG_EXISTS_GLOBAL);
       return;
      }
     EditTag.SetName(txtName);
     Close(DialogResult::OK);
    } break;
   default: throw L"hashtag type not handled";
  }
}
// ///////////////////////////////////////////////////////////////////////////

HashTagDlg::HashTagDlg(PictureItem *pic)
{
 m_Picture = pic;
}

DialogResult HashTagDlg::Show(AWnd *parent)
{
 return ADialog::Show(IDD_HASHTAG, parent); 
}

void HashTagDlg::OnInitDialog()
{
 std::vector<HashTag> globals = App->GetGlobalHashTags();
 Rect tr, r;
 Size sz;

 tabTags.Attach(this, IDC_HASHTAG_TAB);
 tr = tabTags.GetTabRect();

 pnlGlobal.Create(&tabTags, this, IDRETRY, IDYES, HashTagSelectCtrl::enumSelectStyle::GlobalTags);
 tabTags.AddTab(L"Global Hash Tags", &pnlGlobal);
 for(const auto &ght : globals)
   pnlGlobal.HT.AddGlobalTag(ght);
 pnlGlobal.HT.ProcessTags(pnlGlobal.ClientSize().Width);

 pnlFolder.Create(&tabTags, this, IDRETRY, IDNO, HashTagSelectCtrl::enumSelectStyle::FolderTags);
 tabTags.AddTab(L"Folder Hash Tags", &pnlFolder);
 if (m_Picture != nullptr)
  {
   for(const auto &fi : m_Picture->ParentFolder()->FolderHashTags) 
     pnlFolder.HT.AddFolderTag(fi.second);
  }
 pnlFolder.HT.ProcessTags(pnlFolder.ClientSize().Width);
 pnlFolder.Visible(false);

 SelectHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::PictureCtrl, HashTagSelectCtrl::enumSelectStyle::GlobalFolderTags);
 SelectHashTagCtrl.Border(false);
 SelectHashTagCtrl.AutoSize = false;
 SelectHashTagCtrl.SetRect(GetItemRect(IDC_HASHTAG_PH));
 if (m_Picture != nullptr)
  {
   for(const auto &gi : m_Picture->GlobalHashTags)
     SelectHashTagCtrl.AddGlobalTag(gi.second);
   for(const auto &pi : m_Picture->PictureHashTags)
     SelectHashTagCtrl.AddPictureTag(pi.second);     
  }
 SelectHashTagCtrl.ProcessTags(ClientSize().Width - 4);

 tabTags.SetSize(tabTags.GetSize());

 ADialog::OnInitDialog();
}

WMR HashTagDlg::OnCommand(int child, HWND hChild)
{
 WMR ret = WMR::One;

 switch(child)
  {
   case IDYES:    btnGlobal_Click(); break;
   case IDNO:     btnFolder_Click(); break;
   case IDOK:     OnOK();            break;
   case IDCANCEL: OnCancel();        break;
   default:       ret = WMR::Zero;
  }
 return ret;
}

WMR HashTagDlg::MessageHandler(HWND hWndIn, UINT message, WPARAM wParam, LPARAM lParam)
{
 HashTag *ht;
 HWND     hWnd;

 switch(message)
  {
   case WM_HTSC_ITEMCLICK:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == pnlGlobal.HT.Handle()) OnGlobalClicked(ht); 
      if (hWnd == pnlFolder.HT.Handle()) OnFolderClicked(ht); 
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectClicked(ht); 
     } break;
   case WM_HTSC_ITEMCONTEXT:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectContext(ht); 
     } break;
   case WM_HTSC_FILTER_CHANGED:
   case WM_HTSC_NAME_CHANGED:
   case WM_HTSC_LIST_CHANGED:
   default: return ADialog::MessageHandler(hWndIn, message, wParam, lParam);
  }
 return WMR::One;
}

void HashTagDlg::OnGlobalClicked(HashTag *ht)
{
 HashTag ght;
 String msg;

 for(const auto &ght : SelectHashTagCtrl.GlobalMap)
  {
   if (ght.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::GlobalTag) 
    {
     App->Response(App->Prose.TextArgs(DLG_HASHTAG_GLOBAL_SELECTED, ht->Name()));
     return;
    }
  }
 SelectHashTagCtrl.AddGlobalTag(*ht);
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
}

void HashTagDlg::OnFolderClicked(HashTag *ht)
{
 HashTag pht;
 String msg;

 for(const auto &p : SelectHashTagCtrl.PictureMap)
  {
   if (p.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::FolderTag)
    {
     App->Response(App->Prose.TextArgs(DLG_HASHTAG_FOLDER_SELECTED, ht->Name()));
     return;
    }
  }
 pht=HashTag(ht->ID(), ht->Name(), m_Picture->ParentFolder(), HashTag::HashTagType::PictureTag);
 SelectHashTagCtrl.AddPictureTag(pht);
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
}
 
void HashTagDlg::OnFolderContext(HashTag *ht)
{ 
 String msg;

 for(const auto &f : SelectHashTagCtrl.FolderMap)
  {
   if (f.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::FolderTag)
    {
     App->Response(App->Prose.TextArgs(DLG_HASHTAG_FOLDER_SELECTED, ht->Name()));     return;
    }
  }
 SelectHashTagCtrl.AddFolderTag(*ht);
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
}

void HashTagDlg::OnSelectClicked(HashTag *ht)
{
 if (ht->TagType() == HashTag::HashTagType::GlobalTag)
  {
   SelectHashTagCtrl.RemoveGlobalHashTag(ht);
  }
 else
  {
   SelectHashTagCtrl.RemovePictureHashTag(ht);
  }
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
} 

void HashTagDlg::OnSelectContext(HashTag *ht)
{
 DialogResult r;  
 std::vector<HashTag> delList;
 bool found;
 
 HashTagSubDlg dlg(ht, m_Picture->ParentFolder(), this);
 r = dlg.Show(this);
 if (r == DialogResult::OK)
  {
   for(const auto &pht : dlg.PictureHashTags)
    {
     if (ht->SubTags.count(pht.ID()) == false)
       ht->SubTags.insert(std::pair<int, HashTag>(pht.ID(), pht));
    }
   for (const auto &pi : ht->SubTags)
    {
     found = false;
     for (const auto &pht : dlg.PictureHashTags)
      {
       if (pht.ID() == pi.second.ID())
        {
         found=true;
         break; // exit for loop
        }
      }
     if (found == false) 
       delList.push_back(pi.second);
    }
   for(const auto &dl : delList)
    {
     ht->SubTags.erase(dl.ID());
    }
   SelectHashTagCtrl.Refresh();
  }
}

void HashTagDlg::OnOK() // Handles btnOK.Click
{
 std::vector<HashTag> listG;
 std::vector<HashTag> listP;
 HashTag ght, pht;
 bool bFound;

 for(const auto &g : SelectHashTagCtrl.GlobalMap)
   m_Picture->AddGlobalHashTag(g.second);          // add to db
 for(auto &p : SelectHashTagCtrl.PictureMap)
   m_Picture->AddPictureHashTag(p.second);         // add to db incl sub tags
 
 for(const auto &gi : m_Picture->GlobalHashTags)
  {
   ght=gi.second;
   bFound = false;
   for(const auto &g : SelectHashTagCtrl.GlobalMap)
    {
     if (g.second.ID() == ght.ID()) 
      {
       bFound = true;
       break;   // exit for loop
      }
    }
   if (bFound == false) 
     listG.push_back(ght);
  }
 for(const auto &lg : listG)
   m_Picture->RemoveGlobalHashTag(lg);
 
 for(const auto &pi : m_Picture->PictureHashTags)
  {
   pht=pi.second;
   bFound = false;
   for(const auto &p : SelectHashTagCtrl.PictureMap)
    {
     if (p.second.ID() == pht.ID()) 
      {
       bFound = true;
       break;   // exit for loop
      }
    }
   if (bFound == false) 
     listP.push_back(pht);
  }
 for(const auto &lp : listP)
   m_Picture->RemovePictureHashTag(lp);

 Close(DialogResult::OK);; 
}

void HashTagDlg::OnCancel()
{
 Close(DialogResult::Cancel);
}

void HashTagDlg::btnGlobal_Click() 
{
 HashTag ght;
 String txt, msg;

 txt = pnlGlobal.Edit.GetText();
 txt = txt.Trim();

 if (txt.Length() == 0)
  {
   App->Response(DLG_HASHTAG_BLANK);
   return;
  }

 ght = App->AddGlobalHashTag(txt);
 if (ght.TagType() == HashTag::HashTagType::None)
  {
   App->Response(DLG_HASHTAG_EXISTS_GLOBAL);
   return;
  }
 pnlGlobal.HT.AddGlobalTag(ght);
 pnlGlobal.HT.ProcessTags(pnlGlobal.ClientSize().Width);
}  

void HashTagDlg::btnFolder_Click() 
{
 HashTag fht;
 String txt, msg;

 txt = pnlFolder.Edit.GetText();
 txt = txt.Trim();

 if (txt.Length() == 0)
  {
   App->Response(DLG_HASHTAG_BLANK);
   return;
  }
  
 fht = m_Picture->ParentFolder()->AddHashTag(txt);
 if (fht.TagType() == HashTag::HashTagType::None)
  {
   App->Response(App->Prose.TextArgs(DLG_HASHTAG_EXISTS, txt, m_Picture->ParentFolder()->Folder()));
   return;
  }

 pnlFolder.HT.AddFolderTag(fht);
 pnlFolder.HT.ProcessTags(pnlFolder.ClientSize().Width);
}

//////////////////////////////////////////////////////////////////////////////

HashTagSubDlg::HashTagSubDlg(HashTag *ht, FolderItem *fldr, AWnd *parent)
{

 EditTag = ht;
 Folder = fldr;
 ParentWnd = parent;
}

DialogResult HashTagSubDlg::Show(AWnd *parent)
{
 return ADialog::Show(IDD_HASHTAGSUB, parent);
}

void HashTagSubDlg::OnInitDialog()
{
 Rect r;
 Point pt;
 int h;

 SetText(EditTag->Name());

 m_OK.Attach(this, IDOK);
 m_Cancel.Attach(this, IDCANCEL);

 m_OK.SetIcon(IDI_CHECK, Size(16, 16));
 m_Cancel.SetIcon(IDI_CANCEL,Size(16, 16));

 m_Add.Attach(this, IDC_HASHTAGSUB_ADD);
 m_Text.Attach(this, IDC_HASHTAGSUB_EDIT);

 SubHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::FolderCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);
 for(const auto &fi : Folder->FolderHashTags)
  {
   if (fi.second.ID() != EditTag->ID()) 
    {
     SubHashTagCtrl.AddFolderTag(fi.second);
    }
  }

 SelectHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::PictureCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);
 for(const auto &pi : EditTag->SubTags)
      SelectHashTagCtrl.AddPictureTag(pi.second);

 SubHashTagCtrl.ProcessTags(m_Cancel.GetRect().X - 4);
 if (SubHashTagCtrl.GetHashTagHeight() > 0) 
   h = SubHashTagCtrl.GetHashTagHeight(); 
 else 
   h = m_Text.GetSize().Height;
 r.X = 2;
 r.Y = 2;
 r.Width = m_Cancel.Left() - 4;
 r.Height = h;
 SubHashTagCtrl.SetRect(r);
 SubHashTagCtrl.Border(false);

 m_Text.SetTop( r.Y + r.Height + 2);
 m_Add.SetTop( r.Y + r.Height + 2 );

 SelectHashTagCtrl.ProcessTags(ClientSize().Width - 4);
 
 r.X = 2;
 r.Y = m_Text.Top() + m_Text.Height() + 2;

 if (SelectHashTagCtrl.GetHashTagHeight() > 0) 
   h = SelectHashTagCtrl.GetHashTagHeight();
 else
   h = m_Text.Height();

 r.Width = ClientSize().Width - 4;
 r.Height = h;

 SelectHashTagCtrl.SetRect(r);
 SelectHashTagCtrl.Border(false);

 SetSize( Size(Width(), r.Y + r.Height + 37));

 ADialog::OnInitDialog();

}

WMR HashTagSubDlg::OnCommand(int id, HWND hCtrl)
{
 WMR ret = WMR::One;

 switch(id)
  {
   case IDOK:               OnOK();     break;
   case IDCANCEL:           OnCancel(); break;
   case IDC_HASHTAGSUB_ADD: OnAdd();    break;
   default: ret = WMR::Zero;
  }
 return ret;
}

WMR HashTagSubDlg::MessageHandler(HWND hWndIn, UINT message, WPARAM wParam, LPARAM lParam)
{
 HashTag *ht;
 HWND     hWnd;

 switch(message)
  {
   case WM_HTSC_ITEMCLICK:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == SubHashTagCtrl.Handle()) OnChoiceClicked(ht); 
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectClicked(ht); 
     } break;
   case WM_HTSC_ITEMCONTEXT:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectContext(ht); 
     } break;
   default: return ADialog::MessageHandler(hWndIn, message, wParam, lParam);
  }
 return WMR::One;
}

void HashTagSubDlg::OnChoiceClicked(HashTag *ht)
{
 String msg;
 Size sz;

 for(const auto &pht : SelectHashTagCtrl.PictureMap)
  {
   if (pht.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::FolderTag)
    {
     App->Response(App->Prose.TextArgs(DLG_HASHTAG_FOLDER_SELECTED, ht->Name()));
     return;
    }
  }
 SelectHashTagCtrl.PictureMap.insert(std::pair<int, HashTag>(ht->ID(), HashTag(ht->ID(), ht->ToString(), Folder, HashTag::HashTagType::PictureTag)));
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
 sz.Width = ClientSize().Width;
 sz.Height = SelectHashTagCtrl.GetHashTagHeight();
 SelectHashTagCtrl.SetSize(sz);
 sz.Width = Width();
 sz.Height = SelectHashTagCtrl.Top() + SelectHashTagCtrl.Height() + 37;  
 SetSize(sz);
}

void HashTagSubDlg::OnSelectClicked(HashTag *ht)
{
 Size sz;

 SelectHashTagCtrl.PictureMap.erase(ht->ID());
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);
 sz.Width = Width();
 sz.Height = SelectHashTagCtrl.Top() + SelectHashTagCtrl.Height() + 37;  
 SetSize(sz);
} 

void HashTagSubDlg::OnOK() // Handles btnOK.Click
{
 PictureHashTags.clear();

 for(const auto &pht : SelectHashTagCtrl.PictureMap)
   PictureHashTags.push_back(pht.second);

 Close(DialogResult::OK); 
}

void HashTagSubDlg::OnCancel()
{
 Close(DialogResult::Cancel);
}

void HashTagSubDlg::OnAdd()
{
 HashTag sfht;
 String txt;
 Size sz;
 
 txt = m_Text.GetText().Trim();

 if (txt.Length() == 0)
  {
   App->Response(DLG_HASHTAG_BLANK);
   return;
  }

 for(const auto &fht : Folder->FolderHashTags)
  {
   if (String::Compare(fht.second.Name(), txt) == 0) 
    {
     sfht = fht.second;
     break;
    }
  }

 if (sfht.TagType() != HashTag::HashTagType::None)
  {
   for(const auto &pht : SelectHashTagCtrl.PictureMap)
    {
     if (pht.second.ID() == sfht.ID()) 
       return; // tag already present and added to select list
    }
  }
 else
  {
   sfht = Folder->AddHashTag(txt);
   if (sfht.TagType() == HashTag::HashTagType::None) throw L"add folder hashtag failed";
   SubHashTagCtrl.FolderMap.insert(std::pair<int, HashTag>(sfht.ID(),sfht));
   SubHashTagCtrl.ProcessTags(m_OK.Left() - 4);
  }
 
 m_Text.SetText(L"");

 SelectHashTagCtrl.PictureMap.insert(std::pair<int,HashTag>(sfht.ID(), HashTag(sfht.ID(), sfht.ToString(), Folder, HashTag::HashTagType::PictureTag)));
 SelectHashTagCtrl.ProcessTags(ClientSize().Width);

 sz.Width = Width();
 sz.Height = SelectHashTagCtrl.Top() + SelectHashTagCtrl.Height() + 37;  
 SetSize(sz);

}

void HashTagSubDlg::OnKeyDown(int child, KeyEventArgs const &e) // Handles txtAdd.KeyDown
{
 if (child != IDC_HASHTAGSUB_EDIT) return;

 if (e.KeyCode == Keyboard::Enter) 
  {
   OnAdd();
  }
}

void HashTagSubDlg::OnSelectContext(HashTag *ht)
{
 HashTagSubDlg dlg(ht, EditTag->Folder(), nullptr);
 DialogResult r;
 std::vector<HashTag> delList;
 bool found;

 r = dlg.Show(this);
 if (r == DialogResult::OK) 
  {
   for(const auto &pht : dlg.PictureHashTags)
    {
     if (ht->SubTags.count(pht.ID()) == false)
       ht->SubTags.insert(std::pair<int, HashTag>(pht.ID(), pht)); // add HTs that were selected in dlg, they were added
    }
   for(const auto &pht : ht->SubTags)
    {
     found = false;
     for(const auto &cht : dlg.PictureHashTags)
      {
       if (cht.ID() == pht.second.ID())
         found = true;
      }
     if (found == false)
       delList.push_back(pht.second);      // remove HTs that are no longer present in the dlg, they were removed
    }
   for(const auto &pht : delList)
     ht->SubTags.erase(pht.ID());
   SelectHashTagCtrl.Refresh();
  }
}

///////////////////////////////////////////////////////////////////////////////////
SlideDlg::SlideDlg(String const &single, bool darkBackground = true)
{
 m_listImages.push_back(single);
 m_nTotal = 1;
 PrepareImage(single);
 if (m_bmpNextImage != nullptr)
  {
   m_BW = m_bmpNextImage->GetWidth();
   m_BH = m_bmpNextImage->GetHeight();
   m_bmpNextImage->GetHBITMAP(Color::Black, &m_hBitmap);
   delete m_bmpNextImage;
   m_bmpNextImage=nullptr;
  }

 m_Single = true;
 m_Tolerance=0;
 m_MouseMove = true;
 m_DarkBackground = darkBackground;

 m_Delay = 1.0F;

 m_hFont = 0;
 m_hBG = 0;
}

SlideDlg::SlideDlg(std::vector<String> const &listImg, float fDelaySeconds, bool bMouseMove, bool darkBackground = true)
{ 
 m_Delay=fDelaySeconds;
 m_MouseMove=bMouseMove;
 m_Single = false;
 m_DarkBackground = darkBackground;
 m_nCurrent = 1;                       // 1st image is already loaded

 if (listImg.size()>0)
  {
   PrepareImage(listImg[0]);
   m_BW = m_bmpNextImage->GetWidth();
   m_BH = m_bmpNextImage->GetHeight();
   m_bmpNextImage->GetHBITMAP(Color::Black, &m_hBitmap);
   delete m_bmpNextImage;
   m_bmpNextImage=nullptr;
  }

 if (listImg.size()>1)
  {
   PrepareImage(listImg[1]); // mimic how WM_TIMER works 
  }

 m_listImages = listImg;
 m_nTotal = (int)m_listImages.size();

 m_hFont = 0;
 m_hBG = 0;

}

SlideDlg::~SlideDlg()
{
 if (m_hBitmap != 0)  ::DeleteObject(m_hBitmap);
 if (m_bmpNextImage != nullptr) delete m_bmpNextImage;

 if (m_hFont != 0) ::DeleteObject(m_hFont);
 if (m_hBG != 0)   ::DeleteObject(m_hBG);
}

void SlideDlg::Show()
{
 ADialog::Show(IDD_BORDERLESS, 0);
}

void SlideDlg::OnInitDialog()
{
 int delay;

 ::ShowCursor(false);
 ::SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS); // turn off screen saver
 ::ShowWindow(m_hWnd, SW_MAXIMIZE);

 m_hFont = ::CreateFont(30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Times New Roman");

 if (m_DarkBackground == true)
   m_hBG = ::CreateSolidBrush(RGB(0,0,0));
 else
   m_hBG = ::CreateSolidBrush(RGB(255,255,255));

 delay = (int)(m_Delay * 1000.0F);  // check count in case list is empty

 if (::SetTimer(m_hWnd, ID_TIMER_1, 2000, nullptr) == 0) throw L"timer failed";
 if (::SetTimer(m_hWnd, ID_TIMER_2, delay, nullptr) == 0) throw L"timer failed";
 
 Refresh();
}

void SlideDlg::Close(DialogResult r)
{
 ::SetThreadExecutionState(ES_CONTINUOUS); // re-enable screen saver
 ::ShowCursor(true);
 ::KillTimer(m_hWnd, ID_TIMER_1);
 ::KillTimer(m_hWnd, ID_TIMER_2);
 ADialog::Close(r);
}

void SlideDlg::OnMouseDown(MouseEventArgs const &m)
{
 if (m_Single == true)
   Close(DialogResult::OK);
}

void SlideDlg::OnMouseMove(MouseEventArgs const &m)
{
 if (m_Single == true || m_MouseMove==false)
  return;

 // If the mouse moves any more then 15 points close window,after 2 seconds tolerance is reset

 m_Tolerance += 1;
 if (m_Tolerance > 40)
   Close(DialogResult::OK);
}

void SlideDlg::OnKeyDown(KeyEventArgs const &k)
{
 if (m_Single == true) 
   Close(DialogResult::OK);
    
 if (k.KeyCode == Keyboard::Space)
  {
   m_blnPause=!m_blnPause;
   Refresh();
  }
 else  
  {
   Close(DialogResult::OK);
  }
}

WMR SlideDlg::OnPaint()
{
 PAINTSTRUCT ps;
 HDC hDC;
 RECT rct;

 String strDisp;
 double dRi, dRs;
 int nIW, nIH;
 int nSW, nSH;
 int nFX, nFY, nFW, nFH;

 hDC = ::BeginPaint(m_hWnd, &ps);

 ::SetRect(&rct, 0, 0, ClientSize().Width, ClientSize().Height); 
 ::FillRect(hDC, &rct, m_hBG); 

 strDisp.Clear();

 if (m_hBitmap == 0) 
  {
   if (m_strLastImage.Length() == 0)
     strDisp = App->Prose.Text(COMMON_NOTHING_SELECTED);
   else
    {
     strDisp = App->Prose.Text(COMMON_FILE_NOT_FOUND);
     strDisp += L" ";
     strDisp += m_strLastImage;
    }
  } 
 else
  {
   nIW = m_BW;
   nIH = m_BH;
   nSW = ClientSize().Width;
   nSH = ClientSize().Height;

   dRi = (double)nIW / (double)nIH;
   dRs = (double)nSW / (double)nSH;

   if (dRi > dRs)
    {
     nFH = (int)(nSW * nIH / nIW);
     nFX = 0;
     nFY = (int)(nSH / 2) - (int)(nFH / 2);
     nFW = nSW;
    }
   else
    {
     nFW = (int)(nSH * dRi);
     nFY = 0;
     nFX = (int)(nSW / 2) - (int)(nFW / 2);
     nFH = nSH;
    }

   HDC hMem = ::CreateCompatibleDC(hDC);
   HBITMAP hOldB = SelectBitmap(hMem, m_hBitmap);
   ::SetStretchBltMode(hDC, COLORONCOLOR); //important! for smoothness
   ::StretchBlt(hDC, nFX, nFY, nFW, nFH, hMem, 0, 0, m_BW, m_BH, SRCCOPY); 
   SelectBitmap(hMem, hOldB);
   ::DeleteDC(hMem);
  }
 if (m_blnPause == true)
  {
   if (m_strLastImage != L"")
    { 
     strDisp = m_strLastImage;
     strDisp += L" ";
     strDisp += String::Decimal(m_nCurrent);
     strDisp += L" ";
     strDisp += App->Prose.Text(COMMON_OF);
     strDisp += L" ";
     strDisp += String::Decimal(m_nTotal);
    }
   else
     strDisp = m_strLastImage;
  }
 if (strDisp.Length() > 0)
  {
   HFONT hOld = SelectFont(hDC, m_hFont);
   ::SetBkMode(hDC, TRANSPARENT);

   ::SetRect(&rct, 34, 66, ClientSize().Width, ClientSize().Height);
   ::SetTextColor(hDC, RGB(0, 0, 128));
   ::DrawText(hDC, strDisp.Chars(), strDisp.Length(), &rct, DT_LEFT | DT_TOP| DT_SINGLELINE);

   ::SetRect(&rct, 33, 65, ClientSize().Width, ClientSize().Height);
   ::SetTextColor(hDC, RGB(255, 255, 0));
   ::DrawText(hDC, strDisp.Chars(), strDisp.Length(), &rct, DT_LEFT | DT_TOP| DT_SINGLELINE); 
   SelectFont(hDC, hOld);
  }

 ::EndPaint(m_hWnd, &ps);
 return WMR::One;
}

void SlideDlg::OnTimer(int timerID)
{
 String strNext;

 if (m_Single == true)
   return;

 if (timerID == ID_TIMER_1)
  {
   m_Tolerance = 0; // reset mouse move tolerance factor
   return;
  }

 if (timerID != ID_TIMER_2) throw L"Unknown timer";
  
 if (m_listImages.size() == 0) return;

 if (m_blnPause == true) return;

 if ( m_hBitmap != 0)
  {
   DeleteObject(m_hBitmap);
   m_hBitmap = 0;
  }

 m_BW = m_bmpNextImage->GetWidth();
 m_BH = m_bmpNextImage->GetHeight();
 m_bmpNextImage->GetHBITMAP(Color::Black, &m_hBitmap);
 delete m_bmpNextImage;
 Refresh();

 m_strLastImage = m_listImages[m_nCurrent];
 m_nCurrent += 1;

 if (m_nCurrent == (int)m_listImages.size()) 
   m_nCurrent = 0;

 if (m_nCurrent+1==m_listImages.size())
   strNext=m_listImages[0];
 else 
   strNext=m_listImages[m_nCurrent+1];

 PrepareImage(strNext); 
}

void SlideDlg::PrepareImage(String const &file)
{
 try
  {
   m_bmpNextImage=new Bitmap(file.Chars());
  }
 catch(...) 
  {
   m_bmpNextImage=nullptr;
  }
}

//////////////////////////////////////////////////////

void PicturePropertiesDlg::Show(AWnd *parent, ImageParser *px)
{
 m_Pic = px;
 ADialog::Show(IDD_PICTURE_PROPS, parent);
}

void PicturePropertiesDlg::OnInitDialog()
{
 String na;

 na = App->Prose.Text(COMMON_NA);

 SetItemText(IDC_PICPROPS_FILE_DIR, m_Pic->Directory());
 SetItemText(IDC_PICPROPS_FILE_NAME, m_Pic->FileName());
 if (m_Pic->IsNumbered() == true)
  {
   SetItemText(IDC_PICPROPS_FILE_PART, m_Pic->FilePrefix());
   SetItemText(IDC_PICPROPS_FILE_NMBR, String::Decimal(m_Pic->FileNumber()));
  }
 else
  {
   SetItemText(IDC_PICPROPS_FILE_PART, na);
   SetItemText(IDC_PICPROPS_FILE_NMBR, na);
  }
 SetCheckState(IDC_PICPROPS_FILE_MC, m_Pic->Monochrome());
 SetItemText(IDC_PICPROPS_FILE_WIDTH, String::Decimal(m_Pic->Width()));
 SetItemText(IDC_PICPROPS_FILE_HEIGHT, String::Decimal( m_Pic->Height()));
 SetItemText(IDC_PICPROPS_FILE_DATE, m_Pic->FileDate().ToString(DateTime::Format::MDYHMS));

 if (m_Pic->Item() != nullptr)
  {
   SetItemText(IDC_PICPROPS_DB_BASE_ID, String::Decimal(m_Pic->Item()->ParentFolder()->ParentBase()->ID()));
   SetItemText(IDC_PICPROPS_DB_BASE_NAME, m_Pic->Item()->ParentFolder()->ParentBase()->DirName());
   SetItemText(IDC_PICPROPS_DB_BASE_PATH, m_Pic->Item()->ParentFolder()->ParentBase()->DirPath());
   SetItemText(IDC_PICPROPS_DB_FLDR_ID, String::Decimal(m_Pic->Item()->ParentFolder()->ID()));
   SetItemText(IDC_PICPROPS_DB_FLDR_NAME, m_Pic->Item()->ParentFolder()->Folder());
   SetItemText(IDC_PICPROPS_DB_PIC_ID, String::Decimal(m_Pic->Item()->ID()));
   SetItemText(IDC_PICPROPS_DB_PIC_NAME, m_Pic->Item()->FileName());
   SetItemText(IDC_PICPROPS_DB_ADDED, m_Pic->Item()->DateAdded().ToString(DateTime::Format::MDYHMS));
  }
 else
  {
   SetItemText(IDC_PICPROPS_DB_BASE_ID, na);
   SetItemText(IDC_PICPROPS_DB_BASE_NAME, na);
   SetItemText(IDC_PICPROPS_DB_BASE_PATH, na);
   SetItemText(IDC_PICPROPS_DB_FLDR_ID, na);
   SetItemText(IDC_PICPROPS_DB_FLDR_NAME, na);
   SetItemText(IDC_PICPROPS_DB_PIC_ID, na);
   SetItemText(IDC_PICPROPS_DB_PIC_NAME, na);
   SetItemText(IDC_PICPROPS_DB_ADDED, na);
  }
 ADialog::OnInitDialog();
}

WMR PicturePropertiesDlg::OnCommand(int child, HWND hChild)
{
 if (child == IDOK) Close(); 
 return WMR::One;
}

/////////////////////////////////////////////////////////////////

DirDlg::DirDlg(String const &strDir, enumWhichDir eType)
{
 String msg;

 switch(eType)
  {
   case enumWhichDir::ScreenSaver:   
     m_Title = App->Prose.Text(DLG_DIR_BASE_PATH); 
     break;
   case enumWhichDir::WorkingDir:
   case enumWhichDir::GenericFolder: 
     m_Title = App->Prose.Text(DLG_DIR_GENERIC_PATH);
     break;
   case enumWhichDir::NewFolder:     
      m_Title = App->Prose.TextArgs(DLG_DIR_NEW_FOLDER, strDir);
      break;
  }
 m_strDir=strDir;
 m_strDirOut=L"";
 m_eType=eType;
}

DialogResult DirDlg::Show(AWnd *parent)
{
 return ADialog::Show(IDD_DIR, parent);
}

void DirDlg::OnInitDialog()
{
 SetText(m_Title);
 SetItemText(IDC_DIR_EDIT, m_strDir);
}

WMR DirDlg::OnCommand(int id, HWND hCtrl)
{
 switch(id)
  {
   case IDOK:        OnOK(); break;
   case IDCANCEL:    OnCancel(); break;
   case IDC_DIR_BTN: OnDir(); break;
  }
 return WMR::One;
}

void DirDlg::OnOK()
{
 String strDir;
 String msg, txt;

 txt = GetItemText(IDC_DIR_EDIT).Trim();

 switch(m_eType)
  {
   case enumWhichDir::NewFolder:
     if (txt.Length() == 0) { App->Response(DLG_DIR_FOLDER_BLANK); return; }
     strDir = m_strDir;
     strDir += L"\\";
     strDir += txt;
     m_strDirOut = txt;
     break;
   case enumWhichDir::ScreenSaver:
   case enumWhichDir::WorkingDir:
     if (txt.Length()==0) { App->Response(DLG_DIR_BLANK); return; }
     strDir =m_strDir;
     strDir += L"\\";
     strDir +=txt;
     if (Utility::DirectoryExists(strDir) == false)
      {
       msg = L"Directory ";
       msg += strDir;
       msg += L" does not exist.";
       App->Response(msg);
       return;
      }
     m_strDirOut = txt;
     break;
   case enumWhichDir::GenericFolder:
     if (txt.Length() == 0) { App->Response(DLG_DIR_BLANK); return; }
     if (Utility::DirectoryExists(txt) == false) 
      {
       App->Response(App->Prose.TextArgs(COMMON_DIR_EXIST, txt));
       return; 
      }
     m_strDirOut = txt;
     break;
  }
 Close(DialogResult::OK);
}
 
void DirDlg::OnCancel()
{
 Close(DialogResult::Cancel);
}

void DirDlg::OnDir()
{
 String msg, s, ss;

 s = OpenFileDlg::ChooseFolder(m_hWnd, GetText());
 if (s.Length() == 0)
   return;

 if (s.Length() < m_strDir.Length()) 
  { 
   App->Response(App->Prose.TextArgs(DLG_DIR_NOT_UNDER, m_strDir)); 
   return; 
  }

 ss = s.Substring(0, m_strDir.Length()); 
 if (String::Compare(ss, m_strDir)!=0) 
  {
   App->Response(App->Prose.TextArgs(DLG_DIR_NOT_UNDER_2, m_strDir)); 
   return; 
  }
 
 SetItemText(IDC_DIR_EDIT,  s.Substring(m_strDir.Length() + 1));
}

//////////////////////////////////////////////////////////

HashTagMaintDlg::HashTagMaintDlg()
{
 m_Folder = nullptr;
}

HashTagMaintDlg::HashTagMaintDlg(FolderItem *fldr)
{
 m_Folder = fldr;
}

void HashTagMaintDlg::Show(AWnd *parent)
{
 ADialog::Show(IDD_HASHTAG_MAINT, parent);
}

void HashTagMaintDlg::OnInitDialog()
{
 Rect r, r2;

 ADialog::OnInitDialog();

 r = GetItemRect(IDC_HASHTAG_MAINT_EDIT);
 m_HTWidth = ClientSize().Width-8;
 m_HTHeight = ClientSize().Height - (r.Y + r.Height + 8);

 if (m_Folder != nullptr)
  {
   m_Folder->MapTagUse(); 
   m_hashCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::SelectCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);
   for(const auto &fht : m_Folder->FolderHashTags)
      m_hashCtrl.FolderMap.insert(fht);
  }
 else
  {
   m_hashCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::SelectCtrl, HashTagSelectCtrl::enumSelectStyle::GlobalTags);
   for(const auto &fht : App->GlobalHashTags)
      m_hashCtrl.GlobalMap.insert(fht);
  }
  m_hashCtrl.AutoSize = false;
  m_hashCtrl.ProcessTags(m_HTWidth);

 r2.X = 4;
 r2.Y = r.Y + r.Height + 4;
 r2.Width = m_HTWidth;
 r2.Height = m_HTHeight;
   
 m_hashCtrl.SetRect(r2);
 m_hashCtrl.ShowIfNotUsed = true;
}

WMR HashTagMaintDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDC_HASHTAG_MAINT_ADD: OnAdd(); break;
  }
 return WMR::One;
}

WMR HashTagMaintDlg::MessageHandler(HWND hWndIn, UINT message, WPARAM wParam, LPARAM lParam)
{
 HashTag *ht;
 HWND     hWnd;

 switch(message)
  {
   case WM_HTSC_ITEMCLICK:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == m_hashCtrl.Handle()) ItemClicked(ht); 
     } break;
   case WM_HTSC_ITEMCONTEXT:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
     } break;
   case WM_HTSC_FILTER_CHANGED:
   case WM_HTSC_NAME_CHANGED:
   case WM_HTSC_LIST_CHANGED:
   default: return ADialog::MessageHandler(hWndIn, message, wParam, lParam);
  }
 return WMR::One;
}

void HashTagMaintDlg::ItemClicked(HashTag *ht)
{
 if (m_SelectedTag == nullptr)
   m_SelectedTag = ht;
}

void HashTagMaintDlg::OnAdd()
{
 HashTag ht;
 String txt, msg;

 txt = GetItemText(IDC_HASHTAG_MAINT_EDIT).Trim();
 
 if (txt.Length() == 0) { App->Response(DLG_HASHTAG_BLANK); return; }

 if (m_Folder == nullptr)
  {
   ht = App->AddGlobalHashTag(txt);
   if (ht.TagType() == HashTag::HashTagType::None) 
    {
     App->Response(DLG_HASHTAG_EXISTS_GLOBAL);
     return; 
    }
   m_hashCtrl.GlobalMap.insert(std::pair<int, HashTag>(ht.ID(), ht));
  }
 else
  {
   ht = m_Folder->AddHashTag(txt);
   if (ht.TagType() == HashTag::HashTagType::None) 
    {
     App->Response(App->Prose.TextArgs(DLG_HASHTAG_EXISTS, txt, m_Folder->Folder()));
     return; 
    }
   m_hashCtrl.FolderMap.insert(std::pair<int, HashTag>(ht.ID(), ht));
  }
 m_hashCtrl.ProcessTags(m_HTWidth);
 SetItemText(IDC_HASHTAG_MAINT_EDIT, L"");
}

////////////////////////////////////////////////////////////

GroupNameDlg::GroupNameDlg(String const &suggestion, bool blankOK)
{
 m_strSuggest = suggestion;
 m_blnBlankOK = blankOK;
}

DialogResult GroupNameDlg::Show(AWnd *parent, String const &title)
{
 m_Title = title;
 return ADialog::Show(IDD_GROUP_NAME, parent);
}

void GroupNameDlg::OnInitDialog()
{
 ADialog::OnInitDialog();
 if (m_blnBlankOK == false)
  {
   SetItemVisible(IDC_GROUP_NAME_MSG, false);
  }
 SetItemText(IDC_GROUP_NAME_EDIT, m_strSuggest);
 if (m_Title.Length() > 0)
   SetText(m_Title);
}

WMR GroupNameDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDOK:
    {
     OnOK();
    } break;
   case IDCANCEL:
    {
     Close(DialogResult::Cancel);
    } break;
  }
 return WMR::One;
}

void GroupNameDlg::OnOK()
{
 String txt = GetItemText(IDC_GROUP_NAME_EDIT).Trim();

 if (m_blnBlankOK == false) 
  {
   if (txt.Length() == 0)
    {
     App->Response(COMMON_NAME_BLANK);
     return;
    }
  }  
 m_GroupName = txt;
 Close(DialogResult::OK);
}

///////////////////////////////////////////////////////

DialogResult FileSortDlg::Show(AWnd *parent)
{
 return ADialog::Show(IDD_GROUP_SORT, parent);
}

void FileSortDlg::OnInitDialog()
{
 ADialog::OnInitDialog();
 SetCheckState(IDC_GROUP_SORT_ASC, true);
}

WMR FileSortDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDOK:
     OnOK();
     break;
   case IDCANCEL:
     Close(DialogResult::Cancel);
     break;
  }
 return WMR::One;
}

void FileSortDlg::OnOK()
{
 ImageParser::SortChoices s;

 if (GetCheckState(IDC_GROUP_SORT_R1) == true) s = ImageParser::SortChoices::NameWithNumber;
 if (GetCheckState(IDC_GROUP_SORT_R2) == true) s = ImageParser::SortChoices::FileName;
 if (GetCheckState(IDC_GROUP_SORT_R3) == true) s = ImageParser::SortChoices::FileDate;
 if (GetCheckState(IDC_GROUP_SORT_R4) == true) s = ImageParser::SortChoices::FileSize;
 if (GetCheckState(IDC_GROUP_SORT_R5) == true) s = ImageParser::SortChoices::ColorRGB;
 if (GetCheckState(IDC_GROUP_SORT_R6) == true) s = ImageParser::SortChoices::EdgeAverage;

 m_Ascending = GetCheckState(IDC_GROUP_SORT_ASC);
 Close(DialogResult::OK);
}

///////////////////////////////////////////////

DialogResult BaseDirDlg::Show(AWnd *parent, BaseItem *item)
{
 m_Item = item;
 return ADialog::Show(IDD_BASE_PROP, parent);
}

void BaseDirDlg::OnInitDialog()
{
 SetItemText(IDC_BASE_PROP_ID, String::Decimal(m_Item->ID()));
 SetItemText(IDC_BASE_PROP_NAME, m_Item->DirName());
 SetItemText(IDC_BASE_PROP_DIR, m_Item->DirPath());
 ADialog::OnInitDialog();
}

WMR BaseDirDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDOK: OnOK(); break;
   case IDCANCEL: Close(DialogResult::Cancel); break;
   case IDC_BASE_PROP_BROWSE: OnBrowse(); break;
  }
 return WMR::One;
}

void BaseDirDlg::OnOK()
{
 String name, dir, msg;

 name = GetItemText(IDC_BASE_PROP_NAME).Trim();
 dir = GetItemText(IDC_BASE_PROP_DIR).Trim();

 if (name.Length() == 0)
  {
   App->Response(COMMON_NAME_BLANK);
   return;
  }

 if (dir.Length() == 0) 
  {
   App->Response(DLG_DIR_BLANK);
   return;
  }

 if (Utility::DirectoryExists(dir)==false) 
  {
   if (App->Question(App->Prose.TextArgs(DLG_BASE_NOT_FOUND, dir), MB_OKCANCEL) !=DialogResult::OK)
     return;
  }

 m_Name = name;
 m_Dir = dir;
 Close(DialogResult::OK);

}

void BaseDirDlg::OnBrowse()
{
 String dir;

 dir = OpenFileDlg::ChooseFolder(m_hWnd, App->Prose.Text(DLG_BASE_CHOOSE_PATH));

 if (dir.Length() == 0)
   return;

 SetItemText(IDC_BASE_PROP_DIR, dir);
}

/////////////////////////////////////////////////////////

DialogResult HashTagChooseDlg::Show(AWnd *parent)
{
 m_HashTagType = HashTag::HashTagType::GlobalTag;

 return ADialog::Show(IDD_HASHTAG_CHOOSE, parent);
}

void HashTagChooseDlg::OnInitDialog()
{

 m_List.Attach(this,IDC_HASHTAG_CHOOSE_LIST);
 m_List.AddColumn(UI_COLUMN_HASHTAGS, ListViewColumn::ColumnAlign::Left, 60);

 switch(m_HashTagType)
  {
   case HashTag::HashTagType::GlobalTag:
    {
     for(const auto &ht : App->GlobalHashTags)
      {
       ListViewItem item(ht.second.Name());
       item.Tag = ht.second.ID();
       m_List.Insert(item);
      }
    } break;
  }

 m_List.AutoSize(0, ListView::AutoSizes::Content);
 ADialog::OnInitDialog();
}

WMR HashTagChooseDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDOK: 
    {
     if (m_SelectedTag.TagType() == HashTag::HashTagType::None)
       App->Response(DLG_HASHTAG_CHOOSE);
     else
       Close(DialogResult::OK);
    } break;
   case IDCANCEL:
     Close(DialogResult::Cancel);
  }
 return WMR::One;
}

void HashTagChooseDlg::OnListViewItemChanged(int child, NMLISTVIEW *pLVN)
{
 int id;

 if (child != IDC_HASHTAG_CHOOSE_LIST) 
   return;

 id = pLVN->lParam;

 switch(m_HashTagType)
  {
   case HashTag::HashTagType::GlobalTag:
     m_SelectedTag = App->GlobalHashTags[id];
     break;
  }
}

void HashTagChooseDlg::OnDoubleClick(int child, NMITEMACTIVATE *pItemAct)
{
 if (child != IDC_HASHTAG_CHOOSE_LIST) 
   return;

 if (m_SelectedTag.TagType() != HashTag::HashTagType::None)
   Close(DialogResult::OK);
}

///////////////////////////////////////////////////////////////////////

HashTagSelectorDlg::HashTagSelectorDlg()
{
 m_Folder=nullptr;
 m_Item=nullptr;
}

DialogResult HashTagSelectorDlg::Show(AWnd *parent, FolderItem *fldr, eMode mode)
{
 m_Mode = mode;
 m_Folder = fldr;
 m_Item = nullptr;
 return ADialog::Show(IDD_HASHTAG_SELECT, parent);
}

DialogResult HashTagSelectorDlg::Show(AWnd *parent, FolderItem *fldr, PictureItem *pic)
{
 m_Mode = eMode::PictureTags;
 m_Folder = fldr;
 m_Item = pic;
 return ADialog::Show(IDD_HASHTAG_SELECT, parent);
}

void HashTagSelectorDlg::OnInitDialog()
{
 Rect r;
 int g;

 btnOK.Attach(this, IDC_HASHTAG_SELECT_ADD);
 txtFolder.Attach(this, IDC_HASHTAG_SELECT_EDIT);

 g = txtFolder.Left();

 FolderHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::SelectCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);
 FolderHashTagCtrl.AutoSize = false;
 FolderHashTagCtrl.Border(true);

 for(const auto &fht : m_Folder->FolderHashTags)
   FolderHashTagCtrl.FolderMap.insert(fht);
 FolderHashTagCtrl.ProcessTags(ClientSize().Width-8);
 r.X = g;
 r.Y = g; 
 r.Width = ClientSize().Width - g * 2;
 r.Height = txtFolder.Top() - g * 2;
 FolderHashTagCtrl.SetRect(r); 

 if (m_Mode == eMode::FolderTags)
   SelectHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::FolderCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);
 else
   SelectHashTagCtrl.Create(this, HashTagSelectCtrl::enumCtrlStyle::PictureCtrl, HashTagSelectCtrl::enumSelectStyle::FolderTags);

 SelectHashTagCtrl.AutoSize = false;
 SelectHashTagCtrl.Border(true);

 if (m_Item != nullptr)
  {
   for(const auto &pht : m_Item->PictureHashTags)
     SelectHashTagCtrl.PictureMap.insert(pht);
  }

 SelectHashTagCtrl.ProcessTags(btnOK.Left() - 8);
 r.X = g;
 r.Y = txtFolder.Top() + txtFolder.Height() + g;
 r.Width = txtFolder.Width();
 r.Height = ClientSize().Height - ( r.Y + g );
 SelectHashTagCtrl.SetRect(r);

 ADialog::OnInitDialog();
 
}

WMR HashTagSelectorDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDCANCEL: Close(DialogResult::Cancel); break;
   case IDOK:                   OnOK(); break;
   case IDC_HASHTAG_SELECT_ADD: OnAdd(); break;
  }
 return WMR::One;
}

void HashTagSelectorDlg::OnFolderClicked(HashTag *ht)
{
 String msg;

 if (m_Mode == eMode::FolderTags)
  {
   for(const auto &fht : SelectHashTagCtrl.FolderMap)
    {
     if (fht.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::FolderTag) 
      { 
       App->Response(App->Prose.TextArgs(DLG_HASHTAG_FOLDER_SELECTED, ht->Name()));
       return; 
      }
    }
  }
 else
  {
   for(const auto &pht : SelectHashTagCtrl.PictureMap)
    {
     if (pht.second.ID() == ht->ID() && ht->TagType() == HashTag::HashTagType::FolderTag) 
      {
       App->Response(App->Prose.TextArgs(DLG_HASHTAG_GLOBAL_SELECTED, ht->Name()));
       return; 
      }
    }
  }
   
 if (m_Mode == eMode::FolderTags)
   SelectHashTagCtrl.FolderMap.insert(std::pair<int, HashTag>(ht->ID(), *ht));
 else
   SelectHashTagCtrl.PictureMap.insert(std::pair<int, HashTag>(ht->ID(), HashTag(ht->ID(), ht->Name(), m_Folder, HashTag::HashTagType::PictureTag)));
  
 SelectHashTagCtrl.ProcessTags(btnOK.Left() - 8);
}

void HashTagSelectorDlg::OnSelectClicked(HashTag *ht)
{

 if (m_Mode == eMode::FolderTags)
  {
   for(const auto &fht : SelectHashTagCtrl.FolderMap)
    {
     if (fht.second.ID()==ht->ID())
      {
       SelectHashTagCtrl.FolderMap.erase(fht.second.ID());
       break;
      }
    }
  }
 else
  {
   for(const auto &pht : SelectHashTagCtrl.PictureMap)
    {
     if (pht.second.ID()==ht->ID())
      {
       SelectHashTagCtrl.PictureMap.erase(pht.second.ID());
       break;
      }
    }
  }

 SelectHashTagCtrl.ProcessTags(btnOK.Left() - 8);
}

void HashTagSelectorDlg::OnSelectContext(HashTag *ht)
{
 DialogResult r;
 std::vector<HashTag> delList;
 bool found;

 HashTagSubDlg dlg(ht, m_Folder, this);
 r = dlg.Show(this);
 if (r == DialogResult::OK)
  {
   for(const auto &pht : dlg.PictureHashTags)
    {
     if (ht->SubTags.count(pht.ID()) == 0) 
       ht->SubTags.insert(std::pair<int, HashTag>(pht.ID(), pht));  // insert into "ht" if not already present
    }
   for(const auto &pht : ht->SubTags)
    {
     found = false;
     for (const auto &pht2 : dlg.PictureHashTags) // see if tag in HT is not longer present in dlg's new selection
      {
       if (pht2.ID() == pht.second.ID())
        {
         found = true;
         break;
        }
      }
     if (found == false) 
       delList.push_back(pht.second);  // wasn't found in dlg's new set of tags, so remove it from "ht"
    }
   for(const auto &pht : delList)
     ht->SubTags.erase(pht.ID());
   SelectHashTagCtrl.Refresh();
  }
}

void HashTagSelectorDlg::OnHashTagNameChange()
{
 FolderHashTagCtrl.Refresh();
 SelectHashTagCtrl.Refresh();
}

void HashTagSelectorDlg::OnOK()
{
 if (m_Mode == eMode::PictureTags)
  {
   PictureHashTags.clear();
   for(const auto &pht : SelectHashTagCtrl.PictureMap)
     PictureHashTags.push_back(pht.second);
  }
 else 
  {
   FolderHashTags.clear();
   for(const auto &fht : SelectHashTagCtrl.FolderMap)
     FolderHashTags.push_back(fht.second);
  }
 Close(DialogResult::OK);
}

void HashTagSelectorDlg::OnAdd()
{
 HashTag sfht;
 String txt;

 txt = txtFolder.GetText().Trim();

 if (txt.Length() == 0) { App->Response(DLG_HASHTAG_BLANK); return; }

 for(const auto &fht : m_Folder->FolderHashTags)
  {
   if (String::Compare(fht.second.Name(), txt) == 0) 
    {
     sfht = fht.second;
     break;
    }
  }

 if (sfht.TagType() != HashTag::HashTagType::None) 
  {
   for(const auto &pht : SelectHashTagCtrl.PictureMap)
    {
     if (pht.second.ID() == sfht.ID())
       return; // tag already present and added to select list
    }
  }

 if (sfht.TagType() == HashTag::HashTagType::None)
  {
   sfht = m_Folder->AddHashTag(txt);
   if (sfht.TagType() == HashTag::HashTagType::None)  throw L"add folder hashtag failed";
   FolderHashTagCtrl.FolderMap.insert(std::pair<int, HashTag>(sfht.ID(), sfht));
   FolderHashTagCtrl.ProcessTags(ClientSize().Width-8);
  }

 txtFolder.SetText(L"");
 SelectHashTagCtrl.PictureMap.insert(std::pair<int, HashTag>(sfht.ID(), HashTag(sfht.ID(), sfht.Name(), m_Folder, HashTag::HashTagType::PictureTag)));
 SelectHashTagCtrl.ProcessTags(btnOK.Left() - 8);
}

void HashTagSelectorDlg::OnKeyDown(int child, KeyEventArgs const &e)
{
 String txt;

 if (child != IDC_HASHTAG_SELECT_EDIT)
   return;

 if (e.KeyCode == Keyboard::Enter)
  {
   txt = txtFolder.GetText().Trim();
   if (txt.Length() > 0)
     OnAdd();
  }
}

WMR HashTagSelectorDlg::MessageHandler(HWND hWndIn, UINT message, WPARAM wParam, LPARAM lParam)
{
 HashTag *ht;
 HWND     hWnd;

 switch(message)
  {
   case WM_HTSC_ITEMCLICK:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == FolderHashTagCtrl.Handle()) OnFolderClicked(ht); 
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectClicked(ht); 
     } break;
   case WM_HTSC_ITEMCONTEXT:
     {
      hWnd = (HWND)wParam;
      ht = (HashTag *)lParam;
      if (hWnd == SelectHashTagCtrl.Handle()) OnSelectContext(ht);
     } break;
   case WM_HTSC_FILTER_CHANGED:
   case WM_HTSC_NAME_CHANGED:
     OnHashTagNameChange();
     break;
   case WM_HTSC_LIST_CHANGED:
   default: return ADialog::MessageHandler(hWndIn, message, wParam, lParam);
  }
 return WMR::One;
}

//////////////////////////////////////////////////////////////////

DialogResult GroupSelectDlg::Show(AWnd *parent, std::vector<String> const &list)
{
 m_Groups = list;
 return ADialog::Show(IDD_GROUP_SELECT, parent);
}

void GroupSelectDlg::OnInitDialog()
{
 m_List.Attach(this, IDC_GROUP_SELECT_LIST);
 m_List.AddColumn(UI_COLUMN_GROUP_NAME, ListViewColumn::ColumnAlign::Left, 100);
 for(const auto &s : m_Groups)
  {
   m_List.Insert(ListViewItem(s));
  }
 m_List.AutoSize(0, ListView::AutoSizes::Content);
 ADialog::OnInitDialog();
}

WMR GroupSelectDlg::OnCommand(int child, HWND hChild)
{
 switch(child)
  {
   case IDOK: 
     OnOK();
     break;
   case IDCANCEL:
     Close(DialogResult::Cancel);
     break;
  }
 return WMR::One;
}

void GroupSelectDlg::OnOK()
{
 ListViewItem item;
 int ndx;

 if (m_List.SelectedItemsCount() == 0)
  {
   App->Response(L"Select a group name");
   return;
  }

 ndx = m_List.GetSelectedItem(); 
 m_GroupName = m_List.GetItemText(ndx, 0);
 Close(DialogResult::OK);
 
}

////////////////////////////////////////////////////////

DialogResult LanguageDlg::Show()
{
 return ADialog::Show(IDD_LANGUAGE, nullptr);
}

void LanguageDlg::OnInitDialog()
{
 ADropList list;

 ADialog::OnInitDialog();

 m_List.Attach(this, IDC_LANGUAGE_LIST);

 for (const auto &lang : App->Prose.Languages)
  {
   m_List.AddItem(lang.second.Name, lang.second.ID);
  }
 m_List.SetCurSel(0);
}

WMR LanguageDlg::OnCommand(int child, HWND hChild)
{
 WMR ret = WMR::One;
 int id;

 if (child == IDOK)
  {
   if (m_List.IsItemSelected() == false)
    {
     App->Response(L"Select a language");
     return ret;
    }
   id = m_List.GetSelectedItem().Tag;
   m_Lang = App->Prose.Languages[id];
   Close(DialogResult::OK);
  }
 if (child == IDCANCEL)
   Close(DialogResult::Cancel);

 return ret;
}