#pragma once
#include <iostream>

namespace my_ftp
{
	void perr(char* chs);

	void pruntime(char* chs);

	void data_int_to_char(int len, char* tmp);

	void add_buffer_over(char* buf, int n);
}

#define DEBUG