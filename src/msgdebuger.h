#pragma once
// 辅助调试用的消息记录器


template <typename T>
class CMsgDebuger
{
public:
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& , DWORD  = 0)
	{
		fprintf(m_fp, "%-10x%-10x%-10x%-10x\n", uMsg, hWnd, wParam, lParam);
		fflush(m_fp);
		return FALSE;
	}

	CMsgDebuger()
	{
		m_fp = fopen("e:\\a.txt", "w");
	}
	~CMsgDebuger()
	{
		if (m_fp) fclose(m_fp);
	}

private:
	FILE * m_fp;
};