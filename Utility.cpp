#include "pch.h"
#include "app.h"

String::String()
{
 m_Str.clear();
}

String::String(std::wstring const &s)
{
 m_Str=s;
}

String::String(String const &s)
{
 m_Str=s.m_Str;
}

String::String(wchar_t const *str)
{
 m_Str=str;
}

String String::FromStdWstring(std::wstring const &str)
{
 String ret(str);
 return ret;
}

wchar_t *String::Chars() const
{
 return (wchar_t *)m_Str.c_str();
}

int String::Compare(String const &s1, String const &s2)
{
 int c;

 if (s1.Length() == 0 && s2.Length() == 0)
   return 0;

 if (s1.Length()==0) return -1;
 if (s2.Length()==0) return 1; 

 c = _wcsicmp(s1.Chars(), s2.Chars());

 if (c>0) return 1;
 if (c<0) return -1;
 return 0; 
}

int String::Compare(std::wstring const &s1, std::wstring const &s2)
{
 int c;

 if (s1.size() == 0 && s2.size() == 0)
   return 0;

 if (s1.size()==0) return -1;
 if (s2.size()==0) return 1; 

 c = _wcsicmp(s1.c_str(), s2.c_str());

 if (c>0) return 1;
 if (c<0) return -1;
 return 0; 
}

int String::IndexOf(wchar_t chr) const
{
 int i;
 
 if (Length()==0)
   return -1;

 for(i=0;i<Length();i++)
  {
   if (m_Str[i] == chr)
     return i; 
  }
 return -1;
}

int String::IndexOf(String const &str) const
{
 int i,c;

 if (str.Length()==0)
   return -1;

 if (str.Length() > Length())
   return -1; 

 c=Length() - (str.Length()-1);

 for(i=0;i<c;i++)
  {
   if (Substring(i, str.Length()) == str)
     return i; 
  }
 return -1;
}

int String::LastIndexOf(wchar_t const chr) const
{
 int i;

 if (Length() == 0)
   return -1;
 
 for(i=Length()-1;i>=0;i--)
  {
   if (m_Str[i] == chr)
     return i; 
  }
 return -1;
}


int String::LastIndexOf(String const &str) const
{
 int i,c;

 if (str.Length()==0)
   return -1;

 if (str.Length() > Length())
   return -1; 

 c=Length() - (str.Length()-1);

 for(i=c;i>=0;i--)
  {
   if (Substring(i, str.Length()) == str)
     return i; 
  }
 return -1;
}

String String::ToUpper() const
{
 String ret(m_Str);
 ::CharUpper((LPWSTR)ret.Chars()); 
 return ret;
}

String String::ToLower() const
{
 String ret(m_Str);
 ::CharLower((LPWSTR)ret.Chars()); 
 return ret;
}

std::wstring String::ToLower(std::wstring const &str)
{
 String v1, v2;
 std::wstring output;

 v1=str;
 v2=v1.ToLower();
 output = v2.Chars();

 return output;
}

std::wstring String::ToLower(String const &str)
{
 String v2;
 std::wstring output;

 v2=str.ToLower();
 output = v2.Chars();

 return output;
}

String String::Repeat(String const &val, int times)
{
 String out;
 int i;

 for (i=0;i<times;i++)
   out+=val;

 return out;
}

SizeF String::MeasureString(Gdiplus::Font *font)
{
 Gdiplus::StringFormat *format;
 HDC hDC;
 RectF brct, box;
 INT chars, lines; 

 format = new Gdiplus::StringFormat();
 format->SetAlignment(StringAlignment::StringAlignmentNear);
 format->SetFormatFlags(StringFormatFlagsNoWrap);
 format->SetLineAlignment(StringAlignmentNear);
  
 hDC = ::CreateCompatibleDC(0);
  {
   Gdiplus::Graphics g(hDC);
   brct = RectF(0, 0, 1920, 1200);
   g.MeasureString(m_Str.c_str(), (INT)m_Str.size(), font, brct, format, &box,  &chars, &lines);
  }
 ::DeleteDC(hDC);
 delete format;

 return SizeF(box.Width, box.Height);
}

SizeF String::MeasureString(HFONT hFont)
{
 HFONT old;
 HDC hDC;
 SIZE sz;

 hDC = ::CreateCompatibleDC(0);
 old = SelectFont(hDC, hFont); 
 
 GetTextExtentPoint32(hDC, m_Str.c_str(), (int)m_Str.size(), &sz);

 SelectFont(hDC, old);  

 ::DeleteDC(hDC);

 return SizeF(sz.cx, sz.cy);
}

