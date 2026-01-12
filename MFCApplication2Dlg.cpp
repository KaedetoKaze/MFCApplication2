// MFCApplication2Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication2.h"
#include "MFCApplication2Dlg.h"
#include "afxdialogex.h"
#include "cnpy.h"

#include <cmath>
#include <limits>
#include <regex>
#include <string>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication2Dlg 对话框



CMFCApplication2Dlg::CMFCApplication2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CANVAS_COLOR, m_canvasColor);
	DDX_Control(pDX, IDC_CANVAS_DEPTH, m_canvasDepth);
	DDX_Control(pDX, IDC_CANVAS_PROFILE, m_canvasProfile);
}

BEGIN_MESSAGE_MAP(CMFCApplication2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSE_PICTURE, &CMFCApplication2Dlg::OnBnClickedChoosePicture)
END_MESSAGE_MAP()


// CMFCApplication2Dlg 消息处理程序

BOOL CMFCApplication2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication2Dlg::OnBnClickedChoosePicture()
{
    const std::wstring initialDir = GetDataDirectory();
    CFileDialog dlg(TRUE, L"jpg", nullptr,
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        L"JPG 图像 (color_*.jpg)|color_*.jpg|JPEG 文件 (*.jpg;*.jpeg)|*.jpg;*.jpeg||",
        this, 0, TRUE);
    dlg.m_ofn.lpstrInitialDir = initialDir.c_str();

    if (dlg.DoModal() != IDOK)
        return;

    const std::wstring colorPath = dlg.GetPathName().GetString();
    const std::wstring filename = fs::path(colorPath).filename().wstring();
    std::wregex re(L"color_(\\d{8}_\\d{6})\\.jpg", std::regex_constants::icase);
    std::wsmatch match;
    if (!std::regex_match(filename, match, re))
    {
        MessageBox(L"文件名不符合 color_YYYYMMDD_HHMMSS.jpg 规则", L"文件错误", MB_ICONWARNING);
        return;
    }

    const std::wstring ts = match[1].str();
    const fs::path depthPath = fs::path(colorPath).parent_path() / (L"depth_" + ts + L".npy");
    if (!fs::exists(depthPath))
    {
        MessageBox(L"对应的深度文件不存在", L"缺少文件", MB_ICONWARNING);
        return;
    }

    if (!LoadColorImage(colorPath))
        return;

    if (!LoadDepthFile(depthPath.wstring()))
        return;

    const auto depthBmp = CreatePseudoColorBitmap(m_depthNormalized, m_depthWidth, m_depthHeight);
    if (depthBmp)
    {
        m_canvasDepth.SetImage(depthBmp);
    }

    const auto profile = BuildCenterProfile(m_depthNormalized, m_depthWidth, m_depthHeight);
    m_canvasProfile.SetCurveData(profile);
}

std::wstring CMFCApplication2Dlg::GetDataDirectory() const
{
    wchar_t modulePath[MAX_PATH] = {0};
    GetModuleFileName(nullptr, modulePath, MAX_PATH);
    fs::path dir(modulePath);
    dir = dir.parent_path() / L"data";
    return dir.wstring();
}

bool CMFCApplication2Dlg::LoadColorImage(const std::wstring& path)
{
    std::shared_ptr<Gdiplus::Bitmap> bmp(Gdiplus::Bitmap::FromFile(path.c_str(), FALSE));
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok)
    {
        MessageBox(L"无法读取彩色图像", L"错误", MB_ICONERROR);
        return false;
    }
    m_canvasColor.SetImage(bmp);
    return true;
}

namespace
{
    template <typename T>
    bool NormalizeDepth(const cnpy::NpyArray& arr, std::vector<double>& normalized, size_t& width, size_t& height)
    {
        if (arr.shape.size() != 2)
            return false;

        height = arr.shape[0];
        width = arr.shape[1];
        const size_t total = width * height;
        if (total == 0)
            return false;

        const T* ptr = arr.data<T>();
        normalized.resize(total);

        double minv = std::numeric_limits<double>::max();
        double maxv = -std::numeric_limits<double>::max();
        size_t validCount = 0;

        for (size_t i = 0; i < total; ++i)
        {
            double v = static_cast<double>(ptr[i]);
            const bool valid = std::isfinite(v) && v > 0.0;
            if (valid)
            {
                minv = std::min(minv, v);
                maxv = std::max(maxv, v);
                ++validCount;
            }
        }

        if (validCount == 0)
            return false;

        const double denom = (maxv - minv);
        for (size_t i = 0; i < total; ++i)
        {
            double v = static_cast<double>(ptr[i]);
            const bool valid = std::isfinite(v) && v > 0.0;
            if (!valid)
            {
                normalized[i] = std::numeric_limits<double>::quiet_NaN();
                continue;
            }
            if (denom < 1e-9)
            {
                normalized[i] = 1.0;
            }
            else
            {
                normalized[i] = (v - minv) / denom;
            }
        }
        return true;
    }
}

