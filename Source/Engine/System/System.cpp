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
//	関数
//===============================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void System_Initialize();
void System_Finalize();
void Device_Update();
void Debug_Render();

//===============================================
//	グローバル変数
//===============================================
static HWND					g_hWnd = {};						//ハンドルの設定
static HINSTANCE			g_hInstance = {};
static LPDIRECT3D9			g_pd3d9 = NULL;						//ポインタ
static LPDIRECT3DDEVICE9	g_pD3DDevice = NULL;				//デバイス
static MSG					g_Msg = {};							//メッセージ

//===============================================
//	ウィンドウ処理
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

	//システムへの登録
	RegisterClass(&MyWindow_01);
	
	//ウィンドウサイズ
	RECT Window_Rect = { 0,0,WINDOWSCREEN_WIDTH,WINDOWSCREEN_HEIGHT };

	AdjustWindowRect(&Window_Rect, WINDOW_STYLE, FALSE);

	int WinWidth = Window_Rect.right - Window_Rect.left;
	int WinHeight = Window_Rect.bottom - Window_Rect.top;

	//max(a,b);	a,bを比較して、大きい方を返す。
	int WinPosX = max((GetSystemMetrics(SM_CXSCREEN) - WinWidth) / 2, 0);
	int WinPosY = max((GetSystemMetrics(SM_CYSCREEN) - WinHeight) / 2, 0);

	g_hWnd = CreateWindow(CLASS_NAME, WINDOW_CAPTION, WINDOW_STYLE, WinPosX, WinPosY, WinWidth, WinHeight, NULL, NULL, hInstance, NULL);

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	g_hInstance = hInstance;
	
	g_Msg = {};				//Message

	System_Initialize();	//初期化

	//------------------------------------
	//	メインループ
	//------------------------------------
	Main_Initialize();

	do
	{
		//メッセージが届いた処理
		if (PeekMessage(&g_Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&g_Msg);
			DispatchMessage(&g_Msg);
		}
		else
		{
			
	//--------------------------
	//	前更新
	//--------------------------
			Device_Update();

			Main_UpdateBegin();

	//--------------------------
	//	描画
	//--------------------------

			g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, BG_COLOR, 1.0f, 0);
			g_pD3DDevice->BeginScene();

			Main_Render();

	//--------------------------
	//	デバッグ　描画
	//--------------------------
			Debug_Render();	//デバッグ描画

			g_pD3DDevice->EndScene();
			g_pD3DDevice->Present(NULL, NULL, NULL, NULL);

	//--------------------------
	//	後更新
	//--------------------------
			Main_UpdateEnd();
			
		}

	}while (g_Msg.message != WM_QUIT);

	Main_Finalize();		//ゲームの終了処理

	System_Finalize();		//終了処理

	return (int)g_Msg.wParam;
}

