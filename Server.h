#pragma once
#include <boost/asio.hpp>
#include <memory>

using namespace boost::asio;

namespace my_ftp 
{
	class Server_manager;
	class Server
	{

	public:
		Server(ip::tcp::endpoint endpoint);
		~Server();
		void run();
	private:
		void handle_accpet(const boost::system::error_code& error);
		void handle_stop(const boost::system::error_code& error);

		ip::tcp::endpoint endpoint_;
		ip::tcp::acceptor acceptor_;
		io_service io_service_;
		std::shared_ptr<Server_manager> server_manager_;

	};
}
