#pragma once
#include "../Common/d3dApp.h"
#include "../Common/Camera.h"
#include "SkyCubeMap.h"
#include "FrameResouce.h"
#include "ZLSenceRender.h"
#include <DirectXColors.h>
using namespace DirectX;
class TraceZLDemoApp :public D3DApp
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	TraceZLDemoApp(HINSTANCE hInstance);
	/// <summary>
	/// 析构函数
	/// </summary>
	~TraceZLDemoApp();
	/// <summary>
	/// 用来进行初始化
	/// </summary>
	///<returns>返回是否成功初始化</returns>
	virtual bool Initialize()override;

private:
	/// <summary>
	/// 调节窗口大小是调用
	/// </summary>
	virtual void OnResize()override;
	/// <summary>
	/// 每帧调用一次该函数用来更新数据
	/// </summary>
	/// <param name="gt"> 传入时间对象</param>
	virtual void Update(const GameTimer& gt)override;
	/// <summary>
	/// 每帧调用一次该函数，使用GPU渲染
	/// </summary>
	/// <param name="gt"> 传入时间对象</param>
	virtual void Draw(const GameTimer& gt)override;

	

	//virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	//virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	//virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	/// <summary>
	/// 加载图片并返回图片的ID号
	/// </summary>
	UINT LoadTextures(std::string name, std::wstring Filename);
	/// <summary>
	/// 绑定HLSL中的资源(buffer)
	/// </summary>
	void BuildRootSignature();
	/// <summary>
	/// 构建资源堆并填充（贴图）
	/// </summary>
	void BuildDescriptorHeaps();
	/// <summary>
	/// 构建GPU调用函数的入口和入口函数的参数
	/// </summary>
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	/// <summary>
	/// 构建帧资源
	/// </summary>
	void BuildFrameResources();
	/// <summary>
	/// 通过传入贴图构建材质
	/// </summary>
	void BuildMaterials(std::string name, std::wstring Filename,
		XMFLOAT4 DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3 FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f), float Roughness = 1.0f);
	/// <summary>
	/// 构建单个渲染物体
	/// </summary>
	void BuildRenderItems();
	/// <summary>
	/// 渲染单个渲染物体
	/// </summary>
	void DrawRenderItems(ID3D12GraphicsCommandList * cmdList, const std::vector<RenderItem*>& ritems);
	/// <summary>
	/// 返回六种采样方式，在HLSL中运用，可以自己设定几种
	/// </summary>
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
private:
	/// <summary>
	/// 根标签，用来绑定HLSL的资源（buffer)
	/// </summary>
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mSkyRootSignature = nullptr;
	/// <summary>
	/// 存放SRV的资源堆（贴图）
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	/// <summary>
	/// SRV、CBV描述器的大小，在构建SRV资源堆的时候用来偏移的大小
	/// </summary>
	UINT mCbvSrvDescriptorSize = 0;
	/// <summary>
	/// GPU调用函数的入口函数（HLSL的入口函数）
	/// </summary>
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	/// <summary>
	/// HLSL在调用函数的时候传入数据的组织方式（定点入口函数的输入）
	/// </summary>
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	/// <summary>
	/// 渲染管道状态
	/// </summary>
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	//帧资源
	/// <summary>
	/// 帧资源集合
	/// </summary>
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	/// <summary>
	/// 当前帧渲染所用的帧资源
	/// </summary>
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;
	/// <summary>
	/// 所有要渲染的RenderItem
	/// </summary>
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	/// <summary>
	/// 对应PSO要渲染的RenderItem
	/// </summary>
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	/// <summary>
	/// 对应物体的形状
	/// </summary>
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;

	Camera mCamera;
	std::unique_ptr<SkyCubeMap> skyBox;
	PassConstants mMainPassCB;

	//加载贴图用的资源
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::vector<std::string> texNames;
	UINT mTextureIndex = 0;
	//天空盒的标志
	UINT mSkyTexHeapIndex = 0;
	//构建材质所需的资源
	UINT mMaterialIndex = 0;
	/// <summary>
	/// 所有材质的集合
	/// </summary>
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;

};