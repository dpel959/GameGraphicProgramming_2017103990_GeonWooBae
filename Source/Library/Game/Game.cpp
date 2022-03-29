﻿#include "Game/Game.h"

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Game

	  Summary:  Constructor

	  Args:     PCWSTR pszGameName
				  Name of the game

	  Modifies: [m_pszGameName, m_mainWindow, m_renderer].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	Game::Game(_In_ PCWSTR pszGameName) 
		:m_pszGameName(pszGameName), m_mainWindow(std::make_unique<MainWindow>()), m_renderer(std::make_unique<Renderer>())
	{
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Initialize

	  Summary:  Initializes the components of the game

	  Args:     HINSTANCE hInstance
				  Handle to the instance
				INT nCmdShow
				  Is a flag that says whether the main application window
				  will be minimized, maximized, or shown normally

	  Modifies: [m_mainWindow, m_renderer].

	  Returns:  HRESULT
				Status code
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	HRESULT Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow) {
		if (FAILED(m_mainWindow->Initialize(hInstance, nCmdShow, GetGameName()))) {
			DWORD dwError = GetLastError();

			MessageBox(nullptr, L"Initialize Window", L"FAILED", NULL);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
				return HRESULT_FROM_WIN32(dwError);
			}

			return E_FAIL;
		}

		HWND hWnd = m_mainWindow->GetWindow();

		if (FAILED(m_renderer->Initialize(hWnd))) {
			DWORD dwError = GetLastError();

			MessageBox(nullptr, L"Initialize Device", L"FAILED", NULL);

			if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
				return HRESULT_FROM_WIN32(dwError);
			}

			return E_FAIL;
		}

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::Run

	  Summary:  Runs the game loop

	  Returns:  INT
				  Status code to return to the operating system
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	INT Game::Run() {
		MSG msg = { 0 };

		while (WM_QUIT != msg.message) {
			if (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				m_renderer->Render();
			}
		}

		return static_cast<INT>(msg.wParam);
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Game::GetGameName

	  Summary:  Returns the name of the game

	  Returns:  PCWSTR
				  Name of the game
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	PCWSTR Game::GetGameName() const{
		return m_pszGameName;
	}
}