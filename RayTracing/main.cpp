#include <windows.h>
#include <tchar.h>
#include "mesh.h"
#include "objParser.h"
#include "mesh.h"
#include "Camera.h"

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
	Mesh cuboid, plane;
	buildMeshFromFile(cuboid, "Mesh/cuboid.obj");
	buildMeshFromFile(plane, "Mesh/plane.obj");
	cuboid.buildFacetNormal();
	plane.buildFacetNormal();

	int window_width = 800, window_height = 600;
	//��ʼ�����ڲ����ñ���
	char text[] = _T("YeahBin (software render ) - ");
	TCHAR* title = text;

	if (screen_init(window_width, window_height, title))//�������ⲿ����
		return -1;

	//���������
	Camera camera;
	camera.pos = { 5, 2.5, 5 };
	camera.vpn = { -5, -2.5, -5 };
	camera.vpn.normalized();
	camera.up = { 0, 1, 0 };
	camera.nearZ = 5;
	camera.laterialAngle = 0.2 * pi;
	camera.verticalAngle = 0.1 * pi;

	//��ȾĿ������
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