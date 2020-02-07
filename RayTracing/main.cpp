#include <windows.h>
#include <tchar.h>

#include "objParser.h"
#include "debugTool.h"

#include <iostream>
#include "mMath.h"
#include "Obj.h"
#include "Intersection.h"
#include "mesh.h"
#include "light.h"
#include "Shader.h"
#include "Trace.h"
#include "Global.h"
#include "matrix.h"
#include "Camera.h"
using namespace std;
using namespace Trace;

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
	//32位图，多了个透明通道
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
	//初始化光源
	Light light_a;
	light_a.pos = { 2, 5, 2 };
	light_a.color = { 1, 1, 1 };
	lights.push_back(&light_a);


	//初始化模型
		//初始化材质
		Matarial mt_cuboid, mt_plane;
		mt_cuboid.color = { 255, 0, 255 };
		mt_cuboid.reflFactor = 0.01;
		mt_plane.color = { 100, 100, 100 };
		mt_plane.reflFactor = 0.7;

		//初始化网格
		Mesh mesh_cuboid, mesh_plane;
		buildMeshFromFile(mesh_cuboid, "Mesh/cuboid.obj");
		buildMeshFromFile(mesh_plane, "Mesh/plane.obj");
		mesh_cuboid.buildFacet();
		mesh_plane.buildFacet();

		//使用的shader
		Lambert lambert;

		Obj cuboid, plane;
		cuboid.mesh = &mesh_cuboid;
		cuboid.matarial = &mt_cuboid;
		cuboid.shader = &lambert;
		
		plane.mesh = &mesh_plane;
		plane.matarial = &mt_plane;
		plane.shader = &lambert;

		objs.push_back(&cuboid);
		objs.push_back(&plane);


	int window_width = 400, window_height = 300;
	//初始化窗口并设置标题
	char text[] = _T("YeahBin (software render ) - ");
	TCHAR* title = text;

	if (screen_init(window_width, window_height, title))//产生了外部缓存
		return -1;

	//设置主相机
	Camera camera;

	camera.pos = { 3, 3, 3 };
	camera.vpn = -camera.pos;

	/*camera.pos = { -0.4, 1.7, 1.24 };
	camera.vpn = { 1.4, 3.3, 2.2 };*/

	camera.vpn.normalized();
	camera.up = { 0, 1, 0 };
	camera.up.normalized();
	camera.nearZ = 1;
	camera.laterialAngle = 0.25 * pi;
	camera.verticalAngle = 0.25 * pi;

	int** test = new int* [4];
	test[1] = new int[5];
	test[1][0] = 1;
	test[1][1] = 2;

	//建立程序的fb--渲染目标纹理
	UINT32** framebuffer = new UINT32 * [window_height];

	char* framebuf = nullptr;
	if (screen_fb != NULL) framebuf = (char*)screen_fb;
	for (int j = 0; j < window_height; j++) {
		framebuffer[j] = new UINT32[window_width];
		for (int i = 0; i < window_width; i++)
		{
			framebuffer[j] = (UINT32*)(framebuf + j * window_width * 4);
		}
	}

	DWORD t_start;

	while (1) {

		t_start = GetTickCount64();

		float half_h = tan(camera.verticalAngle) * camera.nearZ;
		float half_w = tan(camera.laterialAngle) * camera.nearZ;

		Vector3f rightDir = (camera.up.crossProduct(camera.vpn)).normalized();
		Vector3f upDir = (camera.vpn.crossProduct(rightDir)).normalized();
		Vector3f pos_begin = camera.pos + camera.vpn.normalized() * camera.nearZ
			+ upDir * half_h - rightDir * half_w;
		Vector3f rightFactor = rightDir * half_w * 2;
		Vector3f downFactor = upDir * half_h * 2;

		Ray ray;
		ray.pos = camera.pos;

		for (int j = 0; j < window_height; j++) {
			for (int i = 0; i < window_width; i++)
			{
				ray.dir = pos_begin + rightFactor * ((float)i / (float)window_width)
					- downFactor * ((float)j / (float)window_height) - ray.pos;

				ray.dir.normalized();
				
				//todo: foreach light 微偏移
				framebuffer[j][i] = v3f_2_UINT32(castRay(ray, 0));//((int)122 << 16) + ((int)122 << 8) + 122;
			}
		}

		Vector3f camera_z = camera.vpn.crossProduct(camera.up);
		camera_z.normalized();
		camera_z *= Camera_Speed;

		if (screen_keys[0x41]) camera.pos = camera.pos + camera_z;//a
		if (screen_keys[0x44]) camera.pos = camera.pos - camera_z;//d

		//camera.pos.x += 0.1f;//d
		if (screen_keys[0x57]) camera.pos = camera.pos + camera.vpn;//w
		if (screen_keys[0x53]) camera.pos = camera.pos - camera.vpn;//s

		float thetaY = 0, thetaZ = 0;
		if (screen_keys[VK_LEFT]) thetaY = -Camera_RotateSpeed;
		if (screen_keys[VK_RIGHT]) thetaY = Camera_RotateSpeed;
		if (screen_keys[VK_UP]) thetaZ = -Camera_RotateSpeed;
		if (screen_keys[VK_DOWN]) thetaZ = Camera_RotateSpeed;

		//乘以四元数旋转矩阵
		Matrix4 m1, m2;
		matrix_set_rotate(&m1, 0, 1, 0, thetaY);
		if (camera.vpn.x > 0) matrix_set_rotate(&m2, 0, 0, -1, thetaZ);
		else  matrix_set_rotate(&m2, 0, 0, 1, thetaZ);
		camera.vpn = matrix_apply(m1, camera.vpn);
		camera.vpn = matrix_apply(m2, camera.vpn);

		screen_update();

		float shrub = (float)10000 / (GetTickCount64() - t_start);
		cout << "帧数：" << shrub << " ----------------------------------------"<<endl;

		Sleep(1);
	}

	return 0;
}

