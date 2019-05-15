#pragma once
#define MAX_KEY_NUM 256
class Input
{
public:
	enum class KeyState
	{
		UP,				//0b00
		PRESSED,		//0b01
		RELEASED,		//0b10
		HOLD,			//0b11
	};

	enum class KeyCode
	{
		LMouse = 0x01, RMouse, Cancel, MMouse, XButton1, XButton2,
		Back = 0x08, Tab,
		Clear = 0x0C, Return,
		Shift = 0x10, Control, Menu, Pause, Caps_Lock,
		Escape = 0x1B,
		Space = 0x20, Page_Up, Page_Down, End, Home, Left, Up, Right, Down, Select,
		Print, Execute, Print_Screen, Insert, Delete, Help,
		Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
		A = 0x41,
		B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		LWin, RWin, App,
		Sleep = 0x5F,
		NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
		NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
		Multiply, Add, Separator, Subtract, Decimal, Divide,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20,
		F21, F22, F23, F24,
		Num_Lock = 0x90, Scroll,
		LShift = 0xA0, RShift,
		LControl, RControl, LMenu, RMenu, Browser_Back,
		Browser_Forward, Browser_Refresh, Browser_Stop,
		Browser_Search, Browser_Favorites, Browser_Home,
		Volumn_Mute, Volumn_Down, Volumn_Up,
		Media_Next_Track, Media_Previous_Track, Media_Stop, Media_Play_Pause,
		NUM_KEYCODE
	};

	static bool IsKeyHold(KeyCode keyCode);
	static bool IsKeyPressed(KeyCode keyCode);
	static bool IsKeyReleased(KeyCode keyCode);
	static void GetMousePosition(int& x, int& y);
	static void GetScreenMousePosition(int& x, int& y);
	static void GetMouseRelativePosition(int& x, int& y);
	static void Initialize(HWND hWnd);
	static void UpdateInput();
private:
	static unsigned char m_keyState[MAX_KEY_NUM];
	static bool m_capLock;

	static POINT m_prevMousePos;
	static POINT m_mousePos;
	static HWND m_hWnd;
};

