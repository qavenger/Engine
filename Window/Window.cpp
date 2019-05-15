#include "../Shared/pch.h"
#include "Window.h"

void Window::Initialize(WNDPROC wndProc)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = Title.c_str();
	wcex.lpfnWndProc = wndProc;
	
	if (!RegisterClassEx(&wcex))
	{
		OutputDebugString(L"Failed Register Window Class");
		PostQuitMessage(-1);
	}
	
	UINT dwStyle;
	int x, y;
	if (m_windowed)
	{
		dwStyle = WS_OVERLAPPEDWINDOW;
		x = CW_USEDEFAULT;
		y = CW_USEDEFAULT;
	}
	else
	{
		dwStyle = WS_POPUP;
		x = 0;
		y = 0;
	}

	m_hWnd = CreateWindow(
		wcex.lpszClassName, 
		wcex.lpszClassName, 
		dwStyle, 
		x, y, 
		Width, Height, 
		nullptr, nullptr, 
		wcex.hInstance, 
		0
	);

	if (!m_hWnd)
	{
		PostQuitMessage(-1);
		OutputDebugString(L"Failed Create Window");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}

void Window::OnResize(UINT width, UINT height)
{
	Width = width;
	Height = height;
	RecalculateAspectRatio();
}
