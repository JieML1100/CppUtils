#include <iostream>
//已经存在解决方案中的项目引用,则不需要Graphics.h中对.lib的库文件导入,通过_LIB跳过
#define NOMINMAX
#define _LIB 1
#include "../Graphics/Graphics.h"
#include "../Utils/Utils.h"
#define BMP_SIZE_DEF 52
int main() {
    auto iconBitmap = BitmapSource::CreateEmpty(BMP_SIZE_DEF, BMP_SIZE_DEF);
    auto graphics = D2DGraphics(iconBitmap.get());
    auto iconData = File::ReadAllBytes("F:\\icons.dat");
    DataPack srcIcon = DataPack(iconData);
    DataPack pack = DataPack("ItemIcons");
    for (DataPack& ico : srcIcon.Child) {
        auto bmpSrc = BitmapSource::FromBuffer(ico.Value.data(), ico.Value.size());

        ID2D1Bitmap* bmp = graphics.CreateBitmap(bmpSrc);
        graphics.BeginRender();
        graphics.Clear(Colors::Opacity);

        auto size = bmp->GetSize();

        float scale = std::min(BMP_SIZE_DEF / size.width, BMP_SIZE_DEF / size.height);

        float scaledWidth = size.width * scale;
        float scaledHeight = size.height * scale;

        float x = (BMP_SIZE_DEF - scaledWidth) / 2.0f;
        float y = (BMP_SIZE_DEF - scaledHeight) / 2.0f;

        D2D1_RECT_F destRect = D2D1::RectF(x, y, x + scaledWidth, y + scaledHeight);
        D2D1_RECT_F srcRect = D2D1::RectF(0, 0, size.width, size.height);

        graphics.DrawBitmap(bmp, destRect, srcRect);

        graphics.EndRender();

        std::vector<uint8_t> buffer;
        if(iconBitmap->Save(buffer)) {
            //File::WriteAllBytes(StringHelper::Format("F:\\icons1\\%s.png", ico.Id.c_str()), buffer);
            pack[ico.Id] = buffer;
        }
        bmp->Release();
    }
    File::WriteAllBytes("F:\\icons2.dat", pack.GetBytes());
    return 0;


    auto bitmap = BitmapSource::CreateEmpty(400, 400);
    auto g = D2DGraphics(bitmap.get());

    auto bitmap1 = BitmapSource::CreateEmpty(1920, 1080);
    D2DGraphics g1(bitmap1.get());

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

    HwndGraphics hg(GetConsoleWindow());
    auto hg_drawItem = hg.CreateBitmap(bitmap1);
    while (1) {

        hg.BeginRender();
        hg.Clear(Colors::Blue);
        hg.DrawBitmap(hg_drawItem, 100, 100, 0.5f);
        hg.DrawString(L"Hello, World!", 50, 50, Colors::White);
        hg.EndRender();
    }

	system("pause");
	return 0;
}