String String::Right(int rv) const
{
 String ret;
 
 ret = Substring( Length() - rv, rv);
 return ret;
}

String String::Substring(int index) const
{
 String ret;

 if (index >= Length())
   throw L"index is beyond string boundry";

 ret=FromStdWstring(m_Str.substr(index));
 return ret;
}

String String::Substring(int index, int len) const
{
 String ret;

 if (index >= Length())
   throw L"index is beyond string boundry";

 ret=FromStdWstring(m_Str.substr(index, len));
 return ret;
}

std::vector<String> String::Split(wchar_t chr) const
{
 std::vector<String> list;
 String accum;
 int i;

 accum.Clear();

 for(i=0;i<Length();i++)
  {
   if (m_Str[i]==chr)
    {
     list.push_back(String(accum));
     accum.Clear();
    }
   else
     accum+=m_Str[i];
  }
 if (accum.Length()>0)
   list.push_back(String(accum));
 
 return list;
}

std::vector<String> String::Split(String const &str) const
{
 String ss;
 std::vector<String> list;
 String accum;
 int i;

 if (str.Length() > Length())
   throw L"Length of delimiter > than String";

 accum.Clear();

 for(i=0;i<Length();i++)
  {
   ss=Substring(i, str.Length());
   if (ss == str)
    {
     i+=str.Length()-1;
     list.push_back(String(accum));
     accum.Clear();
    }
   else
     accum+=m_Str[i];
  }
 if (accum.Length()>0)
   list.push_back(String(accum));

 return list;
}

String String::operator =(String const &s) 
{
 m_Str = s.Chars();
 return *this;
}

String String::operator +=(String const &s)
{
 std::wstring rv;

 rv=m_Str;
 rv += s.Chars();
 m_Str=rv;

 return *this;
}

String String::operator +=(std::wstring const &s)
{
 std::wstring rv;

 rv=m_Str;
 rv+=s;
 m_Str=rv;

 return *this;
}

String String::operator +=(wchar_t chr)
{
 std::wstring rv;

 rv=m_Str;
 rv+=chr;
 m_Str=rv;

 return *this;
}

String String::operator +=(wchar_t const  *chr)
{
 std::wstring rv;

 rv=m_Str;
 rv+=chr;
 m_Str=rv;

 return *this;
}

String String::operator+(String const &s)
{
 std::wstring rv;

 rv=m_Str;
 rv += s.Chars();
 m_Str=rv;
 return *this;
}

String String::operator+(wchar_t c)
{
 std::wstring rv;

 rv=m_Str;
 rv += c;
 m_Str=rv;
 return *this;
}

bool String::operator == (wchar_t c) const
{
 if (Length()!=1)
   return false;

 return m_Str[0]==c;
}

bool String::operator == (String const &s) const
{
 return Equal(s);
}

bool String::operator == (wchar_t const *pstr) const
{
 String c(pstr);
 return Equal(c);
}

bool String::operator != (wchar_t const c) const
{
 if (Length()!=1)
   return false;

 return !m_Str[0]==c;
}

bool String::operator != (String const &s) const
{
 return !Equal(s);
}

bool String::operator != (wchar_t const *pstr) const
{
 String c(pstr);
 return !Equal(c);
}

wchar_t String::operator [] (int i) const
{
 if (i<0) throw L"i < 0";
 if (i>=(int)m_Str.size()) throw L"i > string length";

 return m_Str[i];
}

bool String::operator <(const String &other) const
{
 return String::Compare(*this, other) < 0;
}

bool String::Equal(String const &str) const
{
 std::wstring compare=str.Chars();
 return compare == m_Str;
}

bool String::Equal(wchar_t const *str) const
{
 std::wstring compare=str;
 return compare == m_Str;
}

bool String::Equal(wchar_t const c) const
{
 std::wstring compare;

 compare=c;

 return compare == m_Str;
}

