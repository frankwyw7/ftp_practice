#include "Server.h"
#include "Session.h"
#include "Server_manager.h"
#include <boost/bind.hpp>

namespace my_ftp
{
	Server::Server(ip::tcp::endpoint endpoint) :endpoint_(endpoint),
		io_service_(),
		acceptor_(io_service_, endpoint_),
		server_manager_(std::make_shared<Server_manager>())
	{
		
	}

	Server::~Server()
	{

	}

	void Server::run()
	{
		auto ptr = std::make_shared<Session>(io_service_, server_manager_);
		acceptor_.async_accept(ptr->get_socket(), boost::bind(&Server::handle_accpet, this, boost::asio::placeholders::error));
	}

	void Server::handle_accpet(const boost::system::error_code& error)
	{
		auto ptr = std::make_shared<Session>(io_service_, server_manager_);
		acceptor_.async_accept(ptr->get_socket(), boost::bind(&Server::handle_accpet, this, boost::asio::placeholders::error));
	}

	//ÐÅºÅ¿ØÖÆ
	void Server::handle_stop(const boost::system::error_code& error)
	{

	}

}