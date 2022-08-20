#include "pch.h"
#include "app.h"

MySqlConnection::MySqlConnection(String const &cs)
 {
  m_hEnvironment=0;
  m_hODBC=0;
  m_ConnectionString=cs;
  m_Open = false;

 if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnvironment) != SQL_SUCCESS)
   throw L"Unable to allocate an environment handle";
  
 if (::SQLSetEnvAttr(m_hEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3_80, 0) != SQL_SUCCESS)
   throw L"Unable to set environment";

 if (::SQLAllocHandle(SQL_HANDLE_DBC, m_hEnvironment, &m_hODBC) != SQL_SUCCESS)
   throw L"Unable to alloc handle";

 }

MySqlConnection::~MySqlConnection()
{
 if (m_hODBC) 
   ::SQLFreeHandle(SQL_HANDLE_DBC, m_hODBC);
 if (m_hEnvironment)
   ::SQLFreeHandle(SQL_HANDLE_ENV, m_hEnvironment);
}

bool MySqlConnection::Open()
{
 SQLRETURN rc;

 rc=::SQLDriverConnect(m_hODBC, 0, (SQLWCHAR *)m_ConnectionString.Chars(), (SQLSMALLINT)m_ConnectionString.Length(), NULL, 0, NULL, SQL_DRIVER_COMPLETE);
 if (rc == SQL_SUCCESS)
  {
   m_Open=true;
   return true;
  }
 return false;
}

void MySqlConnection::Close()
{
 if (m_Open==false)
   throw L"MySqlConnection not open";

 ::SQLDisconnect(m_hODBC);
}

// SQL_HANDLE_STMT  SQL_HANDLE_DBC

bool MySqlConnection::CheckReturn(SQLRETURN rc, SQLSMALLINT handleType, SQLHANDLE handle, std::vector<MySqlError> &errors )
{
 SQLRETURN lrc;
 SQLSMALLINT iRec=0;
 WCHAR  wszState[SQL_SQLSTATE_SIZE+1];
 SQLINTEGER  iError;
 WCHAR       wszMessage[1000];
 bool loop;
 bool ret;
 bool info;

 errors.clear();
 switch(rc)
  {
   case SQL_SUCCESS:  info=false; ret=true; break;
   case SQL_SUCCESS_WITH_INFO: info=true; ret=true; break;
   default: info=true; ret=false;
  }
 if (info==true)
  {
   loop=true;
   iRec=1;
   while(loop==true)
    {
     lrc=::SQLGetDiagRec(handleType, handle, iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL);
     if (lrc==SQL_SUCCESS)
      {
       errors.push_back(MySqlError(iRec, wszState, wszMessage));
       iRec++;
      }
     else
      {
       loop=false;
      }
    }
  }
 return ret;
}

String MySqlConnection::ConnectionString(String const &server, String const &dbn, String const &user, String const &pwd)
{
 // Driver={MySQL ODBC 8.0 Unicode Driver};Server=brian-idle;charset=UTF8;Database=pic_mgr;User=pic_user;Password=happypixels;Option=3;"
 String cs;

 cs=L"Driver={MySQL ODBC 8.0 Unicode Driver};Server=";
 cs+=server;
 cs+=L";charset=UTF8;Database=";
 cs+=dbn;
 cs+=L";User=";
 cs+=user;
 cs+=L";Password=";
 cs+=pwd;
 cs+=L";Option=3;";
 
 return cs;
}

/////////////////////////////////////////////////////////

MySqlData::MySqlData()
{
 m_IsNullable=false;
 m_ColumnName.Clear();
 m_Column=0;
 m_Ind=0;
 m_ColumnSize=0;
 m_DataType=0;
 m_CType=0;
 m_Binary=nullptr;
 m_Str=nullptr;
 m_Val=0;
}

