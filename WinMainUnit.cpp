//------------------------------------------------------------------------------------------
#include <windows.h>
#include <commdlg.h>

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

//------------------------------------------------------------------------------------------

#define IDI_MAINICON 101

#define IDC_FILE_LIST         1001
#define IDC_ADD_FILES         1002
#define IDC_REMOVE_SELECTED   1003
#define IDC_CLEAR_LIST        1004
#define IDC_CONVERT           1005
#define IDC_STATUS            1006

struct BmpImage
{
	int width = 0;
	int height = 0;
	std::vector<unsigned char> bgraTopDown;
};

struct IconImage
{
	int width = 0;
	int height = 0;
	std::vector<unsigned char> bgraTopDown;
	std::vector<unsigned char> andMask;
};

HWND g_hList = nullptr;
HWND g_hStatus = nullptr;

//------------------------------------------------------------------------------------------

std::wstring GetFolderPart(const std::wstring &path)
{
	size_t pos = path.find_last_of(L"\\/");
	if (pos == std::wstring::npos)
	{
		return L"";
	}
	return path.substr(0, pos + 1);
}

std::wstring GetBaseNameNoExt(const std::wstring &path)
{
	size_t slashPos = path.find_last_of(L"\\/");
	size_t start = (slashPos == std::wstring::npos) ? 0 : slashPos + 1;
	size_t dotPos = path.find_last_of(L'.');
	if (dotPos == std::wstring::npos || dotPos < start)
	{
		return path.substr(start);
	}
	return path.substr(start, dotPos - start);
}

std::wstring MakeIcoPath(const std::wstring &bmpPath)
{
	return GetFolderPart(bmpPath) + GetBaseNameNoExt(bmpPath) + L".ico";
}

void SetStatus(const std::wstring &text)
{
	if (g_hStatus)
	{
		SetWindowTextW(g_hStatus, text.c_str());
	}
}

bool LoadBmpViaGdi(const std::wstring &path, BmpImage &outBmp, std::wstring &outError)
{
	HBITMAP hBmp = (HBITMAP)LoadImageW(nullptr, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hBmp)
	{
		outError = L"Unable to load bitmap file.";
		return false;
	}

	DIBSECTION ds = {};
	if (GetObjectW(hBmp, sizeof(ds), &ds) != sizeof(ds))
	{
		DeleteObject(hBmp);
		outError = L"Unable to query bitmap information.";
		return false;
	}

	if (!ds.dsBm.bmBits)
	{
		DeleteObject(hBmp);
		outError = L"Bitmap data is unavailable.";
		return false;
	}

	const int width = ds.dsBm.bmWidth;
	const int height = std::abs(ds.dsBm.bmHeight);
	const int bpp = ds.dsBm.bmBitsPixel;
	const int stride = ds.dsBm.bmWidthBytes;
	if (width <= 0 || height <= 0)
	{
		DeleteObject(hBmp);
		outError = L"Bitmap dimensions are invalid.";
		return false;
	}

	if (bpp != 24 && bpp != 32)
	{
		DeleteObject(hBmp);
		outError = L"Only 24-bit and 32-bit BMP files are supported.";
		return false;
	}

	const bool srcBottomUp = (ds.dsBmih.biHeight > 0);
	const unsigned char *srcBits = static_cast<const unsigned char *>(ds.dsBm.bmBits);

	outBmp.width = width;
	outBmp.height = height;
	outBmp.bgraTopDown.assign(static_cast<size_t>(width) * static_cast<size_t>(height) * 4, 0);

	for (int y = 0; y < height; ++y)
	{
		int srcY = srcBottomUp ? (height - 1 - y) : y;
		const unsigned char *srcRow = srcBits + static_cast<size_t>(srcY) * static_cast<size_t>(stride);
		unsigned char *dstRow = &outBmp.bgraTopDown[static_cast<size_t>(y) * static_cast<size_t>(width) * 4];

		for (int x = 0; x < width; ++x)
		{
			const unsigned char *srcPx = srcRow + static_cast<size_t>(x) * (bpp / 8);
			unsigned char *dstPx = dstRow + static_cast<size_t>(x) * 4;
			dstPx[0] = srcPx[0];
			dstPx[1] = srcPx[1];
			dstPx[2] = srcPx[2];
			dstPx[3] = 255;
		}
	}

	DeleteObject(hBmp);
	return true;
}

