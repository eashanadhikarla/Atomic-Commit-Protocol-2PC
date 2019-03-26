#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <regex>
#include <vector>
#include <mutex>

#include "hash.h"

using namespace std;

std::vector<mutex> mtx(10000);
Node *p,*q,*p_end;
string retval;

// Initialize the Hash List
myHash_List init_hashlist(void){
  unsigned int i;
  myHash_List mylist;

  mylist = (Hash_List *)malloc(sizeof(Hash_List)); 

  for( i = 0;i < 100000;i++ ){
    mylist->list[i] = (Hash_Header *)malloc(sizeof(Hash_Header)); 
    mylist->list[i]->next = NULL;
  }     

  return mylist;
} 

// PUT function
string put(myHash_List mylist,string key1,string value1){
 // Node *p,*q,*p_end;
 // string retval;
 
 hash<string> hash_fn;
 size_t hash_key = hash_fn(key1);
 unsigned int hashIdx = hash_key % 5000; 

 // **********************************LOCK**********************************
 mtx[hashIdx].lock();

 p_end = (Node *)malloc(sizeof(Node));
 p_end->next = NULL;
 p_end->value = value1;
 p_end->key = key1;

 //nonexistent key-value in the row
 if( NULL == mylist->list[hashIdx]->next ){
  mylist->list[hashIdx]->next = p_end;   
  cout << "PUT Successful" << endl;
  retval = "true";

 }
 //existent key-value in the row
 q = mylist->list[hashIdx]->next;
 while( q ){
   p = q;
   q = q->next; 
   //the key is exsit
   if(key1 == p->key ){
     cout<<"PUT Failed"<<endl;
    retval = "false";
    // mtx[hashIdx].unlock();
   }  
 }
 p->next = p_end;
 cout<<"PUT Successful"<<endl;

 // *********************************UNLOCK**********************************
 mtx[hashIdx].unlock();

 return retval;
}

// Multiple-PUT Function
string mput(myHash_List mylist,string key1,string value1,string key2,string value2,string key3,string value3){
  hash<string> hash_fn;
  size_t hash_key;
 
  hash_key = hash_fn(key1);
  unsigned int hashIdx1 = hash_key % 5000;
  //try lock
  if(mtx[hashIdx1].try_lock() == false){
    return "false";
  }
  hash_key = hash_fn(key1);
  unsigned int hashIdx2 = hash_key % 5000;
  //try lock
  if (mtx[hashIdx2].try_lock() == false) {
    // unlock hashIdx1
    mtx[hashIdx1].unlock();
    return "false";
  }

  hash_key = hash_fn(key1);
  unsigned int hashIdx3 = hash_key % 5000;
  //try lock
  if (mtx[hashIdx3].try_lock() == false) {
      // unlock hashIdx1 & hashIdx2
      mtx[hashIdx1].unlock();
      mtx[hashIdx2].unlock();

      return "false";
    }

 //*********** k1,v1 ***********
 p_end = (Node *)malloc(sizeof(Node));
 p_end->next = NULL;
 p_end->value = value1;
 p_end->key = key1;

 //nonexistent key-value in the row
 if( NULL == mylist->list[hashIdx1]->next ){
  mylist->list[hashIdx1]->next = p_end;   
  cout << "Multi-PUT Successful" << endl;
  retval = "true";

 }
 //existent key-value in the row
 q = mylist->list[hashIdx1]->next;
 while( q ){
   p = q;
   q = q->next; 
   //the key is exsit
   if(key1 == p->key ){
     cout<<"Multi-PUT Failed"<<endl;
    retval = "false";
    // mtx[hashIdx].unlock();
   }  
 }
 p->next = p_end;
 cout<<"Multi-PUT Successful"<<endl;
 mtx[hashIdx1].unlock();

 //*********** k2,v2 ***********
 p_end = (Node *)malloc(sizeof(Node));
 p_end->next = NULL;
 p_end->value = value2;
 p_end->key = key2;

 //nonexistent key-value in the row
 if( NULL == mylist->list[hashIdx2]->next ){
  mylist->list[hashIdx2]->next = p_end;   
  cout << "Multi-PUT Successful" << endl;
  retval = "true";

 }
 //existent key-value in the row
 q = mylist->list[hashIdx2]->next;
 while( q ){
   p = q;
   q = q->next; 
   //the key is exsit
   if(key1 == p->key ){
     cout<<"Multi-PUT Failed"<<endl;
    retval = "false";
    // mtx[hashIdx].unlock();
   }  
 }
 p->next = p_end;
 cout<<"Multi-PUT Successful"<<endl;

 //*********** k3,v3 ***********
 p_end = (Node *)malloc(sizeof(Node));
 p_end->next = NULL;
 p_end->value = value2;
 p_end->key = key2;

 //nonexistent key-value in the row
 if( NULL == mylist->list[hashIdx2]->next ){
  mylist->list[hashIdx2]->next = p_end;   
  cout << "Multi-PUT Successful" << endl;
  retval = "true";

 }
 //existent key-value in the row
 q = mylist->list[hashIdx2]->next;
 while( q ){
   p = q;
   q = q->next; 
   //the key is exsit
   if(key1 == p->key ){
     cout<<"Multi-PUT Failed"<<endl;
    retval = "false";
    // mtx[hashIdx].unlock();
   }  
 }
 p->next = p_end;
 cout<<"Multi-PUT Successful"<<endl;

return retval;
}

// GET function
string get(myHash_List mylist,string key1){

 hash<string> hash_fn;
 size_t hash_key = hash_fn(key1);
 unsigned int hashIdx = hash_key % 10000;

 //Node *p;
 p = mylist->list[hashIdx]->next; 
   //whether the hashlist is empty
 if( NULL == p ){
   cout<<"GET Failed"<<endl; 
   return "Null"; 
 }
   //whether the first node is the key(searched)
 if(key1 == p->key){
   string p_val;
   p_val = p->value;
   cout<<"GET Successful"<<endl; 
   return p_val; 
 }
   //whether second node is empty
 if( NULL == p->next ){
   cout<<"GET Failed"<<endl; 
   return "Null";    
 } 
   //whether the node is the key(searched)
 while( key1 != p->next->key){
   p = p->next;
       //whether the node is empty
   if( NULL == p->next ){
     cout<<"GET Failed"<<endl; 
     return "Null";    
   }       
 } 
 string p_val2;
 p_val2 = p->value;
   //delete successfully
 cout<<"GET Successful"<<endl; 
 return p_val2;  
}
