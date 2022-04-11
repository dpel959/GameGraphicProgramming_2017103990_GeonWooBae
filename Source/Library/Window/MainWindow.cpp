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
        static bool raw_input_initialized = false;
        if (!raw_input_initialized) {
            RAWINPUTDEVICE rid =
            {
                .usUsagePage = 0x01,
                .usUsage = 0x02,
                .dwFlags = 0,
                .hwndTarget = m_hWnd
            };

            if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
                return E_FAIL;
            }
        }
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
        case WM_INPUT: // 윈도우 벗어나도 마우스 캡처&relative 값
            UINT dataSize;

            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

            if (dataSize > 0) {
                std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                if (GetRawInputData(
                    reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT, rawdata.get(),
                    &dataSize,
                    sizeof(RAWINPUTHEADER)
                    ) == dataSize ) 
                {
                    RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                    if (raw->header.dwType == RIM_TYPEMOUSE) {
                        m_mouseRelativeMovement = {
                            .X = raw->data.mouse.lLastX,
                            .Y = raw->data.mouse.lLastY
                        };
                    }
                }
            }

            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);

        case WM_KEYDOWN:
            switch (wParam) {
            case 'W': // 0x57
                m_directions.bFront = true;
                break;
            case 'A': // 0x41
                m_directions.bLeft = true;
                break;
            case 'S': // 0x53
                m_directions.bBack = true;
                break;
            case 'D': // 0x44
                m_directions.bRight = true;
                break;
            case VK_SPACE:
                m_directions.bUp = true;
                break;
            case VK_SHIFT:
                m_directions.bDown = true;
                break;
            }
            return 0;

        case WM_KEYUP:
            switch (wParam) {
            case 'W': // 0x57
                m_directions.bFront = false;
                break;
            case 'A': // 0x41
                m_directions.bLeft = false;
                break;
            case 'S': // 0x53
                m_directions.bBack = false;
                break;
            case 'D': // 0x44
                m_directions.bRight = false;
                break;
            case VK_SPACE:
                m_directions.bUp = false;
                break;
            case VK_SHIFT:
                m_directions.bDown = false;
                break;
            }
            return 0;

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

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections

      Summary:  Returns the keyboard direction input

      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const DirectionsInput& MainWindow::GetDirections() const {
        return m_directions;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement

      Summary:  Returns the mouse relative movement

      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const {
        return m_mouseRelativeMovement;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement

      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MainWindow::ResetMouseMovement() {
        m_mouseRelativeMovement =
        {
            .X = 0,
            .Y = 0
        };
    }
}
