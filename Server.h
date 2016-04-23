#pragma once
#include <boost/asio.hpp>
#include <memory>



namespace my_ftp 
{

	class Server_manager;
	class Server
	{

	public:
		Server(boost::asio::ip::tcp::endpoint endpoint);
		~Server();
		void run();
	private:
		void handle_accpet(const boost::system::error_code& error);
		void handle_stop(const boost::system::error_code& error);
		void start_accept();

		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::endpoint endpoint_;
		boost::asio::ip::tcp::acceptor acceptor_;
		std::shared_ptr<Server_manager> server_manager_;

	};
}
