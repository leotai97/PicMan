#pragma once

class MySqlError
{
 public:
 MySqlError(int line, String const &state, String const &err);

 int Line;
 String State;
 String Error;
};

class MySqlConnection
{
 public:
 MySqlConnection(String const &constr);
~MySqlConnection();

 bool Open();
 void Close();

 inline SQLHDBC Handle() const { return m_hODBC; }
 inline bool IsOpen() { return m_Open; }
 inline String GetConnectionString() { return m_ConnectionString; }

 static bool CheckReturn(SQLRETURN rc, SQLSMALLINT handleType, SQLHANDLE handle, std::vector<MySqlError> &errors);
 static String ConnectionString(String const &server, String const &dbName, String const &user, String const &pwd);

 private:

 bool m_Open;

 
 String m_ConnectionString;
 SQLHENV m_hEnvironment;
 SQLHDBC m_hODBC;

};

class MySqlData
{
 public:
 MySqlData();

~MySqlData();

 void Bind(SQLHSTMT hStmt, SQLSMALLINT nCol);

 String GetString();
 int GetInt32();
 DateTime GetDateTime();
 double GetDouble();
 BYTE *GetBytes();

 inline bool IsNull();
 inline int Length();

 private:

 bool m_IsNullable;

 String m_ColumnName;

 int          m_Column; // column number
 SQLLEN       m_Ind; // binary and string
 int          m_ColumnSize; //  
 int          m_DataType;
 int          m_CType;

 SQLCHAR              *m_Binary;
 SQL_TIMESTAMP_STRUCT  m_DT;
 WCHAR                *m_Str;

 SQLINTEGER   m_Val;
 SQLDOUBLE    m_Float;
 
};


class MySqlDataReader
{
 public:

 MySqlDataReader();
~MySqlDataReader();

 String GetString(int col);
 DateTime GetDateTime(int col);
 int GetInt32(int col);
 double GetDouble(int col);
 BYTE *GetBytes(int col);
 Bitmap *GetBitmap(int col);

 SQLULEN Length(int col);
 

 void Bind(SQLHSTMT hStmt);
 bool Read();
 void Close();
 inline int FieldCount() { return m_Columns; }
 bool IsClosed() { return m_hStatement == 0; }

 bool IsDBNull(int col);
 bool ValidColumn(int col);

 private:

 bool        m_HasColumns;
 bool        m_HasData;
 int         m_Columns;
 MySqlData **m_Data;
 SQLHSTMT    m_hStatement;
 
};

class MySqlParameter
{
 public:

 MySqlParameter(String const &name, int val);
 MySqlParameter(String const &name, String const &val);
 MySqlParameter(String const &name, DateTime const &dt);
 MySqlParameter(String const &name, BYTE *arr, int len);
 MySqlParameter(String const &name, Bitmap *bmp);

~MySqlParameter();

 void Bind(SQLHSTMT hState, int pn);

 inline String Name() const { return m_Name; }

 String ToString() const;

 private:

 String m_Name;
 int m_SQLType;
 int m_Len;

 int                   m_IntVal;
 String                m_StrVal;
 SQL_TIMESTAMP_STRUCT  m_DateVal;
 BYTE                 *m_ByteVal;

 SQLLEN                m_BinaryLength; // passed to SQLBindParameter for binary types

};

class MySqlParameters
{
 public:
 MySqlParameters();
~MySqlParameters();

 void Add(MySqlParameter *param);
 void Clear();

 std::vector<MySqlParameter *> Items;
};

class MySqlCommand
{
 public:
 MySqlCommand();
 MySqlCommand(String const &sql, MySqlConnection const *con);
~MySqlCommand();

 MySqlDataReader *ExecuteReader();
 void ExecuteNonQuery(bool bIngnoreError=false);
 void Dispose();

 private:

 int ParseSQL();
 void AddParam(String const &name, int order);

 public:

 MySqlParameters Parameters;
 

 private:
  
 String m_SQL;      // input containing @param
 String m_SQLFinal; // with ? instead of @param
 SQLHSTMT m_hStatement;
 int m_ParamUsageCount;   // # of times parameters were used

 std::vector<MySqlError> m_Errors;
};


