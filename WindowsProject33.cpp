#include "framework.h"
#include <windowsx.h>
#include <Windows.h>
#include <queue>
#include <vector>
#include <map>
#include "WindowsProject33.h"

#define MAX_LOADSTRING 100



/*                                                          //
                    사용자 정의 전역 변수
*/                                                          //

#define GRID_WIDTH 100
#define GRID_HEIGHT 50

using namespace std;

char g_Tile[GRID_HEIGHT][GRID_WIDTH];




/*                                                          //
                     랜더 사용 전역 변수
*/                                                          //
HBRUSH g_hTileBrush;
HPEN g_hGridPen;

bool g_bErase = false;
bool g_bDrag = false;
bool g_bFirst = false;
bool g_bFinal = false;

bool g_bEndAStar = false;

pair<int, int> Start_Pos = { 0, 0 };
pair<int, int> End_Pos = { 0, 0 };



POINT g_Origin = { 0, 0 };
//POINT position = { 0, 0 };
//int g_MasterY = 0;
//int g_MasterX = 0;
int GRID_SIZE = 16;

HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;
HDC g_hMemDC;
RECT g_MemDCRect;



/*                                                          //
                      우아한 전방 선언
*/                                                          //


int Astar(char(*Map)[100], pair<int, int> start, pair<int, int> goal, int max_X, int max_Y, HWND hWnd);


/*                                                          //
                      우아한 변수의 경계
*/                                                          //

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //RegisterClass
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT33));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT33);
    wcex.lpszClassName = L"abcde";
    wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    hInst = hInstance;

    HWND hWnd = ::CreateWindowW(L"abcde", L"Title", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ::ShowWindow(hWnd, nCmdShow);
    ::UpdateWindow(hWnd);

    MSG msg;

    HDC hdc = GetDC(hWnd);
    while (::GetMessage(&msg, nullptr, 0, 0))
    {
        //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            //Rectangle(hdc, position.x - 5, position.y - 5, position.x + 5, position.y + 5);
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            
        }
    }

}



void RenderGrid(HDC hdc)
{
    int iX = -g_Origin.x;
    int iY = -g_Origin.y;

    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

    for (int i = 0; i <= GRID_WIDTH; i++)
    {
        MoveToEx(hdc, iX, 0, NULL);
        LineTo(hdc, iX, GRID_HEIGHT * GRID_SIZE);

        iX += GRID_SIZE;
    }

    iX = -g_Origin.x; // Reset iX for horizontal lines
    for (int i = 0; i <= GRID_HEIGHT; i++)
    {
        MoveToEx(hdc, 0, iY, NULL);
        LineTo(hdc, GRID_WIDTH * GRID_SIZE, iY);

        iY += GRID_SIZE;
    }

    SelectObject(hdc, hOldPen);
}

//void RenderObstacle(HDC hdc)
//{
//    int iX = 0;
//    int iY = 0;
//    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
//    SelectObject(hdc, GetStockObject(NULL_PEN));
//
//
//    int Render_TRUE_X = g_MasterX + g_MemDCRect.right;
//
//    for (int i = 0; i < GRID_WIDTH; i++)
//    {
//        for (int j = 0; j < GRID_HEIGHT; j++)
//        {
//            if (g_Tile[j][i])
//            {
//                iX = i * GRID_SIZE;
//                iY = j * GRID_SIZE;
//
//
//
//                //if()
//                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
//            }
//        }
//    }
//
//    SelectObject(hdc, hOldBrush);
//}

