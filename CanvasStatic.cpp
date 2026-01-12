#include "pch.h"
#include "CanvasStatic.h"

#include <algorithm>
#include <cmath>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#pragma comment(lib, "gdiplus.lib")

BEGIN_MESSAGE_MAP(CCanvasStatic, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CCanvasStatic::CCanvasStatic()
    : m_background(RGB(240, 240, 240))
{
}

void CCanvasStatic::SetImage(const std::shared_ptr<Gdiplus::Bitmap>& bitmap)
{
    m_image = bitmap;
    m_curveData.clear();
    Invalidate();
}

void CCanvasStatic::SetCurveData(const std::vector<double>& data)
{
    m_curveData = data;
    m_image.reset();
    Invalidate();
}

void CCanvasStatic::Clear()
{
    m_image.reset();
    m_curveData.clear();
    Invalidate();
}

BOOL CCanvasStatic::OnEraseBkgnd(CDC* /*pDC*/)
{
    // Ë«»º³å´¦Àí£¬²»µ¥¶À²Á³ý±³¾°
    return TRUE;
}

void CCanvasStatic::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap buffer;
    buffer.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&buffer);

    memDC.FillSolidRect(&rc, m_background);

    if (m_image)
    {
        DrawImage(memDC, rc);
    }
    else if (!m_curveData.empty())
    {
        DrawCurve(memDC, rc);
    }

    dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(pOldBitmap);
}

void CCanvasStatic::DrawImage(CDC& dc, const CRect& rc)
{
    if (!m_image)
        return;

    const int iw = static_cast<int>(m_image->GetWidth());
    const int ih = static_cast<int>(m_image->GetHeight());
    if (iw <= 0 || ih <= 0)
        return;

    const double sx = static_cast<double>(rc.Width()) / iw;
    const double sy = static_cast<double>(rc.Height()) / ih;
    const double scale = (std::min)(sx, sy);

    const int dw = static_cast<int>(iw * scale);
    const int dh = static_cast<int>(ih * scale);
    const int dx = rc.left + (rc.Width() - dw) / 2;
    const int dy = rc.top + (rc.Height() - dh) / 2;

    Gdiplus::Graphics g(dc.GetSafeHdc());
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.DrawImage(m_image.get(), dx, dy, dw, dh);
}

void CCanvasStatic::DrawCurve(CDC& dc, const CRect& rc)
{
    if (m_curveData.empty())
        return;

    const int margin = 24;
    CRect plotRect = rc;
    plotRect.DeflateRect(margin, margin);
    if (plotRect.Width() <= 1 || plotRect.Height() <= 1)
        return;

    CPen axisPen(PS_SOLID, 1, RGB(80, 80, 80));
    CPen curvePen(PS_SOLID, 2, RGB(60, 120, 230));

    CPen* pOldPen = dc.SelectObject(&axisPen);
    dc.MoveTo(plotRect.left, plotRect.bottom);
    dc.LineTo(plotRect.right, plotRect.bottom);
    dc.MoveTo(plotRect.left, plotRect.top);
    dc.LineTo(plotRect.left, plotRect.bottom);

    const auto minmax = std::minmax_element(m_curveData.begin(), m_curveData.end(), [](double a, double b) {
        const bool aok = std::isfinite(a);
        const bool bok = std::isfinite(b);
        if (aok && bok) return a < b;
        if (aok) return true;
        return false;
    });

    double vmin = *minmax.first;
    double vmax = *minmax.second;
    if (!std::isfinite(vmin) || !std::isfinite(vmax) || vmax - vmin < 1e-9)
    {
        vmin = 0.0;
        vmax = 1.0;
    }

    dc.SelectObject(&curvePen);

    bool hasPrev = false;
    CPoint prev;
    const size_t n = m_curveData.size();
    for (size_t i = 0; i < n; ++i)
    {
        const double v = m_curveData[i];
        if (!std::isfinite(v))
        {
            hasPrev = false;
            continue;
        }

        const double t = n > 1 ? static_cast<double>(i) / static_cast<double>(n - 1) : 0.0;
        const int x = plotRect.left + static_cast<int>(t * plotRect.Width());
        const double norm = (v - vmin) / (vmax - vmin);
        const int y = plotRect.bottom - static_cast<int>(norm * plotRect.Height());

        if (!hasPrev)
        {
            dc.MoveTo(x, y);
            hasPrev = true;
        }
        else
        {
            dc.LineTo(x, y);
        }
        prev = CPoint(x, y);
    }

    dc.SelectObject(pOldPen);
}
