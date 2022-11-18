#include "pch.h"
#include "App.h"


PicManApp::PicManApp()
{
 m_PictureID=1; // don't start with 0, code tests for 0 in the UI
}

PicManApp::~PicManApp()
{
}

bool PicManApp::Init(HINSTANCE hInst, int nCmdShow, String const &appName, String const &displayName)
{
 AFileTextOut logFile;
 LanguageDlg dlg;
 ProseLanguage lang;
 String logPath;
 String txt;
 bool ret;

 if (Application::Init(hInst, nCmdShow, appName, displayName) == false)
  {
    logFile.Write(L"Application::Init Failed, end of log.");
    logFile.Close();
   return false;
  }

 logPath = Application::AppLocalPath();
 logPath += L"\\PicMan.log";
 logFile.Open(logPath);

 txt = L"PicMan Startup ";
 txt += DateTime::Now().ToString(DateTime::Format::MDYHMS);
 logFile.Write(txt);
 logFile.Write(String::StrDup(L'-', txt.Length()));

 logFile.Write(L"Loading 'PicManOptions.opt' File");

 Options.SetName(L"PicManOptions.opt");
 Options.SetPath(OpenFileDlg::AppDataFolder());
 Options.Load();

 logFile.Write(L"Loading 'PicManOptions.opt' File Success");

 logFile.Write(L"Opening Prose");

 if (Prose.OpenProse() == false)
  {
   App->Response(L"Failed to load ProseUnit");
   logFile.Write(L"Opening Prose Failed, end of log.");
   logFile.Close();
   return false;
  }
 if (Prose.Languages.size() == 0)
  {
   App->Response(L"Prose data file has no languages");
   logFile.Write(L"Opening Prose No Languages, end of log");
   logFile.Close();
   return false;   
  }

 if (Prose.Languages.size() == 1)
   lang = Prose.Languages[1];        // english
 else
  {
   if (Options.LanguageID() > 0)
    {
     if (Prose.Languages.count(Options.LanguageID()) > 0)
       lang = Prose.Languages[Options.LanguageID()];
    }
   if (lang.ID == 0)
    {
     if (dlg.Show() == DialogResult::OK)
       lang =dlg.Language();
     else
      {
       logFile.Write(L"Opening Prose Failed, language not selected, end of log.");
       logFile.Close();
       return false; // language not selected
      }
    }
  }

 Options.SaveLanguage(lang.ID);

 Prose.SetLanguage(lang);
 ProseRuntimeClass::LoadReverseKeys(Prose);

 logFile.Write(L"Opening Prose Success.");

 logFile.Write(L"DB Connect");

 DB=new DBCon();
 if (DB->Connect(true)==false)
   return false;

 logFile.Write(L"DB Connect Success");
 logFile.Write(L"Load Global HashTags");

 GlobalHashTags = HashTag::LoadGlobalHashTags();
 if (GlobalHashTags.count(1) == 0)
   AddGlobalHashTag(App->Prose.Text(COMMON_FAVORITE));

 logFile.Write(L"Load Global HashTags Success");
 logFile.Write(L"Load Bases");
 
 Bases = BaseItem::LoadBases();

 logFile.Write(L"Load Bases Success");
 logFile.Write(L"Register MainWnd Class");

 MainWnd::Register(L"MainWndClass");

 logFile.Write(L"Register MainWnd Class Success");
 logFile.Write(L"Create MainWnd");


 Wnd=new MainWnd();
 m_Main = (PopUpWnd *)Wnd;
 ret=m_Main->Create(L"MainWndClass", SW_MAXIMIZE); // create main app window

 if (ret == false)
   logFile.Write(L"Create MainWnd Failed");
 else
   logFile.Write(L"Create MainWnd Success");

 logFile.Close();
 return ret; 
}

void PicManApp::ShutDown()
{
 Application::Shutdown();
}

void PicManApp::ClearPictures()
{
 ImageParser *p;

 for(const auto &it : Pictures)
  {
   p = it.second;
   p->Dispose();
   delete p;
  }
}

std::vector<ImageParser *>PicManApp::ClearAndGetImports()
{
 std::vector<ImageParser *> temp;
 ImageParser *p;

 for(const auto &it : Pictures)
  {
   p = it.second;
   if (p->IsImport() == true)
     temp.push_back(p);
   else
    {
     p->Dispose();
     delete p;
    }
  }

 Pictures.clear();

 return temp;
}

std::vector<HashTag>PicManApp::GetGlobalHashTags()
{
 std::vector<HashTag> list;

 for (const auto &it : GlobalHashTags)
   list.push_back(it.second);

 return list;
}