void BuildAndMask(const IconImage &icon, std::vector<unsigned char> &outMask)
{
	const int maskStride = ((icon.width + 31) / 32) * 4;
	outMask.assign(static_cast<size_t>(maskStride) * static_cast<size_t>(icon.height), 0);

	for (int y = 0; y < icon.height; ++y)
	{
		// AND mask is stored bottom-up.
		unsigned char *maskRow = &outMask[static_cast<size_t>(icon.height - 1 - y) * static_cast<size_t>(maskStride)];
		const unsigned char *srcRow = &icon.bgraTopDown[static_cast<size_t>(y) * static_cast<size_t>(icon.width) * 4];

		for (int x = 0; x < icon.width; ++x)
		{
			const unsigned char alpha = srcRow[static_cast<size_t>(x) * 4 + 3];
			if (alpha == 0)
			{
				maskRow[x / 8] |= (0x80 >> (x % 8));
			}
		}
	}
}

bool BuildIconFromBmp(const BmpImage &bmp, IconImage &outIcon, std::wstring &outError)
{
	if (bmp.width <= 0 || bmp.height <= 0 || bmp.bgraTopDown.empty())
	{
		outError = L"Bitmap data is empty.";
		return false;
	}

	outIcon.width = bmp.width;
	outIcon.height = bmp.height;
	outIcon.bgraTopDown = bmp.bgraTopDown;

	const unsigned char keyB = outIcon.bgraTopDown[0];
	const unsigned char keyG = outIcon.bgraTopDown[1];
	const unsigned char keyR = outIcon.bgraTopDown[2];

	for (int y = 0; y < outIcon.height; ++y)
	{
		unsigned char *row = &outIcon.bgraTopDown[static_cast<size_t>(y) * static_cast<size_t>(outIcon.width) * 4];
		for (int x = 0; x < outIcon.width; ++x)
		{
			unsigned char *px = row + static_cast<size_t>(x) * 4;
			if (px[0] == keyB && px[1] == keyG && px[2] == keyR)
			{
				px[3] = 0;
			}
			else
			{
				px[3] = 255;
			}
		}
	}

	BuildAndMask(outIcon, outIcon.andMask);
	return true;
}

bool SaveIcoFile(const std::wstring &path, const IconImage &icon, std::wstring &outError)
{
	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file)
	{
		outError = L"Unable to create output ICO file.";
		return false;
	}

	#pragma pack(push, 1)
	struct IcoHeader
	{
		WORD reserved;
		WORD type;
		WORD count;
	};

	struct IcoDirEntry
	{
		BYTE width;
		BYTE height;
		BYTE colorCount;
		BYTE reserved;
		WORD planes;
		WORD bitCount;
		DWORD bytesInRes;
		DWORD imageOffset;
	};
	#pragma pack(pop)

	BITMAPINFOHEADER bih = {};
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = icon.width;
	bih.biHeight = icon.height * 2;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biCompression = BI_RGB;

	const int xorStride = icon.width * 4;
	const DWORD xorSize = static_cast<DWORD>(static_cast<size_t>(xorStride) * static_cast<size_t>(icon.height));
	const DWORD andSize = static_cast<DWORD>(icon.andMask.size());
	const DWORD imageDataSize = static_cast<DWORD>(sizeof(BITMAPINFOHEADER)) + xorSize + andSize;

	IcoHeader hdr = {};
	hdr.reserved = 0;
	hdr.type = 1;
	hdr.count = 1;

	IcoDirEntry entry = {};
	entry.width = (icon.width >= 256) ? 0 : static_cast<BYTE>(icon.width);
	entry.height = (icon.height >= 256) ? 0 : static_cast<BYTE>(icon.height);
	entry.colorCount = 0;
	entry.reserved = 0;
	entry.planes = 1;
	entry.bitCount = 32;
	entry.bytesInRes = imageDataSize;
	entry.imageOffset = sizeof(IcoHeader) + sizeof(IcoDirEntry);

	file.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
	file.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
	file.write(reinterpret_cast<const char*>(&bih), sizeof(bih));

	// XOR bitmap in bottom-up BGRA.
	for (int y = icon.height - 1; y >= 0; --y)
	{
		const unsigned char *row = &icon.bgraTopDown[static_cast<size_t>(y) * static_cast<size_t>(icon.width) * 4];
		file.write(reinterpret_cast<const char*>(row), xorStride);
	}

	file.write(reinterpret_cast<const char*>(icon.andMask.data()), icon.andMask.size());

	if (!file.good())
	{
		outError = L"Failed writing ICO bytes to disk.";
		return false;
	}

	return true;
}

