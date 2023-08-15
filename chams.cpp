#include <D3Dcompiler.h>
#include <d3d11.h>
#include <dxgi.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <Windows.h>

#include "chams.hpp" 
#include "minhook/MinHook.h" 
 
 
//globals
int stride_val = -1, index_val = -1, ve_val = -1;
 
//wh
ID3D11DepthStencilState* DepthStencilState_FALSE = NULL; //depth off
ID3D11DepthStencilState* DepthStencilState_ORIG = NULL; //depth on

//rendertarget
ID3D11RenderTargetView *mainRenderTargetViewD3D11;
ID3D11RasterizerState *DEPTHBIASState_FALSE, *DEPTHBIASState_TRUE;
ID3D11RasterizerState *rwState, *rsState;

ID3D11DepthStencilState *m_DepthStencilState, *m_origDepthStencilState;
ID3D11RasterizerState *orig_raster;
//shader
ID3D11PixelShader *psPurple = NULL;
ID3D11PixelShader *psGray = NULL;

// Function pointers that will hold the address of the DirectX functions hooked by MinHook.
// Example :fnc_DrawIndexed is a function pointer that will point to DrawIndexed in d3d11.
void (__stdcall *fnc_CreateQuery) (ID3D11Device *, const D3D11_QUERY_DESC *, ID3D11Query **);
void (__stdcall *fnc_DrawIndexed) (ID3D11DeviceContext *, UINT, UINT, INT);
void (__stdcall *fnc_DrawIndexedInstanced) (ID3D11DeviceContext *, UINT, UINT, UINT, INT, UINT);
HRESULT (__stdcall *fnc_Present) (IDXGISwapChain*, UINT, UINT);
HRESULT (__stdcall *fnc_ResizeBuffers) (IDXGISwapChain *, UINT, UINT, UINT, DXGI_FORMAT, UINT);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
DWORD_PTR *pSwapChainVtable = NULL;
DWORD_PTR *pContextVTable = NULL;
DWORD_PTR *pDeviceVTable = NULL;
bool initonce = false;
HWND window = NULL;
WNDPROC oWndProc;
const int MultisampleCount = 1;

//generate shader func
HRESULT GenerateShader(ID3D11Device* pD3DDevice, ID3D11PixelShader** pShader, float r, float g, float b)
{
	char szCast[] = "struct VS_OUT"
		"{"
		" float4 position : SV_Position;"
		" float4 color : COLOR0;"
		"};"
        "struct PS_OUT"
        "{"
        " float4 color : SV_Target;"
        "};"
		"PS_OUT main(VS_OUT input)"
		"{"
        "  PS_OUT output = (PS_OUT)0;"
		"  float4 fake;"
		"  fake.a = 1.0f;"
		"  fake.r = %f;"
		"  fake.g = %f;"
		"  fake.b = %f;"
		"  output.color = fake;"
        "  return output;"
		"}";
	ID3D10Blob* pBlob;
	char szPixelShader[1000];

	sprintf_s(szPixelShader, szCast, r, g, b);

	ID3DBlob* d3dErrorMsgBlob;

	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, &pBlob, &d3dErrorMsgBlob);

	if(FAILED(hr))
		return hr;

	hr = pD3DDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pShader);

	if(FAILED(hr))
		return hr;

	return S_OK;
}

DWORD __stdcall disable_chams(LPVOID)
{
    if(MH_Uninitialize() != MH_OK)
        return 0; 

    if(MH_DisableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK) 
        return 0; 

    if(MH_DisableHook((DWORD_PTR*)pSwapChainVtable[13]) != MH_OK) 
        return 0;

    if(MH_DisableHook((DWORD_PTR*)pContextVTable[12]) != MH_OK) 
        return 0; 

    if(MH_DisableHook((DWORD_PTR*)pContextVTable[20]) != MH_OK) 
        return 0; 
    return 1;
}    

void log_data(std::string data)
{
    std::string path = getenv("USERPROFILE");
    path += "\\Desktop\\log.txt";
    std::fstream file;
    file.open(path.c_str(), std::fstream::out | std::fstream::app);
    file << data;
    file.close();
}


void __stdcall hook_CreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery)
{
	if (pQueryDesc->Query == D3D11_QUERY_OCCLUSION)
	{
		D3D11_QUERY_DESC oqueryDesc = CD3D11_QUERY_DESC();
		(&oqueryDesc)->MiscFlags = pQueryDesc->MiscFlags;
		(&oqueryDesc)->Query = D3D11_QUERY_TIMESTAMP;
	}
	return fnc_CreateQuery(pDevice, pQueryDesc, ppQuery);
}


