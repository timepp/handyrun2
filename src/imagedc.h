#pragma once

class CImageDC : public CDC
{
public:
// Data members
	HDC m_hDCOriginal;
	RECT m_rcPaint;
	HBITMAP m_hBmpOld;
	BITMAP m_bmp_info;

// Constructor/destructor
	CImageDC(HDC hDC, RECT& rcPaint, HBITMAP bmp) : m_hDCOriginal(hDC), m_hBmpOld(NULL)
	{
		m_rcPaint = rcPaint;
		CreateCompatibleDC(m_hDCOriginal);
		ATLASSERT(m_hDC != NULL);
		//m_bmp.CreateCompatibleBitmap(m_hDCOriginal, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top);
		//ATLASSERT(m_bmp.m_hBitmap != NULL);
		m_hBmpOld = SelectBitmap(bmp);
		::GetObject(bmp, sizeof(m_bmp_info), &m_bmp_info);
		SetViewportOrg(-m_rcPaint.left, -m_rcPaint.top);
	}

	~CImageDC()
	{
		::StretchBlt(m_hDCOriginal, m_rcPaint.left, m_rcPaint.top, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top, 
			m_hDC, 0, 0, m_bmp_info.bmWidth, m_bmp_info.bmHeight, SRCCOPY);
		//::BitBlt(m_hDCOriginal, m_rcPaint.left, m_rcPaint.top, m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top, m_hDC, m_rcPaint.left, m_rcPaint.top, SRCCOPY);
		SelectBitmap(m_hBmpOld);
	}
};