void RenderObstacle(HDC hdc)
{
    int iX = -g_Origin.x;
    int iY = -g_Origin.y;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));

    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            if (g_Tile[j][i] == 1)                      // 1은 벽의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);
            }
            else if (g_Tile[j][i] == 2)                 // 2는 출발지의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                HBRUSH TempBrush = CreateSolidBrush(RGB(0, 0, 0));
                HBRUSH TempOldBrush = (HBRUSH)SelectObject(hdc, TempBrush);
                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);

                SelectObject(hdc, TempOldBrush);

                DeleteObject(TempBrush);
            }
            else if (g_Tile[j][i] == 3)                 // 3은 목적지의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                HBRUSH TempBrush = CreateSolidBrush(RGB(0, 0, 255));
                HBRUSH TempOldBrush = (HBRUSH)SelectObject(hdc, TempBrush);
                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);

                SelectObject(hdc, TempOldBrush);

                DeleteObject(TempBrush);
            }
            else if (g_Tile[j][i] == 4)                 // 4은 Open_List의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                HBRUSH TempBrush = CreateSolidBrush(RGB(255, 0, 255));
                HBRUSH TempOldBrush = (HBRUSH)SelectObject(hdc, TempBrush);
                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);

                SelectObject(hdc, TempOldBrush);

                DeleteObject(TempBrush);
            }
            else if (g_Tile[j][i] == 5)                 // 5는 Closed_List의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                HBRUSH TempBrush = CreateSolidBrush(RGB(51, 255, 255));
                HBRUSH TempOldBrush = (HBRUSH)SelectObject(hdc, TempBrush);
                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);

                SelectObject(hdc, TempOldBrush);

                DeleteObject(TempBrush);
            }
            else if (g_Tile[j][i] == 6)                 // 6은 최단거리의 타일입니다.
            {
                int tileX = i * GRID_SIZE - g_Origin.x;
                int tileY = j * GRID_SIZE - g_Origin.y;

                HBRUSH TempBrush = CreateSolidBrush(RGB(51, 255, 0));
                HBRUSH TempOldBrush = (HBRUSH)SelectObject(hdc, TempBrush);
                Rectangle(hdc, tileX, tileY, tileX + GRID_SIZE, tileY + GRID_SIZE);

                SelectObject(hdc, TempOldBrush);

                DeleteObject(TempBrush);
            }


        }
    }


    SelectObject(hdc, hOldBrush);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CREATE:
    {
        HDC hdc = GetDC(hWnd);
        GetClientRect(hWnd, &g_MemDCRect);
        g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
        g_hMemDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);



        g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        g_hTileBrush = CreateSolidBrush(RGB(100, 100, 100));
        break;
    }
    case WM_LBUTTONDOWN:
    {
        //g_bDrag = true;

        //int xPos = GET_X_LPARAM(lParam);
        //int yPos = GET_Y_LPARAM(lParam);
        ///*int iTileX = (xPos + g_Origin.x * GRID_SIZE) / GRID_SIZE;
        //int iTileY = (yPos + g_Origin.y * GRID_SIZE) / GRID_SIZE;*/
        // 
        //int iTileX = xPos / GRID_SIZE;
        //int iTileY = (yPos / GRID_SIZE);


        //if (iTileX > GRID_WIDTH || iTileY > GRID_HEIGHT)
        //    break;

        //if (g_Tile[iTileY][iTileX] == 1)
        //    g_bErase = true;
        //else
        //    g_bErase = false;
        //break;c

        /*g_bDrag = true;       // 우수

        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int iTileX = (xPos + g_Origin.x * GRID_SIZE) / GRID_SIZE;
        int iTileY = (yPos + g_Origin.y * GRID_SIZE) / GRID_SIZE;

        if (iTileX >= GRID_WIDTH || iTileY >= GRID_HEIGHT)
            break;

        if (g_Tile[iTileY][iTileX] == 1)
            g_bErase = true;
        else
            g_bErase = false;
        break;*/

        if (g_bFirst)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = (xPos + g_Origin.x) / GRID_SIZE;
            int iTileY = (yPos + g_Origin.y) / GRID_SIZE;

            if (iTileX >= GRID_WIDTH || iTileY >= GRID_HEIGHT)
                break;

            if (g_Tile[iTileY][iTileX] == 1)
                g_bErase = true;
            else
                g_bErase = false;

            g_Tile[iTileY][iTileX] = 2; //start;

            Start_Pos.first = iTileX;
            Start_Pos.second = iTileY;

            InvalidateRect(hWnd, NULL, false);

            g_bFirst = !g_bFirst;
            break;
        }

        if (g_bFinal)
        {
            int yPos = GET_Y_LPARAM(lParam);
            int xPos = GET_X_LPARAM(lParam);
            int iTileX = (xPos + g_Origin.x) / GRID_SIZE;
            int iTileY = (yPos + g_Origin.y) / GRID_SIZE;

            if (iTileX >= GRID_WIDTH || iTileY >= GRID_HEIGHT)
                break;

            if (g_Tile[iTileY][iTileX] == 1)
                g_bErase = true;
            else
                g_bErase = false;

            g_Tile[iTileY][iTileX] = 3; //start;

            End_Pos.first = iTileX;
            End_Pos.second = iTileY;
            InvalidateRect(hWnd, NULL, false);
            g_bFinal = !g_bFinal;
            break;
        }

        g_bDrag = true;

        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        int iTileX = (xPos + g_Origin.x) / GRID_SIZE;
        int iTileY = (yPos + g_Origin.y) / GRID_SIZE;

        if (iTileX >= GRID_WIDTH || iTileY >= GRID_HEIGHT)
            break;

        if (g_Tile[iTileY][iTileX] == 1)
            g_bErase = true;
        else
            g_bErase = false;

        g_Tile[iTileY][iTileX] = !g_bErase;
        InvalidateRect(hWnd, NULL, false);

        break;
    }
    case WM_LBUTTONUP:
    {
        g_bDrag = false;
        break;
    }

    case WM_MOUSEWHEEL:
    {

        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int oldGridSize = GRID_SIZE;

        if (zDelta > 0) {
            GRID_SIZE += 1;
        }
        else if (zDelta < 0) {
            GRID_SIZE -= 1;
        }

        if (GRID_SIZE < 1) GRID_SIZE = 1;

        g_Origin.x = g_Origin.x * GRID_SIZE / oldGridSize;
        g_Origin.y = g_Origin.y * GRID_SIZE / oldGridSize;

        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_KEYDOWN:
    {
        bool bInvalidate = false; // 화면 갱신 여부를 나타내는 플래그

        if (wParam == 'R') {
            // R 키가 눌렸을 때 실행할 코드
            MessageBox(hWnd, L"Reset!", L"Key Press", MB_OK);
            memset(g_Tile, 0, sizeof(g_Tile));
            InvalidateRect(hWnd, NULL, false);
        }
        if (wParam == 'T') {
            // R 키가 눌렸을 때 실행할 코드

            g_Origin.x = 0;
            g_Origin.y = 0;
            InvalidateRect(hWnd, NULL, false);
        }

        switch (wParam)
        {
        case 'R':
            memset(g_Tile, 0, sizeof(g_Tile));
            bInvalidate = true;
            break;
        case 'T':
            g_Origin.x = 0;
            g_Origin.y = 0;
            bInvalidate = true;
            break;
        case 'Q':
            g_bFirst = !g_bFirst;
            break;
        case 'E':
            g_bFinal = !g_bFinal;
            break;
        case 'S':
        {
            g_bEndAStar = false;
            Astar(g_Tile, Start_Pos, End_Pos, GRID_WIDTH, GRID_HEIGHT, hWnd);
            InvalidateRect(hWnd, NULL, false);
            break;
        }
        case VK_LEFT:
            g_Origin.x -= GRID_SIZE;
            bInvalidate = true;
            break;
        case VK_RIGHT:
            g_Origin.x += GRID_SIZE;
            bInvalidate = true;
            break;
        case VK_UP:
            g_Origin.y -= GRID_SIZE;
            bInvalidate = true;
            break;
        case VK_DOWN:
            g_Origin.y += GRID_SIZE;
            bInvalidate = true;
            break;
        default:
            break;
        }

        if (bInvalidate)
        {
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }
    break;
    case WM_MOUSEMOVE:
    {
        if (g_bDrag)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = (xPos + g_Origin.x * GRID_SIZE) / GRID_SIZE;
            int iTileY = (yPos + g_Origin.y * GRID_SIZE) / GRID_SIZE;

            if (iTileX >= 0 && iTileX < GRID_WIDTH && iTileY >= 0 && iTileY < GRID_HEIGHT)
            {
                g_Tile[iTileY][iTileX] = !g_bErase;
                InvalidateRect(hWnd, NULL, false);
            }
        }
        break;
    }
    case WM_PAINT:
    {
        PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);
        RenderObstacle(g_hMemDC);
        RenderGrid(g_hMemDC);

        // 우아한 메뉴얼에 대해 출력합니다.
        int x = 0;
        int y = 0;
        int lineSpacing = 20;


        WCHAR Manual1[] = L" Start Position Setting : [Q]";
        WCHAR Manual2[] = L" End Position Setting : [E]";
        WCHAR Manual3[] = L" Reset Grid : [R]";
        WCHAR Manual4[] = L" Reset Focus : [T]";
        WCHAR Manual5[] = L" Start Astar : [S]";
        TextOutW(g_hMemDC, x, y, Manual1, wcslen(Manual1));
        TextOutW(g_hMemDC, x, y + 20, Manual2, wcslen(Manual2));
        TextOutW(g_hMemDC, x, y + 40, Manual3, wcslen(Manual3));
        TextOutW(g_hMemDC, x, y + 60, Manual4, wcslen(Manual4));
        TextOutW(g_hMemDC, x, y + 80, Manual5, wcslen(Manual5));


        /*if (g_bEndAStar)              // Astar 완료 여부 체크를 위한 디버그용 코드
        {
            WCHAR power[] = L"POWER";
            TextOutW(g_hMemDC, 0, 0, power, wcslen(power));
        }*/




        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);

        //BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, g_Origin.x * GRID_SIZE, g_Origin.y * GRID_SIZE, SRCCOPY);





        ::EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
    {
        SelectObject(g_hMemDC, g_hMemDCBitmap_old);
        DeleteObject(g_hMemDC);
        DeleteObject(g_hMemDCBitmap);
        ::PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}




