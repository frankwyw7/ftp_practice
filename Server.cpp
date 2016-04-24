#include "Server.h"
#include "Session.h"
#include "Server_manager.h"
#include <boost/bind.hpp>
#include "help_function.h"

namespace my_ftp
{
	Server::Server(ip::tcp::endpoint endpoint) 
		:io_service_(),
		 endpoint_(endpoint),
		 acceptor_(io_service_, endpoint_),
		 server_manager_(std::make_shared<Server_manager>())
	{
		start_accept();
	}

	Server::~Server()
	{

	}

	void Server::start_accept()
	{
		pruntime("start_accept");
		ptr.reset(new Session(io_service_, server_manager_));
		acceptor_.async_accept(ptr->get_socket(), boost::bind(&Server::handle_accpet, this, boost::asio::placeholders::error));
	}

	void Server::run()
	{
		io_service_.run();
	}

	void Server::handle_accpet(const boost::system::error_code& error)
	{
		pruntime("handle_accept");
		if (!error)
		{
			server_manager_->add_session(ptr);
		}
		else
		{
			std::cout << error << std::endl;
		}
		start_accept();
	}

	//ÐÅºÅ¿ØÖÆ
	void Server::handle_stop(const boost::system::error_code& error)
	{

	}

}