#include "Request.h"
#include <sstream>

namespace my_ftp
{
	//���л��뷴���л�
	void my_ftp_protocol::decode(char * buf)
	{
		std::istringstream iss(buf);
		iss.read(type, 4);
		type[5] = '\0';
		iss.read(length, 4);
		length[5] = '\0';
		iss.read(data, atoi(length)+2);
	}

	void my_ftp_protocol::encode(char * buf)
	{
		strcpy(buf, type);
		strcpy(buf + 4, length);
		strcpy(buf + 8, data);
	}
}

