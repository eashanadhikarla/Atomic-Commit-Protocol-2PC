#include <iostream>
#include <boost/asio.hpp>
#include <fstream>
#include <regex>
#include <string>
#include <array>
#include "hash.h"
#include "asio_thread_pool.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

myHash_List mylist = init_hashlist();

class Session : public std::enable_shared_from_this<Session> 
{
public:
    Session(boost::asio::io_service &io_service) : socket_(io_service),
          strand_ (io_service){ 
    }
    tcp::socket &socket() {  return socket_;  }
    void start() { doRead();  
    }
private:
    void doRead()
    {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buffer_, buffer_.size()),
            strand_.wrap([this, self](boost::system::error_code ec, 
                                      std::size_t length){
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
                    
                    doWrite(length);  
                }
            }));
    }
    void doWrite(std::size_t length)
    {
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_, boost::asio::buffer(buffer_, length),
            strand_.wrap([this, self](boost::system::error_code ec,
                                      std::size_t /* length */){
                            if (!ec) {  
                                doRead();  
                            }
                         }));
    }
private:
    tcp::socket socket_;
    boost::asio::io_service::strand strand_;
    std::array<char, 8192> buffer_;
};
class Server
{
public:
    Server(boost::asio::io_service &io_service, unsigned short port)
        : io_service_ (io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port)){
        doAccept();
    }

    void doAccept(){
        std::cout << "Waiting for asynchronous clients to connect...." << std::endl;
        auto conn = std::make_shared<Session>(io_service_);
        acceptor_.async_accept(conn->socket(),
                               [this, conn](boost::system::error_code ec){
                                if (!ec) {  
                                    std::cout << "Connected." << std::endl;
                                    conn->start();  
                                }
                                this->doAccept();
                            });
    }
    
private: 
    boost::asio::io_service &io_service_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]){
    AsioThreadPool pool(8);
	unsigned short port = 9000;

    // boost::asio::io_context io_service;
    // Server server(io_service, port);
    // io_service.run();
    Server server(pool.getIOService(), port);
    pool.stop();
    
    return 0;
}