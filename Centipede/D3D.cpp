#include <vector>
#include <cassert>

#include "D3DUtil.h"
#include "D3D.h"
#include "WindowUtils.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;



float MyD3D::GetAspectRatio()
{
	int w,h;
	WinUtil::Get().GetClientExtents(w, h);
	return w / (float)h;
}

void MyD3D::BeginRender(const Vector4 & colour)
{
	mpd3dImmediateContext->ClearRenderTargetView(mpRenderTargetView, reinterpret_cast<const float*>(&colour));
	mpd3dImmediateContext->ClearDepthStencilView(mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void MyD3D::EndRender()
{
	HR(mpSwapChain->Present(0, 0));
}



// Resize the swap chain and recreate the render target view.
void MyD3D::ResizeSwapChain(int screenWidth, int screenHeight)
{
	HR(mpSwapChain->ResizeBuffers(1, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(mpd3dDevice->CreateRenderTargetView(backBuffer, 0, &mpRenderTargetView));
	ReleaseCOM(backBuffer);
}

// Create the depth/stencil buffer description
void MyD3D::CreateDepthStencilDescription(D3D11_TEXTURE2D_DESC& dsd, int screenWidth, int screenHeight, bool msaa4X, int maxQuality)
{
	dsd.Width = screenWidth;
	dsd.Height = screenHeight;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (msaa4X)
	{
		dsd.SampleDesc.Count = 4;
		dsd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		dsd.SampleDesc.Count = 1;
		dsd.SampleDesc.Quality = 0;
	}

	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsd.CPUAccessFlags = 0;
	dsd.MiscFlags = 0;
}


void MyD3D::CreateDepthStencilBufferAndView(D3D11_TEXTURE2D_DESC& dsd)
{
	HR(mpd3dDevice->CreateTexture2D(&dsd, 0, &mpDepthStencilBuffer));
	HR(mpd3dDevice->CreateDepthStencilView(mpDepthStencilBuffer, 0, &mpDepthStencilView));
}


void MyD3D::BindRenderTargetViewAndDepthStencilView()
{
	mpd3dImmediateContext->OMSetRenderTargets(1, &mpRenderTargetView, mpDepthStencilView);
}

// Set the viewport transform.
void MyD3D::SetViewportDimensions(int screenWidth, int screenHeight)
{
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(screenWidth);
	mScreenViewport.Height = static_cast<float>(screenHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mpd3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}


//start your engines!
void MyD3D::CreateD3D( D3D_FEATURE_LEVEL desiredFeatureLevel)
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	md3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
	//md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	//figure out how many gpus we have
	IDXGIAdapter * pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;
	IDXGIFactory * pFactory = NULL;
	SIZE_T useIdx = -1, mostRam=-1;
	HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory));
	for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		vAdapters.push_back(pAdapter);
		DXGI_ADAPTER_DESC desc; 
		HR(pAdapter->GetDesc(&desc));
		WDBOUT(L"Found adapter=(" << i << ") " << desc.Description << L" VRAM=" << desc.DedicatedVideoMemory);
		if (desc.DedicatedVideoMemory > mostRam || mostRam==-1)
		{
			useIdx = i;
			mostRam = desc.DedicatedVideoMemory;
		}
	}
	if (pFactory)
		pFactory->Release();
	assert(useIdx >= 0);

	D3D_FEATURE_LEVEL featureLevel;
	HR(D3D11CreateDevice(
		vAdapters[useIdx],                 // default adapter
		md3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&mpd3dDevice,
		&featureLevel,
		&mpd3dImmediateContext));

	if (featureLevel != desiredFeatureLevel)
	{
		DBOUT("Direct3D Feature Level unsupported.");
		assert(false);
	}

}


void MyD3D::CheckMultiSamplingSupport(UINT& quality4xMsaa)
{
	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.
	// 4XMSAA looks good!
	HR(mpd3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality4xMsaa));
	
	//if zero was returned then the hardware cannot do it
	assert(quality4xMsaa > 0);
}

//we need appropriately sized buffers to render into
void MyD3D::CreateSwapChainDescription(DXGI_SWAP_CHAIN_DESC& sd, HWND hMainWnd, bool windowed, int screenWidth, int screenHeight)
{
	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	sd.BufferDesc.Width = screenWidth;
	sd.BufferDesc.Height = screenHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (mEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hMainWnd;
	sd.Windowed = windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	mWindowed = windowed;
}

void MyD3D::CreateSwapChain(DXGI_SWAP_CHAIN_DESC& sd)
{
	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	HR(mpd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(mpd3dDevice, &sd, &mpSwapChain));

	//now it's been created these factory objects are no longer needed
	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);
}


void MyD3D::OnResize_Default(int clientWidth, int clientHeight)
{
	assert(mpd3dImmediateContext);
	assert(mpd3dDevice);
	assert(mpSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	ReleaseCOM(mpRenderTargetView);
	ReleaseCOM(mpDepthStencilView);
	ReleaseCOM(mpDepthStencilBuffer);


	ResizeSwapChain(clientWidth, clientHeight);

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	CreateDepthStencilDescription(depthStencilDesc, clientWidth, clientHeight, mEnable4xMsaa, m4xMsaaQuality - 1);
	CreateDepthStencilBufferAndView(depthStencilDesc);

	BindRenderTargetViewAndDepthStencilView();

	SetViewportDimensions(clientWidth, clientHeight);

}


bool MyD3D::InitDirect3D(void(*pOnResize)(int,int,MyD3D&))
{
	assert(pOnResize);
	mpOnResize = pOnResize;

	// Create the device and device context.
	CreateD3D();

	CheckMultiSamplingSupport(m4xMsaaQuality);

	int w, h;
	WinUtil::Get().GetClientExtents(w,h);
	DXGI_SWAP_CHAIN_DESC sd;
	CreateSwapChainDescription(sd, WinUtil::Get().GetMainWnd(), true, w, h);
	CreateSwapChain(sd);


	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	mpOnResize(w, h, *this);

	CreateWrapSampler(mpWrapSampler);

	return true;
}

void MyD3D::ReleaseD3D(bool extraReporting)
{
	mTexCache.Release();
	//check if full screen - not advisable to exit in full screen mode
	if (mpSwapChain)
	{
		BOOL fullscreen = false;
		HR(mpSwapChain->GetFullscreenState(&fullscreen, nullptr));
		if (fullscreen) //go for a window
			mpSwapChain->SetFullscreenState(false, nullptr);
	}

	ReleaseCOM(mpRenderTargetView);
	ReleaseCOM(mpDepthStencilView);
	ReleaseCOM(mpSwapChain);
	ReleaseCOM(mpDepthStencilBuffer);

	// Restore all default settings.
	if (mpd3dImmediateContext)
	{
		mpd3dImmediateContext->ClearState();
		mpd3dImmediateContext->Flush();
	}

	ReleaseCOM(mpd3dImmediateContext);
	if (extraReporting)
	{
		ID3D11Debug* pd3dDebug;
		HR(mpd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pd3dDebug)));
		HR(pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY));
		ReleaseCOM(pd3dDebug);
	}
	ReleaseCOM(mpd3dDevice);
}

void MyD3D::CreateWrapSampler(ID3D11SamplerState* &pSampler)
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(mpd3dDevice->CreateSamplerState(&sampDesc, &pSampler));
}