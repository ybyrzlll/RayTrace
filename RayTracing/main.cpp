#include <windows.h>
#include <tchar.h>
#include "mesh.h"
#include "objParser.h"
#include "mesh.h"
#include "Camera.h"

//=====================================================================
// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char* screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR* title);	// 屏幕初始化
int screen_close(void);								// 关闭屏幕
void screen_dispatch(void);							// 处理消息
void screen_update(void);							// 显示 FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口并设置标题
int screen_init(int w, int h, const TCHAR* title) {
	//系统支持的结构，用来存储某一类窗口的信息，如ClassStyle，消息处理函数等
	//一个WNDCLASS可以对应多个窗口对象
	//WNDPROC： 
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	//窗口背景为黑色
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//获取当前应用程序或DLL文件的模块句柄（当前进程空间的装入地址，即进程地址空间中可执行文件的基址）
	wc.hInstance = GetModuleHandle(NULL);
	//窗口采用箭头光标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;//在系统注册某一类型的窗体，将WNDCLASS数据注册为一个窗口类

	//CreateWindow将WNDCLASS定义的窗体变成实例
	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	//创建一个与指定设备兼容的内存设备上下文环境
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	//创建一个与设备无关的位图
	//HDC hdc 设备环境句柄。如果iUsage的值是DIB_PAL_COLORS，那么函数使用该设备环境的逻辑调色板对与设备无关位图的颜色进行初始化。
	//CONST BITMAPINFO * pbmi：指向BITMAPINFO结构的指针，该结构指定了设备无关位图的各种属性，其中包括位图的尺寸和颜色。
	//UINT iUsage：指定由pbmi参数指定的BITMAPINFO结构中的成员bmiColors数组包含的数据类型（要么是逻辑调色板索引值，要么是原文的RGB值）。
	//VOID * ppvBits指向一个变量的指针，该变量接收一个指向DIB位数据值的指针
	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	//选择一对象到指定的设备上下文环境中，该新对象替换先前的相同类型的对象。
	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		//GetMessage()函数是从调用线程的消息队列中取出一条消息；对于每一个应用程序窗口线程，操作系统都会
		//为其建立一个消息队列，当我们的窗口有消息时（即所有与这个窗口线程相关的消息），操纵系统会把这个
		//消息放到该线程的消息队列当中，我们的窗口程序就通过这个
		//GetMessage()函数从自己的消息队列中取出一条一条具体的消息并进行响应操作。
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		//DispatchMessage()函数是将我们取出的消息传到窗口的回调函数去处理；可以理解为
		//该函数将取出的消息路由给操作系统，然后操作系统去调用我们的窗口回调函数对这个消息进行处理。
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);//将一幅位图从screen_dc设备场景复制到另一个hDC
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}

int main(void)
{
	Mesh cuboid, plane;
	buildMeshFromFile(cuboid, "Mesh/cuboid.obj");
	buildMeshFromFile(plane, "Mesh/plane.obj");
	cuboid.buildFacetNormal();
	plane.buildFacetNormal();

	int window_width = 800, window_height = 600;
	//初始化窗口并设置标题
	char text[] = _T("YeahBin (software render ) - ");
	TCHAR* title = text;

	if (screen_init(window_width, window_height, title))//产生了外部缓存
		return -1;

	//设置主相机
	Camera camera;
	camera.pos = { 5, 2.5, 5 };
	camera.vpn = { -5, -2.5, -5 };
	camera.vpn.normalized();
	camera.up = { 0, 1, 0 };
	camera.nearZ = 5;
	camera.laterialAngle = 0.2 * pi;
	camera.verticalAngle = 0.1 * pi;

	//渲染目标纹理
	unsigned int** framebuffer = nullptr;

	for (int j = 0; j < window_height; j++) {
		framebuffer[j] = new unsigned int(window_height);
		for (int i = 0; i < window_width; i++)
		{
			/*float sum = ((int)122 << 16) + ((int)122 << 8) + 122;
			screen_fb[i * window_width * 4 + j * 4] = sum;*/
			framebuffer[j] = (unsigned int*)screen_fb[j * window_width * 4];
		}
	}

	

	while (1) {

		for (int j = 200; j < 400; j++) {
			for (int i = 0; i < 200; i++)
			{
				framebuffer[j][i] = ((int)122 << 16) + ((int)122 << 8) + 122;
			}
		}

		screen_update();
		Sleep(1);
	}

	return 0;
}