void String::Test()
{
 String t1, t2, t3, t4;
 
 String test(L"testing");

 t1=L"Test";
 t2=L"Test";
 t3=L"abc";
 t4=L"st";

 if (! t1.Equal(t2)) throw L"String() failed";
 if ( String::Compare(t1, t2) != 0) throw L"Compare failed";
 if ( String::Compare(t3, t1) >= 0) throw L"Compare failed";
 if ( String::Compare(t1, t3) <= 0) throw L"Compare failed";
 if ((t1 == t2)==false) throw L"== failed";
 if ((t1 != t2)==true) throw L"!= failed";
 if ((t1 == L"Test")==false) throw L" ==  wchar_t failed";
 if ((t3 == L"abc") == false) throw L"String(wchar) failed";
 if (t1.Length() != 4) throw L"Length failed";
 if (t1.Chars()[1]!= L'e') throw L"Chars failed";
 if (t1.Substring(0,1).Equal(L'T') == false) throw L"Substring failed";
 if (t1.Substring(1,1).Equal(L'e') == false) throw L"Substring failed";
 if (t1.Substring(1).Equal(L"est") == false) throw L"Substring failed";
 if (t1.Substring(1,2).Equal(L"es") == false) throw L"Substring failed";
 if (t1.IndexOf(L's') != 2) throw L"IndexOf failed";
 if (t1.IndexOf(L'z') != -1) throw L"IndexOf failed";
 if (t1.IndexOf(t4) != 2) throw L"IndexOf failed";
 if (t1.LastIndexOf(L'e') != 1) throw L"LastIndexOf failed";
 if (t1.LastIndexOf(t4) != 2) throw L"LastIndexOf failed";
 if (String::Compare(t1.ToUpper(),L"TEST") != 0) throw L"ToUpper failed";
 if (String::Compare(t1.ToUpper(),L"test") != 0) throw L"ToUpper failed";
 if (t1.Right(3) != L"est") throw L"Right failed"; 
 try
  {
   t1.Substring(5,1);
   throw L"bad Substring didn't throw";
  }
 catch(...)
  {
  }

 String t5=L"abc|def|ghi|jkl|mno|pqr|stu|vwy|z";
 String *s=nullptr;

 std::vector<String> list;

 list=t5.Split(L'|');
 if (list.size() != 9) throw L"split return failed"; 
 if (list[0] != L"abc") throw L"split failed";
 if (list[1] != L"def") throw L"split failed";
 if (list[2] != L"ghi") throw L"split failed";
 if (list[3] != L"jkl") throw L"split failed";
 if (list[8] != L"z") throw L"split failed";


 String t6=L"abc..def..ghi..jkl..mno..pqr..stu..vwy..z";
 String t7=L"..";
 list=t6.Split(t7);
 if (list.size() != 9) throw L"split return failed";
 if (list[0] != L"abc") throw L"split failed";
 if (list[1] != L"def") throw L"split failed";
 if (list[2] != L"ghi") throw L"split failed";
 if (list[3] != L"jkl") throw L"split failed";
 if (list[8] != L"z") throw L"split failed";

 t1=String::Decimal(1234);
 if (t1 != L"1234") throw L"Decimal failed";

 t1=String::Decimal(36,4);
 if (t1 != L"0036") throw L"Decimal failed";

 t1=String::Hexadecimal(32,4);
 if (t1 != L"0x0020") throw L"hexadecimal failed";
 
 t1=L"10";
 int v=0;
 double d;
 if (String::TryIntParse(t1, &v)==false || v!=10) throw L"TryIntParse failed";
 if (String::TryDblParse(t1, &d)==false || d!=10.0) throw L"TryIntParse failed";
 
 t1=L"a";
 t2=L"b";
 t3=L"c";
 String f;

 f=t1 + t2 + t3;
 if (f != L"abc") throw L"concat failed"; 

 t1=L"        test";
 t2=t1.TrimLeft();
 if (t2 != L"test") throw L"TrimLeft return failed";

 t1=L"test                    ";
 t2=t1.TrimRight();
 if (t2 != L"test") throw L"TrimRight return failed";

 t1=L"                         te  st                    ";
 t2=t1.Trim();
 if (t2 != L"te  st") throw L"Trim return failed";

 t1=L"test";
 if (t1[1] != L'e') throw L"[] operator failed";

 t2=L"Test";
 if ( ! (t1.ToLower() == t2.ToLower()) ) throw L"To lower comparison failed";
 if ( t1.ToLower() != t2.ToLower() ) throw L"Tolower failed";

}

String String::Decimal(int value, int places)
{
 String ws, pl;
 wchar_t buffer[100];

 ws=L"%0";
 ws+=Decimal(places);
 ws+=L'd';

 ::swprintf_s(buffer, 100, ws.Chars(), value);
 ws=buffer;

 return ws;
}

