#include "Session.h"
#include "Header_processor.h"
#include "Server_manager.h"
#include <fstream>
#include <iostream>
#include <boost/bind.hpp>

namespace my_ftp
{
	using socket = ip::tcp::socket;

	Session::Session(io_service& io_service_, std::shared_ptr<Server_manager> server_manager__)
		:socket_(io_service_), server_manager_(server_manager__), 
		path_(boost::filesystem::current_path() /= "FTP"),
		header_processor_(std::make_unique<Header_processor>(path_))
		
	{ }

	Session::~Session() 
	{
		
	}

	void Session::stop()
	{
		char over[] = "quit now";
		write(socket_, buffer(over));
		socket_.close();
	}

	socket& Session::get_socket()
	{
		return socket_;
	}

	void Session::start()
	{
		//不能使用std::bind
		async_read(socket_, buffer(input_buffer_), transfer_all(),
			boost::bind(&Session::handle_read, shared_from_this(), boost::asio::placeholders::error));
	}

	//write缓冲区最大字节数以内的数据
	void Session::handle_read(const boost::system::error_code& error)
	{		
		if (!error)
		{
			int result = 0;
			result = header_processor_->procees_request(input_buffer_, shared_from_this(), output_buffer_);
			if(result == -1)
				return;
			else if (result == 1)
			{
				//fstream读取一定字节的位移
				//不能使用std::bind
				std::FILE* f = std::fopen(downld_file.c_str(), "r");
				std::fwrite(&output_buffer_[0], sizeof(char), sizeof(output_buffer_) - 1, f);
				async_write(socket_, buffer(output_buffer_), transfer_all(),
					boost::bind(&Session::handle_translate, shared_from_this(), 
					boost::asio::placeholders::error, f));
			}
			else if (result == 0)
			{
				async_write(socket_, buffer(output_buffer_), transfer_all(),
					boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error));
			}
			
		}

	}
	void Session::handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "process success" << std::endl;
		}
	}

	//再次wirite一定量的数据，如果尚未传输完成，递归（传递fs目前读取的位移）
	void Session::handle_translate(const boost::system::error_code& error, std::FILE* f)
	{
		if (!error)
		{
			async_write(socket_, buffer(output_buffer_), transfer_all(),
				boost::bind(&Session::handle_translate, shared_from_this(),
				boost::asio::placeholders::error, f));
			std::cout << "process success" << std::endl;
		}
	}

	

	std::shared_ptr<Server_manager> Session::get_manager()
	{
		return server_manager_;
	}

	void Session::set_downld_flie(std::string file)
	{
		downld_file = file;
	}
}