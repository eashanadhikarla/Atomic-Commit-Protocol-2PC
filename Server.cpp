#include <iostream>
#include <boost/asio.hpp>
#include <fstream>
#include <regex>
#include <thread>
#include <string>

#include "hash.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;
#define BUF_SIZE 256

myHash_List mylist = init_hashlist();

//session
class Session : public std::enable_shared_from_this<Session> {

public:

	Session(tcp::socket socket) : socket_(std::move(socket)) {
	}

	void Start() {

		DoRead();
	}

	void DoRead() {
		auto self(shared_from_this());
		socket_.async_read_some(
			boost::asio::buffer(buffer_),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					string ht_message;
					string data_buf;
					int key;
					string value;

					for(int i=0;i<length;++i){
						data_buf += buffer_[i];
					}
					// PUT
					if (data_buf[0] == 'p'){
					    string str_key;	
						int index = data_buf.find(",");

						for(int i=4; i<index; i++){
							str_key += buffer_[i];
						}
						
						for(int i=index+1; i<length-1; i++){
							value += buffer_[i];
						}
						// Saving PUT in hash message for hashing.
						ht_message = put(mylist, str_key, value);

						if(ht_message.length()>length){
						    	length = ht_message.length();
						}
						for (int i = 0; i < length; ++i){
						    	buffer_[i] = ht_message[i];
						}

						cout << endl;
					}

					// Multi PUT
					else if (data_buf[0] == 'm'){
						int index = data_buf.find(",");
						string str_key1 = data_buf.substr(5, index-5);
						int prev = index;
						index = data_buf.find(",", prev+1);
						string str_value1 = data_buf.substr(prev+1, index-prev);

						prev = index;
						index = data_buf.find(",", prev+1);
						string str_key2 = data_buf.substr(prev+1, index-prev);
						prev = index;
						index = data_buf.find(",", prev+1);
						string str_value2 = data_buf.substr(prev+1, index-prev);

						prev = index;
						index = data_buf.find(",", prev+1);
						string str_key3 = data_buf.substr(prev+1, index-prev);
						prev = index;
						index = data_buf.find(")", prev+1);
						string str_value3 = data_buf.substr(prev+1, index-prev);
						
						// Saving M-PUT in hash message for hashing.
						ht_message = mput(mylist, str_key1, str_value1, str_key2, str_value2, str_key3, str_value3);

						if(ht_message.length()>length){
						    	length = ht_message.length();
						}
						for (int i = 0; i < length; ++i){
						    	buffer_[i] = ht_message[i];
						}

					}

					// GET
					else{
						string str_key;	
						for(int i=4; i<length-1; i++){
							str_key += buffer_[i];
						}
						// Saving GET in hash message for hashing.
						ht_message = get(mylist, str_key);

						if(ht_message.length()>length){
						    	length = ht_message.length();
						}
						for (int i = 0; i < length; ++i){
						    	buffer_[i] = ht_message[i];
						}

						cout << endl;
					}

					DoWrite(length);
				}
			});
	}
	void DoWrite(std::size_t length) {
		auto self(shared_from_this());
		boost::asio::async_write(
			socket_,
			boost::asio::buffer(buffer_, length),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					DoRead();
				}
			});
	}

private:
	tcp::socket socket_;
	std::array<char, BUF_SIZE> buffer_;
};



// Server
class Server {

public:
	Server(boost::asio::io_context& ioc, std::uint16_t port)
	: acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
		DoAccept();
	}

private:
	void DoAccept() {
		std::cout << "Waiting for asynchronous clients to connect...." << std::endl;
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if (!ec) {
					cout << "Connected." << endl;
					std::make_shared<Session>(std::move(socket))->Start();
				}
				DoAccept();
			});
	}

private:
	tcp::acceptor acceptor_;
};

// Driver Code for Server.
int main(int argc, char* argv[]) {

	std::uint16_t port = 8080;

	boost::asio::io_context ioc;
	Server server(ioc, port);

	ioc.run();
	return 0;
}


