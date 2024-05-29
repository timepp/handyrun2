#pragma once

#include "defs.h"
#include "geometry.h"
#include "config.h"
#include <gdiplus.h>
#include <math.h>

template <typename T>
class CCustomBkDlgImpl
{
public:
	BEGIN_MSG_MAP(CCustomBkDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
		MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestory)
	END_MSG_MAP()

	CCustomBkDlgImpl(): m_bk_bmp(nullptr)
	{
	}

	void SetBackground(const background& bk)
	{
		m_bk = bk;
		T * pDlg = static_cast<T *>(this);
		if (pDlg->IsWindow())
		{
			construct_image();
		}
	}

private:
	background m_bk;
	Gdiplus::CachedBitmap* m_bk_bmp;

	void clear_image()
	{
		if (m_bk_bmp)
		{
			delete m_bk_bmp;
			m_bk_bmp = 0;
		}
	}

	void construct_image()
	{
		clear_image();

		T * dlg = static_cast<T *>(this);
		CRect rc;
		dlg->GetClientRect(&rc);

		CClientDC dc(dlg->m_hWnd);
		Gdiplus::Graphics g(dc);

		if (m_bk.mode == background::bm_img)
		{
			// 把图片画到m_bk_bmp上面
			Gdiplus::Bitmap * bkbmp = new Gdiplus::Bitmap(rc.Width(), rc.Height(), &g);
			Gdiplus::Graphics *pG = Gdiplus::Graphics::FromImage(bkbmp);
			Gdiplus::Bitmap * imgbmp = Gdiplus::Bitmap::FromFile(m_bk.back_img.c_str());
			pG->DrawImage(imgbmp, 0, 0, bkbmp->GetWidth(), bkbmp->GetHeight());
			m_bk_bmp = new Gdiplus::CachedBitmap(bkbmp, &g);
			delete imgbmp;
			delete bkbmp;
			delete pG;
		}
		else 
		{
			if (m_bk.color_list.size() >= 2)
			{
				// 把渐变的背景画到m_bk_bmp上面

				Gdiplus::Bitmap * bkbmp = new Gdiplus::Bitmap(rc.Width(), rc.Height(), &g);
				Gdiplus::Graphics* pG = Gdiplus::Graphics::FromImage(bkbmp);
				int a = rc.right - rc.left;
				int b = rc.bottom - rc.top;
				double theta = m_bk.fill_direction * PI / 180;
				double t = a * fabs(cos(theta)) + b * fabs(sin(theta)) + 1;
				CPoint p1, p2;

				switch (m_bk.fill_direction)
				{
				case background::sd_diag:
					p1.SetPoint(rc.left, rc.top);
					p2.SetPoint(rc.right, rc.bottom);
					break;
				case background::sd_rdiag:
					p1.SetPoint(rc.left, rc.bottom);
					p2.SetPoint(rc.right, rc.top);
					break;
				default:
					switch (m_bk.fill_direction / 90)
					{
					case 0:p1.SetPoint(rc.left,  rc.bottom);   break;
					case 1:p1.SetPoint(rc.right, rc.bottom);   break;
					case 2:p1.SetPoint(rc.right, rc.top);      break;
					case 3:p1.SetPoint(rc.left,  rc.top);      break;
					}
					p2.SetPoint(p1.x + int(t * cos(theta)), p1.y - int(t * sin(theta)));
				}

				Gdiplus::LinearGradientBrush brs(Gdiplus::Point(p1.x, p1.y), Gdiplus::Point(p2.x, p2.y), Gdiplus::Color(), Gdiplus::Color());
				size_t N = m_bk.color_list.size();
				Gdiplus::Color * pColor = new Gdiplus::Color[N];
				Gdiplus::REAL * pPos = new Gdiplus::REAL[N];
				for (size_t i = 0; i < N; i++)
				{
					pColor[i].SetFromCOLORREF(m_bk.color_list[i]);
					pPos[i] = i * 1.0f / (N-1);
				}
				brs.SetInterpolationColors(pColor, pPos, static_cast<INT>(N));
				brs.SetGammaCorrection(TRUE);
				pG->FillRectangle(&brs, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
				delete [] pColor;
				delete [] pPos;

				m_bk_bmp = new Gdiplus::CachedBitmap(bkbmp, &g);
				delete bkbmp;
				delete pG;
			}
		}
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		construct_image();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnHrSettingChange(UINT , WPARAM , LPARAM , BOOL& bHandled)
	{
		construct_image();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcDestory(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		clear_image();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnEraseBkGnd(UINT , WPARAM wParam, LPARAM , BOOL& bHandled)
	{
		CDCHandle dc((HDC)wParam);
		T * dlg = static_cast<T *>(this);
		RECT rc;
		dlg->GetClientRect(&rc);

		if (m_bk.mode == background::bm_color)
		{
			if (m_bk.color_list.size() == 0)
			{
				// 让系统去画吧
				bHandled = FALSE;
				return 0;
			}
			else if (m_bk.color_list.size() == 1)
			{
				dc.FillSolidRect(&rc, m_bk.color_list.front());
			}
			else
			{
				Gdiplus::Graphics g(dc);
				g.DrawCachedBitmap(m_bk_bmp, rc.left, rc.top);
			}
		}
		else if (m_bk.mode == background::bm_img)
		{
			Gdiplus::Graphics g(dc);
			g.DrawCachedBitmap(m_bk_bmp, rc.left, rc.top);
		}
		

		bHandled = TRUE;
		return 1;
	}
};
