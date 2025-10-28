#include "Graphics.h"
#include "../Utils/Convert.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <unordered_map>

#include <d2derr.h>

using Microsoft::WRL::ComPtr;

static Font* DefaultFontObject = new Font(L"Arial", 18.0f);
namespace
{
	std::mutex gWicCacheMutex;
	std::unordered_map<std::wstring, ComPtr<IWICBitmapSource>> gWicBitmapCache;
}

D2DGraphics::D2DGraphics() = default;

D2DGraphics::D2DGraphics(UINT width, UINT height)
{
	InitializeWithSize(width,
		height,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		D2D1_ALPHA_MODE_PREMULTIPLIED,
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS);
}

D2DGraphics::D2DGraphics(ID2D1Bitmap1* bmp)
{
	InitializeWithBitmap(bmp,
		false,
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS);
}

D2DGraphics::D2DGraphics(const InitOptions& options)
{
	Initialize(options);
}

D2DGraphics::~D2DGraphics() = default;

HRESULT D2DGraphics::ConfigDefaultObjects()
{
	if (!pD2DDeviceContext)
	{
		return E_FAIL;
	}

	D2D1_COLOR_F defaultColor{ 0.5f, 0.6f, 0.7f, 1.0f };
	if (!Default_Brush)
	{
		HRESULT hr = pD2DDeviceContext->CreateSolidColorBrush(defaultColor, Default_Brush.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		Default_Brush->SetColor(defaultColor);
	}

	if (!Default_Brush_Back)
	{
		HRESULT hr = pD2DDeviceContext->CreateSolidColorBrush(defaultColor, Default_Brush_Back.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		Default_Brush_Back->SetColor(defaultColor);
	}

	return S_OK;
}

bool D2DGraphics::EnsureDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS options)
{
	if (pD2DDeviceContext && contextOptions == options)
	{
		return true;
	}

	ComPtr<ID2D1DeviceContext> baseContext;
	HRESULT hr = _D2DDevice->CreateDeviceContext(options, &baseContext);
	if (FAILED(hr))
	{
		return false;
	}

	hr = baseContext.As(&pD2DDeviceContext);
	if (FAILED(hr))
	{
		pD2DDeviceContext.Reset();
		return false;
	}

	contextOptions = options;
	return true;
}

HRESULT D2DGraphics::Initialize(const InitOptions& options)
{
	HRESULT hr = S_OK;
	switch (options.kind)
	{
	case SurfaceKind::Offscreen:
		hr = InitializeWithSize(options.width,
			options.height,
			options.format,
			options.alphaMode,
			options.bitmapOptions,
			options.contextOptions);
		break;
	case SurfaceKind::ExternalBitmap:
		hr = InitializeWithBitmap(options.targetBitmap, false, options.contextOptions);
		break;
	case SurfaceKind::DxgiSurface:
		hr = InitializeWithDxgiSurface(options.dxgiSurface, options.contextOptions);
		break;
	case SurfaceKind::SwapChain:
		// Swap chain initialization is handled by derived classes.
		hr = EnsureDeviceContext(options.contextOptions) ? ConfigDefaultObjects() : E_FAIL;
		break;
	case SurfaceKind::None:
	default:
		hr = EnsureDeviceContext(options.contextOptions) ? ConfigDefaultObjects() : E_FAIL;
		break;
	}
	return hr;
}

HRESULT D2DGraphics::InitializeWithSize(UINT width, UINT height, DXGI_FORMAT format, D2D1_ALPHA_MODE alphaMode, D2D1_BITMAP_OPTIONS options, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions)
{
	if (width == 0 || height == 0)
	{
		return E_INVALIDARG;
	}

	if (!EnsureDeviceContext(ctxOptions))
	{
		return E_FAIL;
	}

	ResetTarget();

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(options, D2D1::PixelFormat(format, alphaMode));
	HRESULT hr = pD2DDeviceContext->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0, &bitmapProperties, &pD2DTargetBitmap);
	if (FAILED(hr))
	{
		return hr;
	}

	if (!pD2DTargetBitmap)
	{
		return E_FAIL;
	}
	pD2DDeviceContext->SetTarget(pD2DTargetBitmap.Get());
	surfaceKind = SurfaceKind::Offscreen;
	return ConfigDefaultObjects();
}

HRESULT D2DGraphics::InitializeWithBitmap(ID2D1Bitmap1* bmp, bool takeOwnership, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions)
{
	if (!bmp)
	{
		return E_INVALIDARG;
	}

	if (!EnsureDeviceContext(ctxOptions))
	{
		return E_FAIL;
	}

	ResetTarget();

	if (takeOwnership)
	{
		pD2DTargetBitmap.Attach(bmp);
	}
	else
	{
		pD2DTargetBitmap = bmp;
	}

	pD2DDeviceContext->SetTarget(pD2DTargetBitmap.Get());
	surfaceKind = SurfaceKind::ExternalBitmap;
	return ConfigDefaultObjects();
}

HRESULT D2DGraphics::InitializeWithDxgiSurface(IDXGISurface* surface, D2D1_DEVICE_CONTEXT_OPTIONS ctxOptions)
{
	if (!surface)
	{
		return E_INVALIDARG;
	}

	if (!EnsureDeviceContext(ctxOptions))
	{
		return E_FAIL;
	}

	ResetTarget();

	DXGI_SURFACE_DESC surfaceDesc{};
	HRESULT hr = surface->GetDesc(&surfaceDesc);
	if (FAILED(hr))
	{
		return hr;
	}
	if (surfaceDesc.Format == DXGI_FORMAT_UNKNOWN)
	{
		surfaceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	}

	// 尝试多种 alpha 模式创建位图
	D2D1_ALPHA_MODE alphaModes[] = {
		D2D1_ALPHA_MODE_PREMULTIPLIED,
		D2D1_ALPHA_MODE_IGNORE,
		D2D1_ALPHA_MODE_STRAIGHT
	};

	for (const auto& alphaMode : alphaModes)
	{
		D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(surfaceDesc.Format, alphaMode));
		
		hr = pD2DDeviceContext->CreateBitmapFromDxgiSurface(surface, &props, &pD2DTargetBitmap);
		
		if (SUCCEEDED(hr) && pD2DTargetBitmap)
		{
			break;
		}
		
		pD2DTargetBitmap.Reset();
	}

	// 如果所有 alpha 模式都失败，尝试移除 CANNOT_DRAW 标志
	if (FAILED(hr) || !pD2DTargetBitmap)
	{
		for (const auto& alphaMode : alphaModes)
		{
			D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET,
				D2D1::PixelFormat(surfaceDesc.Format, alphaMode));
			
			hr = pD2DDeviceContext->CreateBitmapFromDxgiSurface(surface, &props, &pD2DTargetBitmap);
			
			if (SUCCEEDED(hr) && pD2DTargetBitmap)
			{
				break;
			}
			
			pD2DTargetBitmap.Reset();
		}
	}

	if (FAILED(hr) || !pD2DTargetBitmap)
	{
		pD2DTargetBitmap.Reset();
		return hr == S_OK ? E_FAIL : hr;
	}

	pD2DDeviceContext->SetTarget(pD2DTargetBitmap.Get());
	surfaceKind = SurfaceKind::DxgiSurface;
	return ConfigDefaultObjects();
}

