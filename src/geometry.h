#pragma once
#include <windows.h>
#include <math.h>

// 定义几何相关的辅助函数

namespace geo
{

	class transformer
	{
	public:
		transformer(POINT& center_point) : m_center_point(center_point)
		{
		}
		transformer& c(POINT& pt)
		{
			pt.x -= m_center_point.x;
			pt.y = m_center_point.y - pt.y;
			return *this;
		}
		transformer& rc(POINT& pt)
		{
			pt.x += m_center_point.x;
			pt.y = m_center_point.y - pt.y;
			return *this;
		}
	private:
		POINT m_center_point;
	};

	class rotater
	{
	public:
		rotater(double alpha)
		{
			m_cos_alpha = cos(alpha);
			m_sin_alpha = sin(alpha);
		}
		rotater& c(POINT& pt)
		{
			LONG x = static_cast<LONG>(pt.x * m_cos_alpha + pt.y * m_sin_alpha);
			LONG y = static_cast<LONG>(pt.y * m_cos_alpha - pt.x * m_sin_alpha);
			pt.x = x;
			pt.y = y;
			return *this;
		}
		rotater& rc(POINT& pt)
		{
			m_sin_alpha = -m_sin_alpha;
			c(pt);
			m_sin_alpha = -m_sin_alpha;
			return *this;
		}

	private:
		double m_cos_alpha;
		double m_sin_alpha;
	};

}