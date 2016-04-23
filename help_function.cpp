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
}