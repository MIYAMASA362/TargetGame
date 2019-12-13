//
//	System.cpp
//		Author:HIROMASA IKEDA	DATE:2018/09/13
//===============================================
#include<Windows.h>
#include<d3dx9.h>
#include"System.h"
#include"input.h"
#include"Sprite.h"
#include"Texture.h"
#include"system_timer.h"
#include"sound.h"
#include"Lighting.h"
#include"Debug_font.h"

//===============================================
//	�֐�
//===============================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void System_Initialize();
void System_Finalize();
void Device_Update();
void Debug_Render();

//===============================================
//	�O���[�o���ϐ�
//===============================================
static HWND					g_hWnd = {};						//�n���h���̐ݒ�
static HINSTANCE			g_hInstance = {};
static LPDIRECT3D9			g_pd3d9 = NULL;						//�|�C���^
static LPDIRECT3DDEVICE9	g_pD3DDevice = NULL;				//�f�o�C�X
static MSG					g_Msg = {};							//���b�Z�[�W

//===============================================
//	�E�B���h�E����
//===============================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASS	MyWindow_01 = {};
	MyWindow_01.lpfnWndProc = WndProc;
	MyWindow_01.lpszClassName = CLASS_NAME;
	MyWindow_01.hInstance = hInstance;
	MyWindow_01.hCursor = LoadCursor(NULL, IDC_ARROW);
	MyWindow_01.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);

	//�V�X�e���ւ̓o�^
	RegisterClass(&MyWindow_01);
	
	//�E�B���h�E�T�C�Y
	RECT Window_Rect = { 0,0,WINDOWSCREEN_WIDTH,WINDOWSCREEN_HEIGHT };

	AdjustWindowRect(&Window_Rect, WINDOW_STYLE, FALSE);

	int WinWidth = Window_Rect.right - Window_Rect.left;
	int WinHeight = Window_Rect.bottom - Window_Rect.top;

	//max(a,b);	a,b���r���āA�傫������Ԃ��B
	int WinPosX = max((GetSystemMetrics(SM_CXSCREEN) - WinWidth) / 2, 0);
	int WinPosY = max((GetSystemMetrics(SM_CYSCREEN) - WinHeight) / 2, 0);

	g_hWnd = CreateWindow(CLASS_NAME, WINDOW_CAPTION, WINDOW_STYLE, WinPosX, WinPosY, WinWidth, WinHeight, NULL, NULL, hInstance, NULL);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	g_hInstance = hInstance;
	
	g_Msg = {};				//Message

	System_Initialize();	//������

	//------------------------------------
	//	���C�����[�v
	//------------------------------------
	Main_Initialize();

	do
	{
		//���b�Z�[�W���͂�������
		if (PeekMessage(&g_Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&g_Msg);
			DispatchMessage(&g_Msg);
		}
		else
		{
			
	//--------------------------
	//	�O�X�V
	//--------------------------
			Device_Update();

			Main_UpdateBegin();

	//--------------------------
	//	�`��
	//--------------------------

			g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, BG_COLOR, 1.0f, 0);
			g_pD3DDevice->BeginScene();

			Main_Render();

	//--------------------------
	//	�f�o�b�O�@�`��
	//--------------------------
			Debug_Render();	//�f�o�b�O�`��

			g_pD3DDevice->EndScene();
			g_pD3DDevice->Present(NULL, NULL, NULL, NULL);

	//--------------------------
	//	��X�V
	//--------------------------
			Main_UpdateEnd();
			
		}

	}while (g_Msg.message != WM_QUIT);

	Main_Finalize();		//�Q�[���̏I������

	System_Finalize();		//�I������

	return (int)g_Msg.wParam;
}

