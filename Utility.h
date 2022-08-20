#pragma once

class String
{
 public:
 
 String();
 String(std::wstring const &str);
 String(String const &str);
 String(wchar_t const *str);

 inline void Clear() { m_Str.clear(); }

 static String FromStdWstring(std::wstring const &str);

 inline int Length() const { return (int)m_Str.size(); }

 wchar_t *Chars() const;

 static int Compare(String const &s1, String const &s2);  // case "insensitive" comparison
 static int Compare(std::wstring const &s1, std::wstring const &s2);
 static String Decimal(int value, int places);
 static String Hexadecimal(int value, int places);
 static String Decimal(int value);
 static String Double(double value);
 static String Double(double value, int places, int decimals);
 static String GetLastErrorMsg(DWORD val);
 static String LoadString(UINT id);
 static bool TryIntParse(String const &val, int *retVal);
 static bool TryDblParse(String const &val, double *retVal);
 static std::wstring ToLower(std::wstring const &val);
 static std::wstring ToLower(String const &val);
 static String Repeat(String const &val, int times);

 SizeF MeasureString(Gdiplus::Font *font);
 SizeF MeasureString(HFONT hFont);

 int IndexOf(wchar_t chr) const;
 int IndexOf(String const &str) const;
 int LastIndexOf(wchar_t const chr) const;
 int LastIndexOf(String const &str) const;
 
 String ToLower() const;
 String ToUpper() const;

 String Right(int rv) const;
 String Substring(int from, int len) const;
 String Substring(int from) const;

 std::vector<String> Split(wchar_t const chr) const;  // returns array of string, needs to be [] deleted
 std::vector<String> Split(String const &str) const;     // returns array of string, needs to be [] deleted

 String operator = (String const &s);
 String operator += (String const &s);
 String operator += (std::wstring const &s);  
 String operator += (wchar_t const c);
 String operator += (wchar_t const *c);
 String operator + (String const &s);
 String operator + (wchar_t const c);
 bool operator == (String const &s) const;
 bool operator == (wchar_t const chr) const;
 bool operator == (wchar_t const *str) const;
 bool operator != (String const &s) const;
 bool operator != (wchar_t const *chr) const;
 bool operator != (wchar_t const chr) const;
 bool operator () (const String &s1, const String &s2) const;
 bool operator < (const String &rhs) const;

 wchar_t operator [] (int i) const;


 bool Equal(String const &s) const;
 bool Equal(wchar_t const *str) const;
 bool Equal(wchar_t const c) const;

 String Trim() const;       // returns trim leading and tailing
 String TrimRight() const;  // returns strim tailing
 String TrimLeft() const;   // returns trim leading

 inline std::wstring StdString() { return m_Str; }

 static void Test();

private:

 std::wstring m_Str;

};

struct // sort Strings
 {
  bool operator()(const String &a, const String &b) const { return String::Compare(a,b) < 0; } // low to high
 } StringSort;

class Random
{
 public:
 Random();
 Random(int max);

 int Next();

 private:

 std::default_random_engine m_RE;
 std::uniform_int_distribution<int> m_Dist;
 
};

class RandomString
{
 public:
 RandomString(String const &txt, int rnd)
 {
  Text = txt;
  Random = rnd;
 }

 String Text;
 int    Random;
};

struct RandomStringSort
{
 bool operator()(RandomString const &a, RandomString const &b) const { return a.Random < b.Random; }
};

class DateTime
{
 public:

 enum class Format : int { YMD, MDYHMS, HMSAP, HMS };
 
 DateTime();
 DateTime(int y, int m, int d, int h, int mn, int s);
 bool IsValid();

 static DateTime Now();
 static DateTime FromSystemTime(SYSTEMTIME const &tm);
 static DateTime MinValue();
 static String GetMonth(int m);

 String ToString(Format fmt);

 bool operator == (DateTime const &d) const;
 bool operator != (DateTime const &d) const;
 bool operator > (DateTime const &d) const;
 bool operator < (DateTime const &d) const;


 bool IsMinValue();
 bool LeapYear();

 static void Test();

 int Year;
 int Month;
 int Day;
 int Hour;
 int Minute;
 int Second;
};

class Utility
{
 public:

 static int IntCompare(int x, int y)
  {
   if (x>y) return 1;
   if (x<y) return -1;
   return 0;
  }
 static bool StdIntCompare(int x, int y)
  {
   if (x<y) 
     return true;
   else
     return false;
  }
 static inline Rect GetRect(RECT const &r) { return Rect(r.left, r.top, r.right - r.left, r.bottom-r.top); }
 static inline Size GetSize(Rect const &r) { Size sz; r.GetSize(&sz); return sz; }

 static bool IsDarkThemeActive();
 static Color GetSystemColor(int syscolor);  // adds 0xff alpha

 static LPARAM AscendDescendColumn( bool ascDesc, int col);

 static bool FileExists(String const &path);
 static bool DirectoryExists(String const &path);
 static bool DirectoryCreate(String const &path, bool bShowError);
 
 static std::vector<String> GetFileNames(String const &directory, String const &filter);
 static std::vector<String> GetFolderNames(String const &directory);

 static std::vector<BYTE> Encrypt(String const &plain);
 static String Decrypt(std::vector<BYTE> const &encrypted);
};