bool PicManApp::UpdateGlobalHashTag(HashTag &ht, String const &s)
{
 if (s == ht.Name()) return false; // no change

 for (const auto &it : GlobalHashTags) 
  {
   if (it.second.ID() != ht.ID() && String::Compare(it.second.Name(), ht.Name()) == 0) 
     return false;  // duplication
  }

 MySqlCommand cmd(L"UPDATE hashtag_global SET name=@s WHERE hashtag_id=@i;", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@s", s));
 cmd.Parameters.Add(new MySqlParameter(L"@i", ht.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose();
 ht.SetName(s);

 return true;
}

void PicManApp::DropGlobalHashTag(HashTag const &ght)
{
 std::vector<HashTag *> list;
 ImageParser *p;

 if (ght.TagType() != HashTag::HashTagType::GlobalTag) throw L"not a global tag";

 if (ght.ID() == 1) 
   return;  // 1 is reserved for favorite

 MySqlCommand cmd(L"DELETE FROM picture_global_hashtag WHERE hashtag_id=@h; DELETE FROM hashtag_global WHERE hashtag_id=@h;",DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@h", ght.ID()));
 cmd.ExecuteNonQuery();
 cmd.Dispose(); 

 GlobalHashTags.erase(ght.ID()); 

 for (const auto &it : Pictures)
  {
   p=it.second;
   if (p->Item() != nullptr)
    {
     if (p->Item()->GlobalHashTags.count(ght.ID()))
       p->Item()->GlobalHashTags.erase(ght.ID());
    }
  }
}

HashTag PicManApp::AddGlobalHashTag(String const &s)
{
 HashTag ht;
 MySqlDataReader *rs;
 MySqlCommand cmd;

 for(const auto &it : GlobalHashTags)
  {
   if (String::Compare(s, it.second.Name()) == 0)
     return HashTag();  // type = none
  } 

 cmd = MySqlCommand(L"INSERT INTO hashtag_global(name) VALUES (@n);", DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@n", s));
 cmd.ExecuteNonQuery();
 cmd.Dispose();

 cmd = MySqlCommand(L"SELECT LAST_INSERT_ID();", DB->Con());
 rs = cmd.ExecuteReader();
 if (rs->Read() == false) throw L"auto ident not read";
 ht = HashTag(rs->GetInt32(0), s, HashTag::HashTagType::GlobalTag);
 rs->Close();
 cmd.Dispose();

 GlobalHashTags.insert(std::pair<int, HashTag>(ht.ID(), ht));
 return ht;
}

HashTag PicManApp::GetFavoriteHashTag()
{
 for (const auto &ht : GlobalHashTags)
  {
   if (ht.second.ID() == 1) 
     return ht.second;
  }
 throw L"Favorite hashtag not found, id = 1 is supposed to be reserved";
}

void PicManApp::SaveSetting(String const &thing, String const &val)
{
 String q, app, name;

 app=AppName();
 name=DB->User();

// q=L"INSERT INTO user_settings (app_name, user_name, setting_name, setting_value) values (@app, @name, @set, @val);";
 
 q=L"CALL user_settings_update_proc(@app, @name, @set, @val); ";

 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@app", app)); 
 cmd.Parameters.Add(new MySqlParameter(L"@name", name));
 cmd.Parameters.Add(new MySqlParameter(L"@set", thing));
 cmd.Parameters.Add(new MySqlParameter(L"@val", val));
 
 cmd.ExecuteNonQuery();
 cmd.Dispose();

}

void PicManApp::SaveSettingInt(String const &thing, int val)
{
 String nval = String::Decimal(val);
 SaveSetting(thing, nval);
}

void PicManApp::SaveSettingBool(String const &thing, bool bVal)
{
 int iVal;

 if (bVal == true)
   iVal = 1;
 else
   iVal = 0;

 String nval = String::Decimal(iVal);
 SaveSetting(thing, nval);
}

void PicManApp::SaveSettingDbl(String const &thing, double val)
{
 String nval = String::Double(val);
 SaveSetting(thing, nval);
}

String PicManApp::GetSetting(String const &thing)
{
 return GetSetting(thing, L"");
}

String PicManApp::GetSetting(String const &thing, String const &def)
{
 MySqlDataReader *rs;
 String q, app, name, val;

 app=AppName();
 name=DB->User();

  q=L"SELECT setting_value ";
 q+=L"FROM user_settings ";
 q+=L"WHERE app_name=@app AND user_name=@name AND setting_name=@set;";
 
 MySqlCommand cmd(q, DB->Con());
 cmd.Parameters.Add(new MySqlParameter(L"@app", app)); 
 cmd.Parameters.Add(new MySqlParameter(L"@name", name));
 cmd.Parameters.Add(new MySqlParameter(L"@set", thing));
 rs=cmd.ExecuteReader();

 val=def;

 if (rs->Read() == true)
   val=rs->GetString(0);
 rs->Close();
 cmd.Dispose();
 
 return val;
}

int PicManApp::GetSettingInt(String const &thing)
{
 return GetSettingInt(thing, 0);
}

int PicManApp::GetSettingInt(String const &thing, int def)
{
 String val = GetSetting(thing);
 int i;

 if (String::TryIntParse(val, &i) == true)
   return i;
 else
   return def; 
}

bool PicManApp::GetSettingBool(String const &thing)
{
 return GetSettingBool(thing, false);
}

bool PicManApp::GetSettingBool(String const &thing, bool def)
{
 int defVal;
 int val;

 if (def == true)
  defVal = 1;
 else
  defVal = 0;

 val = GetSettingInt(thing, defVal);

 return (val == 1);
}


double PicManApp::GetSettingDbl(String const &thing)
{
 return GetSettingDbl(thing, 0.0);
}

double PicManApp::GetSettingDbl(String const &thing, double def)
{
 String val = GetSetting(thing);
 double d;

 if (String::TryDblParse(val, &d) == true)
   return d;
 else
   return def; 
} 

void PicManApp::EditPicture(String const &iFile)
{
 SHELLEXECUTEINFO ShExecInfo={0};
 String editor, file;
 String msg;

 editor = App->GetSetting(L"ImageEditor", L"");
 if (editor.Length() == 0) 
  { 
   App->Response(L"Set the image editor using 'Tools > Set Editor'"); 
   return; 
  }

 file = L"\"";
 file += iFile;
 file += L"\"";
  
 ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
 ShExecInfo.fMask = NULL;
 ShExecInfo.hwnd = Wnd->Handle();
 ShExecInfo.lpVerb = L"open";
 ShExecInfo.lpFile = editor.Chars();
 ShExecInfo.lpParameters = file.Chars();
 ShExecInfo.lpDirectory = NULL;
 ShExecInfo.nShow = SW_MAXIMIZE;
 ShExecInfo.hInstApp = NULL;

 if (ShellExecuteEx(&ShExecInfo) == FALSE)
  {
   msg = L"Failed to open editor.\n\n";
   msg += L"Editor: ";
   msg += editor;
   msg += L"\n\n";
   msg += L"File: ";
   msg += file;
   App->Response(msg);
  }
}

//////////////////////////////////////////////////

DBCon::DBCon()
{
 m_Connection=nullptr;
}

DBCon::~DBCon()
{
 if (m_Connection != nullptr)
  {
   delete m_Connection;
   m_Connection=nullptr;
  }
}

bool DBCon::TestConnect(String const &cs)
{
 MySqlConnection *con=nullptr;
 bool good;

 con = new MySqlConnection(cs);
 good=con->Open();
 if (good == true) 
   con->Close();
 delete con;

 return good;
}

bool DBCon::Connect(bool bSilent)
{
 DialogResult r;
 LoginDlg login;
 String constr;
 bool resolved;
 bool good=false;

 if (m_Connection!=nullptr)
  {
   if (m_Connection->IsOpen() == true && bSilent == false)
     constr = m_Connection->GetConnectionString();  // allow user to cancel dialog and then reconnect if it's already open
   m_Connection->Close();
   delete m_Connection;
   m_Connection=nullptr;
  }

 // loop until database connectoin is made or connection prompt is dismissed
 
 do
  {
   resolved = false;
   if (login.RememberPassword == true && bSilent == true)
    {
     resolved = TestConnect(login.Connection()); // bypass login dialog if possible
     if (resolved == false)
      {  
       if (App->Question(DLG_DB_RETRY, MB_YESNO) != DialogResult::Yes)
        {
         return false;
        }
      }
     else
      {
       m_ConnectionString=login.Connection();
       m_Connection=new MySqlConnection(m_ConnectionString);
       if (m_Connection->Open() == true)
        {
         m_UserName=login.User;
         good=true;
        }
       else
        {
         good=false;
        }
      }
    }
   if (resolved == false)
    {
     r = login.Show();
     if (r == DialogResult::OK) 
      {
       if (TestConnect(login.Connection()) == true)
        {
         login.SaveSettings();
         m_ConnectionString=login.Connection();
         m_Connection=new MySqlConnection(m_ConnectionString);
         m_Connection->Open();
         m_UserName=login.User;
         good=true;
        }
      }
     else
      {
       if (constr.Length() == 0)  // DilaogResult::Cancel
        {
         if (App->Question(DLG_DB_RETRY_2, MB_RETRYCANCEL) == DialogResult::Cancel)
         return false;
        }
       else
        {
         m_ConnectionString = constr;
         m_Connection=new MySqlConnection(m_ConnectionString); // if con was open allow user to cancel & reconnect with existing values
         m_Connection->Open();
         m_UserName=login.User;
         good=true;            
        }
      }
    }
  }
 while( good == false);

 return true;
}

//////////////////////////////////////////////////

CredFileWrite::CredFileWrite()
{
 m_hFile = 0;
}

CredFileWrite::~CredFileWrite()
{
 if (m_hFile != 0)
  {
   CloseHandle(m_hFile);
  }
}

bool CredFileWrite::Open(String const &path)
{
 String proof = L"PicManINI";
 HANDLE hFile;
 DWORD siglen, bw;
 BOOL wr;

 hFile = CreateFile(path.Chars(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);    
 if (hFile == INVALID_HANDLE_VALUE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   return false;
  }
 else
  {
   m_hFile = hFile;
   siglen = proof.Length() * sizeof(wchar_t);
   wr = WriteFile(m_hFile, proof.Chars(), siglen, &bw, NULL);
   if (wr == FALSE)
    {
     m_Error = String::GetLastErrorMsg(GetLastError());
     return false;
    }
  }
 return true;
}

bool CredFileWrite::Write(std::vector<BYTE> const &bytes)
{
 DWORD bw;
 BYTE strLen;
 BYTE *buffer;
 BOOL wr;
 int i;

 if (m_hFile == 0)
   throw L"file not open";
 
 if (bytes.size() > 253)
  {
   m_Error = L"Maximum length of data is limited to 253 bytes in length";
   return false;
  }

 strLen = (BYTE)bytes.size();
 wr = WriteFile(m_hFile, &strLen, 1, &bw, NULL);
 if (wr == FALSE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   return false;
  }

 buffer = new BYTE[strLen];
 for (i=0; i<strLen; i++)
   buffer[i]=bytes[i]; 
 
 wr = WriteFile(m_hFile, buffer, (DWORD)strLen, &bw, NULL); 
 if (wr == FALSE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   return false;
  }
 return true;
}


void CredFileWrite::Close()
{
 if (m_hFile != 0)
  {
   CloseHandle(m_hFile);
   m_hFile = 0;
  }
}

//////////////////////////////////////////////////

CredFileRead::CredFileRead()
{
 m_hFile = 0;
}

CredFileRead::~CredFileRead()
{
 if (m_hFile != 0)
  {
   CloseHandle(m_hFile);
   #ifdef _DEBUG
   throw L"File Not Closed";
   #endif 
  }
}

bool CredFileRead::Open(String const &path)
{
 String proof = L"PicManINI";
 HANDLE hFile;
 wchar_t *buffer;
 DWORD siglen, bw;
 BOOL wr;
 bool ret = true;
 int i;

 hFile = CreateFile(path.Chars(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);    
 if (hFile == INVALID_HANDLE_VALUE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   return false;
  }
 else
  {
   m_hFile = hFile;
   siglen = proof.Length() * sizeof(wchar_t);
   buffer = new wchar_t[proof.Length()];
   wr = ReadFile(m_hFile, buffer, siglen, &bw, NULL);
   if (wr == FALSE)
    {
     m_Error = String::GetLastErrorMsg(GetLastError());
     ret = false;
    }
   if (bw != siglen)
    {
     m_Error = L"FileRead::Open(path) failed to read file signature";
     ret = false;
    }
   else
    {
     for (i=0; i<proof.Length(); i++)
      {
       if (proof[i] != buffer[i])
        {
         m_Error = L"FileRead::Open(path) file signature compare failed";
         ret = false;
        }
      }
    }
   delete [] buffer;  
  }
 return ret;
}

std::vector<BYTE> CredFileRead::ReadBytes()
{
 std::vector<BYTE> bytes;
 DWORD br;
 BYTE *buffer;
 BYTE strLen;
 BOOL rr;
 bool ret;
 int i;

 if (m_hFile == 0)
   throw L"file not open";

 rr = ReadFile(m_hFile, &strLen, 1, &br, NULL);
 if (rr == FALSE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   return bytes;
  }

 if (br == 0)
  {
   return bytes;  // at EOF
  }

 if (strLen > 253)
   throw L"byte length was > 253";

 buffer = new BYTE[strLen];

 rr = ReadFile(m_hFile, buffer, (DWORD)strLen, &br, NULL);
 if (rr == FALSE)
  {
   m_Error = String::GetLastErrorMsg(GetLastError());
   ret = false;
  }
 if (br != strLen)
   throw L"bytes read not = len";

 for (i=0; i<strLen; i++)
  {
   bytes.push_back(buffer[i]);
  }
 
 delete [] buffer;

 return bytes; 
}


void CredFileRead::Close()
{
 if (m_hFile != 0)
  {
   CloseHandle(m_hFile);
   m_hFile = 0;
  }
}

