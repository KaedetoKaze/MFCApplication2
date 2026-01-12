#pragma once

#include <memory>
#include <vector>
#include <afxwin.h>
#include <gdiplus.h>

class CCanvasStatic : public CStatic
{
public:
    CCanvasStatic();

    void SetImage(const std::shared_ptr<Gdiplus::Bitmap>& bitmap);
    void SetCurveData(const std::vector<double>& data);
    void Clear();

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()

private:
    COLORREF m_background;
    std::shared_ptr<Gdiplus::Bitmap> m_image;
    std::vector<double> m_curveData;

    void DrawImage(CDC& dc, const CRect& rc);
    void DrawCurve(CDC& dc, const CRect& rc);
};