void MySqlData::Bind(SQLHSTMT hStmt, SQLSMALLINT nCol)
{
 SQLRETURN rc;
 SQLWCHAR name[100];
 SQLSMALLINT namelen;
 SQLSMALLINT dataType;
 SQLULEN colSize;
 SQLSMALLINT decDigits;
 SQLSMALLINT nullable;

 rc=::SQLDescribeCol(hStmt, nCol, name, 100, &namelen, &dataType, &colSize, &decDigits, &nullable);
 if (rc!=SQL_SUCCESS)
   throw L"SQLDescribeCol failed";

 m_IsNullable=nullable;
 m_ColumnName=name;
 m_Column=nCol;
 m_ColumnSize=(int)colSize;
 m_DataType=dataType;

 switch(m_DataType)
  {
   case SQL_WCHAR:
   case SQL_WVARCHAR:
     m_Str=new WCHAR[colSize+1];
     rc=::SQLBindCol(hStmt, nCol, SQL_C_WCHAR, m_Str, colSize+1, &m_Ind);
     m_CType=SQL_C_WCHAR;
     break;
   case SQL_BINARY:
   case SQL_VARBINARY:
   case SQL_LONGVARBINARY:
     m_Binary=new SQLCHAR[colSize+1];
     rc=::SQLBindCol(hStmt, nCol, SQL_C_BINARY, m_Binary, colSize+1, &m_Ind);
     m_CType=SQL_C_BINARY;
     break;
   case SQL_BIGINT:
   case SQL_INTEGER:
   case SQL_SMALLINT:
   case SQL_TINYINT:
   case SQL_BIT:
     rc=::SQLBindCol(hStmt, nCol, SQL_C_LONG, &m_Val, 4, &m_Ind);
     m_CType=SQL_C_LONG;
     break;
   case SQL_FLOAT:
   case SQL_DOUBLE:
     rc=::SQLBindCol(hStmt, nCol, SQL_C_DOUBLE, &m_Float, 8, &m_Ind);
     m_CType=SQL_C_DOUBLE;
     break;
   case SQL_TYPE_TIMESTAMP:
     rc=::SQLBindCol(hStmt, nCol, SQL_C_TIMESTAMP, &m_DT, sizeof(SQL_TIMESTAMP_STRUCT), &m_Ind);
     m_CType=SQL_C_TIMESTAMP;
     break;
   default:
     throw L"Unhandled data type returned in SQLDescribeCol";
  }
 if (rc!=SQL_SUCCESS)
   throw L"SQLBindCol failed";
}

MySqlData::~MySqlData()
{
}

String MySqlData::GetString()
{
 String val;

 switch(m_CType)
  {
   case SQL_C_WCHAR:
     val=m_Str;
     break;
   default:
     throw L"Conversion not handled";   
  }
 return val;
}

int MySqlData::GetInt32()
{
 int val;

 switch(m_CType)
  {
   case SQL_C_LONG:
     val=(int)m_Val;
     break;
   default:
     throw L"Conversion not handled";   
  }
 return val;
}

DateTime MySqlData::GetDateTime()
{
 DateTime val;

 switch(m_CType)
  {
   case SQL_C_TIMESTAMP:
     val.Year=m_DT.year;
     val.Month=m_DT.month;
     val.Day=m_DT.day;
     val.Hour=m_DT.hour;
     val.Minute=m_DT.minute;
     val.Second=m_DT.second;
     break;
   default:
     throw L"Conversion not handled";   
  }
 return val;
}

double MySqlData::GetDouble()
{
 double val;

 switch(m_CType)
  {
   case SQL_C_DOUBLE:
     val=(double)m_Float;
     break;
   default:
     throw L"Conversion not handled";   
  }
 return val;
}

BYTE *MySqlData::GetBytes()
{
 switch(m_CType)
  {
   case SQL_C_BINARY:
     return m_Binary;
   default:
     throw L"Conversion not handled";
  }
}

bool MySqlData::IsNull()
{
 if (m_Ind == SQL_NULL_DATA)
   return true;
 else
   return false;
}

int MySqlData::Length()
{
 if (m_Ind == SQL_NULL_DATA) 
   return 0;
 if (m_Ind == SQL_NO_TOTAL)
   throw L"what is this?";

 return (int)m_Ind;
}

/////////////////////////////////////////////////////////

MySqlDataReader::MySqlDataReader()
{
 m_hStatement=0;
 m_HasData=false;
 m_HasColumns=false;
 m_Columns=0;
 m_Data=nullptr;
}

MySqlDataReader::~MySqlDataReader()
{
 int i;

 if (m_HasColumns==true)
  {
   for (i=0;i<m_Columns;i++)
    {
     delete m_Data[i];
    }
   delete [] m_Data;
  }
}

