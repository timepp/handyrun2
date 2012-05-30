#ifndef TP_FORMAT_SHIM_BASE_H_INCLUDED
#define TP_FORMAT_SHIM_BASE_H_INCLUDED


namespace tp
{
	/** ��ʽ���ڴ��Ƭ���ࣺһ���ṩ�����ַ�ָ��ת������ʱ�������������Բ�ͬ��ʽ������ʵ�ֲ�ͬ�Ĺ���
	* ��������ʼ�ǿ���ջ�ϵģ���ջ�ϵĻ�������������Ҫʱ���ͻ����´Ӷ��Ϸ����ڴ�
	* һ����˵�����ڹ��캯�������m_buf. ��������ʵ�ʵĿռ��С����resize.
	* \note ��ʽ���ڴ��Ƭ�������ɱ�����б��еĲ���ʱ���������ʽת����const T*����������ʱ����ǰ��&
	*/
	template <typename T, size_t size>
	class format_shim
	{
	public:
		operator const T * () const  { return m_buf; }
		const T * operator& () const { return m_buf; }

	protected:
		format_shim() : m_buf(m_buf_content), m_buf_size(size)
		{
		}
		~format_shim() 
		{ 
			free(); 
		}

		void resize(size_t new_size) 
		{
			if (new_size > m_buf_size)
			{
				free();
				m_buf = new T[new_size];
				m_buf_size = new_size;
			}
		}

		T * m_buf;
		size_t m_buf_size;

	private:
		T m_buf_content[size];

		void init()
		{
			m_buf = m_buf_content;
			m_buf_size = size;
		}
		void free()
		{
			if (m_buf != m_buf_content)
			{
				delete [] m_buf;
			}
		}
	};
};

#endif