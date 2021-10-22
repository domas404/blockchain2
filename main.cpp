#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <ctime>
#include <algorithm>
#include <time.h>
#include "hash.cpp"
#include "sha256.h"

using namespace std;
using hrClock = chrono::high_resolution_clock;


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

class block{ 
    public:
        string prev_hash;
        time_t timestamp;
        float version;
        string merkleroot;
        int nonce;
        string dif_target;
        vector<transaction> transactions;
        block(string prev_hash, time_t timestamp, float version, string merkleroot, int nonce, string dif_target, vector<transaction> transactions){
            this->prev_hash = prev_hash;
            this->timestamp = timestamp;
            this->version = version;
            this->merkleroot = merkleroot;
            this->nonce = nonce;
            this->dif_target = dif_target;
            this->transactions = transactions;
            // copy(transactions.begin(), transactions.end(), this->transactions.begin());
        }
        ~block(){};
};

double randomBalance(){
    static mt19937 mt(static_cast<long unsigned int>(hrClock::now().time_since_epoch().count()));
    static uniform_int_distribution<int> dist(100, 1000000);
    return dist(mt);
}
double randomKey(){
    static mt19937 mt(static_cast<long unsigned int>(hrClock::now().time_since_epoch().count()));
    static uniform_int_distribution<int> dist(0, 255);
    return dist(mt);
}
double randomUser(){
    static mt19937 mt(static_cast<long unsigned int>(hrClock::now().time_since_epoch().count()));
    static uniform_int_distribution<int> dist(0, 999);
    return dist(mt);
}

string generateKey(){
    string key="";
    int tarp=0;
    while(key.length() < 32){
        key += to_nBase(randomKey(), 16);
    }
    if(key.length() > 32) key.pop_back();
    return key;
}
void generateUsers(){
    ofstream u("users.txt");
    string name="user1";
    u << name << " " << randomBalance() << " " << generateKey();
    for(int i=1; i<1000; i++){
        name="user";
        name += to_string(i+1);
        u << "\n" << name << " " << randomBalance() << " " << generateKey();
        // users.push_back(user(name, randomBalance(), generateKey()));
    }
    u.close();
}
void readUsers(vector<user> &users){
    ifstream input("users.txt");
    string name, key;
    int balance;
    while(!input.eof()){
        input >> name >> balance >> key;
        users.push_back(user(name, balance, key));
    }
    input.close();
}

void generateTransactions(vector<transaction> &transactions, vector<user> &users){
    int sender, receiver, balance;
    string id="";
    string hashed;
    for(int i=0; i<10000; ++i){
        sender = randomUser();
        receiver = randomUser();
        balance = randomBalance();
        id += users[sender].public_key + users[receiver].public_key + to_string(balance);
        hashed = sha256(id);
        transactions.push_back(transaction(hashed, users[sender].public_key, users[receiver].public_key, balance));
        id = "";
    }
}

void createBlock(vector<transaction> &transactions){
    vector<transaction> trans;
    vector<block> blockchain;
    string line="";
    string merkleroot;
    string tran="";
    string prev_hash="0000";
    float version = 1;
    int nonce=0;
    string dif_target;
    int n=100;

    for(int i=0; i<100; ++i){
        trans.assign(transactions.begin()+n-100, transactions.begin()+n);
        for(int i=n-100; i<n; ++i){
            tran += transactions[i].id;
        }
        merkleroot = sha256(tran);
        n += 100;
        dif_target = sha256(merkleroot+to_string(nonce));
        ++nonce;
        while((dif_target[0] != '0') || (dif_target[1] != '0') || (dif_target[2] != '0') || (dif_target[3] != '0')){
            dif_target = sha256(merkleroot+to_string(nonce));
            ++nonce;
        }
        if(i == 0){
            blockchain.push_back(block(prev_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        }
        else {
            blockchain.push_back(block(blockchain[i-1].dif_target, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        }
        cout << "\nBlock number: " << i+1 << endl;
        cout << "Previous hash: " << blockchain[i].prev_hash << endl;
        cout << "Timestamp: " << blockchain[i].timestamp << endl;
        cout << "Version: " << blockchain[i].version << endl;
        cout << "Merkleroot: " << blockchain[i].merkleroot << endl;
        cout << "Nonce: " << blockchain[i].nonce << endl;
        cout << "Difficulty target: " << blockchain[i].dif_target << endl;
        // cout << "\nTransaction list: " << endl;
        // for(int i=0; i<100; ++i){
        //     cout << blockchain[0].transactions[i].id << "\n";
        // }
        tran="";
        trans.clear();
        nonce=0;
    }
}

int main(){
    vector<user> users;
    vector<transaction> transactions;

    cout << "Generate new users? (y/n) ";
    string gen;
    cin >> gen;
    if (gen=="y")
        generateUsers();
    
    readUsers(users);
    generateTransactions(transactions, users);

    createBlock(transactions);

    // for(int i=0; i<50; ++i){
    //     cout << transactions[i].id << " " << transactions[i].sender << " " << transactions[i].receiver << " " << transactions[i].sum << endl;
    // }
    return 0;
}