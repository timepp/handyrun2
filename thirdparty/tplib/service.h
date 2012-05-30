#ifndef TP_SERVICE_H_INCLUDED
#define TP_SERVICE_H_INCLUDED

namespace tp
{
	template <typename T>
	class service
	{
	public:
		static T& instance()
		{
			static T t_;
			return t_;
		}
	};
}

#endif