//===============================================
//	�v���V�[�W���[����
//===============================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, "�{���ɏI�����Ă���낵���ł����H", "�m�F", MB_OKCANCEL || MB_DEFBUTTON2) == IDOK) {
			DestroyWindow(hWnd);
		}

		return 0;

	case WM_DESTROY:
		//WM_QUIT�@�Ƃ������b�Z�[�W�𑗂�
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//===============================================
//	������
//===============================================
void System_Initialize()
{
	//--------------------------------------------------------
	//	�f�o�C�X�̐ݒ�
	//--------------------------------------------------------
	g_pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (g_pd3d9 == NULL) 
	{
		return;
	}

	D3DPRESENT_PARAMETERS	d3dpp = {};							//�f�o�C�X�̓����ݒ肷��\����
	d3dpp.BackBufferWidth = WINDOWSCREEN_WIDTH;
	d3dpp.BackBufferHeight = WINDOWSCREEN_HEIGHT;
	d3dpp.BackBufferCount = 1;									//�o�b�N�o�b�t�@�[�̖���
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;					//�g����J���[�̐ݒ�
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;					//�\�Ɨ��̉�ʐ؂�ւ����s�������̎d���ݒ�
	d3dpp.Windowed = TRUE;										//�E�B���h�E:TRUE	�t���X�N���[��:FALSE 
	d3dpp.EnableAutoDepthStencil = TRUE;						//�X�e���V���o�b�t�@�̗L��	FALSE�ɂ���ƕ\��ʂƗ���ʂ���������
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;					//�X�e���V���o�b�t�@�̃f�[�^��
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;	//����������҂^�C�~���O

	HRESULT hr = g_pd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDevice);

	if (FAILED(hr)) {
		MessageBox(g_hWnd, "�f�o�C�X�̎擾�Ɏ��s���܂����B", "�G���[", MB_OK);
		DestroyWindow(g_hWnd);
		return;
	}

	//------------------------------------
	//	�`��ݒ� ���C�e�B���O
	//------------------------------------
	NLighting::Initialize();

	//------------------------------------
	//	���͏����@	������
	//------------------------------------
	Input_Initalize(g_hWnd,g_hInstance);

	//------------------------------------
	//	�^�C�}�[	������
	//------------------------------------
	SystemTimer_Initialize();

	//------------------------------------
	//	�T�E���h�@	������
	//------------------------------------
	InitSound(g_hWnd);

	//------------------------------------
	//	�e�N�X�`���@������
	//------------------------------------
	NTexture::Initialize();

	//------------------------------------
	//	�X�v���C�g�@������
	//------------------------------------
	Sprite_SetDevice(g_pD3DDevice);

	DebugFont_Initialize();
}

//===============================================
//	�f�o�C�X�@�X�V����
//===============================================
void Device_Update()
{
	Input_Update(g_hWnd);

	BringWindowToTop(g_hWnd);
	//�J�[�\�����[�v
	if (!Keyboard_IsPress(DIK_TAB))
	{
		POINT MousePos;							//�}�E�X��XY���W
		GetCursorPos(&MousePos);				
		WINDOWINFO WinInfo;						//�E�B���h�E���
		GetWindowInfo(g_hWnd, &WinInfo);

		//��
		if ((UINT)MousePos.x <= WinInfo.rcWindow.left + WinInfo.cxWindowBorders)
		{
			SetCursorPos(WinInfo.rcWindow.right - WinInfo.cxWindowBorders, MousePos.y);
		}
		//�E
		else if((UINT)MousePos.x >= WinInfo.rcWindow.right - WinInfo.cxWindowBorders)
		{
			SetCursorPos(WinInfo.rcWindow.left + WinInfo.cxWindowBorders, MousePos.y);
		}

		//��
		if((UINT)MousePos.y <= WinInfo.rcWindow.top + WinInfo.cyWindowBorders)
		{
			SetCursorPos( MousePos.x, WinInfo.rcWindow.bottom - WinInfo.cyWindowBorders);
		}
		//��
		else if((UINT)MousePos.y >= WinInfo.rcWindow.bottom - WinInfo.cyWindowBorders)
		{
			SetCursorPos( MousePos.x, WinInfo.rcWindow.top + WinInfo.cyWindowBorders);
		}
	}

}

//===============================================
//	�f�o�b�O�`��
//===============================================
void Debug_Render()
{

}

//===============================================
//	�I������
//===============================================
void System_Finalize()
{
	DebugFont_Finalize();

	//------------------------------------
	//	�e�N�X�`��		�I������
	//------------------------------------
	NTexture::Finalize();
	
	//------------------------------------
	//	�T�E���h	�@	�I������
	//------------------------------------
	UninitSound();

	//------------------------------------
	//	���́@			�I������
	//------------------------------------
	Input_Finalize();

	//------------------------------------
	//	�f�o�C�X�̔j��
	//------------------------------------
	if (g_pD3DDevice != NULL)
	{
		g_pD3DDevice->Release();
		g_pD3DDevice = NULL;
	}

	if (g_pd3d9 != NULL)
	{
		g_pd3d9->Release();
		g_pd3d9 = NULL;
	}
}

//===============================================
//	HWND�̎擾
//===============================================
HWND *System_GethWnd()
{
	return &g_hWnd;
}

//===============================================
//	�C���X�^���X�̎擾
//===============================================
HINSTANCE *System_GethInstance() 
{
	return &g_hInstance;
}

//===============================================
//	�f�o�C�X�̎擾
//===============================================
LPDIRECT3DDEVICE9 System_GetDevice(void)
{
	return g_pD3DDevice;
}