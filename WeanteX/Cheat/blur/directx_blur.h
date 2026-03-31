#pragma once

#include "imgui.h"
#include <d3d11.h>
#include <d3dx11tex.h>
#include "pshader.hpp"

inline ID3D11PixelShader* pshader = nullptr;
inline ID3D11ShaderResourceView* tex = nullptr;
inline ID3D11Device* device = nullptr;
inline ID3D11DeviceContext* ctx = nullptr;

inline static bool updateTexture = false;

struct ImGui_ImplDX11_Data
{
	ID3D11Device* pd3dDevice;
	ID3D11DeviceContext* pd3dDeviceContext;
	IDXGIFactory* pFactory;
	ID3D11Buffer* pVB;
	ID3D11Buffer* pIB;
	ID3D11VertexShader* pVertexShader;
	ID3D11InputLayout* pInputLayout;
	ID3D11Buffer* pVertexConstantBuffer;
	ID3D11PixelShader* pPixelShader;
	ID3D11SamplerState* pFontSampler;
	ID3D11ShaderResourceView* pFontTextureView;
	ID3D11RasterizerState* pRasterizerState;
	ID3D11BlendState* pBlendState;
	ID3D11DepthStencilState* pDepthStencilState;
	int                         VertexBufferSize;
	int                         IndexBufferSize;

	ImGui_ImplDX11_Data() { memset((void*)this, 0, sizeof(*this)); VertexBufferSize = 1; IndexBufferSize = 1; }
};

void safe_release(IUnknown*& resource) {
	if (resource != nullptr) {
		resource->Release();
		resource = nullptr;
	}
}

void get_back_buffer(IDXGISwapChain* swap_chain) {
	ID3D11Texture2D* back_buffer = nullptr;
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

	D3D11_TEXTURE2D_DESC desc;
	back_buffer->GetDesc(&desc);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* back_buffer_tex = NULL;
	device->CreateTexture2D(&desc, nullptr, &back_buffer_tex);
	ctx->CopyResource(back_buffer_tex, back_buffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	safe_release(reinterpret_cast<IUnknown*&>(tex));
	device->CreateShaderResourceView(back_buffer_tex, &srvDesc, &tex);

	safe_release(reinterpret_cast<IUnknown*&>(back_buffer));
	safe_release(reinterpret_cast<IUnknown*&>(back_buffer_tex));
}


void begin(const ImDrawList* d, const ImDrawCmd* cmd) {
	const auto swap_chain = reinterpret_cast<IDXGISwapChain*>(cmd->UserCallbackData);
	auto bd = (ImGui_ImplDX11_Data*)ImGui::GetIO().BackendRendererUserData;

	if (!tex)
		get_back_buffer(swap_chain);

	if (!pshader) {
		device->CreatePixelShader(pshaderd, sizeof(pshaderd), NULL, &pshader);
	}

	ctx->PSSetShader(pshader, nullptr, 0);
	ctx->PSSetSamplers(0, 1, &bd->pFontSampler);
}

void release(const ImDrawList* d, const ImDrawCmd* cmd) {
	safe_release(reinterpret_cast<IUnknown*&>(tex));
}

void draw_background_blur(ImDrawList* draw_list, IDXGISwapChain* swap_chain, ID3D11Device* _device, ID3D11DeviceContext* _ctx, ImVec2 start, ImVec2 end, int rounding = 0, ImDrawFlags flags = 0) {

	device = _device;
	ctx = _ctx;

	draw_list->AddCallback(begin, swap_chain);
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	ImVec2 uv0 = ImVec2(start.x / displaySize.x, start.y / displaySize.y);
	ImVec2 uv1 = ImVec2(end.x / displaySize.x, end.y / displaySize.y);
	draw_list->AddImageRounded(tex, start, end, uv0, uv1, ImColor{ 1.f, 1.f, 1.f, ImGui::GetStyle().Alpha }, rounding, flags);
	draw_list->AddCallback(ImDrawCallback_ResetRenderState, 0);
}

void handle_input() {
		updateTexture = true;
}