void MySqlDataReader::Bind(SQLHSTMT hStmt)
{
 SQLRETURN rc;

 SQLSMALLINT nCols, i, nCol;

 rc=::SQLNumResultCols(hStmt, &nCols);
 m_Columns=nCols;
 nCol=1;

 if (m_Columns>0)
  {
   m_Data=new MySqlData *[nCols];
   for(i=0;i<nCols;i++)
    {
     m_Data[i]=new MySqlData();
     m_Data[i]->Bind(hStmt,nCol); 
     nCol++;
    }
   m_HasColumns=true;
  }
 m_hStatement=hStmt;
}

bool MySqlDataReader::Read()
{
 SQLRETURN rc;

 rc=::SQLFetch(m_hStatement);
 if (rc == SQL_SUCCESS)
  {
   m_HasData=true;
   return true;
  }
 if (rc == SQL_NO_DATA)
  {
   m_HasData=false;
   return false;
  }

 throw L"SQLFetch failed";
}

void MySqlDataReader::Close()
{
 delete this;
}

bool MySqlDataReader::ValidColumn(int col)
{
 if (m_hStatement==0)
   throw L"IsDBNull - data reader isn't prepared";

 if (m_HasColumns==false)
   throw L"sql didn't return columns";

 if (col<0 || col>=m_Columns)
   throw L"IsDBNull - column out of bounds";

 return true;
}

bool MySqlDataReader::IsDBNull(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->IsNull();
 throw L"Invalid Column";
}

String MySqlDataReader::GetString(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->GetString();
 throw L"Invalid Column";
}

DateTime MySqlDataReader::GetDateTime(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->GetDateTime();
 throw L"Invalid Column";
}

int MySqlDataReader::GetInt32(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->GetInt32();
 throw L"Invalid Column";
}

double MySqlDataReader::GetDouble(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->GetDouble();
 throw L"Invalid Column";
}

BYTE *MySqlDataReader::GetBytes(int col)
{
 if (ValidColumn(col)==true)
   return m_Data[col]->GetBytes();
 throw L"Invalid Column";
}

Bitmap *MySqlDataReader::GetBitmap(int col)
{
 Bitmap *bmp;
 IStream *stream;
 BYTE *ptr; 
 SQLULEN len; 

 if (ValidColumn(col)==false)
    throw L"Invalid Column";
 
 ptr = m_Data[col]->GetBytes();
 len = m_Data[col]->Length();

 stream=SHCreateMemStream(ptr, (UINT)len);
 bmp=new Bitmap(stream);
 stream->Release();

 if (bmp->GetLastStatus() != Status::Ok)
   throw L"failed to load bitmap";

 return bmp;
}

SQLULEN MySqlDataReader::Length(int col)
{
 SQLULEN len;

 if (ValidColumn(col)==true)
  {
   len = m_Data[col]->Length();
   return len;
  }
 throw L"invalid column";
}

/////////////////////////////////////////////////////////

MySqlParameter::MySqlParameter(String const &name, int val)
{
 m_Name=name;
 m_IntVal=val;
 m_SQLType = SQL_C_LONG;
 m_Len=0;
}

MySqlParameter::MySqlParameter(String const &name, String const &val)
{
 m_Name=name;
 m_StrVal=val;
 m_SQLType = SQL_C_WCHAR;
 m_Len=(int)m_StrVal.Length();
}

MySqlParameter::MySqlParameter(String const &name, DateTime const &val)
{
 m_Name=name;
 m_DateVal.year=val.Year;
 m_DateVal.month=val.Month;
 m_DateVal.day=val.Day;
 m_DateVal.hour=val.Hour;
 m_DateVal.minute=val.Minute;
 m_DateVal.fraction=0;
 m_SQLType = SQL_C_TIMESTAMP;
 m_Len=sizeof(SQL_TIMESTAMP);
}

MySqlParameter::MySqlParameter(String const &name, BYTE *arr, int len)
{
 int i;

 m_ByteVal=new BYTE[len];
 for(i=0;i<len;i++)
   m_ByteVal[i]=arr[i];
 m_SQLType = SQL_C_BINARY;
 m_Len=len;
}

