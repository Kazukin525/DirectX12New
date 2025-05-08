#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#include <vector>
#endif // _DEBUG

// ヘッダーインクルードとライブラリ
#include<d3d12.h>
#include<dxgi1_6.h>

// ライブラリとリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット（%dとか%fとかの）
// @param 可変長引数
// @remarks この関数はデバッグ用

void DebugOutPutFormatString(const char* format,...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); // OSに対してアプリ終了を教える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // 基底の処理を行う
}


const unsigned int window_width = 1280;
const unsigned int window_height = 720;

// 各オブジェクトを用意
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapChain = nullptr;

#ifdef _DEBUG
int main()
{
#else // _DEBUG
int WINAPI WinMain(HINSTANCE, HINSTANCE,
	LPSTR, int)
{
#endif
	//DebugOutPutFormatString("Show window test");
	//getchar();
	//return 0;

	// ウィンドウクラスの生成＆登録
	WNDCLASSEX w = {};
	
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;			// コールバック関数の指定
	w.lpszClassName = _T("DX12Sample");					// アプリケーションクラス名(適当でよい)
	w.hInstance = GetModuleHandle(nullptr);				// ハンドルの取得
	
	// ウィンドウクラスの指定をOSに伝える
	RegisterClassEx(&w);								// アプリケーションクラス

	RECT wrc = { 0, 0, window_width, window_height };	// ウィンドウサイズを決める

	// 関数を使ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウオブジェクトの作成
	HWND hwnd = CreateWindow(
		w.lpszClassName,
		_T("DX12テスト"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,			// 表示x座標はOSにお任せ
		CW_USEDEFAULT,			// 表示y座標はOSにお任せ
		wrc.right - wrc.left,	// ウィンドウ幅
		wrc.bottom - wrc.top,	// ウィンドウ高
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		w.hInstance,			// 呼び出しアプリケーションハンドル
		nullptr					// 追加パラメータ
	);

	// DX周りの初期化
	// 適切なレベルでないと、上手く使用できない
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	// フューチャーレベルの選択
	for (auto lv : levels)
	{
		// デバイスの作成
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			// 生成可能なバージョンが見つかったらループを打ち切り
			featureLevel = lv;
			
			break;
		}
	}
	
	// DX関数の結果はHRESULTで返ってくる

	// ファクトリーの作成
	if (CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)) != S_OK)
	{
		return -1;
	}

	// 利用可能なアダプターを列挙し、アダプターを登録
	// アダプターの列挙用
	std::vector<IDXGIAdapter*> adapters;

	// ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0; 
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}
	
	// DXGI_ADAPTER_DESC ... Descriptionメンバー変数があり、アダプターの名前が格納されている
	// アダプターを識別する為の情報をループで取得
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);			// アダプターの説明オブジェクト取得

		std::wstring strDesc = adesc.Description;

		// 探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	// ウィンドウの描画
	ShowWindow(hwnd,SW_SHOW);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// アプリケーションが終わるときに message が WM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	// もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
}
