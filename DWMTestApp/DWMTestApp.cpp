// DWMTestApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DWMTestApp.h"

#define MAX_LOADSTRING 100
#define WS_CUSTOMWINDOW (WS_EX_TRANSPARENT | WS_EX_APPWINDOW)

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

DWMTestApp::DComPCM dComPCM;

HWND hwnd;
HWND childHwnd;
HWND textBoxHwnd;

// Forward declarations of functions included in this code module:
HRESULT             InitializeMainWindow();
HRESULT             InitializeLayeredWindow();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT             OnClose(HWND);
LRESULT             OnDestroy(HWND);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitXAML(HWND hwnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance;
    HRESULT hr;

    hr = InitializeMainWindow();
    if (SUCCEEDED(hr)) {
        hr = InitializeLayeredWindow();
    }
    if (SUCCEEDED(hr)) {
        hr = dComPCM.Initialize(hInst, hwnd, childHwnd, textBoxHwnd);
    }
    if (!SUCCEEDED(hr)) {
        return 0;
    }


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DWMTESTAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

HRESULT InitializeMainWindow()
{
    HRESULT hr = S_OK;

    // Register the window class.
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"DirectComposition Window Class";

    RegisterClassEx(&wc);

    hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,                          // Extended window style
        wc.lpszClassName,                                // Name of window class
        NULL,                                            // Title-bar string
        WS_OVERLAPPED | WS_SYSMENU,                      // Top-level window
        CW_USEDEFAULT,                                   // Horizontal position
        CW_USEDEFAULT,                                   // Vertical position
        1000,                                            // Width
        700,                                             // Height
        NULL,                                            // Parent
        NULL,                                            // Class menu
        hInst      ,                                     // Handle to application instance
        NULL                                             // Window-creation data
    );

    if (!hwnd)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

HRESULT InitializeLayeredWindow()
{
    HRESULT hr = S_OK;

    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"DirectCompositionChildWindow-Child";

    RegisterClassEx(&wcex);

    // Register the window class.
    WNDCLASSEX wce = { 0 };
    wce.cbSize = sizeof(wcex);
    wce.style = CS_HREDRAW | CS_VREDRAW;
    wce.lpfnWndProc = WndProc;
    wce.hCursor = LoadCursor(NULL, IDC_ARROW);
    wce.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wce.lpszClassName = L"EDIT";

    RegisterClassEx(&wce);

    // Create the playback control child window.
    childHwnd = CreateWindowEx(WS_EX_LAYERED,
        L"DirectCompositionChildWindow-Child",
        NULL,
        WS_CHILD | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        hwnd,
        NULL,
        hInst,
        NULL);

    if (!childHwnd)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    InitCommonControls();

    if (SUCCEEDED(hr))
    {
        SetLayeredWindowAttributes(childHwnd, 0, 0, LWA_ALPHA);

        //Create TextBox
        textBoxHwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_CLIENTEDGE | WS_EX_LAYERED | WS_EX_TRANSPARENT,
            L"EDIT",
            L"Text Box",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_LEFT,
            100, 300, 300, 50,
            hwnd,
            (HMENU)IDC_TEXTBOX,
            hInst,
            NULL);

        if (!textBoxHwnd)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr)) {
        SetLayeredWindowAttributes(textBoxHwnd, 0, 255, LWA_ALPHA);
    }

    return hr;
}

LRESULT CALLBACK WndProc(HWND hWND, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    int lId = LOWORD(lParam);
    int lIdh = HIWORD(lParam);
    int wId = LOWORD(wParam);
    int wIdh = HIWORD(wParam);

    switch (message)
    {
    case WM_NCPOINTERDOWN:
    case WM_POINTERDOWN:
    case WM_POINTERUPDATE:
        result = dComPCM.DoPCM();
        break;

    case WM_COMMAND:
        result = dComPCM.OnCommand(wParam);
        break;

    case WM_CLOSE:
        result = OnClose(hWND);
        break;

    case WM_DESTROY:
        result = OnDestroy(hWND);
        break;

    default:
        result = DefWindowProc(hWND, message, wParam, lParam);
    }

    return result;
}

// Handles the WM_CLOSE message.
LRESULT OnClose(HWND /*hwnd*/)
{
    // Destroy the main window.
    DestroyWindow(hwnd);

    return 0;
}

// Handles the WM_DESTROY message.
LRESULT OnDestroy(HWND /*hwnd*/)
{

    if (hwnd != NULL)
    {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    if (childHwnd != NULL)
    {
        DestroyWindow(childHwnd);
        childHwnd = NULL;
    }
    if (textBoxHwnd != NULL)
    {
        DestroyWindow(textBoxHwnd);
        textBoxHwnd = NULL;
    }

    dComPCM.Destroy();
    PostQuitMessage(0);

    return 0;
}

void InitXAML(HWND hWnd) {
    //if (_pcmControl == nullptr)
    //{
    //    // Initialize the XAML framework's core window for the current thread.
    //    //winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager winxamlmanager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    //    //auto interop = _desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative2>();
    //    //check_hresult(interop->AttachToWindow(hWnd));
    //    //HWND hWndXamlIsland = nullptr;
    //    //interop->get_WindowHandle(&hWndXamlIsland);
    //    ///*auto window = winrt::Windows::UI::Xaml::Window::Current().as<winrt::Windows::UI::Xaml::IWindowPrivate>();
    //    //window.TransparentBackground(true);*/
    //    //RECT windowRect;
    //    //::GetWindowRect(hWnd, &windowRect);
    //    //::SetWindowPos(hWndXamlIsland, NULL, 0, 0, windowRect.right - windowRect.left-500, windowRect.bottom - windowRect.top-500, SWP_SHOWWINDOW);

    //    //winrt::Windows::UI::Xaml::Controls::Canvas xamlContainer;
    //    //xamlContainer.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });

    //    //winrt::Windows::UI::Xaml::Controls::Canvas textContainer;
    //    ////textContainer.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Black() });
    //    ///*textContainer.Height(2000);
    //    //textContainer.Width(80);*/

    //    //winrt::Windows::UI::Xaml::Controls::TextBlock text;
    //    //text.Text(L"Hello");
    //    //text.Foreground(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Red() });
    //    //textContainer.Children().Append(text);

    //    //xamlContainer.Children().Append(textContainer);

    //    //xamlContainer.UpdateLayout();
    //    //textContainer.UpdateLayout();
    //    //_desktopWindowXamlSource.Content(text);


    //    /*_pcmControl = winrt::PCMApp::PCMControl();
    //    _desktopWindowXamlSource.Content(_pcmControl);*/
    //}
}
