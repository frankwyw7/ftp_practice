#pragma once
#include <boost/asio.hpp>
#include <memory>

namespace my_ftp 
{

	class Server_manager;
	class Session;
	class Server
	{

	public:
		Server(boost::asio::ip::tcp::endpoint endpoint, int thread_pool_size_);
		~Server();
		void run();
		void start_accept();
	private:
		void handle_accpet(const boost::system::error_code& error);
		void handle_stop(const boost::system::error_code& error);
		

		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::endpoint endpoint_;
		boost::asio::ip::tcp::acceptor acceptor_;
		std::shared_ptr<Server_manager> server_manager_;
		std::shared_ptr<Session> ptr;
		boost::asio::signal_set signals_;
		int thread_pool_size;

	};
}
