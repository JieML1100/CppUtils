#pragma once
#include <d3d11.h>
#include <d2d1_1.h>
#include <d2d1_2.h>
#include <dxgi1_2.h>
#include <memory>
#include <optional>
#include <vector>
#include <wrl/client.h>

#include "Font.h"
#include "Colors.h"
#include "Factory.h"

#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d11.lib")

#ifndef _LIB
// MT (静态运行时库)
#if defined(_MT) && !defined(_DLL)
    #if defined(_M_X64) && defined(_DEBUG)
        #pragma comment(lib, "Graphics_x64_MTd.lib")
    #elif defined(_M_X64) && !defined(_DEBUG)
        #pragma comment(lib, "Graphics_x64_MT.lib")
    #elif defined(_M_IX86) && defined(_DEBUG)
        #pragma comment(lib, "Graphics_x86_MTd.lib")
    #elif defined(_M_IX86) && !defined(_DEBUG)
        #pragma comment(lib, "Graphics_x86_MT.lib")
    #else
        #error "Unsupported architecture or configuration"
    #endif
// MD (动态运行时库)
#elif defined(_MT) && defined(_DLL)
    #if defined(_M_X64) && defined(_DEBUG)
        #pragma comment(lib, "Graphics_x64_MDd.lib")
    #elif defined(_M_X64) && !defined(_DEBUG)
        #pragma comment(lib, "Graphics_x64_MD.lib")
    #elif defined(_M_IX86) && defined(_DEBUG)
        #pragma comment(lib, "Graphics_x86_MDd.lib")
    #elif defined(_M_IX86) && !defined(_DEBUG)
        #pragma comment(lib, "Graphics_x86_MD.lib")
    #else
        #error "Unsupported architecture or configuration"
    #endif
#else
    #error "Unsupported runtime library configuration"
#endif
#endif

EXTERN_C Font* DefaultFontObject;
class D2DGraphics
{
public:
	enum class SurfaceKind
	{
		None,
		Offscreen,
		SwapChain,
		DxgiSurface,
		ExternalBitmap
	};

	struct InitOptions
	{
		SurfaceKind kind = SurfaceKind::None;
		UINT width = 0;
		UINT height = 0;
		IDXGISwapChain* swapChain = nullptr;
		IDXGISurface* dxgiSurface = nullptr;
		ID2D1Bitmap1* targetBitmap = nullptr;
		D2D1_DEVICE_CONTEXT_OPTIONS contextOptions = D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;
		DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
		D2D1_ALPHA_MODE alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		D2D1_BITMAP_OPTIONS bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
	};

	D2DGraphics();
	explicit D2DGraphics(UINT width, UINT height);
	explicit D2DGraphics(ID2D1Bitmap1* bmp);
	explicit D2DGraphics(const InitOptions& options);
	virtual ~D2DGraphics();

	virtual void BeginRender();
	virtual void EndRender();
	virtual void ReSize(UINT width, UINT height);

	void Clear(D2D1_COLOR_F color);

	ID2D1SolidColorBrush* GetColorBrush(D2D1_COLOR_F newcolor);
	ID2D1SolidColorBrush* GetColorBrush(COLORREF newcolor);
	ID2D1SolidColorBrush* GetColorBrush(int r, int g, int b);
	ID2D1SolidColorBrush* GetColorBrush(float r, float g, float b, float a = 1.0f);

	ID2D1SolidColorBrush* GetBackColorBrush(D2D1_COLOR_F newcolor);
	ID2D1SolidColorBrush* GetBackColorBrush(COLORREF newcolor);
	ID2D1SolidColorBrush* GetBackColorBrush(int r, int g, int b);
	ID2D1SolidColorBrush* GetBackColorBrush(float r, float g, float b, float a = 1.0f);

	void DrawLine(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_COLOR_F color, float linewidth = 1.0f);
	void DrawLine(D2D1_POINT_2F p1, D2D1_POINT_2F p2, ID2D1Brush* brush, float linewidth = 1.0f);

	void DrawLine(float p1_x, float p1_y, float p2_x, float p2_y, D2D1_COLOR_F color, float linewidth = 1.0f);
	void DrawLine(float p1_x, float p1_y, float p2_x, float p2_y, ID2D1Brush* brush, float linewidth = 1.0f);

