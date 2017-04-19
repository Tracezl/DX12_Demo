#pragma once
#include "FrameResouce.h"
#include "../Common/GeometryGenerator.h"
#include "../Common/Camera.h"
using Microsoft::WRL::ComPtr;
class SkyCubeMap
{
public:
	SkyCubeMap(ID3D12Device * md3dDevice, FrameResource ** mFrameResources, ComPtr<ID3D12RootSignature>* mRootSignature, std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>* mPSOs, ID3D12GraphicsCommandList * mCommandList);
	~SkyCubeMap();
	void LoadTextures(std::string name, std::wstring Filename, std::unordered_map<std::string, std::unique_ptr<Texture>>& mTextures, std::vector<std::string>& texNames, UINT & mTextureIndex);
public:
	void BuildPSOs(bool m4xMsaaState, UINT m4xMsaaQuality, DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
	void Update(Camera mCamera);
	void Draw(ComPtr<ID3D12DescriptorHeap>& mSrvDescriptorHeap);

	void BuildRenderItems();
private:
	void BuildRootSignature();
	//void BuildRenderItems(ID3D12GraphicsCommandList * mCommandList);
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	//std::vector<std::unique_ptr<SkyFrameResource>> mFrameResources;
	SkyData mSkyData;
	FrameResource** mFrameResources;
	//ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	//ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	//Material* mMaterials=nullptr;
	//Texture* mTextures=nullptr;
	//MeshGeometry* mGeometries=nullptr;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	
	//ComPtr<ID3D12PipelineState> mPSOs;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>>* mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	UINT mCbvSrvDescriptorSize = 0;

	ID3D12Device* md3dDevice = nullptr;

	RenderItem* mRitem = nullptr;

	ComPtr<ID3D12RootSignature>* mRootSignature;
	ID3D12GraphicsCommandList * mCommandList;

	//天空盒的标志
	UINT mSkyTexHeapIndex = 0;
};

