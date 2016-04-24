#include "Session.h"
#include "Header_processor.h"
#include "Server_manager.h"
#include "help_function.h"

#include <fstream>
#include <iostream>
#include <boost/bind.hpp>


namespace my_ftp
{
	using socket = ip::tcp::socket;

	Session::Session(io_service& io_service_, std::shared_ptr<Server_manager> server_manager__)
		:socket_(io_service_), server_manager_(server_manager__), 
		path_(boost::filesystem::current_path() /= "FTP"),
		header_processor_(std::make_unique<Header_processor>(path_)),
		deadline_timer_(io_service_),
		go_on(true),
		mutex_()
	{ }

	Session::~Session() 
	{
		pruntime("~Session des");
	}

	void Session::stop()
	{
		char over[] = "quit now";
		write(socket_, buffer(over));

		{
			std::lock_guard<std::mutex> lock(mutex_);
			go_on = false;
		}
		socket_.close();
	}

	socket& Session::get_socket()
	{
		return socket_;
	}

	void Session::start()
	{
		//不能使用std::bind
		pruntime("Session start");
		deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
		async_read(socket_, buffer(input_buffer_), transfer_at_least(6),
			boost::bind(&Session::handle_read, shared_from_this(), boost::asio::placeholders::error));
		deadline_timer_.async_wait(boost::bind(&Session::check_deadline, shared_from_this()));

	}

	//write缓冲区最大字节数以内的数据
	void Session::handle_read(const boost::system::error_code& error)
	{		
		
		pruntime("Session handle_read");
		
		if (!error && go_on)
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			int result = 0;
			result = header_processor_->procees_request(input_buffer_, shared_from_this(), output_buffer_);

			
			if (result == -1)
			{
				this->get_manager()->delete_session(shared_from_this());
				return;
			}
			if (result == 1)
			{
				//fstream读取一定字节的位移
				//不能使用std::bind
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
			perr("Session handle read");
			std::cerr << error << std::endl;
		}

	}
	void Session::handle_write(const boost::system::error_code& error)
	{
		pruntime("Session handle_write");
		if (!error && go_on)
		{
			start();
		}
		else
		{
			perr("Session handle write");
		}
	}

	//再次wirite一定量的数据，如果尚未传输完成，递归（传递fs目前读取的位移）
	void Session::handle_go_write(const boost::system::error_code& error, std::FILE* f)
	{
		pruntime("Session handle_go_write");
		if (!error && go_on)
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			int n = std::fread(&output_buffer_[0], sizeof(char), sizeof(output_buffer_) - 1, f);
			if (n < sizeof(output_buffer_) - 1)
			{
				output_buffer_[n] == '\0';
				async_write(socket_, buffer(output_buffer_), transfer_at_least(1),
					boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error));
				fclose(f);
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
			perr("Session handle_go_write");
			fclose(f);
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


	void Session::check_deadline()
	{		
		if (deadline_timer_.expires_at() <= deadline_timer::traits_type::now())
		{
			{
				std::lock_guard<std::mutex> lock(mutex_);
				go_on = false;
			}
			
			server_manager_->delete_session(shared_from_this());
			return;
		}
		deadline_timer_.async_wait(boost::bind(&Session::check_deadline, shared_from_this()));
	}
}