String String::Decimal(int value)
{
 String ws;
 wchar_t buffer[100];

 ::swprintf_s(buffer, 100, L"%d", value);
 ws=buffer;

 return ws;
}

String String::Hexadecimal(int val, int places)
{
 String ws, pl;
 wchar_t buffer[100];

 ws=L"0x%0";
 ws+=Decimal(places);
 ws+=L"x";

 ::swprintf_s(buffer, 100, ws.Chars(), val);
 ws=buffer;

 return ws;

}

String String::Double(double value)
{
 String ws;
 wchar_t buffer[100];

 ::swprintf_s(buffer, 100, L"%f", value);
 ws=buffer;

 return ws;
}

String String::Double(double value, int places, int decimals)
{
 String arg;
 String ws;
 wchar_t buffer[100];

 arg = L"%";
 arg += Decimal(places);
 arg + L".";
 arg += Decimal(decimals);
 arg += L"f";

 ::swprintf_s(buffer, 100, arg.Chars(), value);
 ws=buffer;

 return ws;
}


String String::GetLastErrorMsg(DWORD val)
{
 String ws;
 wchar_t err[256];
 memset(err, 0, 256);

 ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, val, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255, NULL);
 ws=err;
 return ws;
}

bool String::TryIntParse(String const &iVal, int *retVal)
{
 String check;
 String val;
 int v;
 int i;  

 check = L"0123456789";

 val = iVal.Trim();  // trim spaces

 for (i=0;i<val.Length();i++)  // have to manually check for invalid chrs in the number
  {
   if (check.IndexOf(val[i]) == -1)
    {
     if (i > 0) 
       return false; // found a non numeric chr beyond the 1st
     else
      {
       if (val[i] != L'-') 
         return false;  // only #s and negative sign allowed in 1st position 
      } 
    }
  }

 try
  {
   v=std::stoi(val.Chars(), nullptr, 10);  // atoi will work as long as start of string is numeric
   *retVal=v;
   return true;
  }
 catch(...)
  {
   return false;
  }
}

bool String::TryDblParse(String const &val, double *retVal)
{
 double v;
 try
  {
   v=std::stod(val.Chars(), nullptr);
   *retVal=v;
   return true;
  }
 catch(...)
  {
   return false;
  }
}


String String::LoadString(UINT id)
{
 String ret;
 wchar_t *szText;

 szText=new wchar_t[1000];
 ::LoadString(App->Instance(), id, szText, 1000);
 ret = szText;
 delete [] szText;
 return ret;
}

String String::Trim() const
{
 String ret;

 ret=TrimRight();
 return ret.TrimLeft();
}

String String::TrimLeft() const // trailing spaces
{
 String ns;
 int i,ni,c;

 c=(int)m_Str.size();
 ni=-1;
 ns.Clear();

 for(i=0;i<c;i++)
  {
   if (m_Str[i] != L' ')
    {
     ni=i;
     break;
    }
  } 

 if (ni==0)
   return m_Str;  // no leading spaces;

 if (ni == -1)
  {
   return ns;
  }

 ns=m_Str.substr(ni);
 return ns;
}

String String::TrimRight() const // leading spaces
{
 String ns;
 int i,ni,c;

 c=(int)m_Str.size();
 ni=-1;
 ns.Clear();

 for(i=c-1; i>=0; i--)
  {
   if (m_Str[i] != L' ')
    {
     ni=i;
     break;
    }
  } 

 if (ni==c-1)
   return m_Str;  // no trailing spaces;

 if (ni == -1)
  {
   return ns;  // was all spaces
  }

 ns=m_Str.substr(0, ni+1);
 return ns;
}
/////////////////////////////////////////////////
Random::Random()
{
 std::random_device r;
    
 m_RE = std::default_random_engine(r());
 m_Dist = std::uniform_int_distribution<int>(1, INT_MAX);
}

Random::Random(int max) : Random()
{
 m_Dist = std::uniform_int_distribution<int>(1, max);
}

int Random::Next()
{
 return m_Dist(m_RE);
}


////////////////////////////////////////////////


DateTime::DateTime()
{
 Year=0;
 Month=0;
 Day=0;
 Hour=0;
 Minute=0;
 Second=0;
}

