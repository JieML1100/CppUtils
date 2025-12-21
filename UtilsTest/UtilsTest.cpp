#include <iostream>
//已经存在解决方案中的项目引用,则不需要Graphics.h中对.lib的库文件导入,通过_LIB跳过
#define NOMINMAX
#define _LIB 1
#include "../Utils/Utils.h"
#include "../Utils/httplib.h"
#include "../Graphics/Graphics1.h"
#define BMP_SIZE_DEF 52

// 全局变量：用于窗口渲染
static HwndGraphics1* g_pHwndGraphics = nullptr;
static ID2D1Bitmap* g_pDrawItem = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        if (g_pHwndGraphics) {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            if (width > 0 && height > 0) {
                g_pHwndGraphics->ReSize(width, height);
            }
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        
        if (g_pHwndGraphics && g_pDrawItem) {
            g_pHwndGraphics->BeginRender();
            g_pHwndGraphics->Clear(Colors::Blue);
            g_pHwndGraphics->DrawBitmap(g_pDrawItem, 100, 100, 0.5f);
            g_pHwndGraphics->DrawString(L"Hello, Graphics1 + DeviceContext!", 50, 50, Colors::White);
            g_pHwndGraphics->EndRender();
        }
        
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // 1. 先做离屏渲染测试
    auto bitmap = BitmapSource::CreateEmpty(400, 400);
    auto g = D2DGraphics1(bitmap.get());

    auto bitmap1 = BitmapSource::CreateEmpty(1920, 1080);
    D2DGraphics1 g1(bitmap1.get());

    g.BeginRender();
    g.Clear(Colors::Green);
    g.EndRender();

    auto drawItem = g1.CreateBitmap(bitmap);

    g1.BeginRender();
    g1.Clear(Colors::Red);
    g1.DrawBitmap(drawItem, 100, 100, 0.8f);
    g1.EndRender();

    drawItem->Release();

    bitmap1->Save(L"output.bmp");

    // 2. 注册窗口类
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"Graphics1TestWindow";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"窗口类注册失败!", L"错误", MB_ICONERROR);
        return 1;
    }

    // 3. 创建窗口
    HWND hwnd = CreateWindowExW(
        0,
        L"Graphics1TestWindow",
        L"Graphics1 渲染测试",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"窗口创建失败!", L"错误", MB_ICONERROR);
        return 1;
    }

    // 4. 创建 HwndGraphics1（用真正的窗口）
    g_pHwndGraphics = new HwndGraphics1(hwnd);
    g_pDrawItem = g_pHwndGraphics->CreateBitmap(bitmap1);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 5. 消息循环
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        // 主动触发重绘（实时渲染）
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    // 6. 清理
    if (g_pDrawItem) {
        g_pDrawItem->Release();
        g_pDrawItem = nullptr;
    }
    delete g_pHwndGraphics;
    g_pHwndGraphics = nullptr;

    return (int)msg.wParam;
}

// 保留 main 入口给控制台模式（如果需要）
int main() {
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWNORMAL);
}
