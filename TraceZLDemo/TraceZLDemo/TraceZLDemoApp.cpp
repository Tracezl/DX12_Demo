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
/// ��ں���
/// </summary>
/// <param name="hInstance"> </param>
/// <returns></returns>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)//WINDOS��ں���
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		TraceZLDemoApp theApp(hInstance);
		if (!theApp.Initialize())//����Initialize�������г�ʼ��
			return 0;

		return theApp.Run();//������ó�ʼ�������ɹ���������Run()������������Run()�ķ���ֵ
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);//�������󵯳���OK��ť����Ϣ��
		return 0;
	}
}
	/// <summary>
	/// ���캯��
	/// </summary>
	TraceZLDemoApp::TraceZLDemoApp(HINSTANCE hInstance)
		:D3DApp(hInstance)
	{
	}
	/// <summary>
	/// ��������
	/// </summary>
	TraceZLDemoApp::~TraceZLDemoApp()
	{
	}
	/// <summary>
	/// �������г�ʼ��
	/// </summary>
	///<returns>�����Ƿ�ɹ���ʼ��</returns>
	bool TraceZLDemoApp::Initialize()//��дInitialize()����麯��
	{
		if (!D3DApp::Initialize())//���ø����Initalize����
			return false;

		return true;
	}
	/// <summary>
	/// ���ڴ��ڴ�С�ǵ���
	/// </summary>
	void TraceZLDemoApp::OnResize()//��д�麯����
	{
		D3DApp::OnResize();//���ø���ĺ���
	}
	/// <summary>
	/// ÿ֡����һ�θú���������������
	/// </summary>
	/// <param name="gt"> ����ʱ�����</param>
	void TraceZLDemoApp::Update(const GameTimer& gt)//��д�麯����
	{

	}
	/// <summary>
	/// ÿ֡����һ�θú�����ʹ��GPU��Ⱦ
	/// </summary>
	/// <param name="gt"> ����ʱ�����</param>
	void TraceZLDemoApp::Draw(const GameTimer& gt)//��д�麯����
	{
		ThrowIfFailed(mDirectCmdListAlloc->Reset());//??

		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));//��ʼ���������б�
		//����GPU��CPUͬ�����ϰ��㣿
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// �����ӿڣ���ʾ����Ļ�Ĵ�С��λ�ã�
		mCommandList->RSSetViewports(1, &mScreenViewport);
		//���ü��ÿռ�ת����������ͼ��С
		mCommandList->RSSetScissorRects(1, &mScissorRect);


		// �����Ⱥͺ�̨������
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// ָ��Ҫ��Ⱦ�Ļ�����
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		// ��ʾ����Դʹ��״̬ת����
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// �ر������б�
		ThrowIfFailed(mCommandList->Close());

		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// �Ѻ�̨�������ķ�ת��ǰ̨
		ThrowIfFailed(mSwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

		
		FlushCommandQueue();
	}