DateTime::DateTime(int y, int m, int d, int h, int mn, int s)
{
 Year=y;
 Month=m;
 Day=d;
 Hour=h;
 Minute=mn;
 Second=s;
}

DateTime DateTime::MinValue()
{
 DateTime date;

 date.Year=0;
 date.Month=0;
 date.Day=0;
 date.Hour=0;
 date.Minute=0;
 date.Second=0; 

 return date;
}

bool DateTime::IsMinValue()
{
 if (Year ==0 && Month==0 && Day==0) return true;

 return false;
}


bool DateTime::IsValid()
{
 // if year is divisible by 4 and is not divisible by 100 it's leap 
 // if year divisible by 400 it's leap

 if ( Year<1532 )           return false; // start of gregorian calendar
 if ( Month<1 || Month>12 ) return false;
 if ( Day<1 || Day>31)      return false;

 if ( Month == 2 || Month == 4 || Month ==6 || Month == 9 || Month==11)
  {
   if ( Day==31 ) return false;
  }
 if (Month == 2)
  {
   if ( Day == 30 ) return false;
   if ( Day == 29 )
    {
     if ( Year % 4 == 0 && Year % 100 ) return true;
     if ( Year % 400 ) return true;
    }
  }
 return true;
}

bool DateTime::LeapYear()
{
 if ( Year % 4 == 0 && Year % 100 ) return true;
 if ( Year % 400 ) return true;

 return false;
}

DateTime DateTime::Now()
{
 DateTime date;
 time_t current_time;
 tm     local_time;

 time ( &current_time );
 localtime_s(&local_time, &current_time);

 date.Year   = local_time.tm_year + 1900;
 date.Month  = local_time.tm_mon + 1;
 date.Day    = local_time.tm_mday;

 date.Hour   = local_time.tm_hour;
 date.Minute = local_time.tm_min;
 date.Second = local_time.tm_sec;

 return date;
}

DateTime DateTime::FromSystemTime(SYSTEMTIME const &st)
{
 DateTime date;

 date.Year   = st.wYear;
 date.Month  = st.wMonth;
 date.Day    = st.wDay;

 date.Hour   = st.wHour;
 date.Minute = st.wMinute;
 date.Second = st.wYear;

 return date;
}

String DateTime::ToString(DateTime::Format fmt)
{
 String ret;

 switch(fmt)
  {
   case DateTime::Format::YMD:
    {
      ret =String::Decimal(Year,4);
      ret+=L'/';
      ret+=String::Decimal(Month,2);
      ret+=L'/';
      ret+=String::Decimal(Day,2);
      return ret;
    } 
   case DateTime::Format::MDYHMS:
    {
     ret = GetMonth(Month);
     ret+=L" ";
     ret+=String::Decimal(Day);
     ret+=L" ";
     ret+=String::Decimal(Year, 4);
     ret+=L" ";
     ret+=ToString(DateTime::Format::HMSAP);
     return ret;
    } 
   case DateTime::Format::HMS:
    {
     ret =String::Decimal(Hour, 2);
     ret+=L":";
     ret+=String::Decimal(Minute,2);
     ret+=L":";
     ret+=String::Decimal(Second,2);
     return ret;
    } 
   case DateTime::Format::HMSAP:
    {
     int h;
     if (Hour>12) h=Hour-12; else h=Hour;
     ret =String::Decimal(h, 2);
     ret+=L":";
     ret+=String::Decimal(Minute,2);
     ret+=L":";
     ret+=String::Decimal(Second,2);
     if (Hour>12) ret+=L"AM"; else ret+=L"PM";
     return ret;
    } 
  }
 return ret;
}

String DateTime::GetMonth(int m)
{
 String s;

 switch(m)
  {
   case 1: s=L"Jan"; break;
   case 2: s=L"Feb"; break;
   case 3: s=L"Mar"; break;
   case 4: s=L"Apr"; break;
   case 5: s=L"May"; break;
   case 6: s=L"Jun"; break;
   case 7: s=L"Jul"; break;
   case 8: s=L"Aug"; break;
   case 9: s=L"Sep"; break;
   case 10: s=L"Oct"; break;
   case 11: s=L"Nov"; break;
   case 12: s=L"Dec"; break;
  }
 return s;
}

