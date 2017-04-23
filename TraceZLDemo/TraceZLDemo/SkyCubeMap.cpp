#include "SkyCubeMap.h"

SkyCubeMap::SkyCubeMap(ID3D12Device * md3dDevice,ID3D12GraphicsCommandList * mCommandList
	): mCommandList(mCommandList), md3dDevice(md3dDevice)
{
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SkyCB = std::make_unique<UploadBuffer<SkyData>>(md3dDevice, (UINT)1, true);
}

SkyCubeMap::~SkyCubeMap()
{
}

void SkyCubeMap::LoadTextures(std::string name, std::wstring Filename, 
	std::unordered_map<std::string, std::unique_ptr<Texture>>& mTextures,
	std::vector<std::string>& texNames,
	UINT& mTextureIndex)
{
	auto texMap = std::make_unique<Texture>();
	texMap->Name = name;
	texMap->Filename = Filename;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice,
		mCommandList, texMap->Filename.c_str(),
		texMap->Resource, texMap->UploadHeap));
	texNames.push_back(name);
	mTextures[texMap->Name] = std::move(texMap);
	mSkyTexHeapIndex=mTextureIndex++;
}

void SkyCubeMap::BuildPSOs(bool m4xMsaaState, UINT m4xMsaaQuality, DXGI_FORMAT mBackBufferFormat, DXGI_FORMAT mDepthStencilFormat)
{
	
	mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\mSky.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\mSky.hlsl", nullptr, "PS", "ps_5_1");
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc;
	ZeroMemory(&skyPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	skyPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	skyPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	skyPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	skyPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	skyPsoDesc.SampleMask = UINT_MAX;
	skyPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	skyPsoDesc.NumRenderTargets = 1;
	skyPsoDesc.RTVFormats[0] = mBackBufferFormat;
	skyPsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	skyPsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	skyPsoDesc.DSVFormat = mDepthStencilFormat;
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = (mRootSignature).Get();
	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()),
		mShaders["skyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()),
		mShaders["skyPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&(mPSOs)["sky"])));
}

void SkyCubeMap::Init(std::string name, std::wstring Filename, std::unordered_map<std::string, std::unique_ptr<Texture>>& mTextures, std::vector<std::string>& texNames, UINT & mTextureIndex, bool m4xMsaaState, UINT m4xMsaaQuality, DXGI_FORMAT mBackBufferFormat, DXGI_FORMAT mDepthStencilFormat)
{
	LoadTextures(name, Filename,mTextures,texNames,mTextureIndex);
	BuildRenderItems();
	BuildRootSignature();
	BuildPSOs(m4xMsaaState, m4xMsaaQuality, mBackBufferFormat, mDepthStencilFormat);
}

void SkyCubeMap::Update(Camera mCamera)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMStoreFloat4x4(&mSkyData.gViewProj, XMMatrixTranspose(viewProj));
	mSkyData.gEyePosW = mCamera.GetPosition3f();
	XMStoreFloat4x4(&mSkyData.gWorld, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	mSkyData.ObjPad0 = 0;
	SkyCB.get()->CopyData(0, mSkyData);
	//skyData->CopyData(0, mSkyData);
	int i = 1;
}

void SkyCubeMap::Draw(ComPtr<ID3D12DescriptorHeap>& mSrvDescriptorHeap)
{
	mCommandList->SetGraphicsRootSignature((mRootSignature).Get());
	mCommandList->SetGraphicsRootConstantBufferView(0, SkyCB->Resource()->GetGPUVirtualAddress());
	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(1, skyTexDescriptor);
	mCommandList->SetPipelineState((mPSOs)["sky"].Get());

	auto ri = mRitem;
	mCommandList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(ri->PrimitiveType);

	//D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
	//mCommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
	mCommandList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);


}

void SkyCubeMap::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	//slotRootParameter[1].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS((mRootSignature).GetAddressOf())));

}

void SkyCubeMap::BuildRenderItems()
{
	//加载面片
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData sky = geoGen.CreateSphere(0.5f, 20, 20);
	SubmeshGeometry skySubmesh;
	skySubmesh.IndexCount = (UINT)sky.Indices32.size();
	skySubmesh.StartIndexLocation = 0;
	skySubmesh.BaseVertexLocation = 0;

	std::vector<Vertex> vertices(sky.Vertices.size());
	for (size_t i = 0; i < sky.Vertices.size(); ++i)
	{
		vertices[i].Pos = sky.Vertices[i].Position;
		vertices[i].Normal = sky.Vertices[i].Normal;
		vertices[i].TexC = sky.Vertices[i].TexC;
	}
	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(sky.GetIndices16()), std::end(sky.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size()  * sizeof(std::uint16_t);

	auto geo = new MeshGeometry();
	geo->Name = "skyGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice,
		mCommandList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice,
		mCommandList, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["sky"] = skySubmesh;

	//材质
	auto skyMat = new Material();
	skyMat->Name = "sky";
	skyMat->MatCBIndex = 0;
	skyMat->DiffuseSrvHeapIndex = 0;
	skyMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skyMat->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	skyMat->Roughness = 1.0f;

	//形状
	auto skyRitem = new RenderItem();
	XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->TexTransform = MathHelper::Identity4x4();
	skyRitem->ObjCBIndex = 0;
	skyRitem->Mat = skyMat;
	skyRitem->Geo = geo;
	skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sky"].IndexCount;
	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sky"].StartIndexLocation;
	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sky"].BaseVertexLocation;

	mRitem = skyRitem;

}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> SkyCubeMap::GetStaticSamplers()
{
		// Applications usually only need a handful of samplers.  So just define them all up front
		// and keep them available as part of the root signature.  

		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		return{
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp };

}
