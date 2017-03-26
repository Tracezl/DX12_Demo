#include "../Common/d3dApp.h"
#include <DirectXColors.h>
using namespace DirectX;
	class TraceZLDemoApp :public D3DApp
	{
	public:
		TraceZLDemoApp(HINSTANCE hInstance);
		~TraceZLDemoApp();
		virtual bool Initialize()override;

	private:
		virtual void OnResize()override;
		virtual void Update(const GameTimer& gt)override;
		virtual void Draw(const GameTimer& gt)override;
	};
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
	/// <summary>
	/// 构造函数
	/// </summary>
	TraceZLDemoApp::TraceZLDemoApp(HINSTANCE hInstance)
		:D3DApp(hInstance)
	{
	}
	/// <summary>
	/// 析构函数
	/// </summary>
	TraceZLDemoApp::~TraceZLDemoApp()
	{
	}
	/// <summary>
	/// 用来进行初始化
	/// </summary>
	///<returns>返回是否成功初始化</returns>
	bool TraceZLDemoApp::Initialize()//重写Initialize()这个虚函数
	{
		if (!D3DApp::Initialize())//调用父类的Initalize函数
			return false;

		return true;
	}
	/// <summary>
	/// 调节窗口大小是调用
	/// </summary>
	void TraceZLDemoApp::OnResize()//重写虚函数，
	{
		D3DApp::OnResize();//调用父类的函数
	}
	/// <summary>
	/// 每帧调用一次该函数用来更新数据
	/// </summary>
	/// <param name="gt"> 传入时间对象</param>
	void TraceZLDemoApp::Update(const GameTimer& gt)//重写虚函数，
	{

	}
	/// <summary>
	/// 每帧调用一次该函数，使用GPU渲染
	/// </summary>
	/// <param name="gt"> 传入时间对象</param>
	void TraceZLDemoApp::Draw(const GameTimer& gt)//重写虚函数，
	{
		ThrowIfFailed(mDirectCmdListAlloc->Reset());//??

		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));//初始化命令行列表
		//设置GPU和CPU同步的障碍点？
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// 设置视口（显示到屏幕的大小和位置）
		mCommandList->RSSetViewports(1, &mScreenViewport);
		//设置剪裁空间转换出来的贴图大小
		mCommandList->RSSetScissorRects(1, &mScissorRect);


		// 清空深度和后台缓存区
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// 指定要渲染的缓存区
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

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

		
		FlushCommandQueue();
	}

