#ifndef TP_FORMAT_SHIM_BASE_H_INCLUDED
#define TP_FORMAT_SHIM_BASE_H_INCLUDED


namespace tp
{
	/** 格式化内存垫片基类：一个提供了向字符指针转化的临时对象，派生子类以不同方式构造以实现不同的功能
	* 缓冲区初始是开在栈上的，当栈上的缓冲区不满足需要时，就会重新从堆上分配内存
	* 一般来说子类在构造函数里填充m_buf. 子类需用实际的空间大小调用resize.
	* \note 格式化内存垫片在用作可变参数列表中的参数时，最好先显式转换成const T*，或者在临时对象前加&
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