void AddFilesToList(HWND hWnd)
{
	wchar_t buffer[32768] = {};
	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0]));
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	ofn.lpstrDefExt = L"bmp";

	if (!GetOpenFileNameW(&ofn))
	{
		return;
	}

	std::vector<std::wstring> files;
	const wchar_t *p = buffer;
	std::wstring first = p;
	p += first.size() + 1;

	if (*p == L'\0')
	{
		files.push_back(first);
	}
	else
	{
		std::wstring dir = first;
		while (*p)
		{
			std::wstring leaf = p;
			files.push_back(dir + L"\\" + leaf);
			p += leaf.size() + 1;
		}
	}

	for (size_t i = 0; i < files.size(); ++i)
	{
		if (SendMessageW(g_hList, LB_FINDSTRINGEXACT, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(files[i].c_str())) == LB_ERR)
		{
			SendMessageW(g_hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(files[i].c_str()));
		}
	}

	SetStatus(L"Added " + std::to_wstring(files.size()) + L" file(s).");
}

void RemoveSelectedFiles()
{
	for (;;)
	{
		LRESULT idx = SendMessageW(g_hList, LB_GETCURSEL, 0, 0);
		if (idx == LB_ERR)
		{
			break;
		}
		SendMessageW(g_hList, LB_DELETESTRING, static_cast<WPARAM>(idx), 0);
	}
	SetStatus(L"Removed selected file(s).");
}

void ClearAllFiles()
{
	SendMessageW(g_hList, LB_RESETCONTENT, 0, 0);
	SetStatus(L"Cleared file list.");
}

void ConvertAllFiles()
{
	const int count = static_cast<int>(SendMessageW(g_hList, LB_GETCOUNT, 0, 0));
	if (count <= 0)
	{
		SetStatus(L"No files to convert.");
		MessageBoxW(nullptr, L"Please add one or more BMP files first.", L"BmpToIcon", MB_OK | MB_ICONINFORMATION);
		return;
	}

	int success = 0;
	int failed = 0;
	std::wstring lastError;

	for (int i = 0; i < count; ++i)
	{
		wchar_t pathBuf[MAX_PATH * 4] = {};
		SendMessageW(g_hList, LB_GETTEXT, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(pathBuf));
		std::wstring bmpPath = pathBuf;
		std::wstring icoPath = MakeIcoPath(bmpPath);

		BmpImage bmp;
		IconImage icon;
		std::wstring err;

		if (!LoadBmpViaGdi(bmpPath, bmp, err))
		{
			++failed;
			lastError = bmpPath + L": " + err;
			continue;
		}
		if (!BuildIconFromBmp(bmp, icon, err))
		{
			++failed;
			lastError = bmpPath + L": " + err;
			continue;
		}
		if (!SaveIcoFile(icoPath, icon, err))
		{
			++failed;
			lastError = icoPath + L": " + err;
			continue;
		}

		++success;
	}

	std::wstring summary = L"Converted " + std::to_wstring(success) + L" file(s), failed " + std::to_wstring(failed) + L".";
	SetStatus(summary);

	if (failed > 0)
	{
		MessageBoxW(nullptr, (summary + L"\n\nLast error:\n" + lastError).c_str(), L"BmpToIcon", MB_OK | MB_ICONWARNING);
	}
	else
	{
		MessageBoxW(nullptr, summary.c_str(), L"BmpToIcon", MB_OK | MB_ICONINFORMATION);
	}
}