	void DrawRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float linewidth = 1.0f);
	void DrawRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth = 1.0f);

	void DrawRoundRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float linewidth = 1.0f, float r = 0.5f);
	void DrawRoundRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth = 1.0f, float r = 0.5f);

	void FillRect(D2D1_RECT_F rect, D2D1_COLOR_F color);
	void FillRect(D2D1_RECT_F rect, ID2D1Brush* brush);

	void FillRect(float left, float top, float width, float height, D2D1_COLOR_F color);
	void FillRect(float left, float top, float width, float height, ID2D1Brush* brush);

	void FillRoundRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float r = 0.5f);
	void FillRoundRect(float left, float top, float width, float height, D2D1_COLOR_F color, float r = 0.5f);

	void DrawEllipse(D2D1_POINT_2F cent, float xr, float yr, D2D1_COLOR_F color, float linewidth = 1.0f);
	void DrawEllipse(float x, float y, float xr, float yr, D2D1_COLOR_F color, float linewidth = 1.0f);

	void FillEllipse(D2D1_POINT_2F cent, float xr, float yr, D2D1_COLOR_F color);
	void FillEllipse(float cx, float cy, float xr, float yr, D2D1_COLOR_F color);

	void DrawGeometry(ID2D1Geometry* geo, D2D1_COLOR_F color, float linewidth = 1.0f);
	void DrawGeometry(ID2D1Geometry* geo, ID2D1Brush* brush, float linewidth = 1.0f);

	void FillGeometry(ID2D1Geometry* geo, D2D1_COLOR_F color);
	void FillGeometry(ID2D1Geometry* geo, ID2D1Brush* brush);

	void FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, D2D1_COLOR_F color);
	void FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, ID2D1Brush* brush);

	void DrawBitmap(ID2D1Bitmap* bmp, float x = 0.f, float y = 0.f, float opacity = 1.0f);
	void DrawBitmap(ID2D1Bitmap* bmp, D2D1_RECT_F rect, float opacity = 1.0f);
	void DrawBitmap(ID2D1Bitmap* bmp, D2D1_RECT_F destRect, D2D1_RECT_F srcRect, float opacity = 1.0f);

	void DrawBitmap(ID2D1Bitmap* bmp, float x, float y, float w, float h, float opacity = 1.0f);
	void DrawBitmap(ID2D1Bitmap* bmp, float dest_x, float dest_y, float dest_w, float dest_h, float src_x, float src_y, float src_w, float src_h, float opacity = 1.0f);

	IDWriteTextLayout* CreateStringLayout(std::wstring str, float width, float height, Font* font = nullptr);

	void DrawStringLayout(IDWriteTextLayout* layout, float x, float y, D2D1_COLOR_F color);
	void DrawStringLayout(IDWriteTextLayout* layout, float x, float y, ID2D1Brush* brush);

	void DrawStringLayoutEffect(IDWriteTextLayout* layout, float x, float y, D2D1_COLOR_F color, DWRITE_TEXT_RANGE subRange, D2D1_COLOR_F fontBack, Font* font = NULL);
	void DrawStringLayoutEffect(IDWriteTextLayout* layout, float x, float y, ID2D1Brush* brush, DWRITE_TEXT_RANGE subRange, D2D1_COLOR_F fontBack, Font* font = NULL);

	void DrawString(std::wstring str, float x, float y, D2D1_COLOR_F color, Font* font = nullptr);
	void DrawString(std::wstring str, float x, float y, ID2D1Brush* brush, Font* font = nullptr);
	void DrawString(std::wstring str, float x, float y, float w, float h, D2D1_COLOR_F color, Font* font = nullptr);
	void DrawString(std::wstring str, float x, float y, float w, float h, ID2D1Brush* brush, Font* font = nullptr);

	void DrawTriangle(D2D1_TRIANGLE triangle, D2D1_COLOR_F color, float width = 1.0f);
	void FillTriangle(D2D1_TRIANGLE triangle, D2D1_COLOR_F color);

	void FillPolygon(std::initializer_list<D2D1_POINT_2F> points, D2D1_COLOR_F color);
	void FillPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color);

	void DrawPolygon(std::initializer_list<D2D1_POINT_2F> points, D2D1_COLOR_F color, float width = 1.0f);
	void DrawPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color, float width = 1.0f);

	void DrawArc(D2D1_POINT_2F center, float size, float sa, float ea, D2D1_COLOR_F color, float width = 1.0f);
	void DrawArcCounter(D2D1_POINT_2F center, float size, float sa, float ea, D2D1_COLOR_F color, float width = 1.0f);

	void PushDrawRect(float left, float top, float width, float height);
	void PopDrawRect();
	void SetAntialiasMode(D2D1_ANTIALIAS_MODE antialiasMode);
	void SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE antialiasMode);

	[[nodiscard]] Microsoft::WRL::ComPtr<ID2D1Bitmap1> CloneTargetBitmap() const;
	[[nodiscard]] Microsoft::WRL::ComPtr<IWICBitmap> CopyTargetToWicBitmap() const;
	[[nodiscard]] std::vector<uint8_t> CopyTargetPixels(UINT* stride = nullptr) const;

	ID2D1DeviceContext* GetDeviceContextRaw() const;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> GetDeviceContext() const;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext1> GetDeviceContext1() const;

	ID2D1Bitmap* CreateBitmap(IWICBitmap* wb);
	ID2D1Bitmap* CreateBitmap(std::wstring path);
	ID2D1Bitmap* CreateBitmap(void* data, int size);
	ID2D1Bitmap* CreateBitmap(HBITMAP hb);
	ID2D1Bitmap* CreateBitmap(HICON hb);
	ID2D1Bitmap* CreateBitmap(int width, int height);

	ID2D1Bitmap1* GetBitmap();
	ID2D1Bitmap* GetSharedBitmap();
	IWICBitmap* GetWicBitmap();

	ID2D1LinearGradientBrush* CreateLinearGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount);
	ID2D1RadialGradientBrush* CreateRadialGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount, D2D1_POINT_2F center);
	ID2D1BitmapBrush* CreateitmapBrush(ID2D1Bitmap* bmp);
	ID2D1SolidColorBrush* CreateSolidColorBrush(D2D1_COLOR_F color);
	D2D1_SIZE_F Size();
	void SetTransform(D2D1_MATRIX_3X2_F matrix);
	void ClearTransform();

	std::vector<ID2D1Bitmap*> CreateBitmapFromGif(const char* path);
	std::vector<ID2D1Bitmap*> CreateBitmapFromGifBuffer(void* data, int size);
	static HBITMAP CopyFromScreen(int x, int y, int width, int height);
	static HBITMAP CopyFromWidnow(HWND handle, int x, int y, int width, int height);
	static SIZE GetScreenSize(int index = 0);
	static D2D1_SIZE_F GetTextLayoutSize(IDWriteTextLayout* textLayout);

