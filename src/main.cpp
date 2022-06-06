#include <iostream>
#include <string>

#if _WIN32
#include <Windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <DXGI.h>
#include <D3D11.h>
#include <d3dcompiler.h>

#include "Dixa/Dixa.hpp"

const char* shader_blit = R"(
struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = float4(OUT.uv * 2 - 1, 0, 1);

	// In D3D (0, 0) stands for upper left corner
	OUT.uv.y = 1.0 - OUT.uv.y;

	return OUT;
}

cbuffer Test
{
	float4 color;
};

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	return float4(color.xyz, 1.0);
}

)";

GLFWwindow* CreateWindowDX(int& w, int& h)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);

	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

	auto window = glfwCreateWindow(w, h, "Test DX11 App", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	glfwShowWindow(window);

	glfwGetWindowSize(window, &w, &h);

	return window;
}

bool InitD3Device(ID3D11Device*& device, ID3D11DeviceContext*& context)
{
	uint32_t DeviceCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;

	const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL CreatedFeatureLevel;

	HRESULT Hr = D3D11CreateDevice(nullptr, DriverType, nullptr, DeviceCreationFlags, FeatureLevels, sizeof(FeatureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &device, &CreatedFeatureLevel, &context);

	return !FAILED(Hr);
}

bool InitD3SwapChain(GLFWwindow* window, ID3D11Device* device, IDXGISwapChain*& swapChain)
{
	HWND hwnd = glfwGetWin32Window(window);

	int w, h;
	glfwGetWindowSize(window, &w, &h);

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	memset(&SwapChainDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = w;
	SwapChainDesc.BufferDesc.Height = h;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = hwnd;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGIDevice1* DXGIDevice;
	HRESULT Hr = device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	IDXGIAdapter1* DXGIAdapter;
	Hr = DXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&DXGIAdapter);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	IDXGIFactory1* DXGIFactory;
	Hr = DXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&DXGIFactory);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	Hr = DXGIFactory->CreateSwapChain(DXGIDevice, &SwapChainDesc, &swapChain);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	Hr = DXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	return true;
}

bool InitD3BackBuffer(ID3D11Device* device, IDXGISwapChain* swapChain, ID3D11Texture2D*& backBuffer, ID3D11RenderTargetView*& backBufferView)
{
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	HRESULT Hr = device->CreateRenderTargetView(backBuffer, &RTVDesc, &backBufferView);
	if (!SUCCEEDED(Hr))
	{
		return false;
	}

	return true;
}

HRESULT CompileShaderFromString(const std::string& shaderSource, const std::string& name, const bool debug, const D3D_SHADER_MACRO* macros, _In_ ID3DInclude* include, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

	if (debug)
	{
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	}

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), name.c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

struct vec4
{
	float x, y, z, w;
};

template<typename T>
constexpr T IsAligned(T val, uint64_t alignment)
{
	return !((uint64_t)val & (alignment - 1));
}

static_assert(IsAligned(sizeof(vec4), 12), "vec3 is not aligned to 16!");

int main()
{
	std::cout << "Hello, World!" << std::endl;

	int w = 1280;
	int h = 720;

	GLFWwindow* window = CreateWindowDX(w, h);

	// Init DX11
	ID3D11Device* d11Device;
	ID3D11DeviceContext* d11DeviceContext;

	if(!InitD3Device(d11Device, d11DeviceContext))
	{
		std::cout << "Could not init DX11 device !" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Init Swap chain
	IDXGISwapChain* swapChain;

	if(!InitD3SwapChain(window, d11Device, swapChain))
	{
		std::cout << "Could not create swap chain !" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Init back buffer
	ID3D11Texture2D* backBuffer;
	ID3D11RenderTargetView* backBufferView;

	if(!InitD3BackBuffer(d11Device, swapChain, backBuffer, backBufferView))
	{
		std::cout << "Could not init backbuffer !" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Init viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = w;
	viewport.Height = h;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	ID3DBlob* vsShaderBlob = nullptr;
	HRESULT hr = CompileShaderFromString(shader_blit, "VS", false, nullptr, nullptr, "MainVS", "vs_5_0", &vsShaderBlob);

	if(FAILED(hr))
	{
		std::cout << "Could not compile vertex shader !" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	ID3DBlob* psShaderBlob = nullptr;
	HRESULT hr2 = CompileShaderFromString(shader_blit, "PS", false, nullptr, nullptr, "MainPS", "ps_5_0", &psShaderBlob);

	if(FAILED(hr2))
	{
		std::cout << "Could not compile vertex shader !" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	ID3D11VertexShader* vs;
	if(FAILED(d11Device->CreateVertexShader(vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), nullptr, &vs)))
	{
		std::cout << "Creating VS failed" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	ID3D11PixelShader* ps;
	if(FAILED(d11Device->CreatePixelShader(psShaderBlob->GetBufferPointer(), psShaderBlob->GetBufferSize(), nullptr, &ps)))
	{
		std::cout << "Creating VS failed" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Init raster state
	ID3D11RasterizerState* rasterizerState;

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	if(FAILED(d11Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState)))
	{
		std::cout << "Could not create raster state" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	static float TransparentColor[4]{
		0.2f, 0.6f, 0.8f, 1.0f
	};


	ID3D11Buffer* cbuffer;
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(vec4);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d11Device->CreateBuffer(&constantBufferDesc, nullptr, &cbuffer)))
	{
		std::cout << "Could not create buffer" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	d11Device->CreateShaderResourceView()

	double prevTime = glfwGetTime();
	double accumulator = 0;
	int frameCounter = 0;

	while(!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		double delta = currentTime - prevTime;
		prevTime = currentTime;

		if (accumulator >= 1.0f)
		{
			std::cout << "Frame time: " << delta << ", FPS: " << frameCounter << std::endl;
			accumulator = 0;
			frameCounter = 0;
		}
		accumulator += delta;
		frameCounter++;

		glfwPollEvents();

		{
			vec4 vec = { 0, 1, 2, 1 };

			D3D11_MAPPED_SUBRESOURCE mappedData;
			if (SUCCEEDED(d11DeviceContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
			{
				memcpy(mappedData.pData, &vec, sizeof(vec));
			}
			d11DeviceContext->Unmap(cbuffer, 0);
		}

		d11DeviceContext->OMSetRenderTargets(1, &backBufferView, nullptr);

		d11DeviceContext->RSSetViewports(1, &viewport);
		d11DeviceContext->ClearRenderTargetView(backBufferView, TransparentColor);

		d11DeviceContext->IASetInputLayout(nullptr);
		d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		d11DeviceContext->VSSetShader(vs, nullptr, 0);
		d11DeviceContext->PSSetShader(ps, nullptr, 0);

		d11DeviceContext->PSSetConstantBuffers(0, 1, &cbuffer);

		d11DeviceContext->RSSetState(rasterizerState);
		d11DeviceContext->Draw(4, 0);

		d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

		swapChain->Present(0, 0);
	}

	rasterizerState->Release();

	vs->Release();
	ps->Release();

	vsShaderBlob->Release();
	psShaderBlob->Release();

	backBufferView->Release();
	backBuffer->Release();

	swapChain->Release();
	d11DeviceContext->Release();
	d11Device->Release();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