void LayoutControls(HWND hWnd)
{
	RECT rc = {};
	GetClientRect(hWnd, &rc);

	const int padding = 10;
	const int btnW = 140;
	const int btnH = 28;
	const int statusH = 24;

	int listX = padding;
	int listY = padding;
	int listW = (rc.right - rc.left) - padding * 3 - btnW;
	int listH = (rc.bottom - rc.top) - padding * 3 - statusH;

	if (listW < 100) listW = 100;
	if (listH < 80) listH = 80;

	MoveWindow(g_hList, listX, listY, listW, listH, TRUE);

	const int btnX = listX + listW + padding;
	int btnY = listY;

	MoveWindow(GetDlgItem(hWnd, IDC_ADD_FILES), btnX, btnY, btnW, btnH, TRUE);
	btnY += btnH + 8;
	MoveWindow(GetDlgItem(hWnd, IDC_REMOVE_SELECTED), btnX, btnY, btnW, btnH, TRUE);
	btnY += btnH + 8;
	MoveWindow(GetDlgItem(hWnd, IDC_CLEAR_LIST), btnX, btnY, btnW, btnH, TRUE);
	btnY += btnH + 20;
	MoveWindow(GetDlgItem(hWnd, IDC_CONVERT), btnX, btnY, btnW, btnH + 6, TRUE);

	MoveWindow(g_hStatus, padding, rc.bottom - padding - statusH, rc.right - padding * 2, statusH, TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			g_hList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT,
				0, 0, 100, 100, hWnd, (HMENU)IDC_FILE_LIST, GetModuleHandleW(nullptr), nullptr);

			CreateWindowW(L"BUTTON", L"Add BMP Files...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 100, 24, hWnd, (HMENU)IDC_ADD_FILES, GetModuleHandleW(nullptr), nullptr);

			CreateWindowW(L"BUTTON", L"Remove Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 100, 24, hWnd, (HMENU)IDC_REMOVE_SELECTED, GetModuleHandleW(nullptr), nullptr);

			CreateWindowW(L"BUTTON", L"Clear List", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0, 100, 24, hWnd, (HMENU)IDC_CLEAR_LIST, GetModuleHandleW(nullptr), nullptr);

			CreateWindowW(L"BUTTON", L"Convert to ICO", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				0, 0, 100, 28, hWnd, (HMENU)IDC_CONVERT, GetModuleHandleW(nullptr), nullptr);

			g_hStatus = CreateWindowW(L"STATIC", L"Ready.", WS_CHILD | WS_VISIBLE | SS_LEFT,
				0, 0, 100, 20, hWnd, (HMENU)IDC_STATUS, GetModuleHandleW(nullptr), nullptr);

			LayoutControls(hWnd);
			return 0;
		}

		case WM_SIZE:
			LayoutControls(hWnd);
			return 0;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ADD_FILES:
					AddFilesToList(hWnd);
					return 0;
				case IDC_REMOVE_SELECTED:
					RemoveSelectedFiles();
					return 0;
				case IDC_CLEAR_LIST:
					ClearAllFiles();
					return 0;
				case IDC_CONVERT:
					ConvertAllFiles();
					return 0;
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
	const wchar_t *kClassName = L"BmpToIconMainWindow";

	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAINICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = kClassName;
	wc.hIconSm = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAINICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	if (!RegisterClassExW(&wc))
	{
		MessageBoxW(nullptr, L"Failed to register the main window class.", L"BmpToIcon", MB_OK | MB_ICONERROR);
		return 1;
	}

	HWND hWnd = CreateWindowExW(0, kClassName, L"BmpToIcon - BMP Batch to ICO",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 820, 480,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		MessageBoxW(nullptr, L"Failed to create the main window.", L"BmpToIcon", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg = {};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}
//------------------------------------------------------------------------------------------
