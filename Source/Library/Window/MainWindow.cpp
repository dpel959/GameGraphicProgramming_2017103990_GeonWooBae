#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) {
        HRESULT hr = (initialize(hInstance, nCmdShow, pszWindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX));
        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    PCWSTR MainWindow::GetWindowClassName() const {
        return L"Game Graphics Window Class";
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
            if (MessageBox(m_hWnd, L"Really quit?", L"Game Graphic Programming", MB_OKCANCEL) == IDOK) {
                DestroyWindow(m_hWnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return TRUE;
    }
}