MySqlParameter::MySqlParameter(String const &name, Bitmap *bmp)
{
 STATSTG stat;
 HRESULT hr;
 CLSID cid;
 IStream *stream;
 LARGE_INTEGER li;
 ULONG size;
 ULONG actual;

 hr = ::CreateStreamOnHGlobal(NULL, TRUE, &stream);
 if (FAILED(hr))
   throw L"Create stream failed";

 cid=App->EncoderPNG();
 bmp->Save(stream, &cid, nullptr);
 hr=stream->Stat(&stat, STATFLAG::STATFLAG_NONAME);
 if (FAILED(hr))
   L"get stat failed";
 
 size=stat.cbSize.QuadPart;
 m_ByteVal=new BYTE[size];

 li.HighPart=0;
 li.LowPart=0;
 hr=stream->Seek(li, STREAM_SEEK_SET, nullptr); // rewind
 if (FAILED(hr))
   throw L"failed to rewind stream";

 hr=stream->Read(m_ByteVal, size, &actual);
 if (FAILED(hr))
   throw L"failed to read stream into memory";

 m_SQLType=SQL_C_BINARY;
 m_Len=size;  
}

MySqlParameter::~MySqlParameter()
{
 switch(m_SQLType)
  {
   case SQL_C_WCHAR:  m_StrVal.Clear(); break;
   case SQL_C_BINARY: delete [] m_ByteVal;
  }
}

String MySqlParameter::ToString() const
{
 String msg;

 msg = L"Name: ";
 msg += m_Name;
 msg += L" Data Type: ";
 
 switch(m_SQLType)
  {
   case SQL_C_BINARY:
     msg += L" Binary"; 
     break;
   case SQL_C_WCHAR:   
     msg += L" WChar Value:";
     msg += m_StrVal;
     break;
   case SQL_C_LONG:    
     msg += L" Long Value:"; 
     msg += String::Decimal(m_IntVal);  
     break;
   case SQL_C_TIMESTAMP: 
     msg += L" DateTime Value";
     msg += String::Decimal(m_DateVal.year, 4);
     msg += L"/";
     msg += String::Decimal(m_DateVal.month, 2);
     msg += L"/";
     msg += String::Decimal(m_DateVal.day, 2);
     msg += L" ";
     msg += String::Decimal(m_DateVal.hour, 2);
     msg += L":";
     msg += String::Decimal(m_DateVal.minute, 2);
     msg += L":";
     msg += String::Decimal(m_DateVal.second, 2);
     break;
  }
 return msg;
}


void MySqlParameter::Bind(SQLHSTMT hStmt, int col) // for SQL_C_WCHAR BufferLength is "bytes" not wchar chars
{
 SQLRETURN rc;

 switch(m_SQLType)
  {
   case SQL_C_LONG:
     rc=::SQLBindParameter(hStmt, col, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&m_IntVal, 0, NULL);
     break;
   case SQL_C_WCHAR:
     rc=::SQLBindParameter(hStmt, col, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, (SQLULEN)m_Len*2, 0, (SQLPOINTER)m_StrVal.Chars(), m_Len*2, NULL);
     break;
   case SQL_C_TIMESTAMP:
     rc=::SQLBindParameter(hStmt, col, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, (SQLULEN) m_StrVal.Length(), 0, (SQLPOINTER)&m_DateVal, m_Len, NULL);
     break;     
   case SQL_C_BINARY: 
     rc=::SQLBindParameter(hStmt, col, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, (SQLULEN)m_Len, 0, (SQLPOINTER)m_ByteVal, m_Len, NULL);
     break;
   default: 
     throw L"unhandled case";
  }
 if (rc!=SQL_SUCCESS)
   throw L"SQLBindParameter failed";
}

////////////////////////////////////////////////////////

MySqlParameters::MySqlParameters()
{
}

MySqlParameters::~MySqlParameters()
{
 for (const auto &p : Items)
   delete p;
}

void MySqlParameters::Add(MySqlParameter *param)
{
 for (const auto &p : Items)
  {
   if (p->Name()==param->Name())
     throw L"MySqlParameters add name already exists";
  }
 Items.push_back(param);
}

/////////////////////////////////////////////////////////

MySqlError::MySqlError(int line, String const &state, String const &err)
{
 Line=line;
 State=state;
 Error=err;
}