ComPtr<ID2D1Bitmap1> D2DGraphics::CreateCpuReadableSnapshot() const
{
	ComPtr<ID2D1Bitmap1> cpuBitmap;
	if (!pD2DDeviceContext || !pD2DTargetBitmap)
	{
		return cpuBitmap;
	}

	const auto pixelFormat = pD2DTargetBitmap->GetPixelFormat();
	D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		pixelFormat);

	HRESULT hr = pD2DDeviceContext->CreateBitmap(
		pD2DTargetBitmap->GetPixelSize(),
		nullptr,
		0,
		&properties,
		&cpuBitmap);
	if (FAILED(hr))
	{
		return {};
	}

	hr = cpuBitmap->CopyFromBitmap(nullptr, pD2DTargetBitmap.Get(), nullptr);
	if (FAILED(hr))
	{
		cpuBitmap.Reset();
	}
	return cpuBitmap;
}

void D2DGraphics::ResetTarget()
{
	if (pD2DDeviceContext)
	{
		pD2DDeviceContext->SetTarget(nullptr);
	}
	pD2DTargetBitmap.Reset();
	surfaceKind = SurfaceKind::None;
}

ComPtr<ID2D1Bitmap1> D2DGraphics::CloneTargetBitmap() const
{
	ComPtr<ID2D1Bitmap1> clone;
	if (!pD2DDeviceContext || !pD2DTargetBitmap)
	{
		return clone;
	}

	FLOAT dpiX = 96.0f;
	FLOAT dpiY = 96.0f;
	pD2DTargetBitmap->GetDpi(&dpiX, &dpiY);

	const auto pixelFormat = pD2DTargetBitmap->GetPixelFormat();
	const auto options = pD2DTargetBitmap->GetOptions();

	ComPtr<ID2D1ColorContext> colorContext;
	pD2DTargetBitmap->GetColorContext(&colorContext);

	D2D1_BITMAP_PROPERTIES1 properties = D2D1::BitmapProperties1(
		options,
		pixelFormat,
		dpiX,
		dpiY,
		colorContext.Get());
	HRESULT hr = pD2DDeviceContext->CreateBitmap(
		pD2DTargetBitmap->GetPixelSize(),
		nullptr,
		0,
		&properties,
		&clone);
	if (FAILED(hr))
	{
		return {};
	}

	hr = clone->CopyFromBitmap(nullptr, pD2DTargetBitmap.Get(), nullptr);
	if (FAILED(hr))
	{
		return {};
	}
	return clone;
}

ComPtr<IWICBitmap> D2DGraphics::CopyTargetToWicBitmap() const
{
	ComPtr<IWICBitmap> wicBitmap;
	if (!pD2DDeviceContext || !pD2DTargetBitmap)
	{
		return wicBitmap;
	}

	IWICBitmap* raw = nullptr;
	HRESULT hr = Factory::ExtractID2D1Bitmap1ToIWICBitmap(
		pD2DDeviceContext.Get(),
		pD2DTargetBitmap.Get(),
		_ImageFactory,
		&raw);
	if (SUCCEEDED(hr) && raw)
	{
		wicBitmap.Attach(raw);
	}
	return wicBitmap;
}

std::vector<uint8_t> D2DGraphics::CopyTargetPixels(UINT* strideOut) const
{
	std::vector<uint8_t> pixels;
	ComPtr<ID2D1Bitmap1> cpuBitmap = CreateCpuReadableSnapshot();
	if (!cpuBitmap)
	{
		return pixels;
	}

	D2D1_MAPPED_RECT mapped{};
	HRESULT hr = cpuBitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped);
	if (FAILED(hr))
	{
		return pixels;
	}

	const D2D1_SIZE_U size = cpuBitmap->GetPixelSize();
	const UINT stride = mapped.pitch;
	const size_t total = static_cast<size_t>(stride) * size.height;
	pixels.resize(total);

	for (UINT y = 0; y < size.height; ++y)
	{
		std::copy_n(
			mapped.bits + static_cast<size_t>(y) * mapped.pitch,
			stride,
			pixels.data() + static_cast<size_t>(y) * stride);
	}

	cpuBitmap->Unmap();

	if (strideOut)
	{
		*strideOut = stride;
	}

	return pixels;
}

ID2D1DeviceContext* D2DGraphics::GetDeviceContextRaw() const
{
	return pD2DDeviceContext.Get();
}

ComPtr<ID2D1DeviceContext> D2DGraphics::GetDeviceContext() const
{
	ComPtr<ID2D1DeviceContext> ctx;
	if (pD2DDeviceContext)
	{
		ctx = pD2DDeviceContext;
	}
	return ctx;
}

