#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <time.h>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
#define BUF_SIZE 256

typedef boost::asio::ip::tcp::socket tcpsock;
boost::asio::io_service io_serv;
unsigned int hashIdx;
bool flag;
int put_true,put_false,get_null,get_val,mput_true,mput_fail;
size_t sum_bytes = 0;
double sum_ex_time = 0;
double sum_latency = 0;
long int sleepTime = 10000L+(long)((1e5-1e4)*rand()/(RAND_MAX+1.0));

/* Auto-generation of Operations. */
string get_rand(){
	int key_range = 10000;
	int random = rand();
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

bool read(tcpsock &socket){
	size_t recd = 0;
	bool flag;
	flag = 1;
	while (recd < 4) {
		char buf[256];
		//memset(buf, 0, 256); // Empty the buffer evereytime.
		string buf_str;
		// Recieve and read reply from the Server.
		boost::system::error_code ignored_error; 
		size_t len = socket.read_some(boost::asio::buffer(buf), ignored_error);
		std::cout << "Read back from the server." << len << std::endl;
		if (len > 0) {
			recd += len;
			buf[len] = 0;
			//cout << buf;

			for(int i=0;i<4;++i){
				buf_str += buf[i];
			}
			std::cout << "Buf_str " << buf_str <<std::endl;
			// Counting the Results for performance measure.
			if(buf_str == "lock"){
				flag = 1;
			}
			else if(buf_str == "fals"){
				flag = 0;
			}
			else if(buf_str == "Null"){
				flag = 1;
			}
		} 
	}
	return flag;
}

void get_client(tcpsock &socket_ref1, tcpsock &socket_rep1, string data) {

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	// Start Time.
	gettimeofday(&start_time, nullptr);

	// Write data to the Socket and send it to the Server.
	boost::system::error_code ignored_error; 
	write(socket_ref1, buffer(data), ignored_error);

	bool flag = read(socket_ref1);
	cout << "GET Successful";
	get_val++;

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

bool put_client(tcpsock &socket_ref1, tcpsock &socket_rep1, string key1, string data) {

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	boost::system::error_code ignored_error;
	string try_data1;
	try_data1 = "t("+key1+")";
	gettimeofday(&start_time, nullptr); // Start Time.

	// Write data to the Socket and send it to the Server.
	// Prepare Phase | Phase 1
	write(socket_ref1, buffer(try_data1), ignored_error);
	std::cout << "Prepared Socket Reference" << socket_ref1.native_handle() << std::endl;
	bool flag = read(socket_ref1);
	if(flag == 1){
		write(socket_rep1, buffer(try_data1), ignored_error);
		std::cout << "Prepared Socket Replication" << socket_rep1.native_handle() << std::endl;
		flag = read(socket_rep1);
		if(flag == 1){
			// Commit Phase | Phase 2
			write(socket_ref1, buffer(data), ignored_error);
			std::cout << "Commit Reference" << socket_ref1.native_handle() << std::endl;
			write(socket_rep1, buffer(data), ignored_error);
			std::cout << "Commit Replication" << socket_rep1.native_handle() << std::endl;
			xBytes += data.length();
			gettimeofday(&start_time_s, nullptr);
		}
		else{
			try_data1 = "a("+key1+")";
			write(socket_ref1, buffer(try_data1), ignored_error);
			usleep(sleepTime);
			return false;// retry
		}
	}
	else {
		usleep(sleepTime);
		return false;// retry
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
	// If 'successfull commit' then abort and unlock.
	try_data1 = "a("+key1+")";
	write(socket_ref1, buffer(try_data1), ignored_error);
	write(socket_rep1, buffer(try_data1), ignored_error);
	put_true++;
	return true;
}

bool mput_client(tcpsock &socket_ref1, tcpsock &socket_ref2, tcpsock &socket_ref3, tcpsock &socket_rep1, tcpsock &socket_rep2, tcpsock &socket_rep3, string key1, string key2, string key3, string data, string data2, string data3){

	size_t xBytes = 0;
	struct timeval start_time, end_time;
	struct timeval start_time_s, end_time_s;
	string try_data1,try_data2,try_data3;
	try_data1 = "t("+key1+")";
	try_data2 = "t("+key2+")";
	try_data3 = "t("+key3+")";
	gettimeofday(&start_time, nullptr); // Start Time.

	// ************************* Actual Node *****************************	
	boost::system::error_code ignored_error;
	write(socket_ref1, buffer(try_data1), ignored_error);
 	bool flag = read(socket_ref1);
	if(flag == 1){
		write(socket_ref2, buffer(try_data2), ignored_error);
		flag = read(socket_ref2);
		if(flag == 1){
			write(socket_ref3, buffer(try_data3), ignored_error);
			flag = read(socket_ref3);
			if(flag == 1){
				// ************************* Replicated Node *****************************
					write(socket_rep1, buffer(try_data1), ignored_error);
					flag = read(socket_rep1);
					if(flag == 1){
						write(socket_rep2, buffer(try_data2), ignored_error);
						flag = read(socket_rep2);
						if(flag == 1){
							write(socket_rep3, buffer(try_data3), ignored_error);
							flag = read(socket_rep3);
							if(flag == 1){
								// Finally all the 5 nodes are locked.
								// // ************************* Do the commit *****************************
								write(socket_ref1, buffer(data), ignored_error); 
								xBytes += data.length();
								write(socket_ref2, buffer(data2), ignored_error);
								//xBytes += data2.length();
								write(socket_ref3, buffer(data3), ignored_error);
								//xBytes += data3.length();
								write(socket_rep1, buffer(data), ignored_error);
								//xBytes += data2.length();
								write(socket_rep2, buffer(data2), ignored_error);
								write(socket_rep3, buffer(data3), ignored_error);

								gettimeofday(&start_time_s, nullptr);
							}
							else{
								try_data1 = "a("+key1+")";
								try_data2 = "a("+key2+")";
								try_data3 = "a("+key3+")";
								// Unlocking
								write(socket_ref1, buffer(try_data1), ignored_error);
								write(socket_ref2, buffer(try_data2), ignored_error);
								write(socket_ref3, buffer(try_data3), ignored_error);
								write(socket_rep1, buffer(try_data1), ignored_error);
								write(socket_rep2, buffer(try_data2), ignored_error);
								usleep(sleepTime);
								return false;// retry
							}
						}
						else {
							try_data1 = "a("+key1+")";
							try_data2 = "a("+key2+")";
							try_data3 = "a("+key3+")";
							write(socket_ref1, buffer(try_data1), ignored_error);
							write(socket_ref2, buffer(try_data2), ignored_error);
							write(socket_ref3, buffer(try_data3), ignored_error);
							write(socket_rep1, buffer(try_data1), ignored_error);
							usleep(sleepTime);
							return false;// retry
						}
					}
					else{
						try_data1 = "a("+key1+")";
						try_data2 = "a("+key2+")";
						try_data3 = "a("+key3+")";
						write(socket_ref1, buffer(try_data1), ignored_error);
						write(socket_ref2, buffer(try_data2), ignored_error);
						write(socket_ref3, buffer(try_data3), ignored_error);
						usleep(sleepTime);
						return false;// retry
					}
			}
			else{
				try_data1 = "a("+key1+")";
				try_data2 = "a("+key2+")";
				write(socket_ref1, buffer(try_data1), ignored_error);
				write(socket_ref2, buffer(try_data2), ignored_error);
				usleep(sleepTime);
				return false;// retry
			}
		}
		else{
			try_data1 = "a("+key1+")";
			write(socket_ref1, buffer(try_data1), ignored_error);
			usleep(sleepTime);
			return false;// retry
		}
	}
	else {
		usleep(sleepTime);
		return false; // retry
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

	// Successfull PUT operation, so send abort to unlock.
	try_data1 = "a("+key1+")";
	try_data2 = "a("+key2+")";
	try_data3 = "a("+key3+")";
	write(socket_ref1, buffer(try_data1), ignored_error);
	write(socket_ref2, buffer(try_data2), ignored_error);
	write(socket_ref3, buffer(try_data3), ignored_error);
	write(socket_rep1, buffer(try_data1), ignored_error);
	write(socket_rep2, buffer(try_data2), ignored_error);
	write(socket_rep3, buffer(try_data3), ignored_error);
	put_true = put_true+1;
	return true;
}

void worker(std::vector<string> servers_ip){
	string data,data2,data3, key1, key2, key3, value1, value2, value3;
	string ports = "9000";

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
		for (int i = 0; i < 10000; i++){
		key1 = get_rand();
		value1 = get_rand();

		key2 = get_rand();
		value2 = get_rand();

		key3 = get_rand();
		value3 = get_rand();
		int random = rand(); 

    		hash<string> hash_fn;
		size_t hash_key1 = hash_fn(key1);
		size_t hash_key2 = hash_fn(key2);
		size_t hash_key3 = hash_fn(key3);

		// Sockets are hashed on the basis of 5 nodes and direected towards its respetive socket. 
		auto socket_ind1 = hash_key1 % 5;
		auto socket_rep_ind1 = socket_ind1 + 1;
		if (socket_rep_ind1 > 5) socket_rep_ind1 = 0; // socket_rep_ind = (socket_ind + 1) % 5 
		auto &socket_ref1 = socket_list[socket_ind1];
		auto &socket_rep1 = socket_list[socket_rep_ind1];

		auto socket_ind2 = hash_key2 % 5;
		auto socket_rep_ind2 = socket_ind2 + 1;
		if (socket_rep_ind2 > 5) socket_rep_ind2 = 0;
		auto &socket_ref2 = socket_list[socket_ind2];
		auto &socket_rep2 = socket_list[socket_rep_ind2];

		auto socket_ind3 = hash_key3 % 5;
		auto socket_rep_ind3 = socket_ind3 + 1;
		if (socket_rep_ind3 > 5) socket_rep_ind3 = 0;
		auto &socket_ref3 = socket_list[socket_ind3];
		auto &socket_rep3 = socket_list[socket_rep_ind3];

		try {
			// 20% probability : PUT
			if(random % 10 < 9) {
				data = "commit("+ key1 + "," + value1 + ")";
				put_client(*socket_ref1, *socket_rep1, key1, data);
				while(false){
					size_t x = 0; x=+1;
					if(x == 2){break;}
					put_client(*socket_ref1, *socket_rep1, key1, data);
				}
			}
			// 20% probability : M-PUT
			else if (random % 10 < 4)
        		{
				//For every PUT operation, I have to be at all the 6 sockets, if anyone fails all abort.
				data = "commit("+ key1 + "," + value1 + ")";
				data2 = "commit("+ key2 + "," + value2 + ")";
				data3 = "commit("+ key3 + "," + value3 + ")";
				while(false){
					size_t x = 0; x=+1;
					if(x == 2){break;}
					mput_client(*socket_ref1, *socket_ref2, *socket_ref3, *socket_rep1, *socket_rep2, *socket_rep3, key1, key2, key3, data, data2, data3);
				}
			}
			// 60% probability : GET
			else {
				data = "get("+ key1 + ")";
				get_client(*socket_ref1, *socket_rep1, data);
			}
		} 
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
		}
	}
	// Close the sockets after done.
	socket1.close();
	socket2.close();
	socket3.close();
	socket4.close();
	socket5.close();
}

int main(int argc, char *argv[]) {

	// string ports = "9000"; 
	std::vector<string> servers_ip;
	// Read the list of Hostnames from a text file.
	ifstream file;
	file.open("Hosttnames.txt", ios::in);
	string eachLine;
	while(getline(file,eachLine)){
		if(eachLine.empty()){
			continue;
		}
		string the_ip = eachLine.substr(8);
		servers_ip.push_back(the_ip);
	}
	file.close();

	static const int MAX_THREAD = 1;
	std::thread t[MAX_THREAD];
	for(int i=0; i<MAX_THREAD; ++i){
		t[i] = std::thread(worker,servers_ip);
		cout << "CHECK POINT 1" << i << endl;
	} 

	for(int i=0; i<MAX_THREAD; ++i){
		t[i].join();
		std::cout << "Thread :" << i << endl;
	}
	
	std::cout << std::endl;

	std::cout << "******* Performance Measure *******"<< std::endl;

	std::cout << "PUT Successful (true): " << put_true << std::endl;
	std::cout << "PUT Failed (false): " << put_false << std::endl;
	
	std::cout << "GET Successful: " << get_val << std::endl;
	std::cout << "GET Failed: " << get_null << std::endl;
	
	std::cout << "Total Bytes Transmitted: " << sum_bytes << std::endl;
	std::cout << "Total Executed Time (seconds): " << sum_ex_time << std::endl;
	
	std::cout << "Latency (seconds): " << sum_latency << std::endl;
	std::cout << "Throughout: " << 10000 / sum_ex_time << std::endl;
	
	std::cout << "Average Latency (microseconds): " << sum_latency * 100 << std::endl;

	return 0;
}
