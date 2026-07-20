#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>

#include <string>

namespace win32_ui {

enum class LoginAction {
    None,
    LoginRequested,
    RegisterRequested,
};

class LoginWindow {
public:
    LoginWindow();
    ~LoginWindow();

    LoginWindow(const LoginWindow&) = delete;
    LoginWindow& operator=(const LoginWindow&) = delete;

    bool create();

    bool pumpMessages();

    LoginAction takeRequestedAction();

    std::string email() const;
    std::string password() const;

    void setStatus(const std::string& text);

    void setEnabled(bool enabled);

    void destroy();

private:
    HWND hwnd_ = nullptr;
    HWND emailEdit_ = nullptr;
    HWND passwordEdit_ = nullptr;
    HWND loginButton_ = nullptr;
    HWND registerButton_ = nullptr;
    HWND statusStatic_ = nullptr;
    bool closed_ = false;
    LoginAction pendingAction_ = LoginAction::None;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

}
