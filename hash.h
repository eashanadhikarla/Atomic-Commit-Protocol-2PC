#include <string>
#define u32 unsigned int
#define HASH_NUM_MAX 65536

using namespace std;

typedef struct _Node{
	string key;
	string value;
	u32 id;
	struct _Node *next;        
}Node,*myNode;

typedef struct _Hash_Header{
	struct _Node *next;    
}Hash_Header,*myHash_Header;

typedef struct _Hash_List{
	struct  _Hash_Header* list[10000];          
}Hash_List,*myHash_List;

myHash_List init_hashlist(void);
//string put(myHash_List mylist,string key,string value);
string get(myHash_List mylist,string key);
string mput_try(myHash_List mylist,string key);
string mput_commit(myHash_List mylist, string key, string value);
string mput_abort(myHash_List mylist, string key);



