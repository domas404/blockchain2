#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <time.h>

using namespace std;
using hrClock = chrono::high_resolution_clock;

// vartotojai
class user{
    private:
        string name;
        double balance;
    public:
        user(string name, double balance, string key){
            this->name = name;
            this->balance = balance;
            public_key = key;
        }
        string public_key;
        void setName(string name){ this->name = name; }
        void setBalance(double balance){this->balance = balance; }
        string getName() const { return name; }
        double getBalance() const { return balance; }

        ~user(){};
};

//transakcijos
class transaction{
    public:
        string id;
        string sender;
        string receiver;
        double sum;
        transaction(string id, string sender, string receiver, double sum){
            this->id = id;
            this->sender = sender;
            this->receiver = receiver;
            this->sum = sum;
        }
        ~transaction(){};
};

// blokai
class block{ 
    public:
        string prev_hash;
        time_t timestamp;
        float version;
        string merkleroot;
        int nonce;
        int dif_target;
        vector<transaction> transactions;
        block(string prev_hash, time_t timestamp, float version, string merkleroot, int nonce, int dif_target, vector<transaction> transactions){
            this->prev_hash = prev_hash;
            this->timestamp = timestamp;
            this->version = version;
            this->merkleroot = merkleroot;
            this->nonce = nonce;
            this->dif_target = dif_target;
            this->transactions = transactions;
        }
        ~block(){};
};

// blokai kandidatai
class queue: public block{
    public:
        int a;
        double time;
        int num_transact;
        int num_thread;
        string block_hash;
        queue(string prev_hash, time_t timestamp, float version, string merkleroot, int nonce, int dif_target,
        vector<transaction> transactions, int a, double time, int num_transact, string block_hash, int num_thread)
        : block(prev_hash, timestamp, version, merkleroot, nonce, dif_target, transactions){
            this->a = a;
            this->time = time;
            this-> num_transact = num_transact;
            this->block_hash = block_hash;
            this->num_thread = num_thread;
        }
        ~queue(){};
};

// merkle tree nodes
struct node{
    string hash;
    node *left;
    node *right;
    node(string hash){
        this->hash = hash;
    }
};

const float version = 1;

#endif