ComPtr<ID2D1DeviceContext1> D2DGraphics::GetDeviceContext1() const
{
	return pD2DDeviceContext;
}
ID2D1SolidColorBrush* D2DGraphics::GetColorBrush(D2D1_COLOR_F newcolor) {
	if (!Default_Brush && pD2DDeviceContext)
	{
		if (FAILED(pD2DDeviceContext->CreateSolidColorBrush(newcolor, Default_Brush.ReleaseAndGetAddressOf())))
		{
			return nullptr;
		}
	}
	if (!Default_Brush)
	{
		return nullptr;
	}
	Default_Brush->SetColor(newcolor);
	return Default_Brush.Get();
}
ID2D1SolidColorBrush* D2DGraphics::GetColorBrush(COLORREF newcolor) {
	return GetColorBrush(D2D1_COLOR_F{ GetRValue(newcolor) / 255.0f,GetGValue(newcolor) / 255.0f,GetBValue(newcolor) / 255.0f,1.0f });
}
ID2D1SolidColorBrush* D2DGraphics::GetColorBrush(int r, int g, int b) {
	return GetColorBrush(D2D1_COLOR_F{ r / 255.0f,g / 255.0f,b / 255.0f,1.0f });
}
ID2D1SolidColorBrush* D2DGraphics::GetColorBrush(float r, float g, float b, float a) {
	return GetColorBrush(D2D1_COLOR_F{ r,g,b,a });
}
ID2D1SolidColorBrush* D2DGraphics::GetBackColorBrush(D2D1_COLOR_F newcolor) {
	if (!Default_Brush_Back && pD2DDeviceContext)
	{
		if (FAILED(pD2DDeviceContext->CreateSolidColorBrush(newcolor, Default_Brush_Back.ReleaseAndGetAddressOf())))
		{
			return nullptr;
		}
	}
	if (!Default_Brush_Back)
	{
		return nullptr;
	}
	Default_Brush_Back->SetColor(newcolor);
	return Default_Brush_Back.Get();
}
ID2D1SolidColorBrush* D2DGraphics::GetBackColorBrush(COLORREF newcolor) {
	D2D1_COLOR_F _newcolor = { GetRValue(newcolor) / 255.0f,GetGValue(newcolor) / 255.0f,GetBValue(newcolor) / 255.0f,1.0f };
	return GetBackColorBrush(_newcolor);
}
ID2D1SolidColorBrush* D2DGraphics::GetBackColorBrush(int r, int g, int b) {
	D2D1_COLOR_F _newcolor = { r / 255.0f,g / 255.0f,b / 255.0f,1.0f };
	return GetBackColorBrush(_newcolor);
}
ID2D1SolidColorBrush* D2DGraphics::GetBackColorBrush(float r, float g, float b, float a) {
	D2D1_COLOR_F _newcolor = { r,g,b,a };
	return GetBackColorBrush(_newcolor);
}
void D2DGraphics::BeginRender() {
	if (!pD2DDeviceContext)
	{
		return;
	}
	pD2DDeviceContext->BeginDraw();
}
void D2DGraphics::EndRender() {
	if (!pD2DDeviceContext)
	{
		return;
	}
	pD2DDeviceContext->EndDraw();
}
void D2DGraphics::ReSize(UINT width, UINT height) {}
void D2DGraphics::Clear(D2D1_COLOR_F color) {
	if (!pD2DDeviceContext)
	{
		return;
	}
	pD2DDeviceContext->Clear(color);
}
void D2DGraphics::DrawLine(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_COLOR_F color, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawLine(p1, p2, brush, linewidth);
}
void D2DGraphics::DrawLine(float p1_x, float p1_y, float p2_x, float p2_y, D2D1_COLOR_F color, float linewidth) {
	DrawLine(D2D1::Point2F(p1_x, p1_y), D2D1::Point2F(p2_x, p2_y), color, linewidth);
}
void D2DGraphics::DrawLine(D2D1_POINT_2F p1, D2D1_POINT_2F p2, ID2D1Brush* brush, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !brush)
	{
		return;
	}
	ctx->DrawLine(p1, p2, brush, linewidth);
}
void D2DGraphics::DrawLine(float p1_x, float p1_y, float p2_x, float p2_y, ID2D1Brush* brush, float linewidth) {
	DrawLine(D2D1::Point2F(p1_x, p1_y), D2D1::Point2F(p2_x, p2_y), brush, linewidth);
}
void D2DGraphics::DrawRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawRectangle(rect, brush, linewidth);
}
void D2DGraphics::DrawRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth) {
	DrawRect(D2D1::RectF(left, top, left + width, top + height), color, linewidth);
}
void D2DGraphics::DrawRoundRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float linewidth, float r) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawRoundedRectangle(D2D1::RoundedRect(rect, r, r), brush, linewidth);
}
void D2DGraphics::DrawRoundRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth, float r) {
	DrawRoundRect(D2D1::RectF(left, top, left + width, top + height), color, linewidth, r);
}
void D2DGraphics::FillRect(D2D1_RECT_F rect, D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->FillRectangle(rect, brush);
}
void D2DGraphics::FillRect(D2D1_RECT_F rect, ID2D1Brush* brush) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !brush)
	{
		return;
	}
	ctx->FillRectangle(rect, brush);
}
void D2DGraphics::FillRect(float left, float top, float width, float height, D2D1_COLOR_F color) {
	FillRect(D2D1::RectF(left, top, left + width, top + height), color);
}
void D2DGraphics::FillRect(float left, float top, float width, float height, ID2D1Brush* brush) {
	FillRect(D2D1::RectF(left, top, left + width, top + height), brush);
}
void D2DGraphics::FillRoundRect(D2D1_RECT_F rect, D2D1_COLOR_F color, float r) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->FillRoundedRectangle(D2D1::RoundedRect(rect, r, r), brush);
}
void D2DGraphics::FillRoundRect(float left, float top, float width, float height, D2D1_COLOR_F color, float r) {
	FillRoundRect(D2D1::RectF(left, top, left + width, top + height), color, r);
}
void D2DGraphics::DrawEllipse(D2D1_POINT_2F cent, float xr, float yr, D2D1_COLOR_F color, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawEllipse(D2D1::Ellipse(cent, xr, yr), brush, linewidth);
}
void D2DGraphics::DrawEllipse(float x, float y, float xr, float yr, D2D1_COLOR_F color, float linewidth) {
	DrawEllipse(D2D1::Point2F(x, y), xr, yr, color, linewidth);
}
void D2DGraphics::FillEllipse(D2D1_POINT_2F cent, float xr, float yr, D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->FillEllipse(D2D1::Ellipse(cent, xr, yr), brush);
}
void D2DGraphics::FillEllipse(float cx, float cy, float xr, float yr, D2D1_COLOR_F color) {
	FillEllipse(D2D1::Point2F(cx, cy), xr, yr, color);
}
void D2DGraphics::DrawGeometry(ID2D1Geometry* geo, D2D1_COLOR_F color, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !geo)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawGeometry(geo, brush, linewidth);
}
void D2DGraphics::FillGeometry(ID2D1Geometry* geo, D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !geo)
	{
		return;
	}
	auto brush = this->GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->FillGeometry(geo, brush);
}
void D2DGraphics::DrawGeometry(ID2D1Geometry* geo, ID2D1Brush* brush, float linewidth) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !geo || !brush)
	{
		return;
	}
	ctx->DrawGeometry(geo, brush, linewidth);
}
void D2DGraphics::FillGeometry(ID2D1Geometry* geo, ID2D1Brush* brush) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !geo || !brush)
	{
		return;
	}
	ctx->FillGeometry(geo, brush);
}
void D2DGraphics::FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, D2D1_COLOR_F color)
{
	constexpr float degToRad = 3.14159265359f / 180.0f;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(center, D2D1_FIGURE_BEGIN_FILLED);
	float startRad = startAngle * degToRad;
	float endRad = (startAngle + sweepAngle) * degToRad;
	D2D1_POINT_2F startPoint{ center.x + (width * 0.5f) * cosf(startRad), center.y - (height * 0.5f) * sinf(startRad) };
	D2D1_POINT_2F endPoint{ center.x + (width * 0.5f) * cosf(endRad), center.y - (height * 0.5f) * sinf(endRad) };
	sink->AddLine(startPoint);
	D2D1_SIZE_F arcSize{ width * 0.5f, height * 0.5f };
	D2D1_ARC_SIZE arcSizeFlag = (std::fabs(sweepAngle) <= 180.0f) ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
	D2D1_SWEEP_DIRECTION sweepDir = sweepAngle >= 0.0f ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	sink->AddArc(D2D1::ArcSegment(endPoint, arcSize, 0.0f, sweepDir, arcSizeFlag));
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	ctx->FillGeometry(geo.Get(), brush);
}
void D2DGraphics::FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, ID2D1Brush* brush)
{
	constexpr float degToRad = 3.14159265359f / 180.0f;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(center, D2D1_FIGURE_BEGIN_FILLED);
	float startRad = startAngle * degToRad;
	float endRad = (startAngle + sweepAngle) * degToRad;
	D2D1_POINT_2F startPoint{ center.x + (width * 0.5f) * cosf(startRad), center.y - (height * 0.5f) * sinf(startRad) };
	D2D1_POINT_2F endPoint{ center.x + (width * 0.5f) * cosf(endRad), center.y - (height * 0.5f) * sinf(endRad) };
	sink->AddLine(startPoint);
	D2D1_SIZE_F arcSize{ width * 0.5f, height * 0.5f };
	D2D1_ARC_SIZE arcSizeFlag = (std::fabs(sweepAngle) <= 180.0f) ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
	D2D1_SWEEP_DIRECTION sweepDir = sweepAngle >= 0.0f ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
	sink->AddArc(D2D1::ArcSegment(endPoint, arcSize, 0.0f, sweepDir, arcSizeFlag));
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	ctx->FillGeometry(geo.Get(), brush);
}
void D2DGraphics::DrawBitmap(ID2D1Bitmap* bmp, float x, float y, float opacity) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return;
	}
	D2D1_SIZE_F siz = bmp->GetSize();
	ctx->DrawBitmap(bmp, D2D1::RectF(x, y, siz.width + x, siz.height + y), opacity);
}
void D2DGraphics::DrawBitmap(ID2D1Bitmap* bmp, D2D1_RECT_F rect, float opacity) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return;
	}
	ctx->DrawBitmap(bmp, rect, opacity);
}
void D2DGraphics::DrawBitmap(ID2D1Bitmap* bmp, D2D1_RECT_F destRect, D2D1_RECT_F srcRect, float opacity) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return;
	}
	ctx->DrawBitmap(bmp, destRect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
}
void D2DGraphics::DrawBitmap(ID2D1Bitmap* bmp, float x, float y, float w, float h, float opacity) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return;
	}
	D2D1_RECT_F rect = D2D1::RectF(x, y, w + x, h + y);
	ctx->DrawBitmap(bmp, rect, opacity);
}
void D2DGraphics::DrawBitmap(ID2D1Bitmap* bmp, float dest_x, float dest_y, float dest_w, float dest_h, float src_x, float src_y, float src_w, float src_h, float opacity) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return;
	}
	ctx->DrawBitmap(bmp, D2D1::RectF(dest_x, dest_y, dest_w + dest_x, dest_h + dest_y), opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(src_x, src_y, src_w + src_x, src_h + src_y));
}
IDWriteTextLayout* D2DGraphics::CreateStringLayout(std::wstring str, float width, float height, Font* font) {
	IDWriteTextLayout* textLayout = NULL;
	Font* resolvedFont = font ? font : DefaultFontObject;
	if (!resolvedFont)
	{
		return nullptr;
	}
	IDWriteTextFormat* fnt = resolvedFont->FontObject;
	_DWriteFactory->CreateTextLayout(str.c_str(), str.size(), fnt, width, height, &textLayout);
	return textLayout;
}
void D2DGraphics::DrawStringLayout(IDWriteTextLayout* layout, float x, float y, D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !layout)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawTextLayout(D2D1::Point2F(x, y), layout, brush);
}
void D2DGraphics::DrawStringLayout(IDWriteTextLayout* layout, float x, float y, ID2D1Brush* brush) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !layout || !brush)
	{
		return;
	}
	ctx->DrawTextLayout(D2D1::Point2F(x, y), layout, brush);
}
void D2DGraphics::DrawStringLayoutEffect(IDWriteTextLayout* layout, float x, float y, D2D1_COLOR_F color, DWRITE_TEXT_RANGE subRange, D2D1_COLOR_F fontBack, Font* font) {
	if (!layout)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto backBrush = GetBackColorBrush(fontBack);
	auto frontBrush = GetColorBrush(color);
	if (!backBrush || !frontBrush)
	{
		return;
	}
	layout->SetDrawingEffect(NULL, DWRITE_TEXT_RANGE{ 0, UINT_MAX });
	layout->SetDrawingEffect(backBrush, subRange);
	ctx->DrawTextLayout(D2D1::Point2F(x, y), layout, frontBrush);
}
void D2DGraphics::DrawStringLayoutEffect(IDWriteTextLayout* layout, float x, float y, ID2D1Brush* brush, DWRITE_TEXT_RANGE subRange, D2D1_COLOR_F fontBack, Font* font) {
	if (!layout || !brush)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto backBrush = GetBackColorBrush(fontBack);
	if (!backBrush)
	{
		return;
	}
	layout->SetDrawingEffect(NULL, DWRITE_TEXT_RANGE{ 0, UINT_MAX });
	layout->SetDrawingEffect(backBrush, subRange);
	ctx->DrawTextLayout(D2D1::Point2F(x, y), layout, brush);
}
void D2DGraphics::DrawString(std::wstring str, float x, float y, D2D1_COLOR_F color, Font* font) {
	D2D1_RECT_F rect = D2D1::RectF(x, y, FLT_MAX, FLT_MAX);
	Font* resolvedFont = font ? font : DefaultFontObject;
	if (!resolvedFont)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ctx->DrawText(str.c_str(), static_cast<UINT32>(str.size()), resolvedFont->FontObject, &rect, brush);
}
void D2DGraphics::DrawString(std::wstring str, float x, float y, ID2D1Brush* brush, Font* font) {
	D2D1_RECT_F rect = D2D1::RectF(x, y, FLT_MAX, FLT_MAX);
	Font* resolvedFont = font ? font : DefaultFontObject;
	if (!resolvedFont)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !brush)
	{
		return;
	}
	auto fnt = resolvedFont->FontObject;
	ctx->DrawText(str.c_str(), static_cast<UINT32>(str.size()), fnt, &rect, brush);
}
void D2DGraphics::DrawString(std::wstring str, float x, float y, float w, float h, D2D1_COLOR_F color, Font* font) {
	IDWriteTextLayout* textLayout = CreateStringLayout(str, w, h, font ? font : DefaultFontObject);
	if (!textLayout) return;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		textLayout->Release();
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		textLayout->Release();
		return;
	}
	ctx->DrawTextLayout({ x,y }, textLayout, brush);
	textLayout->Release();
}
void D2DGraphics::DrawString(std::wstring str, float x, float y, float w, float h, ID2D1Brush* brush, Font* font) {
	IDWriteTextLayout* textLayout = CreateStringLayout(str, w, h, font ? font : DefaultFontObject);
	if (!textLayout) return;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !brush)
	{
		textLayout->Release();
		return;
	}
	ctx->DrawTextLayout({ x,y }, textLayout, brush);
	textLayout->Release();
}
void D2DGraphics::FillTriangle(D2D1_TRIANGLE triangle, D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(triangle.point1, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine(triangle.point2);
	sink->AddLine(triangle.point3);
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	ctx->FillGeometry(geo.Get(), brush);
}
void D2DGraphics::DrawTriangle(D2D1_TRIANGLE triangle, D2D1_COLOR_F color, float width) {
	this->DrawLine(triangle.point1, triangle.point2, color, width);
	this->DrawLine(triangle.point2, triangle.point3, color, width);
	this->DrawLine(triangle.point3, triangle.point1, color, width);
}
void D2DGraphics::FillPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color) {
	if (points.size() <= 2)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(points.front(), D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(points.data() + 1, static_cast<UINT32>(points.size() - 1));
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	ctx->FillGeometry(geo.Get(), brush);
}
void D2DGraphics::FillPolygon(std::initializer_list<D2D1_POINT_2F> points, D2D1_COLOR_F color) {
	if (points.size() <= 2)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(*points.begin(), D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLines(points.begin() + 1, static_cast<UINT32>(points.size() - 1));
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	ctx->FillGeometry(geo.Get(), brush);
}
void D2DGraphics::DrawPolygon(std::initializer_list<D2D1_POINT_2F> points, D2D1_COLOR_F color, float width) {
	if (points.size() <= 1)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(*points.begin(), D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLines(points.begin() + 1, static_cast<UINT32>(points.size() - 1));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->Close();
	ctx->DrawGeometry(geo.Get(), brush, width);
}
void D2DGraphics::DrawPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color, float width) {
	if (points.size() <= 1)
	{
		return;
	}
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	sink->BeginFigure(points.front(), D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddLines(points.data() + 1, static_cast<UINT32>(points.size() - 1));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->Close();
	ctx->DrawGeometry(geo.Get(), brush, width);
}
void D2DGraphics::DrawArc(D2D1_POINT_2F center, float size, float sa, float ea, D2D1_COLOR_F color, float width) {
	const auto __AngleToPoint = [](D2D1_POINT_2F Cent, float Angle, float Len) {
		return Len > 0 ? D2D1::Point2F((Cent.x + (sin(Angle * (float)M_PI / 180.0f) * Len)), (Cent.y - (cos(Angle * (float)M_PI / 180.0f) * Len))) : Cent;
		};
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	float ts = sa, te = ea;
	if (te < ts) te += 360.0f;
	D2D1_ARC_SIZE sweep = (te - ts < 180.0f) ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	auto start = __AngleToPoint(center, sa, size), end = __AngleToPoint(center, ea, size);
	sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddArc(D2D1::ArcSegment(end, D2D1::SizeF(size, size), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, sweep));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->Close();
	ctx->DrawGeometry(geo.Get(), brush, width);
}
void D2DGraphics::DrawArcCounter(D2D1_POINT_2F center, float size, float sa, float ea, D2D1_COLOR_F color, float width) {
	const auto __AngleToPoint = [](D2D1_POINT_2F Cent, float Angle, float Len) {
		return Len > 0 ? D2D1::Point2F((Cent.x + (sin(Angle * (float)M_PI / 180.0f) * Len)), (Cent.y - (cos(Angle * (float)M_PI / 180.0f) * Len))) : Cent;
		};
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	auto brush = GetColorBrush(color);
	if (!brush)
	{
		return;
	}
	ComPtr<ID2D1PathGeometry> geo;
	geo.Attach(Factory::CreateGeomtry());
	if (!geo)
	{
		return;
	}
	float ts = sa, te = ea;
	if (te < ts) te += 360.0f;
	D2D1_ARC_SIZE sweep = (te - ts < 180.0f) ? D2D1_ARC_SIZE_SMALL : D2D1_ARC_SIZE_LARGE;
	ComPtr<ID2D1GeometrySink> sink;
	if (FAILED(geo->Open(&sink)))
	{
		return;
	}
	auto start = __AngleToPoint(center, sa, size), end = __AngleToPoint(center, ea, size);
	sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);
	sink->AddArc(D2D1::ArcSegment(end, D2D1::SizeF(size, size), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, sweep));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	sink->Close();
	ctx->DrawGeometry(geo.Get(), brush, width);
}
void D2DGraphics::PushDrawRect(float left, float top, float width, float height) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	ctx->PushAxisAlignedClip(D2D1::RectF(left, top, left + width, top + height), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}
void D2DGraphics::PopDrawRect() {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	ctx->PopAxisAlignedClip();
	
}
void D2DGraphics::SetAntialiasMode(D2D1_ANTIALIAS_MODE antialiasMode) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	ctx->SetAntialiasMode(antialiasMode);
}
void D2DGraphics::SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE antialiasMode) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return;
	}
	ctx->SetTextAntialiasMode(antialiasMode);
}
ID2D1Bitmap* D2DGraphics::CreateBitmap(IWICBitmap* wb) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !wb)
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(wb, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap* D2DGraphics::CreateBitmap(std::wstring path) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || path.empty())
	{
		return nullptr;
	}

	ComPtr<IWICBitmapSource> source;
	{
		std::lock_guard<std::mutex> lock(gWicCacheMutex);
		auto it = gWicBitmapCache.find(path);
		if (it != gWicBitmapCache.end())
		{
			source = it->second;
		}
		else
		{
			ComPtr<IWICBitmapDecoder> decoder;
			if (SUCCEEDED(_ImageFactory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
			{
				ComPtr<IWICBitmapFrameDecode> frame;
				if (SUCCEEDED(decoder->GetFrame(0, &frame)))
				{
					ComPtr<IWICFormatConverter> converter;
					if (SUCCEEDED(_ImageFactory->CreateFormatConverter(&converter)))
					{
						if (SUCCEEDED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom)))
						{
							converter.As(&source);
							if (source)
							{
								gWicBitmapCache.emplace(path, source);
							}
						}
					}
				}
			}
		}
	}

	if (!source)
	{
		return nullptr;
	}

	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(source.Get(), nullptr, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap* D2DGraphics::CreateBitmap(void* data, int size) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !data || size <= 0)
	{
		return nullptr;
	}

	ComPtr<IWICStream> stream;
	if (FAILED(_ImageFactory->CreateStream(&stream)))
	{
		return nullptr;
	}
	if (FAILED(stream->InitializeFromMemory(static_cast<WICInProcPointer>(data), size)))
	{
		return nullptr;
	}
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(_ImageFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder)))
	{
		return nullptr;
	}
	ComPtr<IWICBitmapFrameDecode> frame;
	if (FAILED(decoder->GetFrame(0, &frame)))
	{
		return nullptr;
	}
	ComPtr<IWICFormatConverter> converter;
	if (FAILED(_ImageFactory->CreateFormatConverter(&converter)))
	{
		return nullptr;
	}
	if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom)))
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap* D2DGraphics::CreateBitmap(HBITMAP hb) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !hb)
	{
		return nullptr;
	}
	ComPtr<IWICBitmap> wb;
	if (FAILED(_ImageFactory->CreateBitmapFromHBITMAP(hb, 0, WICBitmapUsePremultipliedAlpha, &wb)))
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(wb.Get(), nullptr, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap* D2DGraphics::CreateBitmap(HICON hb) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !hb)
	{
		return nullptr;
	}
	ComPtr<IWICBitmap> wb;
	if (FAILED(_ImageFactory->CreateBitmapFromHICON(hb, &wb)))
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(wb.Get(), nullptr, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap* D2DGraphics::CreateBitmap(int width, int height) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || width <= 0 || height <= 0)
	{
		return nullptr;
	}
	ComPtr<IWICBitmap> wb;
	if (FAILED(_ImageFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &wb)))
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> bitmap;
	if (FAILED(ctx->CreateBitmapFromWicBitmap(wb.Get(), nullptr, &bitmap)))
	{
		return nullptr;
	}
	return bitmap.Detach();
}

