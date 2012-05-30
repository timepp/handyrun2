#pragma once

#include <string>

// todo: replace __int32

namespace tp
{
	bool is_path_seperator(wchar_t ch)
	{
		return ch == L'\\' || ch == L'/';
	}
	std::wstring parent_path(const std::wstring& path)
	{
		bool has_normal_char = false;
		for (size_t i = 0; i < path.length(); i++)
		{
			size_t pos = path.length() - i - 1;
			if (!is_path_seperator(path[pos]))
			{
				has_normal_char = true;
			}
			else if (has_normal_char)
			{
				return path.substr(0, pos);
			}
		}
		return L"";
	}

	struct algo
	{
		static unsigned __int32 crc32(const void * buf, size_t len)
		{
			return crc32_update(0xFFFFFFFF, buf, len) ^ 0xFFFFFFFF;
		}
		static void crc32_make_table(unsigned __int32 (&ct)[256], unsigned __int32 polynom)
		{
			for (unsigned __int32 n = 0; n < 256; n++) 
			{
				unsigned __int32 c = n;
				for (size_t k = 0; k < 8; k++) 
				{
					if (c & 1)
						c = polynom ^ (c >> 1);
					else
						c = c >> 1;
				}
				ct[n] = c;
			}
		}
		static unsigned __int32 crc32_update(unsigned __int32 old_crc, const void * buf, size_t len)
		{
			static bool crc_table_computed = false;
			static unsigned __int32 crc_table[256];

			if (!crc_table_computed)
			{
				crc32_make_table(crc_table, 0xedb88320);
				crc_table_computed = true;
			}

			const unsigned char * byte_buf = static_cast<const unsigned char*>(buf);
			for (size_t i = 0; i < len; i++) 
			{
				old_crc = crc_table[(old_crc ^ byte_buf[i]) & 0xff] ^ (old_crc >> 8);
			}

			return old_crc;
		}

	private:

	};
}