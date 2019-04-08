#include <iostream>
#include <boost/asio.hpp>
#include <fstream>
#include <regex>
#include <string>
#include "hash.h"

#include <boost/thread/thread.hpp>
#include <thread>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;
#define BUF_SIZE 256

myHash_List mylist = init_hashlist();

//session
class Session : public std::enable_shared_from_this<Session>{
public:
	Session(tcp::socket socket) : socket_(std::move(socket)){
	}
	void Start(){
		DoRead();
	}
	void DoRead() {
		auto self(shared_from_this());
		socket_.async_read_some(
			boost::asio::buffer(buffer_),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					string ht_message,data_buf,value,str_key;
					int key;

					for(int i=0;i<length;++i){
						data_buf += buffer_[i];
					}
					// PUT
					if (data_buf[0] == 'p'){
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

					// Try Locking
					else if(data_buf[0] == 't'){
						for(int i=2; i<length-1; i++){
							str_key += buffer_[i];
						}
						ht_message = mput_try(mylist, str_key);
						if(ht_message.length()>length){
						    	length = ht_message.length();
						}
						for (int i = 0; i < length; ++i){
						    	buffer_[i] = ht_message[i];
						}
						cout << endl;
					}

					// Commit 2 Phase
					else if(data_buf[0] == 'c'){
						int index = data_buf.find(",");

						for(int i=7; i<index; i++){
							str_key += buffer_[i];
						}
						for(int i=index+1; i<length-1; i++){
							value += buffer_[i];
						}
						// Saving PUT in hash message for hashing.
						ht_message = mput_commit(mylist, str_key, value);

						if(ht_message.length()>length){
						    	length = ht_message.length();
						}
						for (int i = 0; i < length; ++i){
						    	buffer_[i] = ht_message[i];
						}
						cout << endl;
					}
					// Abort / Unlock all
					else if(data_buf[0] == 'a'){	
						for(int i=2; i<length-1; i++){
							str_key += buffer_[i];
						}
						ht_message = mput_abort(mylist, str_key);
						// if(ht_message.length()>length){
						//     	length = ht_message.length();
						// }
						// for (int i = 0; i < length; ++i){
						//     	buffer_[i] = ht_message[i];
						// }
						// cout << endl;
					}

					// GET
					else{
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
class Server{
public:
	Server(boost::asio::io_context& io_service, std::uint16_t port)
	: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
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

// class handler(){
// 	public:
// 		//Server 
// 		//Session
// 		~handler();
// 	private:
// }

// Driver Code for Server.
int main(int argc, char* argv[]) {

	std::uint16_t port = 9000;
	//handler handle(io_service);

	boost::asio::io_context io_service;
	Server server(io_service, port);

	io_service.run();
	return 0;
}