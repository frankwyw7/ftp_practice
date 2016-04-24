#include "Session.h"
#include "Header_processor.h"
#include "Server_manager.h"
#include <fstream>
#include <iostream>
#include <boost/bind.hpp>
#include "help_function.h"

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
		pruntime("~Session des");
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
		//����ʹ��std::bind
		pruntime("start");
		async_read(socket_, buffer(input_buffer_), transfer_at_least(6),
			boost::bind(&Session::handle_read, shared_from_this(), boost::asio::placeholders::error));
	}

	//write����������ֽ������ڵ�����
	void Session::handle_read(const boost::system::error_code& error)
	{		
		pruntime("handle_read");
		if (!error)
		{
			int result = 0;
			result = header_processor_->procees_request(input_buffer_, shared_from_this(), output_buffer_);

			
			if (result == -1)
			{
				this->get_manager()->delete_session(shared_from_this());
				return;
			}
			if (result == 1)
			{
				//fstream��ȡһ���ֽڵ�λ��
				//����ʹ��std::bind
				std::FILE* f = std::fopen(downld_file.c_str(), "r");

				int n = std::fread(&output_buffer_[0], sizeof(char), sizeof(output_buffer_) - 1, f);
				if (n < sizeof(output_buffer_) - 1)
				{
					output_buffer_[n] == '\0';
					async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
						boost::bind(&Session::handle_write, shared_from_this(), 
							boost::asio::placeholders::error));
				}
				else
				{
					async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
						boost::bind(&Session::handle_go_write, shared_from_this(),
							boost::asio::placeholders::error, f));
				}

				
			}
			else if (result == 0)
			{
				async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
					boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error));
			}
			
		}
		else
		{
			perr("handle read");
			std::cout << error;
		}

	}
	void Session::handle_write(const boost::system::error_code& error)
	{
		pruntime("handle_write");
		if (!error)
		{
			start();
		}
		else
		{
			perr("handle write");
		}
	}

	//�ٴ�wiriteһ���������ݣ������δ������ɣ��ݹ飨����fsĿǰ��ȡ��λ�ƣ�
	void Session::handle_go_write(const boost::system::error_code& error, std::FILE* f)
	{
		pruntime("handle_go_write");
		if (!error)
		{
			int n = std::fread(&output_buffer_[0], sizeof(char), sizeof(output_buffer_) - 1, f);
			if (n < sizeof(output_buffer_) - 1)
			{
				output_buffer_[n] == '\0';
				async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
					boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error));
			}
			else
			{
				async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
					boost::bind(&Session::handle_go_write, shared_from_this(),
						boost::asio::placeholders::error, f));
			}

		}
		else
		{
			perr("handle_go_write");
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