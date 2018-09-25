#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <gl/gl.h>

using namespace std;

int _w_width=600;
int _w_height=400;

enum types
{
    type_linethickness=1,
    type_col,
    type_dot
};

struct draw_event
{
    draw_event()
    {
        type=0;
        val1=val2=val3=0;
    }
    draw_event(int _type, float _val1)
    {
        type=_type;
        val1=_val1;
        val2=0;
        val3=0;
    }
    draw_event(int _type, float _val1, float _val2)
    {
        type=_type;
        val1=_val1;
        val2=_val2;
        val3=0;
    }
    draw_event(int _type, float _val1, float _val2, float _val3)
    {
        type=_type;
        val1=_val1;
        val2=_val2;
        val3=_val3;
    }

    int type;
    float val1,val2,val3;
};

float scale_x=1;
float scale_y=1;
vector<draw_event> g_vec_events;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

bool init();
bool draw();

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "VectorDraw";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if(!init())
    {
        return 0;
    }


    if (!RegisterClassEx(&wcex))
        return 0;

    hwnd = CreateWindowEx(0,
                          "VectorDraw",
                          "VectorDraw",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          _w_width,
                          _w_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            draw();
            SwapBuffers(hDC);
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);

    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,_w_width,_w_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,_w_width,_w_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /*//Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);*/
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

bool init()
{
    //read input data
    ifstream input_file("input.txt");
    if(input_file==0)
    {
        return false;
    }

    string line,word;
    while(getline(input_file,line))
    {
        //extract word
        stringstream ss(line);
        ss>>word;
        if(word=="scale")
        {
            ss>>word;
            float val1=atof(word.c_str());
            ss>>word;
            float val2=atof(word.c_str());
            scale_x=val1;
            scale_y=val2;
        }
        if(word=="line")
        {
            ss>>word;
            float val1=atof(word.c_str());
            g_vec_events.push_back( draw_event(type_linethickness,val1) );
        }
        if(word=="col")
        {
            ss>>word;
            float val1=atof(word.c_str());
            ss>>word;
            float val2=atof(word.c_str());
            ss>>word;
            float val3=atof(word.c_str());
            g_vec_events.push_back( draw_event(type_col,val1,val2,val3) );
        }
        if(word=="dot")
        {
            ss>>word;
            float val1=atof(word.c_str());
            ss>>word;
            float val2=atof(word.c_str());
            g_vec_events.push_back( draw_event(type_dot,val1,val2) );
        }
    }

    //determine window size
    float x_max=-999999;
    float y_max=-999999;
    float x_min=999999;
    float y_min=999999;
    for(int i=0;i<(int)g_vec_events.size();i++)
    {
        if(g_vec_events[i].type==type_dot)
        {
            if(g_vec_events[i].val1>x_max) x_max=g_vec_events[i].val1;
            if(g_vec_events[i].val1<x_min) x_min=g_vec_events[i].val1;
            if(g_vec_events[i].val2>y_max) y_max=g_vec_events[i].val2;
            if(g_vec_events[i].val2<y_min) y_min=g_vec_events[i].val2;
        }
    }
    _w_width=int( (x_max-x_min)*scale_x );
    _w_height=int( (y_max-y_min)*scale_y );
    if(_w_width<200) _w_width=200;
    if(_w_height<200) _w_height=200;
    _w_width+=20;
    _w_height+=40;

    return true;
}

bool draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();

    glBegin(GL_LINES);
    for(int i=0;i<(int)g_vec_events.size();i++)
    {
        switch(g_vec_events[i].type)
        {
            case type_linethickness:
            {
                glEnd();
                glLineWidth((int)g_vec_events[i].val1);
                glBegin(GL_LINES);
            }break;

            case type_col:
            {
                glColor3f(g_vec_events[i].val1,g_vec_events[i].val2,g_vec_events[i].val3);
            }break;

            case type_dot:
            {
                glVertex2f(_w_width*0.5+g_vec_events[i].val1*scale_x,-_w_height*0.5+_w_height-g_vec_events[i].val2*scale_y+12);
            }break;
        }
    }
    glEnd();

    glPopMatrix();

    return true;
}

