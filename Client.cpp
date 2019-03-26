#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <time.h>
#include <boost/asio.hpp>
#include <fstream>
#include <string>
#include <regex>
#include <thread>
#include <vector>

using namespace std;
using namespace boost::asio;
#define BUF_SIZE 256

boost::asio::io_service io_serv;
unsigned int hashIdx;
bool flag;
string data, key1, key2, key3, value1, value2, value3;
int put_true,put_false,get_null,get_val,mput_true,mput_fail;
size_t sum_bytes = 0;
double sum_ex_time = 0;
double sum_latency = 0;

/* Auto-generation of transactions. */
string get_rand(){
	int key_range = 1000;
	int random = rand() % 100;
	int key = random % key_range;
	string key_str = to_string(key);

  return key_str;
}

// Client
boost::asio::ip::tcp::socket connect_to_server(std::string hostname, std::string port) {
	ip::tcp::resolver resolver(io_serv);
	ip::tcp::resolver::query query(hostname, port);
	ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	ip::tcp::socket socket(io_serv);
	connect(socket, endpoint_iterator);
	return socket;
}

void get_client(boost::asio::ip::tcp::socket &socket) {

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	// Start Time.
	gettimeofday(&start_time, nullptr);

	// Write data to the Socket and send it to the Server.
	boost::system::error_code ignored_error; 
	write(socket, buffer(data), ignored_error); 
	xBytes += data.length();

	gettimeofday(&start_time_s, nullptr);
     
	//std::cout << "Server: ";
	size_t recd = 0;
	while (recd < data.length()) {
		char buf[256];
		string buf_str;
		// Recieve and read reply from the Server.
		size_t len = socket.read_some(boost::asio::buffer(buf), ignored_error); 
		if (len > 0) {
			recd += len;
			buf[len] = 0;
			cout << buf;

			for(int i=0;i<4;++i){
				buf_str += buf[i];
			}
			if(buf_str == "Null"){
				get_null++;
			}
			else{
				get_val++;
			}
		} 
	}
	gettimeofday(&end_time_s, nullptr);
	std::cout << std::endl;
	// End Time
	gettimeofday(&end_time, nullptr);
	double ex_time = (end_time.tv_sec - start_time.tv_sec)*1000000+(end_time.tv_usec - start_time.tv_usec);
	ex_time /= 1000000;
	double latency = (end_time_s.tv_sec - start_time_s.tv_sec)*1000000+(end_time_s.tv_usec - start_time_s.tv_usec);
	latency /= 1000000;
	sum_bytes += xBytes;
	sum_ex_time += ex_time;
	sum_latency += latency;

	std::cout << std::endl;
}

void put_client(boost::asio::ip::tcp::socket &socket) {

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	// Start Time.
	gettimeofday(&start_time, nullptr);

	// Write data to the Socket and send it to the Server.
	boost::system::error_code ignored_error; 
	write(socket, buffer(data), ignored_error); 
	xBytes += data.length();

	gettimeofday(&start_time_s, nullptr);

	// std::cout << "Server: ";
	size_t recd = 0;
	while (recd < data.length()) {
		char buf[256];
		string buf_str;
		// Recieve and read reply from the Server.
		size_t len = socket.read_some(boost::asio::buffer(buf), ignored_error); 
		if (len > 0) {
			recd += len;
			buf[len] = 0;
			cout << buf;

			for(int i=0;i<4;++i){
				buf_str += buf[i];
			}
			// Counting the Results for performance measure.
			if(buf_str == "true"){
				put_true++;
				// Continue with next transaction if current is successful
				// flag = 0;
			}
			else if(buf_str == "false"){
				put_false++;
				// Retry if failed
				// flag = 1;
			}
		} 
	}
	gettimeofday(&end_time_s, nullptr);
	std::cout << std::endl;
	// End Time
	gettimeofday(&end_time, nullptr);
	double ex_time = (end_time.tv_sec - start_time.tv_sec)*1000000+(end_time.tv_usec - start_time.tv_usec);
	ex_time /= 1000000;
	double latency = (end_time_s.tv_sec - start_time_s.tv_sec)*1000000+(end_time_s.tv_usec - start_time_s.tv_usec);
	latency /= 1000000;
	sum_bytes += xBytes;
	sum_ex_time += ex_time;
	sum_latency += latency;

	std::cout << std::endl;
}

