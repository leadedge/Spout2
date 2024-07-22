/*

	spoutDX12.h

	Functions to manage DirectX 12 texture sharing by way of the D3D11On12

	Copyright (c) 2020-2024, Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#pragma once
#ifndef __spoutDX12__ 
#define __spoutDX12__

//
// SpoutDX base class
//
// Include files are in the same folder as SpoutDX.h for build using a dll or library.
// SpoutDX.h is one level above for build using the repository folder structure.
//
#if __has_include("SpoutCommon.h")
#include "SpoutDX.h" 
#else
#include "..\\SpoutDX.h"
#endif

#include <d3d12.h>
#include <d3d11on12.h>
#include <dxgi1_6.h> // for IDXGIFactory6 in GetHardwareAdapter
#pragma comment (lib, "d3d12.lib")// the Direct3D 11 Library file
#pragma comment (lib, "DXGI.lib") // for CreateDXGIFactory1


// Copied from Microsoft examples
struct DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
	DX12_HEAP_PROPERTIES() = default;
	explicit DX12_HEAP_PROPERTIES(const D3D12_HEAP_PROPERTIES &o) noexcept :
		D3D12_HEAP_PROPERTIES(o)
	{}
	DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
		D3D12_MEMORY_POOL memoryPoolPreference,
		UINT creationNodeMask = 1,
		UINT nodeMask = 1) noexcept
	{
		Type = D3D12_HEAP_TYPE_CUSTOM;
		CPUPageProperty = cpuPageProperty;
		MemoryPoolPreference = memoryPoolPreference;
		CreationNodeMask = creationNodeMask;
		VisibleNodeMask = nodeMask;
	}
	explicit DX12_HEAP_PROPERTIES(
		D3D12_HEAP_TYPE type,
		UINT creationNodeMask = 1,
		UINT nodeMask = 1) noexcept
	{
		Type = type;
		CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		CreationNodeMask = creationNodeMask;
		VisibleNodeMask = nodeMask;
	}
	bool IsCPUAccessible() const noexcept
	{
		return Type == D3D12_HEAP_TYPE_UPLOAD || Type == D3D12_HEAP_TYPE_READBACK || (Type == D3D12_HEAP_TYPE_CUSTOM &&
			(CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE || CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK));
	}
};


class SPOUT_DLLEXP spoutDX12 : public spoutDX {

	public:

		spoutDX12();
		~spoutDX12();

		// Initialize and prepare DirectX 12
		bool OpenDirectX12();
		bool OpenDirectX12(ID3D12Device* pd3dDevice12, IUnknown** ppCommandQueue = nullptr);

		// Release DirectX 12
		void CloseDirectX12();

		// Send wrapped D3D11on12 D3D11 texture resource
		bool SendDX11Resource(ID3D11Resource *pWrappedResource);

		// Receive a texture from a sender to a D3D12 texture resource
		bool ReceiveDX12Resource(ID3D12Resource** ppDX12Resource);

		// Create a D3D11on12 device
		ID3D11On12Device* CreateDX11on12device(ID3D12Device* pDevice12, IUnknown** ppCommandQueue = nullptr);

		// Wrap a D3D12 resource for use with D3D11
		bool WrapDX12Resource(ID3D12Resource* pDX12Resource, ID3D11Resource** ppWrapped11Resource, D3D12_RESOURCE_STATES InitialState);

		// Update a wrapped D3D11 texture resource with a D3D11 texture
		void UpdateWrappedResource(ID3D11Resource* pWrappedResource, ID3D11Resource *pResource);
		
		// Create a D3D12 texture resource
		bool CreateDX12texture(ID3D12Device* pDevice12, 
			unsigned int width, unsigned int height,
			D3D12_RESOURCE_STATES InitialState,
			DXGI_FORMAT format,
			ID3D12Resource** ppTexture);

		//
		// Adapter functions
		//
		
		// Get IDXGIAdapter1 pointer for a given adapter (-1 means current)
		IDXGIAdapter1* GetAdapterPointer1(int index = -1);
		// Set required graphics adapter for creating a class D3D12 device
		void SetAdapterPointer1(IDXGIAdapter1* pAdapter);


		// Device pointers
		ID3D12Device* GetD3D12device(); // D3D12 device
		ID3D11Device* GetD3D11device(); // D3D11 device
		ID3D11DeviceContext* GetD3D11context(); // D3D11 context
		ID3D11On12Device* GetD3D11On12device(); // D3D11on12 device
		
	protected:

		ID3D12Device* CreateDX12device();
		void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

		ID3D12Device* m_pd3dDevice12; // D3D12 device
		ID3D11Device* m_pd3dDevice11; // D3D11 device
		ID3D11DeviceContext* m_pd3dDeviceContext11; // D3D11 context
		ID3D11On12Device* m_pd3d11On12Device; // D3D11on12 device
		bool m_bClassDevice; // Using a class or application device

		// The wrapped D3D11 resource for D3D12
		ID3D11Resource* m_pReceivedResource11;

		// Class adapter pointer
		IDXGIAdapter1* m_pAdapterDX12;

};

#endif