ID2D1Bitmap1* D2DGraphics::GetBitmap() {
	return pD2DTargetBitmap.Get();
}

ID2D1Bitmap* D2DGraphics::GetSharedBitmap() {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !pD2DTargetBitmap)
	{
		return nullptr;
	}
	ComPtr<ID2D1Bitmap> sharedBitmap;
	if (FAILED(ctx->CreateSharedBitmap(__uuidof(ID2D1Bitmap1), static_cast<void*>(pD2DTargetBitmap.Get()), nullptr, &sharedBitmap)))
	{
		return nullptr;
	}
	return sharedBitmap.Detach();
}

IWICBitmap* D2DGraphics::GetWicBitmap() {
	if (!pD2DDeviceContext || !pD2DTargetBitmap)
	{
		return nullptr;
	}
	IWICBitmap* wic = nullptr;
	HRESULT hr = Factory::ExtractID2D1Bitmap1ToIWICBitmap(pD2DDeviceContext.Get(), pD2DTargetBitmap.Get(), _ImageFactory, &wic);
	return SUCCEEDED(hr) ? wic : nullptr;
}
ID2D1LinearGradientBrush* D2DGraphics::CreateLinearGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !stops || stopcount == 0)
	{
		return nullptr;
	}
	ComPtr<ID2D1GradientStopCollection> collection;
	if (FAILED(ctx->CreateGradientStopCollection(stops, stopcount, &collection)))
	{
		return nullptr;
	}
	ComPtr<ID2D1LinearGradientBrush> brush;
	if (FAILED(ctx->CreateLinearGradientBrush({}, collection.Get(), &brush)))
	{
		return nullptr;
	}
	return brush.Detach();
}
ID2D1RadialGradientBrush* D2DGraphics::CreateRadialGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount, D2D1_POINT_2F center) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !stops || stopcount == 0)
	{
		return nullptr;
	}
	ComPtr<ID2D1GradientStopCollection> collection;
	if (FAILED(ctx->CreateGradientStopCollection(stops, stopcount, &collection)))
	{
		return nullptr;
	}
	D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props = {};
	props.center = center;
	ComPtr<ID2D1RadialGradientBrush> brush;
	if (FAILED(ctx->CreateRadialGradientBrush(props, collection.Get(), &brush)))
	{
		return nullptr;
	}
	return brush.Detach();
}	
ID2D1BitmapBrush* D2DGraphics::CreateitmapBrush(ID2D1Bitmap* bmp)
{
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !bmp)
	{
		return nullptr;
	}
	ComPtr<ID2D1BitmapBrush> brush;
	if (FAILED(ctx->CreateBitmapBrush(bmp, nullptr, nullptr, &brush)))
	{
		return nullptr;
	}
	return brush.Detach();
}
ID2D1SolidColorBrush* D2DGraphics::CreateSolidColorBrush(D2D1_COLOR_F color) {
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx)
	{
		return nullptr;
	}
	ComPtr<ID2D1SolidColorBrush> result;
	if (FAILED(ctx->CreateSolidColorBrush(color, &result)))
	{
		return nullptr;
	}
	return result.Detach();
}
D2D1_SIZE_F D2DGraphics::Size() {
	return pD2DDeviceContext ? pD2DDeviceContext->GetSize() : D2D1::SizeF(0.0f, 0.0f);
}

