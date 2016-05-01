#include "../common/Request.h"
#include <sstream>

namespace my_ftp
{
	//反序列化
	void my_ftp_protocol::decode(char * buf)
	{
		std::istringstream iss(buf);
		iss.read(type, 4);
		type[4] = '\0';
		iss.read(length, 4);
		length[4] = '\0';
		iss.read(data, atoi(length)+2);
	}
	//序列化
	void my_ftp_protocol::encode(char * buf)
	{
		strcpy(buf, type);
		strcpy(buf + 4, length);
		strcpy(buf + 8, data);
	}

	void my_ftp_protocol::decode_header(char* buf)
	{
		std::istringstream iss(buf);
		iss.read(type, 4);
		type[4] = '\0';
		iss.read(length, 4);
		length[4] = '\0';
	}

}