protected:
	HRESULT Initialize(const InitOptions& options);
	HRESULT InitializeWithSize(UINT width, UINT height, DXGI_FORMAT format, D2D1_ALPHA_MODE alphaMode, D2D1_BITMAP_OPTIONS options, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions);
	HRESULT InitializeWithBitmap(ID2D1Bitmap1* bmp, bool takeOwnership, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions);
	HRESULT InitializeWithDxgiSurface(IDXGISurface* surface, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions);
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> CreateCpuReadableSnapshot() const;
	void ResetTarget();
	HRESULT ConfigDefaultObjects();
	bool EnsureDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS options);

protected:
	Microsoft::WRL::ComPtr<ID2D1DeviceContext1> pD2DDeviceContext;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> Default_Brush;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> Default_Brush_Back;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> pD2DTargetBitmap;
	SurfaceKind surfaceKind = SurfaceKind::None;
	D2D1_DEVICE_CONTEXT_OPTIONS contextOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
};
class HwndGraphics : public D2DGraphics
{
private:
	HWND hwnd;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain;
	HRESULT InitDevice();
public:
	HwndGraphics(HWND hWnd);
	void ReSize(UINT width, UINT height) override;
	void BeginRender() override;
	void EndRender() override;
};
class IDXGISwapChainGraphics : public D2DGraphics
{
private:
	Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain;
public:
	IDXGISwapChainGraphics(IDXGISwapChain* swapchain);
	void ReSize(UINT width, UINT height) override;
	void BeginRender() override;
	void EndRender() override;
};
class DxgiSurfaceGraphics : public D2DGraphics
{
private:
	Microsoft::WRL::ComPtr<IDXGISurface> pDxgiSurface;
public:
	DxgiSurfaceGraphics(IDXGISurface* dxgiSurface);
	void ReSize(UINT width, UINT height) override;
	void BeginRender() override;
	void EndRender() override;
};
