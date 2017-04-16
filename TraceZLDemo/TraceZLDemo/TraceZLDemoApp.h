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
	std::unique_ptr<SkyBox> sky;
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
//struct RenderItem
//{
//	RenderItem() = default;
//	RenderItem(const RenderItem& rhs) = delete;
//
//	// World matrix of the shape that describes the object's local space
//	// relative to the world space, which defines the position, orientation,
//	// and scale of the object in the world.
//	XMFLOAT4X4 World = MathHelper::Identity4x4();
//
//	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
//
//	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
//	// Because we have an object cbuffer for each FrameResource, we have to apply the
//	// update to each FrameResource.  Thus, when we modify obect data we should set 
//	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
//	int NumFramesDirty = gNumFrameResources;
//
//	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
//	UINT ObjCBIndex = -1;
//
//	Material* Mat = nullptr;
//	MeshGeometry* Geo = nullptr;
//
//	// Primitive topology.
//	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//
//	// DrawIndexedInstanced parameters.
//	UINT IndexCount = 0;
//	UINT StartIndexLocation = 0;
//	int BaseVertexLocation = 0;
//};
//struct ObjectConstants
//{
//	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
//	UINT     MaterialIndex;
//	UINT     ObjPad0;
//	UINT     ObjPad1;
//	UINT     ObjPad2;
//};
//struct PassConstants
//{
//	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 ViewProjTex = MathHelper::Identity4x4();
//	DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();
//	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
//	float cbPerObjectPad1 = 0.0f;
//	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
//	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
//	float NearZ = 0.0f;
//	float FarZ = 0.0f;
//	float TotalTime = 0.0f;
//	float DeltaTime = 0.0f;
//
//	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
//	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
//	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
//	// are spot lights for a maximum of MaxLights per object.
//	Light Lights[MaxLights];
//};
//struct MaterialData
//{
//	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
//	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
//	float Roughness = 0.5f;
//
//	// Used in texture mapping.
//	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
//
//	UINT DiffuseMapIndex = 0;
//	UINT NormalMapIndex = 0;
//	UINT MaterialPad1;
//	UINT MaterialPad2;
//};
//struct Vertex
//{
//	DirectX::XMFLOAT3 Pos;
//	DirectX::XMFLOAT3 Normal;
//	DirectX::XMFLOAT2 TexC;
//	DirectX::XMFLOAT3 TangentU;
//};