#include <string>

using namespace std;

	typedef struct _Node{
		string key;
		string value;
		struct _Node *next;        
	}Node,*myNode;

	typedef struct _Hash_Header{
		struct _Node *next;    
	}Hash_Header,*myHash_Header;

	typedef struct _Hash_List{
		struct  _Hash_Header* list[10000];          
	}Hash_List,*myHash_List;

	myHash_List init_hashlist(void);
	string put(myHash_List mylist,string key1,string value1);
	string get(myHash_List mylist,string key1);
	string mput(myHash_List mylist,string key1,string value1,string key2,string value2,string key3,string value3);