void D2DGraphics::SetTransform(D2D1_MATRIX_3X2_F matrix)
{
	if (pD2DDeviceContext)
	{
		pD2DDeviceContext->SetTransform(matrix);
	}
}
void D2DGraphics::ClearTransform()
{
	if (pD2DDeviceContext)
	{
		pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	}
}
std::vector<ID2D1Bitmap*> D2DGraphics::CreateBitmapFromGif(const char* path) {
	std::vector<ID2D1Bitmap*> result;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !path)
	{
		return result;
	}
	ComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = _ImageFactory->CreateDecoderFromFilename(Convert::string_to_wstring(path).c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr))
	{
		return result;
	}
	UINT count = 0;
	if (FAILED(decoder->GetFrameCount(&count)))
	{
		return result;
	}
	for (UINT i = 0; i < count; ++i)
	{
		ComPtr<IWICBitmapFrameDecode> frame;
		if (FAILED(decoder->GetFrame(i, &frame)))
		{
			continue;
		}
		ComPtr<IWICFormatConverter> converter;
		if (FAILED(_ImageFactory->CreateFormatConverter(&converter)))
		{
			continue;
		}
		if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom)))
		{
			continue;
		}
		ComPtr<ID2D1Bitmap> bitmap;
		if (SUCCEEDED(ctx->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap)))
		{
			result.push_back(bitmap.Detach());
		}
	}
	return result;
}
std::vector<ID2D1Bitmap*> D2DGraphics::CreateBitmapFromGifBuffer(void* data, int size) {
	std::vector<ID2D1Bitmap*> result;
	auto* ctx = pD2DDeviceContext.Get();
	if (!ctx || !data || size <= 0)
	{
		return result;
	}
	ComPtr<IWICStream> stream;
	if (FAILED(_ImageFactory->CreateStream(&stream)))
	{
		return result;
	}
	if (FAILED(stream->InitializeFromMemory(static_cast<WICInProcPointer>(data), size)))
	{
		return result;
	}
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(_ImageFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder)))
	{
		return result;
	}
	UINT count = 0;
	if (FAILED(decoder->GetFrameCount(&count)))
	{
		return result;
	}
	for (UINT i = 0; i < count; ++i)
	{
		ComPtr<IWICBitmapFrameDecode> frame;
		if (FAILED(decoder->GetFrame(i, &frame)))
		{
			continue;
		}
		ComPtr<IWICFormatConverter> converter;
		if (FAILED(_ImageFactory->CreateFormatConverter(&converter)))
		{
			continue;
		}
		if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom)))
		{
			continue;
		}
		ComPtr<ID2D1Bitmap> bitmap;
		if (SUCCEEDED(ctx->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &bitmap)))
		{
			result.push_back(bitmap.Detach());
		}
	}
	return result;
}
HBITMAP D2DGraphics::CopyFromScreen(int x, int y, int width, int height) {
	HDC sourceDC = GetDC(NULL);
	HDC momDC = CreateCompatibleDC(sourceDC);
	HBITMAP memBitmap = CreateCompatibleBitmap(sourceDC, width, height);
	SelectObject(momDC, memBitmap);
	BitBlt(momDC, 0, 0, width, height, sourceDC, x, y, SRCCOPY);
	DeleteDC(momDC);
	ReleaseDC(NULL, sourceDC);
	return memBitmap;
}
HBITMAP D2DGraphics::CopyFromWidnow(HWND hWnd, int x, int y, int width, int height) {
	HDC sourceDC = GetDC(hWnd);
	HDC momDC;
	momDC = ::CreateCompatibleDC(sourceDC);
	HBITMAP memBitmap;
	memBitmap = ::CreateCompatibleBitmap(sourceDC, width, height);
	SelectObject(momDC, memBitmap);
	BitBlt(momDC, 0, 0, width, height, sourceDC, x, y, SRCCOPY);
	DeleteDC(momDC);
	ReleaseDC(hWnd, sourceDC);
	return memBitmap;
}
SIZE D2DGraphics::GetScreenSize(int index) {
	std::vector<SIZE> sizes = std::vector<SIZE>();
	auto callback = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		std::vector<SIZE>* tmp = (std::vector<SIZE>*)dwData;
		MONITORINFOEX miex{};
		miex.cbSize = sizeof(miex);
		GetMonitorInfo(hMonitor, &miex);
		DEVMODE dm{};
		dm.dmSize = sizeof(dm);
		dm.dmDriverExtra = 0;
		EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
		tmp->push_back(SIZE{ (int)dm.dmPelsWidth,(int)dm.dmPelsHeight });
		return TRUE;
		};
	EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)callback, (LPARAM)&sizes);
	if (sizes.size() > index) {
		return sizes[index];
	}
	return{ GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
}

