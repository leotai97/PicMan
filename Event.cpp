#include "pch.h"
#include "App.h"

KeyEventArgs::KeyEventArgs()
{
 ClearValues();
}

KeyEventArgs::KeyEventArgs(WPARAM wParam, LPARAM lParam)
{
 KeyCode=(Keyboard)wParam;
 KeyValue=(int)wParam;
 Shift=false;
 Capslock=false;
 Caps=false;
 Numlock=false;
 Alt=false;
 FirstPress= ( lParam & 1<<30 ) == 0;  // bit 30 zero means previous key state was up
 if ((GetKeyState(VK_CAPITAL) & 0x0001) == 1) Capslock = true;  // low bit is toggled numlock, caps
 if ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000) Shift = true;  // high bit is key is down
 if ((GetKeyState(VK_NUMLOCK) & 0x0001) == 1) Numlock=true;
 if ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000) Control = true;
 if ((GetKeyState(VK_MENU)    & 0x8000) == 0x8000) Alt = true;
 if (Shift == true || Capslock == true) Caps=true;
}

KeyEventArgs::KeyEventArgs(Keyboard code, Keyboard mdfr, bool bFirst)
{
  KeyCode = code;
  KeyValue = (int)code;
  Shift=false;
  Numlock=false;
  Alt=false;
  FirstPress = bFirst; 
};

void KeyEventArgs::ClearValues()
{
 KeyCode=Keyboard::None;
 KeyValue=0;
 Shift=false;
 Numlock=false;
 Alt=false;
 FirstPress=false;
}

std::wstring KeyEventArgs::Numeric() const
{
 switch(KeyCode)
  {
   case Keyboard::NumPad0:      if (Numlock) return L"0";
   case Keyboard::NumPad1:      if (Numlock) return L"1";
   case Keyboard::NumPad2:      if (Numlock) return L"2";
   case Keyboard::NumPad3:      if (Numlock) return L"3";
   case Keyboard::NumPad4:      if (Numlock) return L"4";
   case Keyboard::NumPad5:      if (Numlock) return L"5";
   case Keyboard::NumPad6:      if (Numlock) return L"6";
   case Keyboard::NumPad7:      if (Numlock) return L"7";
   case Keyboard::NumPad8:      if (Numlock) return L"8";
   case Keyboard::NumPad9:      if (Numlock) return L"9";

   case Keyboard::D1: if (Shift) return L""; else return L"1";
   case Keyboard::D2: if (Shift) return L""; else return L"2"; 
   case Keyboard::D3: if (Shift) return L""; else return L"3";
   case Keyboard::D4: if (Shift) return L""; else return L"4";
   case Keyboard::D5: if (Shift) return L""; else return L"5";
   case Keyboard::D6: if (Shift) return L""; else return L"6";
   case Keyboard::D7: if (Shift) return L""; else return L"7";
   case Keyboard::D8: if (Shift) return L""; else return L"8";
   case Keyboard::D9: if (Shift) return L""; else return L"9";
   case Keyboard::D0: if (Shift) return L""; else return L"0";
  }
 return L"";
}

