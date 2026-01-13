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
    : m_background(RGB(240, 240, 240)), m_drawCenterLine(false)
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

void CCanvasStatic::EnableCenterLine(bool enable)
{
    m_drawCenterLine = enable;
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

    if (m_drawCenterLine)
    {
        CPen pen(PS_DASH, 2, RGB(255, 0, 0));
        CPen* pOldPen = dc.SelectObject(&pen);
        const int y = dy + dh / 2;
        const int oldBk = dc.SetBkMode(TRANSPARENT);
        dc.MoveTo(dx, y);
        dc.LineTo(dx + dw, y);
        dc.SetBkMode(oldBk);
        dc.SelectObject(pOldPen);
    }
}

void CCanvasStatic::DrawCurve(CDC& dc, const CRect& rc)
{
    if (m_curveData.empty())
        return;

    const int marginLeft = 64;
    const int marginRight = 16;
    const int marginTop = 40;
    const int marginBottom = 32;
    CRect plotRect = rc;
    plotRect.DeflateRect(marginLeft, marginTop, marginRight, marginBottom);
    if (plotRect.Width() <= 1 || plotRect.Height() <= 1)
        return;

    CPen axisPen(PS_SOLID, 1, RGB(80, 80, 80));
    CPen curvePen(PS_SOLID, 2, RGB(60, 120, 230));

    CPen* pOldPen = dc.SelectObject(&axisPen);
    dc.MoveTo(plotRect.left, plotRect.bottom);
    dc.LineTo(plotRect.right, plotRect.bottom);
    dc.MoveTo(plotRect.left, plotRect.top);
    dc.LineTo(plotRect.left, plotRect.bottom);

    double vmin = std::numeric_limits<double>::max();
    double vmax = -std::numeric_limits<double>::max();
    size_t validCount = 0;
    for (double v : m_curveData)
    {
        if (std::isfinite(v))
        {
            vmin = (std::min)(vmin, v);
            vmax = (std::max)(vmax, v);
            ++validCount;
        }
    }
    if (validCount == 0)
    {
        dc.SelectObject(pOldPen);
        return;
    }
    if (vmax - vmin < 1e-9)
    {
        vmin -= 0.5;
        vmax += 0.5;
    }

    const int textBk = dc.SetBkMode(TRANSPARENT);
    const int tickLen = 4;
    const int xTicks = 5;
    for (int i = 0; i <= xTicks; ++i)
    {
        const double t = static_cast<double>(i) / xTicks;
        const int x = plotRect.left + static_cast<int>(t * plotRect.Width());
        dc.MoveTo(x, plotRect.bottom);
        dc.LineTo(x, plotRect.bottom + tickLen);
        CString label;
        label.Format(L"%d", static_cast<int>(t * (static_cast<int>(m_curveData.size()) - 1)));
        CRect tr(x - 30, plotRect.bottom + tickLen, x + 30, plotRect.bottom + tickLen + 16);
        dc.DrawText(label, &tr, DT_CENTER | DT_TOP | DT_SINGLELINE);
    }

    const int yTicks = 4;
    for (int i = 0; i <= yTicks; ++i)
    {
        const double t = static_cast<double>(i) / yTicks;
        const int y = plotRect.bottom - static_cast<int>(t * plotRect.Height());
        dc.MoveTo(plotRect.left - tickLen, y);
        dc.LineTo(plotRect.left, y);
        const double valCm = vmin + (vmax - vmin) * t;
        const double valDisplay = valCm / 100.0;
        CString label;
        label.Format(L"%.0f", valDisplay);
        CRect tr(plotRect.left - 54, y - 8, plotRect.left - tickLen - 2, y + 8);
        dc.DrawText(label, &tr, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }

    CRect xLabelRect(plotRect.left, plotRect.bottom + tickLen + 16, plotRect.right, plotRect.bottom + tickLen + 32);
    dc.DrawText(L"ÏñËØ", &xLabelRect, DT_CENTER | DT_SINGLELINE | DT_TOP);

    // Position Y-axis title above the axis
    CRect yLabelRect(plotRect.left - 20, plotRect.top - 30, plotRect.left + 80, plotRect.top - 5);
    dc.DrawText(L"¸ß¶È (mm)", &yLabelRect, DT_LEFT | DT_SINGLELINE | DT_BOTTOM);

    dc.SelectObject(&curvePen);

    bool hasPrev = false;
    int prevX = 0, prevY = 0;
    const size_t n = m_curveData.size();
    for (size_t i = 0; i < n; ++i)
    {
        const double v = m_curveData[i];
        if (!std::isfinite(v))
            continue;

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
            dc.MoveTo(prevX, prevY);
            dc.LineTo(x, y);
        }
        prevX = x;
        prevY = y;
    }

    dc.SetBkMode(textBk);
    dc.SelectObject(pOldPen);
}
