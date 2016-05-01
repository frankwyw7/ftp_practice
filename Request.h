#pragma once

#include <iostream>

namespace my_ftp
{
	static const int P_LEN = 4002;

	enum Request_t
	{
		login = 0,
		query = 1,
		dowld = 2,
		quitt = 3,
		valid = 4,
		ontrn = 5
	};

	//总长度 4 + 4 + 4000 + 2
	//末尾以'\r\n'结尾
	struct my_ftp_protocol
	{
		//header
		char type[5];
		char length[5];

		//data
		char data[P_LEN];

		void decode(char * buf);

		void encode(char * buf);
	};



	
}