std::wstring KeyEventArgs::Ascii() const
{
 switch(KeyCode)
  {
   case Keyboard::A: if (Caps) return L"A"; else return L"a";
   case Keyboard::B: if (Caps) return L"B"; else return L"b";
   case Keyboard::C: if (Caps) return L"C"; else return L"c";
   case Keyboard::D: if (Caps) return L"D"; else return L"d";
   case Keyboard::E: if (Caps) return L"E"; else return L"e";
   case Keyboard::F: if (Caps) return L"F"; else return L"f";
   case Keyboard::G: if (Caps) return L"G"; else return L"g";
   case Keyboard::H: if (Caps) return L"H"; else return L"h";
   case Keyboard::I: if (Caps) return L"I"; else return L"i";
   case Keyboard::J: if (Caps) return L"J"; else return L"j";
   case Keyboard::K: if (Caps) return L"K"; else return L"k";
   case Keyboard::L: if (Caps) return L"L"; else return L"l";
   case Keyboard::M: if (Caps) return L"M"; else return L"m";
   case Keyboard::N: if (Caps) return L"N"; else return L"n";
   case Keyboard::O: if (Caps) return L"O"; else return L"o";
   case Keyboard::P: if (Caps) return L"P"; else return L"p";
   case Keyboard::Q: if (Caps) return L"Q"; else return L"q";
   case Keyboard::R: if (Caps) return L"R"; else return L"r";
   case Keyboard::S: if (Caps) return L"S"; else return L"s";
   case Keyboard::T: if (Caps) return L"T"; else return L"t";
   case Keyboard::U: if (Caps) return L"U"; else return L"u";
   case Keyboard::V: if (Caps) return L"V"; else return L"v";
   case Keyboard::W: if (Caps) return L"W"; else return L"w";
   case Keyboard::X: if (Caps) return L"X"; else return L"x";
   case Keyboard::Y: if (Caps) return L"Y"; else return L"y";
   case Keyboard::Z: if (Caps) return L"Z"; else return L"z";     

   case Keyboard::Oemtilde: if (Shift) return L"~"; else return L"`";
   case Keyboard::D1: if (Shift) return L"!"; else return L"1";
   case Keyboard::D2: if (Shift) return L"@"; else return L"2"; 
   case Keyboard::D3: if (Shift) return L"#"; else return L"3";
   case Keyboard::D4: if (Shift) return L"$"; else return L"4";
   case Keyboard::D5: if (Shift) return L"%"; else return L"5";
   case Keyboard::D6: if (Shift) return L"^"; else return L"6";
   case Keyboard::D7: if (Shift) return L"&"; else return L"7";
   case Keyboard::D8: if (Shift) return L"*"; else return L"8";
   case Keyboard::D9: if (Shift) return L"("; else return L"9";
   case Keyboard::D0: if (Shift) return L")"; else return L"0";

   case Keyboard::OemMinus: if (Shift) return L"_"; else return L"-";
   case Keyboard::Oemplus:  if (Shift) return L"+"; else return L"=";

   case Keyboard::OemOpenBrackets: if (Shift) return L"{"; else return L"[";
   case Keyboard::OemCloseBrackets: if (Shift) return L"}"; else return L"]";
   case Keyboard::OemPipe: if (Shift) return L"\\"; else return L"|";
  
   case Keyboard::OemSemicolon: if (Shift) return L":"; else return L";";   
   case Keyboard::OemQuotes:    if (Shift) return L"\""; else return L"'";

   case Keyboard::Oemcomma:    if (Shift) return L"<"; else return L",";  
   case Keyboard::OemPeriod:   if (Shift) return L">"; else return L".";
   case Keyboard::OemQuestion: if (Shift) return L"?"; else return L"/";

   case Keyboard::NumPad0:      if (Numlock) return L"0";
   case Keyboard::NumPad1:      if (Numlock) return L"1";
   case Keyboard::NumPad2:      if (Numlock) return L"2";
   case Keyboard::NumPad3:      if (Numlock) return L"3";
   case Keyboard::NumPad4:      if (Numlock) return L"4";
   case Keyboard::NumPad5:      if (Numlock) return L"5";
   case Keyboard::NumPad6:      if (Numlock) return L"6";
   case Keyboard::NumPad7:      if (Numlock) return L"7";
   case Keyboard::NumPad8:      if (Numlock) return L"8";
   case Keyboard::NumPad9:      if (Numlock) return L"9";

   case Keyboard::Divide:   return L"/";
   case Keyboard::Multiply: return L"*";
   case Keyboard::Subtract: return L"-";
   case Keyboard::Add:      return L"+";
   case Keyboard::Space:    return L" ";

   case Keyboard::Decimal:  if (Numlock)  return L".";   
  }
 return L"";
}
MouseEventArgs::MouseEventArgs()
{
 Clear();
}

MouseEventArgs::MouseEventArgs(MouseEventArgs *m)
{
 Left=m->Left;
 Right=m->Right;
 Middle=m->Middle;

 DLeft=m->DLeft;
 DRight=m->DRight;
 DMiddle=m->DMiddle;

 X=m->X;
 Y=m->Y;
 
 Delta=m->Delta; 
}

MouseEventArgs::MouseEventArgs(MouseEventArgs const &m)
{
 Left=m.Left;
 Right=m.Right;
 Middle=m.Middle;

 DLeft=m.DLeft;
 DRight=m.DRight;
 DMiddle=m.DMiddle;

 X=m.X;
 Y=m.Y;
 
 Delta=m.Delta; 
}

MouseEventArgs::MouseEventArgs(HWND hWnd, WPARAM wParam, LPARAM lParam, DoubleClick dClick, int delta)
{
 POINT pt;

 GetCursorPos(&pt);
 ScreenToClient(hWnd, &pt);
 
 X = pt.x; // GET_X_LPARAM(lParam);
 Y = pt.y; // GET_Y_LPARAM(lParam);
  

 DLeft =   (dClick == DoubleClick::DCLeft);
 DRight =  (dClick == DoubleClick::DCRight);
 DMiddle = (dClick == DoubleClick::DCMiddle);

 if (dClick==DoubleClick::None)
  {
  Left = ((wParam & MK_LBUTTON) != 0);
  Right = ((wParam & MK_RBUTTON) != 0);
  Middle = ((wParam & MK_MBUTTON) != 0);
 }
else
 {
  Left = false;
  Right = false;
  Middle = false;
 }

 Delta=delta;
}

void MouseEventArgs::Clear()
{
 Left=false;
 Right=false;
 Middle=false;
 DLeft=false;
 DRight=false;
 DMiddle=false;
 X=0;
 Y=0;
 Delta=0;
}

