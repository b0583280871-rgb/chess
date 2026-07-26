#include "LoginWindow.hpp"

namespace win32_ui {

namespace {
    const wchar_t* WINDOW_CLASS_NAME = L"ChessLoginWindowClass";

    const int ID_EDIT_EMAIL = 101;
    const int ID_EDIT_PASSWORD = 102;
    const int ID_BUTTON_LOGIN = 103;
    const int ID_BUTTON_REGISTER = 104;
    const int ID_STATIC_STATUS = 105;

    std::wstring utf8ToWide(const std::string& s) {
        if (s.empty()) return std::wstring();
        int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        std::wstring result(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &result[0], len);
        return result;
    }

    std::string wideToUtf8(const std::wstring& s) {
        if (s.empty()) return std::string();
        int len = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
        std::string result(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), &result[0], len, nullptr, nullptr);
        return result;
    }

    std::string getWindowTextUtf8(HWND hwnd) {
        int len = GetWindowTextLengthW(hwnd);
        std::wstring buf(len, L'\0');
        if (len > 0) {
            GetWindowTextW(hwnd, &buf[0], len + 1);
        }
        return wideToUtf8(buf);
    }
}

LoginWindow::LoginWindow() {}

LoginWindow::~LoginWindow() {
    destroy();
}

bool LoginWindow::create() {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = LoginWindow::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    hwnd_ = CreateWindowExW(
        0, WINDOW_CLASS_NAME, L"Chess Login",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 230,
        nullptr, nullptr, hInstance, this
    );
    if (!hwnd_) return false;

    HWND emailLabel = CreateWindowExW(0, L"STATIC", L"Email:",
        WS_CHILD | WS_VISIBLE,
        20, 22, 60, 20, hwnd_, nullptr, hInstance, nullptr);

    emailEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        90, 20, 210, 22, hwnd_, (HMENU)(INT_PTR)ID_EDIT_EMAIL, hInstance, nullptr);

    HWND passwordLabel = CreateWindowExW(0, L"STATIC", L"Password:",
        WS_CHILD | WS_VISIBLE,
        20, 54, 60, 20, hwnd_, nullptr, hInstance, nullptr);

    passwordEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
        90, 52, 210, 22, hwnd_, (HMENU)(INT_PTR)ID_EDIT_PASSWORD, hInstance, nullptr);

    loginButton_ = CreateWindowExW(0, L"BUTTON", L"Login",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        90, 90, 100, 28, hwnd_, (HMENU)(INT_PTR)ID_BUTTON_LOGIN, hInstance, nullptr);

    registerButton_ = CreateWindowExW(0, L"BUTTON", L"Register",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        200, 90, 100, 28, hwnd_, (HMENU)(INT_PTR)ID_BUTTON_REGISTER, hInstance, nullptr);

    statusStatic_ = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 130, 290, 60, hwnd_, (HMENU)(INT_PTR)ID_STATIC_STATUS, hInstance, nullptr);

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HWND controlsNeedingFont[] = {
        emailLabel, emailEdit_, passwordLabel, passwordEdit_,
        loginButton_, registerButton_, statusStatic_
    };
    for (HWND h : controlsNeedingFont) {
        SendMessageW(h, WM_SETFONT, (WPARAM)font, TRUE);
    }

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    return true;
}

bool LoginWindow::pumpMessages() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return !closed_;
}

LoginAction LoginWindow::takeRequestedAction() {
    LoginAction action = pendingAction_;
    pendingAction_ = LoginAction::None;
    return action;
}

std::string LoginWindow::email() const {
    return getWindowTextUtf8(emailEdit_);
}

std::string LoginWindow::password() const {
    return getWindowTextUtf8(passwordEdit_);
}

void LoginWindow::setStatus(const std::string& text) {
    if (!statusStatic_) return;
    SetWindowTextW(statusStatic_, utf8ToWide(text).c_str());
}

void LoginWindow::setEnabled(bool enabled) {
    EnableWindow(emailEdit_, enabled);
    EnableWindow(passwordEdit_, enabled);
    EnableWindow(loginButton_, enabled);
    EnableWindow(registerButton_, enabled);
}

void LoginWindow::destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

LRESULT CALLBACK LoginWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    LoginWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<LoginWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<LoginWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->handleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT LoginWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            if (HIWORD(wParam) == BN_CLICKED) {
                int id = LOWORD(wParam);
                if (id == ID_BUTTON_LOGIN) {
                    pendingAction_ = LoginAction::LoginRequested;
                } else if (id == ID_BUTTON_REGISTER) {
                    pendingAction_ = LoginAction::RegisterRequested;
                }
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            closed_ = true;
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

}
