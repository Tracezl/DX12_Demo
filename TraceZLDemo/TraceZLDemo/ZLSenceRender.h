#pragma once
#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/GameTimer.h"
#include "../Common/Camera.h"
#include "../Common/GeometryGenerator.h"
#include "FrameResouce.h"
using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace DirectX::PackedVector;
class ZLSenceRender
{
public:
	ZLSenceRender(ID3D12Device* md3dDevice);
	~ZLSenceRender();

	UINT LoadTextures(std::string name, std::wstring Filename, ID3D12GraphicsCommandList * mCommandList);
	void BuildMaterials(std::string name, std::wstring Filename, ID3D12GraphicsCommandList * mCommandList, XMFLOAT4 DiffuseAlbedo, XMFLOAT3 FresnelR0, float Roughness);
private:
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	//void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::vector<std::string> texNames;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	UINT mSkyTexHeapIndex = 0;
	UINT mTextureIndex = 0;
	UINT mMaterialIndex = 0;
	ID3D12Device* md3dDevice = nullptr;

	PassConstants mMainPassCB;
};
class SkyBox
{
public:
	SkyBox(ID3D12Device* md3dDevice);
	~SkyBox();
	void LoadTextures(std::string name, std::wstring Filename, ID3D12GraphicsCommandList * mCommandList);
	void BuildPSOs(DXGI_FORMAT mBackBufferFormat, bool m4xMsaaState, UINT m4xMsaaQuality, DXGI_FORMAT mDepthStencilFormat);
	void Update(Camera mCamera);
	void DrawF(ID3D12GraphicsCommandList * mCommandList);
	void Draw(ID3D12GraphicsCommandList * mCommandList);
	
private:
	void BuildRootSignature();
	void BuildRenderItems(ID3D12GraphicsCommandList * mCommandList);
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	//std::vector<std::unique_ptr<SkyFrameResource>> mFrameResources;
	SkyData mSkyData;
	SkyFrameResource* mFrameResources;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	//Material* mMaterials=nullptr;
	//Texture* mTextures=nullptr;
	//MeshGeometry* mGeometries=nullptr;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	//ComPtr<ID3D12PipelineState> mPSOs;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ID3D12Device* md3dDevice = nullptr;

	RenderItem* mRitem=nullptr;
};