////////////////////////////////////////////////////////


MySqlCommand::MySqlCommand(String const &sql, MySqlConnection const *conn)
{
 SQLRETURN rc;

 m_hStatement=0;
 m_SQL=sql;

 rc=::SQLAllocHandle(SQL_HANDLE_STMT, conn->Handle(), &m_hStatement);
 if (rc != SQL_SUCCESS)
   throw L"SQLAllocHandle failed, make sure connection is open";
}

MySqlCommand::~MySqlCommand()
{
 if (m_hStatement != 0)
   throw L"not disposed";
}

void MySqlCommand::ExecuteNonQuery(bool bIgnoreError)
{
 SQLRETURN rc;
 String msg;

 ParseSQL();

 rc = SQLExecDirect(m_hStatement, (SQLWCHAR *)m_SQLFinal.Chars(), (SQLINTEGER)m_SQL.Length());
 if (bIgnoreError == true)
   return;

 if (MySqlConnection::CheckReturn(rc, SQL_HANDLE_STMT, m_hStatement, m_Errors)==false)
  {
   msg = m_Errors[0].Error;
   msg += L"\n\n";
   msg += m_SQL;
   msg += L"\n\n";
   msg += m_SQLFinal;
   if (Parameters.Items.size() > 0)
    {
     msg += L"\n\nParameters";
     for (const auto &pm : Parameters.Items)
      {
       msg += L"\n";
       msg += pm->ToString();
      }
    }
   App->Response(msg);
   throw L"ExecuteNonQuery failed";
  }
}

MySqlDataReader *MySqlCommand::ExecuteReader()
{
 MySqlDataReader *reader;
 SQLRETURN rc;
 String msg;

 m_ParamUsageCount=ParseSQL();

 rc = ::SQLExecDirect(m_hStatement, (SQLWCHAR *)m_SQLFinal.Chars(), (SQLINTEGER)m_SQLFinal.Length());
 if (MySqlConnection::CheckReturn(rc, SQL_HANDLE_STMT, m_hStatement, m_Errors)==false)
  {
   msg = m_Errors[0].Error;
   msg += L"\n\n";
   msg += m_SQL;
   msg += L"\n\n";
   msg += m_SQLFinal;
   App->Response(msg);
   throw L"ExecuteNonQuery failed";
  }

 reader=new MySqlDataReader(); 
 reader->Bind(m_hStatement);
 
 return reader;
}

int MySqlCommand::ParseSQL()
{
 wchar_t c;
 String var;
 int order;
 int i;
 bool gather, quotes;

 m_SQLFinal.Clear();
 var.Clear();
 gather=false;
 quotes=false;
 order=1;

 for (i=0;i<m_SQL.Length();i++)
  {
   c=m_SQL[i];
   switch(c)
    {
     case L'@':
      {
       if (gather==true)
         throw L"encountered @ while gathering param name";
       if (quotes==false)
        {
         gather=true;
         var=L"@";
        }     
      } break;
     case L'\'':
      {
       if (gather==true)
         throw L"encountered single quote while gathering param name";
       if (quotes==true)
         quotes=false;
       m_SQLFinal+=c;
      } break;
     case L')':
     case L';':
     case L',':
     case L' ':
      {
       if (gather==true)
        {
         m_SQLFinal+=L'?';
         AddParam(var, order);
         order++;
         gather=false;
        }
       m_SQLFinal+=c;
      } break;
     default:
      {
       if (gather==false)
         m_SQLFinal+=c;
       else
         var+=c;
      }
    }
  }
 if (gather==true)
  {
   m_SQLFinal+=L'?';
   AddParam(var, order);
  }

 return order;
}

void MySqlCommand::AddParam(String const &name, int order)
{
 bool found;

 found=false;
 for (const auto &p : Parameters.Items)
  {
   if (String::Compare(p->Name(),name)==0)
    {
     p->Bind(m_hStatement, order);
     found=true;
    }
  }
 if (found==false)
   throw L"didn't find parameter 'pname'";
}

void MySqlCommand::Dispose()
{
 if (m_hStatement != 0) 
   SQLFreeHandle(SQL_HANDLE_STMT, m_hStatement);

 m_hStatement=0;
}