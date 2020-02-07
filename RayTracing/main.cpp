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
// Win32 ���ڼ�ͼ�λ��ƣ�Ϊ device �ṩһ�� DibSection �� FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// ��ǰ���̰���״̬
static HWND screen_handle = NULL;		// ������ HWND
static HDC screen_dc = NULL;			// ���׵� HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// �ϵ� BITMAP
unsigned char* screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR* title);	// ��Ļ��ʼ��
int screen_close(void);								// �ر���Ļ
void screen_dispatch(void);							// ������Ϣ
void screen_update(void);							// ��ʾ FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// ��ʼ�����ڲ����ñ���
int screen_init(int w, int h, const TCHAR* title) {
	//ϵͳ֧�ֵĽṹ�������洢ĳһ�ര�ڵ���Ϣ����ClassStyle����Ϣ��������
	//һ��WNDCLASS���Զ�Ӧ������ڶ���
	//WNDPROC�� 
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	//32λͼ�����˸�͸��ͨ��
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	//���ڱ���Ϊ��ɫ
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//��ȡ��ǰӦ�ó����DLL�ļ���ģ��������ǰ���̿ռ��װ���ַ�������̵�ַ�ռ��п�ִ���ļ��Ļ�ַ��
	wc.hInstance = GetModuleHandle(NULL);
	//���ڲ��ü�ͷ���
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;//��ϵͳע��ĳһ���͵Ĵ��壬��WNDCLASS����ע��Ϊһ��������

	//CreateWindow��WNDCLASS����Ĵ�����ʵ��
	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	//����һ����ָ���豸���ݵ��ڴ��豸�����Ļ���
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	//����һ�����豸�޹ص�λͼ
	//HDC hdc �豸������������iUsage��ֵ��DIB_PAL_COLORS����ô����ʹ�ø��豸�������߼���ɫ������豸�޹�λͼ����ɫ���г�ʼ����
	//CONST BITMAPINFO * pbmi��ָ��BITMAPINFO�ṹ��ָ�룬�ýṹָ�����豸�޹�λͼ�ĸ������ԣ����а���λͼ�ĳߴ����ɫ��
	//UINT iUsage��ָ����pbmi����ָ����BITMAPINFO�ṹ�еĳ�ԱbmiColors����������������ͣ�Ҫô���߼���ɫ������ֵ��Ҫô��ԭ�ĵ�RGBֵ����
	//VOID * ppvBitsָ��һ��������ָ�룬�ñ�������һ��ָ��DIBλ����ֵ��ָ��
	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	//ѡ��һ����ָ�����豸�����Ļ����У����¶����滻��ǰ����ͬ���͵Ķ���
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
		//GetMessage()�����Ǵӵ����̵߳���Ϣ������ȡ��һ����Ϣ������ÿһ��Ӧ�ó��򴰿��̣߳�����ϵͳ����
		//Ϊ�佨��һ����Ϣ���У������ǵĴ�������Ϣʱ������������������߳���ص���Ϣ��������ϵͳ������
		//��Ϣ�ŵ����̵߳���Ϣ���е��У����ǵĴ��ڳ����ͨ�����
		//GetMessage()�������Լ�����Ϣ������ȡ��һ��һ���������Ϣ��������Ӧ������
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		//DispatchMessage()�����ǽ�����ȡ������Ϣ�������ڵĻص�����ȥ�����������Ϊ
		//�ú�����ȡ������Ϣ·�ɸ�����ϵͳ��Ȼ�����ϵͳȥ�������ǵĴ��ڻص������������Ϣ���д���
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);//��һ��λͼ��screen_dc�豸�������Ƶ���һ��hDC
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}




int main(void)
{
	//��ʼ����Դ
	Light light_a;
	light_a.pos = { 2, 5, 2 };
	light_a.color = { 1, 1, 1 };
	lights.push_back(&light_a);


	//��ʼ��ģ��
		//��ʼ������
		Matarial mt_cuboid, mt_plane;
		mt_cuboid.color = { 255, 0, 255 };
		mt_cuboid.reflFactor = 0.01;
		mt_plane.color = { 100, 100, 100 };
		mt_plane.reflFactor = 0.7;

		//��ʼ������
		Mesh mesh_cuboid, mesh_plane;
		buildMeshFromFile(mesh_cuboid, "Mesh/cuboid.obj");
		buildMeshFromFile(mesh_plane, "Mesh/plane.obj");
		mesh_cuboid.buildFacet();
		mesh_plane.buildFacet();

		//ʹ�õ�shader
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
	//��ʼ�����ڲ����ñ���
	char text[] = _T("YeahBin (software render ) - ");
	TCHAR* title = text;

	if (screen_init(window_width, window_height, title))//�������ⲿ����
		return -1;

	//���������
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

	//���������fb--��ȾĿ������
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
				
				//todo: foreach light ΢ƫ��
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

		//������Ԫ����ת����
		Matrix4 m1, m2;
		matrix_set_rotate(&m1, 0, 1, 0, thetaY);
		if (camera.vpn.x > 0) matrix_set_rotate(&m2, 0, 0, -1, thetaZ);
		else  matrix_set_rotate(&m2, 0, 0, 1, thetaZ);
		camera.vpn = matrix_apply(m1, camera.vpn);
		camera.vpn = matrix_apply(m2, camera.vpn);

		screen_update();

		float shrub = (float)10000 / (GetTickCount64() - t_start);
		cout << "֡����" << shrub << " ----------------------------------------"<<endl;

		Sleep(1);
	}

	return 0;
}

