#include "TraceZLDemoApp.h" 
const int gNumFrameResources = 3;
/// <summary>
/// 入口函数
/// </summary>
/// <param name="hInstance"> </param>
/// <returns></returns>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)//WINDOS入口函数
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		TraceZLDemoApp theApp(hInstance);
		if (!theApp.Initialize())//调用Initialize函数进行初始化
			return 0;

		return theApp.Run();//如果调用初始化函数成功继续调用Run()函数，并返回Run()的返回值
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);//发生错误弹出带OK按钮的消息框
		return 0;
	}
}
	TraceZLDemoApp::TraceZLDemoApp(HINSTANCE hInstance)
		:D3DApp(hInstance)
	{
	}
	TraceZLDemoApp::~TraceZLDemoApp()
	{
	}
	bool TraceZLDemoApp::Initialize()//重写Initialize()这个虚函数
	{
		if (!D3DApp::Initialize())//调用父类的Initalize函数
			return false;
		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
		mCamera.SetPosition(0.0f, 2.0f, -15.0f);
		mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		skyBox = std::make_unique<SkyCubeMap>(md3dDevice.Get(), &mCurrFrameResource, &mSkyRootSignature, &mPSOs, mCommandList.Get());
		//LoadTextures("skyCubeMap1", L"Textures/grasscube1024.dds");
		skyBox->LoadTextures("sky", L"Textures/snowcube1024.dds", mTextures, texNames, mTextureIndex);
		skyBox->BuildRenderItems();
		//BuildRootSignature();
		skyBox->BuildPSOs(m4xMsaaState, m4xMsaaQuality);
		//加载天空盒
		//mSkyTexHeapIndex =LoadTextures("skyCubeMap1", L"Textures/grasscube1024.dds");
		BuildDescriptorHeaps();
		//BuildMaterials("skyCubeMap", L"Textures/grasscube1024.dds");
		//BuildRootSignature();
		//BuildShadersAndInputLayout();
		//BuildShapeGeometry();
		//BuildRenderItems();
		BuildFrameResources();
		
		//BuildPSOs();

		////////////尝试的东西////
		///*sky = std::make_unique<SkyBox>(md3dDevice.Get());
		//sky->LoadTextures("skyCubeMap", L"Textures/grasscube1024.dds", mCommandList.Get());
		//sky->BuildPSOs(mBackBufferFormat, m4xMsaaState, m4xMsaaQuality, mDepthStencilFormat);*/
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until initialization is complete.
		FlushCommandQueue();
		return true;
	}
	void TraceZLDemoApp::OnResize()//重写虚函数，
	{
		D3DApp::OnResize();//调用父类的函数
		mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	}
	void TraceZLDemoApp::Update(const GameTimer& gt)//重写虚函数，
	{
		mCamera.UpdateViewMatrix();
		

		mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		///sky->Update(mCamera);
		//AnimateMaterials(gt);
		//UpdateObjectCBs(gt);
		//UpdateMaterialBuffer(gt);
		//UpdateMainPassCB(gt);
		skyBox->Update(mCamera);
	}
	void TraceZLDemoApp::Draw(const GameTimer& gt)//重写虚函数，
	{
		//ThrowIfFailed(mDirectCmdListAlloc->Reset());//??

		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
		ThrowIfFailed(cmdListAlloc->Reset());
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["sky"].Get()));
		/////ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));//初始化命令行列表
		//sky->DrawF(mCommandList.Get());
		//设置GPU和CPU同步的障碍点
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// 设置视口（显示到屏幕的大小和位置）
		mCommandList->RSSetViewports(1, &mScreenViewport);
		//设置剪裁空间转换出来的贴图大小
		mCommandList->RSSetScissorRects(1, &mScissorRect);


		// 清空深度和后台缓存区
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightGreen, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// 指定要渲染的缓存区
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
		ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		//mCommandList->SetGraphicsRootSignature(mSkyRootSignature.Get());

		/*auto passCB = mCurrFrameResource->PassCB->Resource();
		mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

		auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
		mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
*/
		//CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		//skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvDescriptorSize);
		//mCommandList->SetGraphicsRootDescriptorTable(1, skyTexDescriptor);

		// Bind all the textures used in this scene.  Observe
		// that we only have to specify the first descriptor in the table.  
		// The root signature knows how many descriptors are expected in the table.
		///mCommandList->SetGraphicsRootDescriptorTable(4, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		
		//auto passCB = (mCurrFrameResource)->SkyCB->Resource();
		//mCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());
		skyBox->Draw(mSrvDescriptorHeap);
		//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
		//sky->Draw(mCommandList.Get());
		// 显示在资源使用状态转换。
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// 关闭命令列表
		ThrowIfFailed(mCommandList->Close());

		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// 把后台缓冲区的翻转到前台
		ThrowIfFailed(mSwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
		mCurrFrameResource->Fence = ++mCurrentFence;

		mCommandQueue->Signal(mFence.Get(), mCurrentFence);
		///同步GPU和CPU
		///FlushCommandQueue();
	}
	void TraceZLDemoApp::AnimateMaterials(const GameTimer & gt)
	{
		XMMATRIX view = mCamera.GetView();
		XMMATRIX proj = mCamera.GetProj();
		SkyData mSkyData;
		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMStoreFloat4x4(&mSkyData.gViewProj, XMMatrixTranspose(viewProj));
		mSkyData.gEyePosW = mCamera.GetPosition3f();
		XMStoreFloat4x4(&mSkyData.gWorld, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
		mSkyData.ObjPad0 = 0;
		//mFrameResources->SkyCB.get()->CopyData(0, mSkyData);
		auto skyData = mCurrFrameResource->SkyCB.get();
		skyData->CopyData(0, mSkyData);
	}
	void TraceZLDemoApp::UpdateObjectCBs(const GameTimer & gt)
	{
		auto currObjectCB = mCurrFrameResource->ObjectCB.get();
		for (auto& e : mAllRitems)
		{
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (e->NumFramesDirty > 0)
			{
				XMMATRIX world = XMLoadFloat4x4(&e->World);
				XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
				objConstants.MaterialIndex = 0;// e->Mat->MatCBIndex;

				currObjectCB->CopyData(e->ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				e->NumFramesDirty--;
			}
		}
	}
	void TraceZLDemoApp::UpdateMaterialBuffer(const GameTimer & gt)
	{
		auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
		for (auto& e : mMaterials)
		{
			// Only update the cbuffer data if the constants have changed.  If the cbuffer
			// data changes, it needs to be updated for each FrameResource.
			Material* mat = e.second.get();
			if (mat->NumFramesDirty > 0)
			{
				XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

				MaterialData matData;
				matData.DiffuseAlbedo = mat->DiffuseAlbedo;
				matData.FresnelR0 = mat->FresnelR0;
				matData.Roughness = mat->Roughness;
				XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
				matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

				currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

				// Next FrameResource need to be updated too.
				mat->NumFramesDirty--;
			}
		}
	}
	void TraceZLDemoApp::UpdateMainPassCB(const GameTimer & gt)
	{
		XMMATRIX view = mCamera.GetView();
		XMMATRIX proj = mCamera.GetProj();

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		mMainPassCB.EyePosW = mCamera.GetPosition3f();
		mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
		mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
		mMainPassCB.NearZ = 1.0f;
		mMainPassCB.FarZ = 1000.0f;
		mMainPassCB.TotalTime = gt.TotalTime();
		mMainPassCB.DeltaTime = gt.DeltaTime();
		mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
		mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
		mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

		auto currPassCB = mCurrFrameResource->PassCB.get();
		currPassCB->CopyData(0, mMainPassCB);
	}
	/// <summary>
	/// 加载图片并返回图片的ID号
	/// </summary>
	UINT TraceZLDemoApp::LoadTextures(std::string name, std::wstring Filename)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->Name = name;
		texMap->Filename = Filename;
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));
		texNames.push_back(name);
		mTextures[texMap->Name] = std::move(texMap);
		return mTextureIndex++;
	}
	/// <summary>
	/// 绑定HLSL中的资源(buffer)
	/// </summary>
	void TraceZLDemoApp::BuildRootSignature()
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


		//CD3DX12_DESCRIPTOR_RANGE texTable0;
		//texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		//CD3DX12_DESCRIPTOR_RANGE texTable1;
		//texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, 0);

		//// Root parameter can be a table, root descriptor or root constants.
		//CD3DX12_ROOT_PARAMETER slotRootParameter[5];

		//// Perfomance TIP: Order from most frequent to least frequent.
		//slotRootParameter[0].InitAsConstantBufferView(0);
		//slotRootParameter[1].InitAsConstantBufferView(1);
		//slotRootParameter[2].InitAsShaderResourceView(0, 1);
		//slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
		//slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);


		//auto staticSamplers = GetStaticSamplers();

		//// A root signature is an array of root parameters.
		//CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		//	(UINT)staticSamplers.size(), staticSamplers.data(),
		//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		//ComPtr<ID3DBlob> serializedRootSig = nullptr;
		//ComPtr<ID3DBlob> errorBlob = nullptr;
		//HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		//	serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		//if (errorBlob != nullptr)
		//{
		//	::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		//}
		//ThrowIfFailed(hr);

	 //   ThrowIfFailed(md3dDevice->CreateRootSignature(
		//	0,
		//	serializedRootSig->GetBufferPointer(),
		//	serializedRootSig->GetBufferSize(),
		//	IID_PPV_ARGS(mRootSignature.GetAddressOf())));
	}
	/// <summary>
	/// 构建资源堆并填充（贴图）
	/// </summary>
	void TraceZLDemoApp::BuildDescriptorHeaps()
	{
		//
		// Create the SRV heap.
		//
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = texNames.size();
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

		//
		// Fill out the heap with actual descriptors.
		//
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		if (texNames.size() > 0)
		{
			if (mSkyTexHeapIndex== 0)
			{	
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = mTextures[texNames[0]]->Resource->GetDesc().MipLevels;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				srvDesc.Format = mTextures[texNames[0]]->Resource->GetDesc().Format;
				md3dDevice->CreateShaderResourceView(mTextures[texNames[0]]->Resource.Get(), &srvDesc, hDescriptor);
			}
			else {
				srvDesc.Format = mTextures[texNames[0]]->Resource->GetDesc().Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = mTextures[texNames[0]]->Resource->GetDesc().MipLevels;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				md3dDevice->CreateShaderResourceView(mTextures[texNames[0]]->Resource.Get(), &srvDesc, hDescriptor);
			}
			for (UINT i = 1; i < (UINT)texNames.size(); i++)
			{
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
				if (mSkyTexHeapIndex == i)
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = mTextures[texNames[i]]->Resource->GetDesc().MipLevels;
					srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
					srvDesc.Format = mTextures[texNames[i]]->Resource->GetDesc().Format;
					md3dDevice->CreateShaderResourceView(mTextures[texNames[i]]->Resource.Get(), &srvDesc, hDescriptor);
				}
				else {
					srvDesc.Format = mTextures[texNames[i]]->Resource->GetDesc().Format;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = mTextures[texNames[i]]->Resource->GetDesc().MipLevels;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
					md3dDevice->CreateShaderResourceView(mTextures[texNames[i]]->Resource.Get(), &srvDesc, hDescriptor);
				}
			}
		}
	}
	/// <summary>
	/// 构建GPU调用函数的入口和入口函数的参数
	/// </summary>
	void TraceZLDemoApp::BuildShadersAndInputLayout()
	{
		const D3D_SHADER_MACRO alphaTestDefines[] =
		{
			"ALPHA_TEST", "1",
			NULL, NULL
		};

		mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

		mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\mSky.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\mSky.hlsl", nullptr, "PS", "ps_5_1");

		mInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	}

	void TraceZLDemoApp::BuildShapeGeometry()
	{
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

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "skyGeo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["sky"] = skySubmesh;
		mGeometries[geo->Name] = std::move(geo);
	}
	/// <summary>
	/// 构建渲染管道状态
	/// </summary>
	void TraceZLDemoApp::BuildPSOs()
	{

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
	/// <summary>
	/// 构建帧资源
	/// </summary>
	void TraceZLDemoApp::BuildFrameResources()
	{
		for (int i = 0; i < gNumFrameResources; ++i)
		{
			int ritemsNum = 1;
			if (mAllRitems.size()>0)
			{
				ritemsNum = mAllRitems.size();
			}
			int materialNum = 1;
			if (mMaterials.size()>0)
			{
				materialNum = mMaterials.size();
			}
			mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
				1, (UINT)ritemsNum, (UINT)materialNum));
		}
	}
	/// <summary>
	/// 通过传入贴图的位置构建材质
	/// </summary>
	void TraceZLDemoApp::BuildMaterials(std::string name, std::wstring Filename,
		XMFLOAT4 DiffuseAlbedo, XMFLOAT3 FresnelR0, float Roughness)
	{
		auto mat = std::make_unique<Material>();
		mat->Name = name;
		mat->MatCBIndex = mMaterialIndex++;
		mat->DiffuseSrvHeapIndex = LoadTextures(name, Filename);
		mat->DiffuseAlbedo = DiffuseAlbedo;
		mat->FresnelR0 = FresnelR0;
		mat->Roughness = Roughness;

		mMaterials[name] = std::move(mat);
	}
	/// <summary>
	/// 构建单个渲染物体
	/// </summary>
	void TraceZLDemoApp::BuildRenderItems()
	{
		auto skyRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
		skyRitem->TexTransform = MathHelper::Identity4x4();
		skyRitem->ObjCBIndex = 0;
		skyRitem->Mat = mMaterials["skyCubeMap"].get();
		skyRitem->Geo = mGeometries["skyGeo"].get();
		skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sky"].IndexCount;
		skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sky"].StartIndexLocation;
		skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sky"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(skyRitem.get());
		mAllRitems.push_back(std::move(skyRitem));
	}
	/// <summary>
	/// 渲染单个渲染物体
	/// </summary>
	void TraceZLDemoApp::DrawRenderItems(ID3D12GraphicsCommandList * cmdList, const std::vector<RenderItem*>& ritems)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		auto objectCB = mCurrFrameResource->ObjectCB->Resource();

		// For each render item...
		for (size_t i = 0; i < ritems.size(); ++i)
		{
			auto ri = ritems[0];

			cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;

			//cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

			auto passCB = (mCurrFrameResource)->SkyCB->Resource();
			mCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());


			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
	}
	/// <summary>
	/// 返回六种采样方式，在HLSL中运用，可以自己设定几种
	/// </summary>
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> TraceZLDemoApp::GetStaticSamplers()
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