void mput_client(boost::asio::ip::tcp::socket &socket) {

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	// Start Time.
	gettimeofday(&start_time, nullptr);

	// Write data to the Socket and send it to the Server.
	boost::system::error_code ignored_error; 
	write(socket, buffer(data), ignored_error); 
	xBytes += data.length();

	gettimeofday(&start_time_s, nullptr);

	// std::cout << "Server: ";
	size_t recd = 0;
	while (recd < data.length()) {
		char buf[256];
		string buf_str;
		// Recieve and read reply from the Server.
		size_t len = socket.read_some(boost::asio::buffer(buf), ignored_error); 
		if (len > 0) {
			recd += len;
			buf[len] = 0;
			cout << buf;

			for(int i=0;i<4;++i){
				buf_str += buf[i];
			}
			// Counting the Results for performance measure.
			if(buf_str == "true"){
				mput_true++;
				// Continue with next transaction if current is successful
				// flag = 0;
			}
			else if(buf_str == "false"){
				mput_fail++;
				// Retry if failed
				// flag = 1;
			}
		} 
	}
	gettimeofday(&end_time_s, nullptr);
	std::cout << std::endl;
	// End Time
	gettimeofday(&end_time, nullptr);
	double ex_time = (end_time.tv_sec - start_time.tv_sec)*1000000+(end_time.tv_usec - start_time.tv_usec);
	ex_time /= 1000000;
	double latency = (end_time_s.tv_sec - start_time_s.tv_sec)*1000000+(end_time_s.tv_usec - start_time_s.tv_usec);
	latency /= 1000000;
	sum_bytes += xBytes;
	sum_ex_time += ex_time;
	sum_latency += latency;

	std::cout << std::endl;
}

int main(int argc, char *argv[]) {

	srand(time(NULL));
	string ports = "8080"; 
	// string server_ip_connect = "localhost";
	std::vector<string> servers_ip;

	// Read the list of Hostnames from a text file.
	ifstream file;
	file.open("Hostnames.txt", ios::in);

	string eachLine;
	while(getline(file,eachLine)){
		if(eachLine.empty()){
			continue;
		}
		string the_ip = eachLine.substr(8);
		servers_ip.push_back(the_ip);
	}
    
	file.close();

    // Opening 5 sockets for connecting it to 5 nodes.
	// auto socket = connect_to_server(server_ip_connect, ports);
	auto socket1 = connect_to_server(servers_ip[0], ports);
	auto socket2 = connect_to_server(servers_ip[1], ports); 
	auto socket3 = connect_to_server(servers_ip[2], ports);
	auto socket4 = connect_to_server(servers_ip[3], ports);
	auto socket5 = connect_to_server(servers_ip[4], ports);

	// Vector of Sockets.
	std::vector<boost::asio::ip::tcp::socket*> socket_list;
	socket_list.push_back(&socket1);
	socket_list.push_back(&socket2);
	socket_list.push_back(&socket3);
	socket_list.push_back(&socket4);
	socket_list.push_back(&socket5);
	
    // read from stdin as long as it isn't EOF, send to server, print reply
	
	for (int i = 0; i < 100; i++){
        hash<string> hash_fn;
		size_t hash_key = hash_fn(key1);
		// hashIdx = hash_key % 500;

		auto socket_ind = hash_key % 5;
		auto socket_rep_ind = socket_ind + 1;
		if (socket_rep_ind > 5) socket_rep_ind = 0;
		// socket_rep_ind = (socket_ind + 1) % 5 //TOM

		auto &socket_ref = socket_list[socket_ind];
		auto &socket_rep = socket_list[socket_rep_ind];

		key1 = get_rand();
		value1 = get_rand();

		key2 = get_rand();
		value2 = get_rand();

		key3 = get_rand();
		value3 = get_rand();
		int random = rand(); 
		try {
			// 20% probability : PUT
			if(random % 10 < 2) {
				data = "put("+ key1 + "," + value1 + ")";
				put_client(*socket_ref);
				put_client(*socket_rep);
				// put_client(socket);
			}
			// 20% probability : M-PUT
			else if (random % 10 < 4){
				data = "mput("+ key1 + "," + value1 + "," + key2 + "," + value2 + "," + key3 + "," + value3 +")";
				mput_client(*socket_ref);
				mput_client(*socket_rep);
				//mput_client(socket);
			}
			// 60% probability : GET
			else {
				data = "get("+ key1 + ")";
				get_client(*socket_ref);
				get_client(*socket_rep);
				// get_client(socket);
			}
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
		}
	}

	// Close the sockets after done.
	// socket.close();
	socket1.close();
	socket2.close();
	socket3.close();
	socket4.close();
	socket5.close();

	std::cout << std::endl;

	std::cout << "******* Performance Measure *******"<< std::endl;
	std::cout << "PUT Successful (true): " << put_true << std::endl;
	std::cout << "PUT Failed (false): " << put_false << std::endl;
	std::cout << "Multi-PUT Successful (true): " << mput_true << std::endl;
	std::cout << "Multi-PUT Failed (false): " << mput_fail << std::endl;
	std::cout << "GET Successful: " << get_val << std::endl;
	std::cout << "GET Failed: " << get_null << std::endl;
	std::cout << "Total Bytes Transmitted: " << sum_bytes << std::endl;
	std::cout << "Total Executed Time (seconds): " << sum_ex_time << std::endl;
	std::cout << "Latency (seconds): " << sum_latency << std::endl;
	std::cout << "Throughout: " << 10000 / sum_ex_time << std::endl;
	std::cout << "Average Latency (microseconds): " << sum_latency * 100 << std::endl;

	return 0;
}