#include <windows.h>
#include <wingdi.h>
#include <vector>
#include <string>
#include <fstream>
#include <regex>

using namespace std;

#define WIDTH 800
#define HEIGHT 325

int col;
int row;
vector<vector<string>> textMatrix{};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawVBorder(HDC hdc, int cWidth, int cHeight, int wndHeight);
void DrawHBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win);
int readFile();
INT GetColumnCount();
INT DrawCell(HDC hdc, RECT clientRect);
HFONT CreateMyFont(INT height, INT width, INT weight);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (readFile()) {
		const wchar_t* const WND_CREATE_ERROR = L"Cannot read file";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 0;
	}

	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };

	LPCWSTR const className = L"MyAppClass";
	LPCWSTR windowName = L"Table";

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.style = CS_DBLCLKS;

	if (!RegisterClassEx(&wc)) {
		return EXIT_FAILURE;
	}

	hWnd = CreateWindowEx(
		0, className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIDTH, HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		const wchar_t* const WND_CREATE_ERROR = L"Cannot create window";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	RECT clientRect;

	switch (message) {

	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &clientRect);

		int fontHeight = 20;
		int fontWidth = 8;

		HFONT hFont = CreateMyFont(fontHeight, fontWidth, 8);
		HPEN tablePen = CreatePen(PS_SOLID, 2, RGB(100, 200, 250));

		SelectObject(hdc, hFont);
		SelectObject(hdc, tablePen);

		TEXTMETRIC textMetric;
		GetTextMetrics(hdc, &textMetric);

		bool step = false;
		PatBlt(GetDC(hWnd), 0, 0, clientRect.right, clientRect.bottom, WHITENESS);
		while (DrawCell(hdc, clientRect) > clientRect.bottom - clientRect.top)
		{

			if (fontWidth == 1)
			{
				break;
			}

			fontHeight--;
			if (step)
			{
				fontWidth--;
			}
			step = !step;

			hFont = CreateMyFont(fontHeight, fontWidth, 8);
			SelectObject(hdc, hFont);

			PatBlt(GetDC(hWnd), 0, 0, clientRect.right, clientRect.bottom, WHITENESS);
		}

		int tableHeight = DrawCell(hdc, clientRect);

		MoveToEx(hdc, 0, 1, nullptr);
		LineTo(hdc, clientRect.right - clientRect.left, 0);

		int columnCount = GetColumnCount();
		for (int i = 1; i < columnCount; i++) {
			MoveToEx(hdc, i * (clientRect.right - clientRect.left) / columnCount, 0, nullptr);
			LineTo(hdc, i * (clientRect.right - clientRect.left) / columnCount, tableHeight);
		}

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZE: {
		GetClientRect(hWnd, &clientRect);
		InvalidateRect(hWnd, nullptr, TRUE);
	}
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HFONT CreateMyFont(INT height, INT width, INT weight)
{
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = DEFAULT_PITCH;

	lf.lfHeight = height;
	lf.lfWidth = width;
	lf.lfWeight = weight;

	lf.lfItalic = 1;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfEscapement = 0;

	return CreateFontIndirect(&lf);
}

INT DrawCell(HDC hdc, RECT clientRect)
{
	TEXTMETRIC textMetric;
	GetTextMetrics(hdc, &textMetric);

	int tableHeight = 0;
	int columnCount = GetColumnCount();

	int cellWidth = (clientRect.right - clientRect.left) / columnCount;
	int minCellHeight = textMetric.tmHeight;
	
	for (int i = 0; i < textMatrix.size(); i++)
	{
		int currentCellHeight = minCellHeight;
		for (int j = 0; j < textMatrix[i].size(); j++)
		{
			std::wstring text = std::wstring(textMatrix[i][j].begin(), textMatrix[i][j].end());
			RECT rect = { j * cellWidth, tableHeight + minCellHeight / 5, (j + 1) * cellWidth, clientRect.bottom - clientRect.top };
			int tempHeight = DrawText(hdc, text.c_str(), text.size(), &rect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);

			if (tempHeight > currentCellHeight)
			{
				currentCellHeight = tempHeight;
			}
		}

		tableHeight += currentCellHeight + minCellHeight / 5;
		MoveToEx(hdc, 0, tableHeight, nullptr);
		LineTo(hdc, clientRect.right - clientRect.left, tableHeight);
	}

	return tableHeight;
}

INT GetColumnCount()
{
	int column = 0;
	for (int i = 0; i < textMatrix.size(); i++)
	{
		if (textMatrix[i].size() > column)
		{
			column = textMatrix[i].size();
		}
	}

	return column;
}


void DrawVBorder(HDC hdc, int cWidth, int cHeight, int wndHeight) {
	for (int i = 0; i <= col; i++) {
		MoveToEx(hdc, cWidth * i, 0, nullptr);
		LineTo(hdc, cWidth * i, wndHeight - cHeight / 2);
	}
}

void DrawHBorder(HDC hdc, int cWidth, int cHeight, int wndHeight, RECT win) {
	MoveToEx(hdc, 0, wndHeight - cHeight / 2, nullptr);
	LineTo(hdc, win.right, wndHeight - cHeight / 2);
}

int readFile() {
	std::ifstream file("../tableData.txt");
	if (!file) {
		return -1;
	}
	string line;
	while (std::getline(file, line)) {
		std::regex regex{ R"([,][\s]+)" };
		std::sregex_token_iterator iterator{ line.begin(), line.end(), regex, -1 };
		textMatrix.emplace_back(vector<string>{iterator, {}});
	}
	row = textMatrix.size();
	col = textMatrix[0].size();

	file.close();
	return 0;
}
