#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#include <vector>
#endif // _DEBUG

// �w�b�_�[�C���N���[�h�ƃ��C�u����
#include<d3d12.h>
#include<dxgi1_6.h>

// ���C�u�����ƃ����N
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format �t�H�[�}�b�g�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p

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
	// �E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); // OS�ɑ΂��ăA�v���I����������
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // ���̏������s��
}


const unsigned int window_width = 1280;
const unsigned int window_height = 720;

// �e�I�u�W�F�N�g��p��
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

	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};
	
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;			// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");					// �A�v���P�[�V�����N���X��(�K���ł悢)
	w.hInstance = GetModuleHandle(nullptr);				// �n���h���̎擾
	
	// �E�B���h�E�N���X�̎w���OS�ɓ`����
	RegisterClassEx(&w);								// �A�v���P�[�V�����N���X

	RECT wrc = { 0, 0, window_width, window_height };	// �E�B���h�E�T�C�Y�����߂�

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�I�u�W�F�N�g�̍쐬
	HWND hwnd = CreateWindow(
		w.lpszClassName,
		_T("DX12�e�X�g"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,			// �\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,			// �\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,	// �E�B���h�E��
		wrc.bottom - wrc.top,	// �E�B���h�E��
		nullptr,				// �e�E�B���h�E�n���h��
		nullptr,				// ���j���[�n���h��
		w.hInstance,			// �Ăяo���A�v���P�[�V�����n���h��
		nullptr					// �ǉ��p�����[�^
	);

	// DX����̏�����
	// �K�؂ȃ��x���łȂ��ƁA��肭�g�p�ł��Ȃ�
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	// �t���[�`���[���x���̑I��
	for (auto lv : levels)
	{
		// �f�o�C�X�̍쐬
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			// �����\�ȃo�[�W���������������烋�[�v��ł��؂�
			featureLevel = lv;
			
			break;
		}
	}
	
	// DX�֐��̌��ʂ�HRESULT�ŕԂ��Ă���

	// �t�@�N�g���[�̍쐬
	if (CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)) != S_OK)
	{
		return -1;
	}

	// ���p�\�ȃA�_�v�^�[��񋓂��A�A�_�v�^�[��o�^
	// �A�_�v�^�[�̗񋓗p
	std::vector<IDXGIAdapter*> adapters;

	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0; 
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}
	
	// DXGI_ADAPTER_DESC ... Description�����o�[�ϐ�������A�A�_�v�^�[�̖��O���i�[����Ă���
	// �A�_�v�^�[�����ʂ���ׂ̏������[�v�Ŏ擾
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);			// �A�_�v�^�[�̐����I�u�W�F�N�g�擾

		std::wstring strDesc = adesc.Description;

		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	// �E�B���h�E�̕`��
	ShowWindow(hwnd,SW_SHOW);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// �A�v���P�[�V�������I���Ƃ��� message �� WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	// �����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
}