/*                                                          //
*                       A Star Render
*/                                                          //

typedef struct Node
{
    Node(int x, int y) : _x(x), _y(y)
    {
    }
    int _x, _y;
    int G, H;
    Node* parent = nullptr;
};


struct cmp
{
    bool operator()(const Node* First, const Node* Second)
    {
        //if (First->G + First->H == Second->G + Second->H)		// F = G + H, F가 낮은 순으로 정렬, 힙이니 > 이것이 맞음	
        //{
        //    return First->H > Second->H;						// 만약 H 값이 동일할 시, H가 낮은 순으로.
        //}
        //else
        {
            return First->G + First->H > Second->G + Second->H;	// 동일하지 않다면, F 값이 낮은 순으로, 등호는 > 이것이 맞음.
        }
    }
}; 

 
int Astar(char(*Map)[100], pair<int, int> start, pair<int, int> goal, int max_X, int max_Y, HWND hWnd)
{

    priority_queue<Node*, vector<Node*>, cmp> pq;
    bool Close_Check[100][100] = { 0 , };

    Node* Open_List_Find[100][100] = { nullptr };

    vector<Node*> Close_List;


    int dx[8] = { 0,1,0,-1,1,1,-1,-1 };				// 방향좌표 ↑→↓←↘↗↙↖
    int dy[8] = { -1,0,1,0,1,-1,1,-1 };


    Node* startNode = new Node(start.first, start.second);
    startNode->G = 0;
    startNode->H = (abs(goal.first - startNode->_x) + abs(goal.second - startNode->_y)) * 10;
    startNode->parent = nullptr;

    pq.push(startNode);
    Close_Check[startNode->_y][startNode->_x] = 1;  // 1: closelist
    Open_List_Find[startNode->_y][startNode->_x] = startNode;
    

    g_Tile[startNode->_y][startNode->_x] = 4;   // 4 : 오픈 리스트



    InvalidateRect(hWnd, NULL, false);

    while (!pq.empty())
    {
        Node* Cur = pq.top();

        Close_List.push_back(Cur);

        pq.pop();
        Close_Check[Cur->_y][Cur->_x] = true;

        if (Cur->_x == goal.first && Cur->_y == goal.second)
            break; // Goal의 first는 x 좌표, 이 cpp의 모든 코드는 x가 먼저. 2차원 bool 배열은 예외. ex.. bool map[y][x]


        g_Tile[startNode->_y][startNode->_x] = 5;   // 5: closelist
        InvalidateRect(hWnd, NULL, false);

        for (int i = 0; i < 8; i++)
        {
            int nx = dx[i] + Cur->_x;
            int ny = dy[i] + Cur->_y;

            // 맵에서 유효한 위치인지 확인
            if (nx >= 0 && nx < max_X && ny >= 0 && ny < max_Y && Map[ny][nx] != 1)         // 벽만 아니면 일단 들어와
            {
                // 방문한 노드면 리턴                                                         
                if (Close_Check[ny][nx] != 0)
                    continue;


                int nextG = i < 4 ? Cur->G + 10 : Cur->G + 14; // 유클리드 그라운드 이동
                int nextH = (abs(goal.first - nx) + abs(goal.second - ny)) * 10; // 맨허튼 이동

                if (Open_List_Find[ny][nx] != nullptr)                                                      // Open_List 에 이미 있는 노드가 더 좋은 노드라면 노드 생성 x, Continue
                {
                    if (Open_List_Find[ny][nx]->G + Open_List_Find[ny][nx]->H <= nextG + nextH)             // 이게 핵심이였음.  최적의 길을 못 찾았던 이유
                        continue;
                }
                
                Node* addNode = new Node(nx, ny);
                addNode->G = nextG;
                addNode->H = nextH;
                addNode->parent = Cur;

                pq.push(addNode);
                Open_List_Find[ny][nx] = addNode;

                // Render 찍기용 //
                g_Tile[addNode->_y][addNode->_x] = 4;
                // Render 찍기용 //
                    
                    
                // Rander 찍기용 //
                    
                InvalidateRect(hWnd, NULL, false);
                MSG msg;
                if (::GetMessage(&msg, nullptr, 0, 0))
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
                // Rander 찍기용 //
                   
                

            }
        }
        //While 문 마지막 부분


        /*InvalidateRect(hWnd, NULL, false);
        MSG msg;
        if (::GetMessage(&msg, nullptr, 0, 0))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }*/

    }



    g_Tile[start.second][start.first] = 2;
    g_Tile[goal.second][goal.first] = 3;


     //최단 경로를 타고 가면서, 경로를 원복함
    Node* li = Close_List.back();

    while (li->parent != nullptr)           // 시작 노드는 부모가 없는 nullptr 상태로 가지고 있음, 안정성을 생각하면 임의로 값을 주는 방식도 괜찮을 듯. 
    {
        g_Tile[li->parent->_y][li->parent->_x] = 6;
        li = li->parent;
    }
    

   
    

    InvalidateRect(hWnd, NULL, false);
    return 0;
}





