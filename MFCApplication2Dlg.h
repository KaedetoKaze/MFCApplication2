// MFCApplication2Dlg.h: 头文件
//

#pragma once

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#include <memory>
#include <string>
#include <vector>

#if defined(__cpp_lib_filesystem) && __cpp_lib_filesystem >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#elif defined(_MSC_VER) && defined(_HAS_CXX17) && _HAS_CXX17
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Filesystem support is required"
#endif

#include <gdiplus.h>
#include "CanvasStatic.h"

// CMFCApplication2Dlg 对话框
class CMFCApplication2Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication2Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION2_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedChoosePicture();
	DECLARE_MESSAGE_MAP()

private:
	CCanvasStatic m_canvasColor;
	CCanvasStatic m_canvasDepth;
	CCanvasStatic m_canvasProfile;

	std::vector<double> m_depthNormalized;
	size_t m_depthWidth{0};
	size_t m_depthHeight{0};

	std::wstring GetDataDirectory() const;
	bool LoadColorImage(const std::wstring& path);
	bool LoadDepthFile(const std::wstring& path);
	std::shared_ptr<Gdiplus::Bitmap> CreatePseudoColorBitmap(const std::vector<double>& data, size_t width, size_t height);
	std::vector<double> BuildCenterProfile(const std::vector<double>& data, size_t width, size_t height) const;
};