bool CMFCApplication2Dlg::LoadDepthFile(const std::wstring& path)
{
    cnpy::NpyArray arr;
    try
    {
        arr = cnpy::npy_load(std::string(fs::path(path).u8string()));
    }
    catch (const std::exception&)
    {
        MessageBox(L"读取深度文件失败", L"错误", MB_ICONERROR);
        return false;
    }

    m_depthNormalized.clear();
    m_depthWidth = m_depthHeight = 0;

    bool ok = false;
    if (arr.word_size == sizeof(float))
    {
        ok = NormalizeDepth<float>(arr, m_depthNormalized, m_depthWidth, m_depthHeight);
    }
    else if (arr.word_size == sizeof(uint16_t))
    {
        ok = NormalizeDepth<uint16_t>(arr, m_depthNormalized, m_depthWidth, m_depthHeight);
    }

    if (!ok)
    {
        MessageBox(L"深度数据格式不支持或数据为空", L"错误", MB_ICONERROR);
        return false;
    }

    return true;
}

static Gdiplus::Color JetColor(double v)
{
    v = (std::max)(0.0, (std::min)(1.0, v));
    double r = 0, g = 0, b = 0;
    if (v < 0.25)
    {
        r = 0;
        g = 4 * v;
        b = 1;
    }
    else if (v < 0.5)
    {
        r = 0;
        g = 1;
        b = 1 + 4 * (0.25 - v);
    }
    else if (v < 0.75)
    {
        r = 4 * (v - 0.5);
        g = 1;
        b = 0;
    }
    else
    {
        r = 1;
        g = 1 + 4 * (0.75 - v);
        b = 0;
    }
    r = (std::max)(0.0, (std::min)(1.0, r));
    g = (std::max)(0.0, (std::min)(1.0, g));
    b = (std::max)(0.0, (std::min)(1.0, b));
    return Gdiplus::Color(255, static_cast<BYTE>(r * 255), static_cast<BYTE>(g * 255), static_cast<BYTE>(b * 255));
}

std::shared_ptr<Gdiplus::Bitmap> CMFCApplication2Dlg::CreatePseudoColorBitmap(const std::vector<double>& data, size_t width, size_t height)
{
    if (data.empty() || width == 0 || height == 0)
        return nullptr;

    std::shared_ptr<Gdiplus::Bitmap> bmp = std::make_shared<Gdiplus::Bitmap>(static_cast<INT>(width), static_cast<INT>(height), PixelFormat24bppRGB);
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok)
        return nullptr;

    Gdiplus::Rect rect(0, 0, static_cast<INT>(width), static_cast<INT>(height));
    Gdiplus::BitmapData bd;
    if (bmp->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &bd) != Gdiplus::Ok)
        return nullptr;

    for (size_t y = 0; y < height; ++y)
    {
        BYTE* row = static_cast<BYTE*>(bd.Scan0) + y * bd.Stride;
        for (size_t x = 0; x < width; ++x)
        {
            double v = data[y * width + x];
            if (!std::isfinite(v))
            {
                row[x * 3 + 0] = row[x * 3 + 1] = row[x * 3 + 2] = 0;
                continue;
            }
            const Gdiplus::Color c = JetColor(v);
            row[x * 3 + 0] = c.GetBlue();
            row[x * 3 + 1] = c.GetGreen();
            row[x * 3 + 2] = c.GetRed();
        }
    }

    bmp->UnlockBits(&bd);
    return bmp;
}

std::vector<double> CMFCApplication2Dlg::BuildCenterProfile(const std::vector<double>& data, size_t width, size_t height) const
{
    std::vector<double> profile;
    if (data.empty() || width == 0 || height == 0)
        return profile;

    const size_t row = height / 2;
    profile.reserve(width);
    for (size_t x = 0; x < width; ++x)
    {
        profile.push_back(data[row * width + x]);
    }
    return profile;
}