void __stdcall hook_DrawIndexed(ID3D11DeviceContext *p_context, UINT IndexCount,
                                 UINT StartIndexLocation, INT BaseVertexLocation)
{
	ID3D11Buffer* veBuffer;
	UINT veWidth;
	UINT Stride;
	UINT veBufferOffset;
	D3D11_BUFFER_DESC veDesc;
 
	//get models
	p_context->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
	if(veBuffer) 
    {
		veBuffer->GetDesc(&veDesc);
		veWidth = veDesc.ByteWidth;
	}
	if(NULL != veBuffer) 
    {
		veBuffer->Release();
		veBuffer = NULL;
	}
 
	//the models
	if(veWidth == 33019776|| //blazkowicz
		veWidth == 32416480|| //ranger
		veWidth == 37471908|| //scalebearer
		veWidth == 36695844|| //visor
		veWidth == 34416600|| //anarki
		veWidth == 27186138|| //nyx
		veWidth == 49464832|| //sorlag
		veWidth == 37147024|| //clutch
		veWidth == 40442608|| //galena
		veWidth == 36916404|| //slash
		veWidth == 30639408|| //doom slayer
		veWidth == 35216464|| //keel
		veWidth == 65903852|| //strogg
		veWidth == 36858108|| //death knight
		veWidth == 27980754|| //athena
		veWidth == 43159020) //eisen
	{
        UINT stencilValue = 0;
        p_context->OMGetDepthStencilState(&m_origDepthStencilState, &stencilValue);
        p_context->PSSetShader(psGray, NULL, NULL);
        p_context->OMSetDepthStencilState(m_DepthStencilState, 0); //wh on
        fnc_DrawIndexed(p_context, IndexCount, StartIndexLocation, BaseVertexLocation);
        p_context->OMSetDepthStencilState(m_origDepthStencilState, stencilValue); //wh off*/
        p_context->PSSetShader(psPurple, NULL, NULL);
        fnc_DrawIndexed(p_context, IndexCount, StartIndexLocation, BaseVertexLocation);
        p_context->PSSetShader(NULL, NULL, NULL);
	}	
 
	//ctf flag
	if(IndexCount == 16002 && veWidth == 339528)
	{
        UINT stencilValue = 0;
        p_context->OMGetDepthStencilState(&m_origDepthStencilState, &stencilValue);
        p_context->OMSetDepthStencilState(m_DepthStencilState, 0); //wh on
        fnc_DrawIndexed(p_context, IndexCount, StartIndexLocation, BaseVertexLocation);
        p_context->OMSetDepthStencilState(m_origDepthStencilState, stencilValue); //wh off
	}
 
	//fix wallhack
	if ((IndexCount == 18963 && veWidth == 54718412) ||  //awoken map1
		(IndexCount == 22494 && veWidth == 61246792) ||  //blood covenant
		(IndexCount == 11178 && veWidth == 62215376) ||  //blood run
		(IndexCount == 20319 && veWidth == 66169560) ||  //burial chamber, whats IndexCount == 20199 && veWidth == 422082
		(IndexCount == 29340 && veWidth == 66682728) ||  //church of azathoth
		(IndexCount == 60762 && veWidth == 66982464) ||  //citadel
		(IndexCount == 19824 && veWidth == 66088608) ||  //corrupted keep
		(IndexCount == 4986 && veWidth == 67011072)  ||  //deep embrace
		(IndexCount == 648 && veWidth == 36378192)   ||  //exile
		(IndexCount == 11265 && veWidth == 23886216) ||  //insomnia
		(IndexCount == 13548 && veWidth == 59518192) ||  //lockbox
		(IndexCount == 35826 && veWidth == 62882780) ||  //ruins of sarnath
		(IndexCount == 28224 && veWidth == 66891856) ||  //tempest shrine
		(IndexCount == 16887 && veWidth == 4053728)  ||  //the dark zone
		(IndexCount == 6300 && veWidth == 63972264)  ||  //the longest yard
		(IndexCount == 6792 && veWidth == 66900360)  ||  //the molten falls
		(IndexCount == 11514 && veWidth == 51351000) ||  //tower of koth
		(IndexCount == 9810 && veWidth == 41727180)) //vale of pnath
		{
			return;
		}
 


	/*if(stride_val == Stride && index_val < IndexCount < IndexCount && ve_val < veWidth)
	{
        if(GetAsyncKeyState(VK_END) & 1)//log key
        {
            std::ostringstream out;
            out << "Stride ==  " << Stride << " IndexCount == " << IndexCount << " veWidth == " << veWidth << std::endl; 
            out << "stride_val == " << stride_val << std::endl;
            out << "index_val == " << index_val << std::endl;
            log_data(out.str());
        }
		return;
	}*/


    return fnc_DrawIndexed(p_context, IndexCount, StartIndexLocation, BaseVertexLocation);
}



LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
 
 
void __stdcall hook_DrawIndexedInstanced(ID3D11DeviceContext *p_context, 
                                              UINT IndexCountPerInstance, UINT InstanceCount, 
                                              UINT StartIndexLocation, INT BaseVertexLocation, 
                                              UINT StartInstanceLocation)
{
	ID3D11Buffer* veBuffer2;
	UINT veWidth2;
	UINT Stride2;
	UINT veBufferOffset2;
	D3D11_BUFFER_DESC veDesc2;
 
	//get models
	p_context->IAGetVertexBuffers(0, 1, &veBuffer2, &Stride2, &veBufferOffset2);
	if(veBuffer2) 
    {
		veBuffer2->GetDesc(&veDesc2);
		veWidth2 = veDesc2.ByteWidth;
	}
	if(NULL != veBuffer2) 
    {
		veBuffer2->Release();
		veBuffer2 = NULL;
	}
	
	//wallhack items(health, armor, powerups)
	if((IndexCountPerInstance == 948 && veWidth2 == 16824)|| //mega health
		(IndexCountPerInstance == 4266 && veWidth2 == 47208) || //heavy armor
		(IndexCountPerInstance == 2184 && veWidth2 == 25824) || //light armour
		(IndexCountPerInstance == 972 && veWidth2 == 30672) || //small health inside
		(IndexCountPerInstance == 2280 && veWidth2 == 30672) || //small health bubble
		(IndexCountPerInstance == 1008 && veWidth2 == 24352) || //quad damage
		(IndexCountPerInstance == 1020 && veWidth2 == 21984) || //battle suit
		(IndexCountPerInstance == 2592 && veWidth2 == 99700) || //hourglass
		(IndexCountPerInstance == 2550 && veWidth2 == 40232))  //armor shards
	{
		pContext->RSSetState(DEPTHBIASState_FALSE); //wh on
		fnc_DrawIndexedInstanced(p_context, IndexCountPerInstance, InstanceCount, 
                                 StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		pContext->RSSetState(DEPTHBIASState_TRUE); //wh off
	}
 
	
	/*
	//logger
	if (stride_val == IndexCountPerInstance / 100)//1000
		if (GetAsyncKeyState(VK_END) & 1)//log key
			Log("Stride2 == %d && IndexCountPerInstance == %d && veWidth2 == %d", Stride2, IndexCountPerInstance, veWidth2);
 
	if (stride_val == IndexCountPerInstance / 100)//1000
	{
		//p_context->RSSetState(DEPTHBIASState_FALSE); //wh on
		//fnc_DrawIndexedInstanced(p_context, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		//p_context->RSSetState(DEPTHBIASState_TRUE); //wh off
		return;
	}
 
	//hold down P key until a texture is erased, press END to log values of those textures
	if (GetAsyncKeyState('O') & 1) //-
		stride_val--;
	if (GetAsyncKeyState('P') & 1) //+
		stride_val++;
	if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState('9') & 1) //reset, set to -1
		stride_val = -1;
	*/
	return fnc_DrawIndexedInstanced(p_context, IndexCountPerInstance, InstanceCount, 
                                    StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

 
HRESULT __stdcall hook_Present(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!initonce)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetViewD3D11);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
 
            // Disabling Z-Buffering
            D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
            depthStencilDesc.DepthEnable = TRUE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilReadMask = 0xFF;
            depthStencilDesc.StencilWriteMask = 0xFF;

            // Stencil operations if pixel is front-facing
            depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
            depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
            depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            // Stencil operations if pixel is back-facing
            depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
            depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            
            pDevice->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);  

			//create depthbias rasterizer state
			D3D11_RASTERIZER_DESC rasterizer_desc;
			ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
			rasterizer_desc.FillMode = D3D11_FILL_SOLID;
			rasterizer_desc.CullMode = D3D11_CULL_NONE; 
			rasterizer_desc.FrontCounterClockwise = false;
			rasterizer_desc.SlopeScaledDepthBias = 0.0f;
			rasterizer_desc.DepthBiasClamp = 0.0f;
			rasterizer_desc.DepthClipEnable = true;
			rasterizer_desc.ScissorEnable = false;
			rasterizer_desc.MultisampleEnable = false;
			rasterizer_desc.AntialiasedLineEnable = false;
			rasterizer_desc.DepthBias = 0x7FFFFFFF;
			pDevice->CreateRasterizerState(&rasterizer_desc, &DEPTHBIASState_FALSE);            
			initonce = true;
		}
        else
			return hook_Present(pSwapChain, SyncInterval, Flags);
	}
    
    if(!psPurple)
	    GenerateShader(pDevice, &psPurple, 1.0f, 0.0f, 1.0f);
    if(!psGray)
        GenerateShader(pDevice, &psGray, 0.0703f, 0.07032f, 0.0781f);
 
	//recreate rendertarget on reset
	if(mainRenderTargetViewD3D11 == NULL)
	{
		ID3D11Texture2D* pBackBuffer = NULL;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetViewD3D11);
		pBackBuffer->Release();
	}
	return fnc_Present(pSwapChain, SyncInterval, Flags);
}