bool DateTime:: operator == (DateTime const &dt) const
{
 if (dt.Year != Year) return false;
 if (dt.Month != Month) return false;
 if (dt.Day != Day) return false;
 if (dt.Hour != Hour) return false;
 if (dt.Minute != Minute) return false;
 if (dt.Second != Second) return false;
 return true;
}

bool DateTime:: operator != (DateTime const &dt) const
{
 if (dt.Year != Year) return true;
 if (dt.Month != Month) return true;
 if (dt.Day != Day) return true;
 if (dt.Hour != Hour) return true;
 if (dt.Minute != Minute) return true;
 if (dt.Second != Second) return true;
 return false;
}


bool DateTime:: operator < (DateTime const &dt) const
{
 if (Year < dt.Year) return true;
 if (Year > dt.Year) return false;

 if (Month < dt.Month) return true;
 if (Month > dt.Month) return false;

 if (Day < dt.Day) return true;
 if (Day > dt.Day) return false;

 if (Hour < dt.Hour) return true;
 if (Hour > dt.Hour) return false;

 if (Minute < dt.Minute) return true;
 if (Minute > dt.Minute) return false;

 if (Second < dt.Second) return true;

 return false;
}

bool DateTime:: operator > (DateTime const &dt) const
{
 if (Year > dt.Year) return true;
 if (Year < dt.Year) return false;

 if (Month > dt.Month) return true;
 if (Month < dt.Month) return false;

 if (Day > dt.Day) return true;
 if (Day < dt.Day) return false;

 if (Hour > dt.Hour) return true;
 if (Hour < dt.Hour) return false;

 if (Minute > dt.Minute) return true;
 if (Minute < dt.Minute) return false;

 if (Second > dt.Second) return true;

 return false;
}

void DateTime::Test()
{
 DateTime d1(2002, 12, 1, 3, 4, 5);
 DateTime d2(2002, 12, 1, 3, 4, 5);
 DateTime d3(2002, 12, 1, 3, 4, 6);
 
 if (d1.ToString(DateTime::Format::YMD) != L"2002/12/01") throw L"Tostring failed";
 
 if (!(d1 == d2)) throw L" == failed, they are equal";
 if ( (d1 == d3)) throw L" == failed, they are not equal";
 if ( (d1 != d2)) throw L" != failed, they are equal";
 if( !(d1 != d3)) throw L" != failed, they are not equal";

 if ( (d1 > d2)) throw L" > failed they are equal";
 if ( (d1 < d2)) throw L" < failed they are equal";

 if ( !(d1 < d3)) throw L" < failed, d1 is < d3";
 if ( !(d3 > d1)) throw L" < failed, d3 is > d3";
}

// //////////////////////////////////////////////

bool Utility::IsDarkThemeActive()
{
    DWORD   type;
    DWORD   value;
    DWORD   count = 4;
    LSTATUS st = RegGetValue(
        HKEY_CURRENT_USER,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        TEXT("AppsUseLightTheme"),
        RRF_RT_REG_DWORD,
        &type,
        &value,
        &count );
    if ( st == ERROR_SUCCESS && type == REG_DWORD )
        return value == 0;
    return false;
}


Color Utility::GetSystemColor(int syscolor)
{
 DWORD c = ::GetSysColor(syscolor);
 int a,r,g,b;

 r=GetRValue(c);
 g=GetRValue(c);
 b=GetRValue(c);
 a=0xff;
 return Color(a,r,g,b);
}

LPARAM Utility::AscendDescendColumn(bool asc, int col)
{
 LPARAM v=0;

 if (asc == true)
   v = 0x0100;
 else
   v = 0x0000;

 v |= col;

 return v; 
}

bool Utility::FileExists(String const &path)
{ 
 struct _stat status;

 if ( _taccess_s( path.Chars(), 0 ) == 0 )
  { 
   return true; 
  }
 return false;
}

bool Utility::DirectoryExists(String const &path)
{ 
 struct _stat status;

 if ( _taccess_s( path.Chars(), 0 ) == 0 )
  {  
   _tstat( path.Chars(), &status );
   return (status.st_mode & S_IFDIR) != 0;
  }
 return false;
}


bool Utility::DirectoryCreate(String const &path, bool bShowError)
{
 DWORD err;
 String msg;

 if (::CreateDirectory(path.Chars(), 0) == FALSE)
  {
   err = GetLastError();
   switch(err)
    {
     case ERROR_ALREADY_EXISTS: msg = L"Directory Already Exists: "; break;
     case ERROR_PATH_NOT_FOUND: msg = L"Part of the path not found: "; break;
     default: msg = L"Unknown Error: "; break;
    }
   msg += path;
   if (bShowError == true)
     App->Response(msg);
   return false;
  }

 return true;
}