D2D1_SIZE_F D2DGraphics::GetTextLayoutSize(IDWriteTextLayout* textLayout)
{
	D2D1_SIZE_F minSize = { 0,0 };
	DWRITE_TEXT_METRICS metrics;
	HRESULT hr = textLayout->GetMetrics(&metrics);
	if SUCCEEDED(hr)
	{
		minSize = D2D1::Size((float)ceil(metrics.widthIncludingTrailingWhitespace), (float)ceil(metrics.height));
		return minSize;
	}
	return D2D1_SIZE_F{ 0,0 };
}
HRESULT HwndGraphics::InitDevice() {
	RECT rc{};
	GetClientRect(hwnd, &rc);
	UINT width = std::max<UINT>(1, static_cast<UINT>(rc.right - rc.left));
	UINT height = std::max<UINT>(1, static_cast<UINT>(rc.bottom - rc.top));

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	ComPtr<IDXGISwapChain> basicSwapChain;
	HRESULT hr = _DxgiFactory->CreateSwapChain(_D3DDevice, &swapChainDesc, &basicSwapChain);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = basicSwapChain.As(&pSwapChain);
	if (FAILED(hr))
	{
		return hr;
	}

	contextOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	if (!EnsureDeviceContext(contextOptions))
	{
		return E_FAIL;
	}
	ReSize(width, height);
	if (!pD2DTargetBitmap)
	{
		return E_FAIL;
	}
	surfaceKind = SurfaceKind::SwapChain;
	return S_OK;
}
HwndGraphics::HwndGraphics(HWND hWnd) {
	hwnd = hWnd;
	InitDevice();
}
void HwndGraphics::ReSize(UINT width, UINT height) {
	width = std::max<UINT>(1, width);
	height = std::max<UINT>(1, height);
	if (!pSwapChain)
	{
		return;
	}
	if (!EnsureDeviceContext(contextOptions))
	{
		return;
	}
	ResetTarget();
	HRESULT hr = pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		return;
	}
	ComPtr<IDXGISurface> dxgiBackBuffer;
	hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	if (FAILED(hr))
	{
		return;
	}
	hr = InitializeWithDxgiSurface(dxgiBackBuffer.Get(), contextOptions);
	if (SUCCEEDED(hr))
	{
		surfaceKind = SurfaceKind::SwapChain;
	}
}
void HwndGraphics::BeginRender() {
	D2DGraphics::BeginRender();
}
void HwndGraphics::EndRender() {
	D2DGraphics::EndRender();
	if (pSwapChain)
	{
		pSwapChain->Present(1, 0);
	}
}
void IDXGISwapChainGraphics::ReSize(UINT width, UINT height) {
	width = std::max<UINT>(1, width);
	height = std::max<UINT>(1, height);
	if (!pSwapChain)
	{
		return;
	}
	if (!EnsureDeviceContext(contextOptions))
	{
		return;
	}
	ResetTarget();
	HRESULT hr = pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		return;
	}
	ComPtr<IDXGISurface> dxgiBackBuffer;
	hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	if (FAILED(hr))
	{
		return;
	}
	hr = InitializeWithDxgiSurface(dxgiBackBuffer.Get(), contextOptions);
	if (SUCCEEDED(hr))
	{
		surfaceKind = SurfaceKind::SwapChain;
	}
}
IDXGISwapChainGraphics::IDXGISwapChainGraphics(IDXGISwapChain* swapchain) {
	if (!swapchain)
	{
		return;
	}
	swapchain->AddRef();
	ComPtr<IDXGISwapChain> basicSwapChain;
	basicSwapChain.Attach(swapchain);
	HRESULT hr = basicSwapChain.As(&pSwapChain);
	if (FAILED(hr))
	{
		return;
	}

	contextOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	if (!EnsureDeviceContext(contextOptions))
	{
		return;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	hr = pSwapChain->GetDesc(&swapChainDesc);
	if (FAILED(hr))
	{
		return;
	}
	ReSize(swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height);
	surfaceKind = SurfaceKind::SwapChain;
	if (!pD2DTargetBitmap)
	{
		pSwapChain.Reset();
	}
}
void IDXGISwapChainGraphics::BeginRender() {
	D2DGraphics::BeginRender();
}
void IDXGISwapChainGraphics::EndRender() {
	D2DGraphics::EndRender();
	if (pSwapChain)
	{
		pSwapChain->Present(1, 0);
	}
}
void DxgiSurfaceGraphics::ReSize(UINT width, UINT height) {
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);
	if (!pDxgiSurface)
	{
		return;
	}
	HRESULT hr = InitializeWithDxgiSurface(pDxgiSurface.Get(), contextOptions);
	if (SUCCEEDED(hr))
	{
		surfaceKind = SurfaceKind::DxgiSurface;
	}
}
DxgiSurfaceGraphics::DxgiSurfaceGraphics(IDXGISurface* dxgiSurface) {
	if (!dxgiSurface)
	{
		return;
	}
	dxgiSurface->AddRef();
	pDxgiSurface.Attach(dxgiSurface);
	contextOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	HRESULT hr = InitializeWithDxgiSurface(pDxgiSurface.Get(), contextOptions);
	if (FAILED(hr))
	{
		pDxgiSurface.Reset();
	}
}
void DxgiSurfaceGraphics::BeginRender() {
	D2DGraphics::BeginRender();
}
void DxgiSurfaceGraphics::EndRender() {
	D2DGraphics::EndRender();
}