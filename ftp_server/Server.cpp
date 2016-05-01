#include "Server.h"
#include "Session.h"
#include "Server_manager.h"
#include "../common/help_function.h"
#include <boost/bind.hpp>

namespace my_ftp
{
	Server::Server(ip::tcp::endpoint endpoint) 
		:io_service_(),
		 endpoint_(endpoint),
		 acceptor_(io_service_, endpoint_),
		 signals_(io_service_),
		 server_manager_(std::make_shared<Server_manager>())
	{
		signals_.add(SIGINT);
		signals_.add(SIGTERM);
	#if defined(SIGQUIT)
		signals_.add(SIGQUIT);
	#endif
		start_accept();
		signals_.async_wait(boost::bind(&Server::handle_stop, this, 
			boost::asio::placeholders::error));		
	}

	Server::~Server()
	{

	}

	void Server::start_accept()
	{
		pruntime("Server start_accept");
		ptr.reset(new Session(io_service_, server_manager_));
		acceptor_.async_accept(ptr->get_socket(), 
			boost::bind(&Server::handle_accpet, this, boost::asio::placeholders::error));
	}

	void Server::run()
	{
		io_service_.run();
	}

	void Server::handle_accpet(const boost::system::error_code& error)
	{
		pruntime("Server handle_accept");
		if (!error)
		{
			server_manager_->add_session(ptr);
			start_accept();
		}
		else
		{
			std::cout << error << std::endl;
		}
		
	}

	//ÐÅºÅ¿ØÖÆ
	void Server::handle_stop(const boost::system::error_code& error)
	{
		acceptor_.close();
		server_manager_->delete_all();
	}

}