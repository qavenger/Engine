#include "../Shared/pch.h"
#include "Input.h"

unsigned char Input::m_keyState[MAX_KEY_NUM];
bool Input::m_capLock;
POINT Input::m_prevMousePos;
POINT Input::m_mousePos;
HWND Input::m_hWnd;

void Input::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;
	memset(m_keyState, 0, MAX_KEY_NUM);
}

bool Input::IsKeyHold(KeyCode keyCode)
{
	return m_keyState[(int)keyCode] & (WORD)KeyState::PRESSED;
}

bool Input::IsKeyPressed(KeyCode keyCode)
{
	return (m_keyState[(int)keyCode] & 0b11) == (WORD)KeyState::PRESSED;
}

bool Input::IsKeyReleased(KeyCode keyCode)
{
	return (m_keyState[(int)keyCode] & 0b11) == (WORD)KeyState::RELEASED;
}

void Input::GetMousePosition(int & x, int & y)
{
	POINT pos = m_mousePos;
	ScreenToClient(m_hWnd, &pos);
	x = pos.x;
	y = pos.y;
}

void Input::GetScreenMousePosition(int & x, int & y)
{
	x = m_mousePos.x;
	y = m_mousePos.y;
}

void Input::GetMouseRelativePosition(int & x, int & y)
{
	x = m_mousePos.x - m_prevMousePos.x;
	y = m_mousePos.y - m_prevMousePos.y;
}

void Input::UpdateInput()
{
	m_prevMousePos = m_mousePos;
	GetPhysicalCursorPos(&m_mousePos);
	
	for (UINT i = 0; i < MAX_KEY_NUM; ++i)
	{
		m_keyState[i] <<= 1;
		m_keyState[i] |= ((GetKeyState(i) & 0x80) >> 7);
	}
	m_capLock = GetKeyState((int)KeyCode::Caps_Lock) & 1;
}