std::vector<String> Utility::GetFileNames(String const &dir, String const &filter)
{
 WIN32_FIND_DATA data;
 HANDLE fr;
 std::vector<String> list;
 String sSearch;
 void *oldVal;

 Wow64DisableWow64FsRedirection(&oldVal);

 if (DirectoryExists(dir) == false)
   return list;

 #ifdef _DEBUG
 if (dir.Substring(dir.Length()-1,1) == L"\\") throw L"dir cannot end with \\";
 #endif

 sSearch = dir;
 sSearch += L"\\";
 sSearch += filter;

 fr = ::FindFirstFile(sSearch.Chars(), &data);
 if (fr == INVALID_HANDLE_VALUE)
  {
   return list;  // empty
  }
 do
  {
   list.push_back(data.cFileName);
  }
 while(::FindNextFile(fr, &data)==TRUE);
 ::FindClose(fr);

 Wow64RevertWow64FsRedirection(&oldVal);

 return list;
}

std::vector<String> Utility::GetFolderNames(String const &dir)
{
 WIN32_FIND_DATA data;
 HANDLE fr;
 std::vector<String> list;
 String sSearch;
 void *oldVal;

 Wow64DisableWow64FsRedirection(&oldVal);

 if (DirectoryExists(dir) == false)
   return list;

 sSearch = dir;
 sSearch += L"\\*";

 fr = ::FindFirstFileEx(sSearch.Chars(), FindExInfoStandard, &data, FindExSearchLimitToDirectories, NULL, 0);
 if (fr == INVALID_HANDLE_VALUE)
  {
   return list;  // empty
  }
 do
  {
   list.push_back(data.cFileName);
  }
 while(::FindNextFile(fr, &data)==TRUE);
 ::FindClose(fr);

 Wow64RevertWow64FsRedirection(&oldVal);


 return list;
}

static const BYTE rgbIV[] =
{
    0xF1, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0xAD, 0x0D, 0x0E, 0x0F
};

static const BYTE rgbAES128Key[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0xB5, 0x06, 0x07, 
    0x08, 0xC7, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

std::vector<BYTE> Utility::Encrypt(String const &plain)
{
 BCRYPT_ALG_HANDLE hAesAlg;
 BCRYPT_KEY_HANDLE hKey;
 DWORD cbKeyObject, cbData, cbBlockLen, cbPlainText, cbCipherText;
 BYTE *pbKeyObject, *pbIV, *plainBuff, *pbPlainText, *pbCipherText;
 NTSTATUS ret;
 std::vector<BYTE> output;
 int nLen;

 nLen = plain.Length() * sizeof(wchar_t);
 if (nLen==0)
   return output;

 plainBuff = new BYTE[nLen];
 memcpy(plainBuff, plain.Chars(), nLen);

 ret = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
 if (ret < 0)
   throw L"failed";

 ret = BCryptGetProperty(hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0); 
 if (ret < 0)
   throw L"failed";

 pbKeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbKeyObject);
 if (pbKeyObject == nullptr)
   throw L"failed";

 ret = BCryptGetProperty(hAesAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbBlockLen, sizeof(DWORD), &cbData, 0);
 if (ret < 0)
   throw L"failed";

 if (cbBlockLen > sizeof (rgbIV))
   throw L"failed";

 pbIV = (PBYTE) HeapAlloc (GetProcessHeap (), 0, cbBlockLen);
  if(pbIV == nullptr)
    throw L"faied";
 
 memcpy(pbIV, rgbIV, cbBlockLen);

 ret = BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
 if (ret < 0) 
   throw L"failed";

 ret = BCryptGenerateSymmetricKey(hAesAlg, &hKey, pbKeyObject, cbKeyObject, (PBYTE)rgbAES128Key, sizeof(rgbAES128Key), 0);
 if (ret < 0)
   throw L"failed";

 cbPlainText = nLen;
 pbPlainText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbPlainText);
 if (pbPlainText == nullptr)
   throw L"failed";

 memcpy(pbPlainText, plainBuff, nLen);

 ret = BCryptEncrypt(hKey, pbPlainText, cbPlainText, NULL, pbIV, cbBlockLen, NULL, 0, &cbCipherText, BCRYPT_BLOCK_PADDING);
 if (ret < 0)
   throw L"failed";

 pbCipherText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbCipherText);
 if(pbCipherText == nullptr)
   throw L"failed";
 
 ret = BCryptEncrypt(hKey, pbPlainText, cbPlainText, NULL, pbIV, cbBlockLen, pbCipherText, cbCipherText, &cbData, BCRYPT_BLOCK_PADDING);
 if (ret < 0)
   throw L"failed";

 for (DWORD i=0;i<cbCipherText;i++)
  {
   output.push_back(pbCipherText[i]);
  }

 if (hAesAlg)
   BCryptCloseAlgorithmProvider(hAesAlg,0);
    
 if (hKey)    
   BCryptDestroyKey(hKey);
    

 if (pbCipherText)
   HeapFree(GetProcessHeap(), 0, pbCipherText);
   
 if (pbPlainText)
   HeapFree(GetProcessHeap(), 0, pbPlainText);
    
 if (pbKeyObject)
   HeapFree(GetProcessHeap(), 0, pbKeyObject);

 if (pbIV)
  HeapFree(GetProcessHeap(), 0, pbIV);

 if (plainBuff)
   delete [] plainBuff;

 return output;
}

