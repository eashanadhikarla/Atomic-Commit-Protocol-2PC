#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <regex>
#include <vector>
#include <mutex>
#include <fstream>
#include "hash.h"

using namespace std;

std::vector<mutex> mtx(10000);
Node *p,*q,*p_end;
string retval;
hash<string> hash_fn;
unsigned int hashIdx;

// Initialize the Hash List
myHash_List init_hashlist(void){
  unsigned int i;
  myHash_List mylist;

  mylist = (Hash_List *)malloc(sizeof(Hash_List)); 

  for( i = 0;i < 10000;i++ ){
    mylist->list[i] = (Hash_Header *)malloc(sizeof(Hash_Header)); 
    mylist->list[i]->next = NULL;
  }     
  return mylist;
} 

// void backup_for_recovery(myHash_List mylist){
//   u32 i;
//   myNode p; 
//   cerr << "Writing into the log:" << endl; 
//   for( i = 0;i < HASH_NUM_MAX;i++){
//     p = mylist->list[i]->next;
//     while( NULL != p ){
//       // We need to have the cordinator name in future after every operation.
//       // We need to know where we are in the current phase.
//       cerr << "ID = " << p->id << " key = " << p->key << " Value = " << p->value << endl;
//       p = p->next;
//     }        
//   }
//   cerr << endl;
//   ofstream out("hash.log", ios::app);
//   if (out.is_open()){
//     out << mylist << endl;
//     out.close();
//   }
// }

// // PUT function
// string put(myHash_List mylist,string key,string value){
 
//  //hash<string> hash_fn;
//  size_t hash_key = hash_fn(key);
//  unsigned int hashIdx = hash_key % 10000; 

//  // **********************************LOCK**********************************
//  mtx[hashIdx].lock();
//  cerr << "Locked" << endl;

//  p_end = (Node *)malloc(sizeof(Node));
//  p_end->next = NULL;
//  p_end->value = value;
//  p_end->key = key;

//  //nonexistent key-value in the row
//  if( NULL == mylist->list[hashIdx]->next ){
//   mylist->list[hashIdx]->next = p_end;   
//   cerr << "Commit Successful" << endl;
//   retval = "true";

//  }
//  //existent key-value in the row
//  q = mylist->list[hashIdx]->next;
//  while( q ){
//    p = q;
//    q = q->next; 
//    //the key is exsit
//    if(key == p->key ){
//      cerr<<"Commit Failed"<<endl;
//     retval = "fals";
//     mtx[hashIdx].unlock();
//    }  
//  }
//  p->next = p_end;
//  cerr<<"Commit Successful"<<endl;

// //  backup_for_recovery(mylist);

//  // *********************************UNLOCK**********************************
//  mtx[hashIdx].unlock();
//  cerr << "Unlocked" << endl;

//  return retval;
// }

// GET function
string get(myHash_List mylist,string key){

 //hash<string> hash_fn;
 size_t hash_key = hash_fn(key);
 unsigned int hashIdx = hash_key % 10000;

 //Node *p;
 p = mylist->list[hashIdx]->next; 
   //whether the hashlist is empty
 if( NULL == p ){
   cerr<<"GET Failed"<<endl; 
   return "Null"; 
 }
   //whether the first node is the key(searched)
 if(key == p->key){
   string p_val;
   p_val = p->value;
   cerr<<"GET Successful"<<endl; 
   return p_val; 
 }
   //whether second node is empty
 if( NULL == p->next ){
   cerr<<"GET Failed"<<endl; 
   return "Null";    
 } 
   //whether the node is the key(searched)
 while( key != p->next->key){
   p = p->next;
       //whether the node is empty
   if( NULL == p->next ){
     cerr<<"GET Failed"<<endl; 
     return "Null";    
   }       
 } 
 string p_val2;
 p_val2 = p->value;
   //delete successfully
 cerr<<"GET Successful"<<endl; 
 return p_val2;  
}

// Multiple-PUT Try locking
string mput_try(myHash_List mylist,string key){
  //missing condition for two put at the same time try lock, then one of them will lock 
  //and the other will get error in try_lock.
  //hash<string> hash_fn;

  size_t hash_key = hash_fn(key);
  hashIdx = hash_key % 10000;
  //try lock
  if(mtx[hashIdx].try_lock() == false){
    return "fals";
  }
  return "lock";
  cerr << "Locked" << endl;
}

string mput_commit(myHash_List mylist, string key, string value) {
 // Actually put the thing; same as put(), but it doesn't need to lock the bucket,
 // because we know it's already locked by mput_try
 //hash<string> hash_fn;

 //  size_t hash_key = hash_fn(key);
 //  unsigned int hashIdx = hash_key % 10000;
 
 p_end = (Node *)malloc(sizeof(Node));
 p_end->next = NULL;
 p_end->value = value;
 p_end->key = key;

 //nonexistent key-value in the row
 if( NULL == mylist->list[hashIdx]->next ){
  mylist->list[hashIdx]->next = p_end;   
  cerr << "Commit Successful" << endl;
  retval = "true";
 }
 //existent key-value in the row
 q = mylist->list[hashIdx]->next;
 while(q){
   p = q;
   q = q->next; 
   //the key is exsit
   if(key == p->key ){
     cerr<<"Commit Failed"<<endl;
    retval = "fals";
    mtx[hashIdx].unlock();
   }  
 }
 p->next = p_end;
 cerr<<"Commit Successful"<<endl;
 //  backup_for_recovery(mylist);
 mtx[hashIdx].unlock();
 cerr << "Unlocked" << endl;

 return retval;
}

string mput_abort(myHash_List mylist, string key) {
  // Don't put k/v, just unlock the bucket.
  //hash<string> hash_fn;
  
  // size_t hash_key = hash_fn(key);
  // unsigned int hashIdx = hash_key % 10000;
  
  mtx[hashIdx].unlock();
  return "";
  cerr << "Unlocked" << endl;
 }
