/*+===================================================================
  File:      MAIN.CPP

  Summary:   This application demonstrates creating a Direct3D 11 device

  Origin:    http://msdn.microsoft.com/en-us/library/windows/apps/ff729718.aspx

  Originally created by Microsoft Corporation under MIT License
  © 2022 Kyung Hee University
===================================================================+*/

#include "Common.h"

#include "Game/Game.h"

using namespace library;

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: wWinMain

  Summary:  Entry point to the program. Initializes everything and
            goes into a message processing loop. Idle time is used to
            render the scene.

  Args:     HINSTANCE hInstance
              Handle to an instance.
            HINSTANCE hPrevInstance
              Has no meaning.
            LPWSTR lpCmdLine
              Contains the command-line arguments as a Unicode
              string
            INT nCmdShow
              Flag that says whether the main application window
              will be minimized, maximized, or shown normally

  Returns:  INT
              Status code.
-----------------------------------------------------------------F-F*/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{

    UNREFERENCED_PARAMETER(lpCmdLine); UNREFERENCED_PARAMETER(hPrevInstance);

    if (FAILED(InitWindow(hInstance, nCmdShow))) {
        DWORD dwError = GetLastError();

        MessageBox(nullptr, L"Failed :", L"Initialize Window", NULL);

        if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
            return HRESULT_FROM_WIN32(dwError);
        }

        return E_FAIL;
    }

    if (FAILED(InitDevice())) {
        DWORD dwError = GetLastError();

        MessageBox(nullptr, L"Failed :", L"Initialize Device", NULL);

        if (dwError != ERROR_CLASS_ALREADY_EXISTS) {
            return HRESULT_FROM_WIN32(dwError);
        }

        return E_FAIL;
    }

    // Main message loop
    MSG msg = { 0 };

    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    CleanupDevice();

    return static_cast<INT>(msg.wParam);
}

