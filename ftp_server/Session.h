#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <boost/filesystem.hpp>
#include <fstream>
#include <mutex>

namespace my_ftp
{
	using namespace boost::asio;

	class Header_processor;
	class Server_manager;

	class Session : public std::enable_shared_from_this<Session>
	{
		using socket = ip::tcp::socket;
		static const int datalen = 5120;
		
	public:
		Session(io_service& io_service, std::shared_ptr<Server_manager> server_manager);
		~Session();
		socket& get_socket();
		void start();
		void stop();

		bool set_downld_flie(std::string file);
		std::shared_ptr<Server_manager> get_manager();
	private:
		void handle_read(const boost::system::error_code& error);		
		void handle_write(const boost::system::error_code& error);
		void handle_go_write(const boost::system::error_code& error);
		void handle_file_write(const boost::system::error_code& error);
		bool receive_trnbk();
		void check_deadline();

		bool go_on;

		std::mutex mutex_;
		std::fstream fs;

		char input_buffer_[4096];
		char output_buffer_[4096];

		socket socket_;
		std::shared_ptr<Server_manager> server_manager_;
		std::unique_ptr<Header_processor> header_processor_;
		boost::filesystem::path path_;
		boost::asio::deadline_timer deadline_timer_;

	};
}