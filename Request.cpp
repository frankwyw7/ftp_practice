#include "Request.h"

namespace my_ftp
{
	//���л��뷴���л�
	std::ostream& operator <<(std::ostream & os, struct my_ftp_protocol& ptl)
	{
		os << ptl.type << ptl.length <<ptl.data;
		return os;
	}


	std::istream& operator >>(std::istream & is, struct my_ftp_protocol& ptl)
	{
		is >> ptl.type>>ptl.length>>ptl.data;
		return is;
	}
}