String Utility::Decrypt(std::vector<BYTE>const &encrypted)
{
 BCRYPT_ALG_HANDLE hAesAlg;
 BCRYPT_KEY_HANDLE hKey;
 DWORD cbKeyObject, cbData, cbBlockLen, cbBlob, cbPlainText, cbCipherText, ic;
 BYTE *pbKeyObject, *pbIV, *plainBuff, *pbPlainText, *pbCipherText;
 wchar_t *outputBuff;
 NTSTATUS ret;
 String d;
 int nLen;

 if (encrypted.size() == 0)
   return d;

 ret = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
 if (ret < 0)
   throw L"failed";

 ret = BCryptGetProperty(hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0); 
 if (ret < 0)
   throw L"failed";

 pbKeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbKeyObject);
 if (pbKeyObject == nullptr)
   throw L"failed";

 ret = BCryptGetProperty(hAesAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbBlockLen, sizeof(DWORD), &cbData, 0);
 if (ret < 0)
   throw L"failed";

 if (cbBlockLen > sizeof (rgbIV))
   throw L"failed";

 pbIV = (PBYTE) HeapAlloc (GetProcessHeap (), 0, cbBlockLen);
  if(pbIV == nullptr)
    throw L"faied";
 
 memcpy(pbIV, rgbIV, cbBlockLen);

 ret = BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
 if (ret < 0) 
   throw L"failed";

 ret = BCryptGenerateSymmetricKey(hAesAlg, &hKey, pbKeyObject, cbKeyObject, (PBYTE)rgbAES128Key, sizeof(rgbAES128Key), 0);
 if (ret < 0)
   throw L"failed";

 cbPlainText = encrypted.size();
 pbPlainText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbPlainText);
 if (pbPlainText == nullptr)
   throw L"failed";

 cbCipherText = encrypted.size();
 pbCipherText = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbCipherText);
 for (ic = 0; ic < encrypted.size(); ic++)
   pbCipherText[ic] = encrypted[ic];

 ret = BCryptDecrypt(hKey, pbCipherText, cbCipherText, NULL, pbIV, cbBlockLen, pbPlainText, cbPlainText, &cbPlainText, BCRYPT_BLOCK_PADDING);
 if (ret < 0)
   throw L"failed";

 nLen = cbPlainText / sizeof(wchar_t);

 outputBuff = new wchar_t[nLen];
 memcpy(outputBuff, pbPlainText, cbPlainText);
 for(int i=0; i<nLen;i++)
  d += outputBuff[i];
 delete [] outputBuff;


 if (hAesAlg)
   BCryptCloseAlgorithmProvider(hAesAlg,0);
    
 if (hKey)    
   BCryptDestroyKey(hKey);
    

 if (pbCipherText)
   HeapFree(GetProcessHeap(), 0, pbCipherText);
   
 if (pbPlainText)
   HeapFree(GetProcessHeap(), 0, pbPlainText);
    
 if (pbKeyObject)
   HeapFree(GetProcessHeap(), 0, pbKeyObject);

 if (pbIV)
  HeapFree(GetProcessHeap(), 0, pbIV);

 return d;
}