//===============================================
//	プロシージャー処理
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
		if (MessageBox(hWnd, "本当に終了してもよろしいですか？", "確認", MB_OKCANCEL || MB_DEFBUTTON2) == IDOK) {
			DestroyWindow(hWnd);
		}

		return 0;

	case WM_DESTROY:
		//WM_QUIT　というメッセージを送る
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//===============================================
//	初期化
//===============================================
void System_Initialize()
{
	//--------------------------------------------------------
	//	デバイスの設定
	//--------------------------------------------------------
	g_pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (g_pd3d9 == NULL) 
	{
		return;
	}

	D3DPRESENT_PARAMETERS	d3dpp = {};							//デバイスの動作を設定する構造体
	d3dpp.BackBufferWidth = WINDOWSCREEN_WIDTH;
	d3dpp.BackBufferHeight = WINDOWSCREEN_HEIGHT;
	d3dpp.BackBufferCount = 1;									//バックバッファーの枚数
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;					//使えるカラーの設定
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;					//表と裏の画面切り替えを行う処理の仕方設定
	d3dpp.Windowed = TRUE;										//ウィンドウ:TRUE	フルスクリーン:FALSE 
	d3dpp.EnableAutoDepthStencil = TRUE;						//ステンシルバッファの有無	FALSEにすると表画面と裏画面が混合する
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;					//ステンシルバッファのデータ量
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;	//垂直同期を待つタイミング

	HRESULT hr = g_pd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDevice);

	if (FAILED(hr)) {
		MessageBox(g_hWnd, "デバイスの取得に失敗しました。", "エラー", MB_OK);
		DestroyWindow(g_hWnd);
		return;
	}

	//------------------------------------
	//	描画設定 ライティング
	//------------------------------------
	NLighting::Initialize();

	//------------------------------------
	//	入力処理　	初期化
	//------------------------------------
	Input_Initalize(g_hWnd,g_hInstance);

	//------------------------------------
	//	タイマー	初期化
	//------------------------------------
	SystemTimer_Initialize();

	//------------------------------------
	//	サウンド　	初期化
	//------------------------------------
	InitSound(g_hWnd);

	//------------------------------------
	//	テクスチャ　初期化
	//------------------------------------
	NTexture::Initialize();

	//------------------------------------
	//	スプライト　初期化
	//------------------------------------
	Sprite_SetDevice(g_pD3DDevice);

	DebugFont_Initialize();
}

//===============================================
//	デバイス　更新処理
//===============================================
void Device_Update()
{
	Input_Update(g_hWnd);

	BringWindowToTop(g_hWnd);
	//カーソルループ
	if (!Keyboard_IsPress(DIK_TAB))
	{
		POINT MousePos;							//マウスのXY座標
		GetCursorPos(&MousePos);				
		WINDOWINFO WinInfo;						//ウィンドウ情報
		GetWindowInfo(g_hWnd, &WinInfo);

		//左
		if ((UINT)MousePos.x <= WinInfo.rcWindow.left + WinInfo.cxWindowBorders)
		{
			SetCursorPos(WinInfo.rcWindow.right - WinInfo.cxWindowBorders, MousePos.y);
		}
		//右
		else if((UINT)MousePos.x >= WinInfo.rcWindow.right - WinInfo.cxWindowBorders)
		{
			SetCursorPos(WinInfo.rcWindow.left + WinInfo.cxWindowBorders, MousePos.y);
		}

		//上
		if((UINT)MousePos.y <= WinInfo.rcWindow.top + WinInfo.cyWindowBorders)
		{
			SetCursorPos( MousePos.x, WinInfo.rcWindow.bottom - WinInfo.cyWindowBorders);
		}
		//下
		else if((UINT)MousePos.y >= WinInfo.rcWindow.bottom - WinInfo.cyWindowBorders)
		{
			SetCursorPos( MousePos.x, WinInfo.rcWindow.top + WinInfo.cyWindowBorders);
		}
	}

}

//===============================================
//	デバッグ描画
//===============================================
void Debug_Render()
{

}

//===============================================
//	終了処理
//===============================================
void System_Finalize()
{
	DebugFont_Finalize();

	//------------------------------------
	//	テクスチャ		終了処理
	//------------------------------------
	NTexture::Finalize();
	
	//------------------------------------
	//	サウンド	　	終了処理
	//------------------------------------
	UninitSound();

	//------------------------------------
	//	入力　			終了処理
	//------------------------------------
	Input_Finalize();

	//------------------------------------
	//	デバイスの破棄
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
//	HWNDの取得
//===============================================
HWND *System_GethWnd()
{
	return &g_hWnd;
}

//===============================================
//	インスタンスの取得
//===============================================
HINSTANCE *System_GethInstance() 
{
	return &g_hInstance;
}

//===============================================
//	デバイスの取得
//===============================================
LPDIRECT3DDEVICE9 System_GetDevice(void)
{
	return g_pD3DDevice;
}
