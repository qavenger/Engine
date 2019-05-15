#pragma once
class Window
{
public:
	void Initialize(WNDPROC wndProc);
	void OnResize(UINT width, UINT height);
	HWND Handle()const { return m_hWnd; }
	bool WindowMode()const { return m_windowed; }
	void SetWindowMode(bool enable) { m_windowed = enable; }
private:
	void RecalculateAspectRatio() { AspectRatio = (float)Width / Height; }
public:
	std::wstring		Title;
	UINT				Width : 16;
	UINT				Height : 16;
	FLOAT				AspectRatio;
private:
	HWND				m_hWnd;
	BYTE				m_windowed : 1;
};