HRESULT __stdcall hook_ResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, 
                                     UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if(nullptr != mainRenderTargetViewD3D11) 
    { 
        mainRenderTargetViewD3D11->Release(); 
        mainRenderTargetViewD3D11 = nullptr; 
    }
	return fnc_ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}


LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


DWORD __stdcall install_chams(LPVOID)
{
    // 1. Get handle of dxgi.dll library.
    HMODULE hDXGIDLL = NULL;
    while(hDXGIDLL == NULL)
    {
        hDXGIDLL = GetModuleHandle("dxgi.dll");
        Sleep(4000);    
    }
	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
    IDXGISwapChain* pSwapChain;
 
	WNDCLASSEXA wc = {sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), 
                      NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, 
                              wc.hInstance, NULL);
 
	D3D_FEATURE_LEVEL requestedLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;
 
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
 
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = MultisampleCount;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
 
	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;
 
	UINT createFlags = 0;
    #ifdef _DEBUG
	// This flag gives you some quite wonderful debug text. Not wonderful for performance, though!
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
 
	IDXGISwapChain *d3dSwapChain = 0;
 
	if(FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		createFlags, requestedLevels, sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION, &scd, &pSwapChain, &pDevice, &obtainedLevel, &pContext)))
	{
		MessageBox(hWnd, "Failed to create DirectX device and swapchain!", "Error", MB_ICONERROR);
		return NULL;
	}
 

    pSwapChainVtable = (DWORD_PTR*)pSwapChain;
    pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
 
    pContextVTable = (DWORD_PTR*)pContext;
    pContextVTable = (DWORD_PTR*)pContextVTable[0];
 
	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	if(MH_Initialize() != MH_OK)
        return 1;
        
    if(MH_CreateHook((DWORD_PTR*)pContextVTable[12], hook_DrawIndexed, 
        reinterpret_cast<LPVOID*>(&fnc_DrawIndexed)) != MH_OK) 
        return 1;
	if(MH_EnableHook((DWORD_PTR*)pContextVTable[12]) != MH_OK)
        return 1;
    
	if(MH_CreateHook((DWORD_PTR*)pContextVTable[20], hook_DrawIndexedInstanced, 
       reinterpret_cast<LPVOID*>(&fnc_DrawIndexedInstanced)) != MH_OK)
        return 1;
	if(MH_EnableHook((DWORD_PTR*)pContextVTable[20]) != MH_OK)
        return 1;
    
	if(MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hook_Present, 
       reinterpret_cast<LPVOID*>(&fnc_Present)) != MH_OK) 
        return 1;
	if(MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK)
        return 1; 
    
	if(MH_CreateHook((DWORD_PTR*)pSwapChainVtable[13], hook_ResizeBuffers, 
       reinterpret_cast<LPVOID*>(&fnc_ResizeBuffers)) != MH_OK)
        return 1;
	if(MH_EnableHook((DWORD_PTR*)pSwapChainVtable[13]) != MH_OK)
        return 1; 

	if (MH_CreateHook((DWORD_PTR*)pDeviceVTable[24], hook_CreateQuery, reinterpret_cast<void**>(&fnc_CreateQuery)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pDeviceVTable[24]) != MH_OK) { return 1; }
    VirtualProtect(hook_Present, 2, PAGE_EXECUTE_READWRITE, NULL);
  
	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();
    return 1;
}

