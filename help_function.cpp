#include "help_function.h"

namespace my_ftp
{
	void perr(char* chs)
	{
		std::cerr << chs << " error" << std::endl;
	}

#ifdef DEBUG
	void pruntime(char* chs)
	{
		std::cout << chs << " run " << std::endl;
	}
#else
	void pruntime(char* chs)
	{

	}
#endif

	void data_int_to_char(int len, char* tmp)
	{
		if (tmp == nullptr)
			return;

		for (int i = 3; i >= 0; --i, len /= 10)
		{
			int n = len % 10;

			tmp[i] = n >= 0 ? '0' + n : '0';
		}
		tmp[4] = '\0';
	}
}