#include <iostream>
#define _LIB 1
#include "../Graphics/Graphics.h"
int main()
{
    HwndGraphics hg(GetConsoleWindow());
    D2DGraphics g_tmp(400, 400);
    D2DGraphics g(1920,1080);
    g_tmp.BeginRender();
    g_tmp.Clear(Colors::Green);
    g_tmp.EndRender();

    auto drawItem = g_tmp.GetBitmap();

    g.BeginRender();
    g.Clear(Colors::Red);
	g.DrawBitmap(drawItem, 100, 100, 0.8f);
    g.EndRender();
    auto bmp = g.CopyTargetToWicBitmap();
    Factory::SaveBitmap(bmp.Get(), L"output.bmp");

    std::cout << "Hello World!\n";
}
