#include "Server.h"
#include <boost/asio.hpp>
#include <iostream>


int main()
{
	//test port
	static const int port = 10001;
	try
	{
		boost::asio::ip::tcp::endpoint ed(boost::asio::ip::tcp::v4(), port);

		my_ftp::Server server(ed, 4);

		server.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}
	
}