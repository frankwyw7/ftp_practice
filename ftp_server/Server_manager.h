#pragma once
#include <set>
#include <memory>

namespace my_ftp
{
	class Session;
	class Server_manager
	{
	public:
		Server_manager();
		~Server_manager();

		void add_session(std::shared_ptr<Session> session);
		void delete_session(std::shared_ptr<Session> session);
		void delete_all();
	private:
		std::set<std::shared_ptr<Session>> sessions;
	};
}