#include <windows.h>
#include <gl/GL.h>
#include<math.h>
#include <gl/GLU.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 800
#define PI 3.1415926535898f
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

//Prototype Of WndProc() declared Globally
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Global variable declarations
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;
GLfloat gfCubeAngle = 0.0f;
GLfloat bladeRotationSpeed = 360.0f, fanSwing = -145.0f, dir = 0.02f;


GLUquadric *quadric = NULL;

//main()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function prototype
	void initialize(void);
	void display(void);
	void uninitialize(void);
	void rotateFan(void);

	//variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("RTROGL");
	bool bDone = false;

	//code
	//initializing members of struct WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;

	//Registering Class
	RegisterClassEx(&wndclass);

	//Create Window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szClassName,
		TEXT("OpenGL Fixed Function Pipeline Using Native Windowing : Fan"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0,
		0,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	//initialize
	initialize();

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Message Loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
					bDone = true;
				rotateFan();
				// render
				display();

			}
		}
	}

	uninitialize();
	return((int)msg.wParam);
}

//WndProc()
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function prototype
	void resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);

	//code
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			gbEscapeKeyIsPressed = true;
			break;
		case 0x46: //for 'f' or 'F'
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//variable declarations
	MONITORINFO mi;

	//code
	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}

	else
	{
		//code
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void initialize(void)
{
	//function prototypes
	void resize(int, int);

	//variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	//code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	//Initialization of structure 'PIXELFORMATDESCRIPTOR'
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f); // set depth buffer
	glEnable(GL_DEPTH_TEST); // enable depth testing
	glDepthFunc(GL_LEQUAL); // type of depth testing

	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{

	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_MODELVIEW); // select modelview matrix
	glLoadIdentity(); // reset current modelview matrix

	glTranslatef(0.0f, 8.5f, -24.0f);
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	glScalef(25.0f, 25.0f, 25.0f);
	glBegin(GL_QUADS);


	//right face
	glColor3f(0.25f, 0.25f, 0.25f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glColor3f(0.84f, 0.85f, 0.85f);
	glVertex3f(1.0f, 1.0f, 1.0f);


	//back face
	glColor3f(0.7f, 0.7f, 0.7f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glColor3f(0.3f, 0.3f, 0.3f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//bottom face
	glColor3f(0.3f, 0.3f, 0.3f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glMatrixMode(GL_MODELVIEW); // select modelview matrix
	glLoadIdentity(); // reset current modelview matrix
	glPushMatrix();

	// view transformation
	glTranslatef(0.0f, -1.7f, -20.0f);
	glPushMatrix();
	//rotar stand
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, -3.0f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.75f, 0.75f, 0.75f);
	gluCylinder(quadric, 0.25f, 0.25f, 6.0f, 20, 20);
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//base disk
	glPushMatrix();
	glTranslatef(0.0f, -5.0f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.75f, 0.75f, 0.75f);
	gluDisk(quadric, 0.0f, 2.0f, 20, 20);
	glPopMatrix();

	//rotar
	glPushMatrix();
	glTranslatef(0.0f, 0.5f, -1.3f);
	glRotatef((GLfloat)fanSwing, 0.0f, 1.0f, 0.0f);
	glScalef(1.0f, 1.0f, 2.0f);
	quadric = gluNewQuadric();
	glColor3f(0.8f, 0.8f, 0.8f);
	gluSphere(quadric, 0.5, 10, 10);
	glPopMatrix();


	glTranslatef(0.05f, 1.1f, 10.0f);
	glRotatef((GLfloat)fanSwing, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.05f, 0.0f, 0.0f);
	glPushMatrix();

	//1st blade	
	glRotatef(bladeRotationSpeed + 45.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.55f, 0.0f, 0.0f);
	glScalef(0.7f, 0.7f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.0, 0.74f, 1.0f);
	gluSphere(quadric, 0.5, 10, 10);
	glPopMatrix();

	//2nd blade
	glPushMatrix();
	glRotatef(bladeRotationSpeed + 135.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.55f, 0.0f, 0.0f);
	glScalef(0.7f, 0.7f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.0, 0.74f, 1.0f);
	gluSphere(quadric, 0.5, 10, 10);
	glPopMatrix();

	//3rd blade
	glPushMatrix();
	glRotatef(bladeRotationSpeed + 225.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.55f, 0.0f, 0.0f);
	glScalef(0.7f, 0.7f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.0, 0.74f, 1.0f);
	gluSphere(quadric, 0.5, 10, 10);
	glPopMatrix();

	//4th blade
	glPushMatrix();
	glRotatef(bladeRotationSpeed + 315.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.55f, 0.0f, 0.0f);
	glScalef(0.7f, 0.7f, 0.0f);
	quadric = gluNewQuadric();
	glColor3f(0.0, 0.74f, 1.0f);
	gluSphere(quadric, 0.5, 10, 10);
	glPopMatrix();

	//mesh cover
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glScalef(0.5f, 0.5f, 0.1f);
	quadric = gluNewQuadric();
	glColor3f(0.9f, 0.9f, 0.9f);
	gluSphere(quadric, 2.3, 20, 20);
	
	glPopMatrix();
	SwapBuffers(ghdc);
}

void rotateFan(void) {
	bladeRotationSpeed = (bladeRotationSpeed - 0.5f);
	if (bladeRotationSpeed <= 0.0f)
		bladeRotationSpeed = 360.0f;
	fanSwing = fanSwing + dir;

	/*if (fanSwing >= 360.0f)
		fanSwing = 0.0f;*/
	if (fanSwing <= -210.0f)
		dir = 0.01f;
	if (fanSwing >= -145.0f)
		dir = -0.01f;
	
}

void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION); // select projection matrix
	glLoadIdentity(); // reset projection matrix

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f); // calculate the aspect ratio of the view
}

void uninitialize(void)
{
	//UNINITIALIZATION CODE

	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	DestroyWindow(ghwnd);
	ghwnd = NULL;

	gluDeleteQuadric(quadric);
}
