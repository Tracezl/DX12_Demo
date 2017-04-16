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
	/// ���캯��
	/// </summary>
	TraceZLDemoApp(HINSTANCE hInstance);
	/// <summary>
	/// ��������
	/// </summary>
	~TraceZLDemoApp();
	/// <summary>
	/// �������г�ʼ��
	/// </summary>
	///<returns>�����Ƿ�ɹ���ʼ��</returns>
	virtual bool Initialize()override;

private:
	/// <summary>
	/// ���ڴ��ڴ�С�ǵ���
	/// </summary>
	virtual void OnResize()override;
	/// <summary>
	/// ÿ֡����һ�θú���������������
	/// </summary>
	/// <param name="gt"> ����ʱ�����</param>
	virtual void Update(const GameTimer& gt)override;
	/// <summary>
	/// ÿ֡����һ�θú�����ʹ��GPU��Ⱦ
	/// </summary>
	/// <param name="gt"> ����ʱ�����</param>
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
	/// ����ͼƬ������ͼƬ��ID��
	/// </summary>
	UINT LoadTextures(std::string name, std::wstring Filename);
	/// <summary>
	/// ��HLSL�е���Դ(buffer)
	/// </summary>
	void BuildRootSignature();
	/// <summary>
	/// ������Դ�Ѳ���䣨��ͼ��
	/// </summary>
	void BuildDescriptorHeaps();
	/// <summary>
	/// ����GPU���ú�������ں���ں����Ĳ���
	/// </summary>
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	/// <summary>
	/// ����֡��Դ
	/// </summary>
	void BuildFrameResources();
	/// <summary>
	/// ͨ��������ͼ��������
	/// </summary>
	void BuildMaterials(std::string name, std::wstring Filename,
		XMFLOAT4 DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3 FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f), float Roughness = 1.0f);
	/// <summary>
	/// ����������Ⱦ����
	/// </summary>
	void BuildRenderItems();
	/// <summary>
	/// ��Ⱦ������Ⱦ����
	/// </summary>
	void DrawRenderItems(ID3D12GraphicsCommandList * cmdList, const std::vector<RenderItem*>& ritems);
	/// <summary>
	/// �������ֲ�����ʽ����HLSL�����ã������Լ��趨����
	/// </summary>
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
private:
	/// <summary>
	/// ����ǩ��������HLSL����Դ��buffer)
	/// </summary>
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mSkyRootSignature = nullptr;
	/// <summary>
	/// ���SRV����Դ�ѣ���ͼ��
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	/// <summary>
	/// SRV��CBV�������Ĵ�С���ڹ���SRV��Դ�ѵ�ʱ������ƫ�ƵĴ�С
	/// </summary>
	UINT mCbvSrvDescriptorSize = 0;
	/// <summary>
	/// GPU���ú�������ں�����HLSL����ں�����
	/// </summary>
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	/// <summary>
	/// HLSL�ڵ��ú�����ʱ�������ݵ���֯��ʽ��������ں��������룩
	/// </summary>
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	/// <summary>
	/// ��Ⱦ�ܵ�״̬
	/// </summary>
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	//֡��Դ
	/// <summary>
	/// ֡��Դ����
	/// </summary>
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	/// <summary>
	/// ��ǰ֡��Ⱦ���õ�֡��Դ
	/// </summary>
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;
	/// <summary>
	/// ����Ҫ��Ⱦ��RenderItem
	/// </summary>
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	/// <summary>
	/// ��ӦPSOҪ��Ⱦ��RenderItem
	/// </summary>
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	/// <summary>
	/// ��Ӧ�������״
	/// </summary>
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;


	Camera mCamera;
	std::unique_ptr<SkyBox> sky;
	std::unique_ptr<SkyCubeMap> skyBox;
	PassConstants mMainPassCB;


	//������ͼ�õ���Դ
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::vector<std::string> texNames;
	UINT mTextureIndex = 0;
	//��պеı�־
	UINT mSkyTexHeapIndex = 0;
	//���������������Դ
	UINT mMaterialIndex = 0;
	/// <summary>
	/// ���в��ʵļ���
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