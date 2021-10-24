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
        float dif_target;
        vector<transaction> transactions;
        block(string prev_hash, time_t timestamp, float version, string merkleroot, int nonce, float dif_target, vector<transaction> transactions){
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
double randomSum(){
    static mt19937 mt(static_cast<long unsigned int>(hrClock::now().time_since_epoch().count()));
    static uniform_int_distribution<int> dist(10, 100000);
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
        balance = randomSum();
        id += users[sender].public_key + users[receiver].public_key + to_string(balance);
        hashed = sha256(id);
        transactions.push_back(transaction(hashed, users[sender].public_key, users[receiver].public_key, balance));
        id = "";
    }
}

string to_lil_endian(string big_end){
    int n=big_end.length();
    string lil_end="";
    if(n%2!=0){
        big_end += "0";
    }
    while(n > 0){
        lil_end += big_end[n-2];
        lil_end += big_end[n-1];
        n -= 2;
    }
    return lil_end;
}

void merkle_hash(vector<transaction> &transactions, vector<transaction> &trans, string &merkleroot){ 
    string tran="";              // continious string of all transactions within a block

    trans.assign(transactions.begin(), transactions.begin()+100);
    for(int i=0; i<100; ++i)
        tran += transactions[i].id;

    merkleroot = sha256(tran);
}

void executeTransactions(vector<transaction> &transactions, vector<user> &users){
    vector<user>::iterator it;
    vector<user>::iterator rec;
    int i=transactions.size()-1;
    int successful=0;
    while(i>=0){
        for(it=users.begin(); it!=users.end(); ++it){
            if(transactions[i].sender == (*it).public_key){
                if((*it).getBalance() >= transactions[i].sum){
                    (*it).setBalance((*it).getBalance() - transactions[i].sum);
                    for(int j=0; j<1000; ++j){
                        if(users[j].public_key == transactions[i].receiver){
                            users[j].setBalance((*it).getBalance() + transactions[i].sum);
                            break;
                        }
                    }
                    ++successful;
                    break;
                }
                else{
                    // cout << (*it).getBalance() << " is not enough for transaction of " << transactions[i].sum << endl;
                    transactions.erase(transactions.begin()+i);
                    break;
                }
            }
        }
        --i;
    }
    // cout << successful << " successful transactions out of 100" << endl;
}

void createBlock(vector<transaction> &transactions, vector<user> &users){
    vector<block> blockchain;
    vector<transaction> trans;   // transactions within a single block

    float version = 1;
    string prev_hash="0000000000000000000000000000000000000000000000000000000000000000";
    string merkleroot;           // transactions hash
    time_t timestamp;
    float dif_target=10000;
    int nonce=0;
    
    string header="";
    string header_hash="";
    string prev_block_hash;
    
    string header_line1, header_line2;
    double time_taken;
    
    ofstream blocks("blocks.txt");

    for(int i=0; i<10; ++i){
        // ++nonce;
        auto pr = chrono::high_resolution_clock::now();
        executeTransactions(trans, users);
        merkle_hash(transactions, trans, merkleroot);
        if(i == 0){
            header = (
                to_lil_endian(to_nBase(int(version), 16)) +
                to_lil_endian(prev_hash) +
                to_lil_endian(merkleroot)
            );
            do{
                ++nonce;
                header_hash = sha256(
                    header +
                    to_lil_endian(to_nBase((int)time(nullptr), 16)) +
                    to_lil_endian(to_nBase(dif_target, 16)) +
                    to_lil_endian(to_nBase(nonce, 16))
                );
            } while((header_hash[0] != '0') || (header_hash[1] != '0') || (header_hash[2] != '0') || (header_hash[3] != '0')/* || (header_hash[4] != '0')*/);
            
            blockchain.push_back(block(prev_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        }
        else {
            header_line2 = (
                to_lil_endian(to_nBase((int)blockchain[i-1].version, 16)) +
                to_lil_endian(blockchain[i-1].prev_hash) +
                to_lil_endian(blockchain[i-1].merkleroot) +
                to_lil_endian(to_nBase((int)blockchain[i-1].timestamp, 16)) +
                to_lil_endian(to_nBase(blockchain[i-1].dif_target, 16)) +
                to_lil_endian(to_nBase(blockchain[i-1].nonce, 16))
            );
            prev_block_hash = sha256(header_line2);
            
            header = (
                to_lil_endian(to_nBase(int(version), 16)) +
                to_lil_endian(prev_block_hash) +
                to_lil_endian(merkleroot)
            );
            
            do{
                ++nonce;
                header_line1 = (
                    header +
                    to_lil_endian(to_nBase((int)time(nullptr), 16)) +
                    to_lil_endian(to_nBase(dif_target, 16)) +
                    to_lil_endian(to_nBase(nonce, 16))
                );
                header_hash = sha256(header_line1);
            } while((header_hash[0] != '0') || (header_hash[1] != '0') || (header_hash[2] != '0') || (header_hash[3] != '0')/* || (header_hash[4] != '0')*/);
            
            blockchain.push_back(block(prev_block_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        }
        auto pab = chrono::high_resolution_clock::now();
        time_taken = chrono::duration_cast<chrono::nanoseconds>(pab - pr).count();
        time_taken *= 1e-9;
        cout << "\nBlock number: " << i+1 << endl;
        cout << "Previous hash: " << blockchain[i].prev_hash << endl;
        cout << "Timestamp: " << blockchain[i].timestamp << endl;
        cout << "Version: " << blockchain[i].version << endl;
        cout << "Merkleroot: " << blockchain[i].merkleroot << endl;
        cout << "Nonce: " << blockchain[i].nonce << endl;
        cout << "Difficulty target: " << blockchain[i].dif_target << endl;
        cout << "Block hash: " << header_hash << endl;
        cout << "Time since last block: " << time_taken << " s" << endl;
        cout << "Number of transactions: " << trans.size() << endl;
        // for(int j=0; j<100; ++j){
        //     blocks << blockchain[i].transactions[j].sender << " " << blockchain[i].transactions[j].receiver << " "
        //     << blockchain[i].transactions[j].sum << endl;
        // }
        // cout << "\nTransaction list: " << endl;
        // for(int i=0; i<100; ++i){
        //     cout << blockchain[0].transactions[i].id << "\n";
        // }
        header_hash = "";
        nonce=0;

        trans.clear();
        transactions.erase(transactions.begin(), transactions.begin()+100);
    }
    blocks.close();
}

void outputUsers(string fileName, vector<user> &users){
    ofstream out(fileName);
    vector<user>::iterator it=users.begin();
    for(it; it!=users.end(); ++it){
        out << (*it).getName() << " " << (*it).getBalance() << endl;
    }
    out.close();
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

    createBlock(transactions, users);
    outputUsers("after_mining.txt", users);
    // for(int i=0; i<50; ++i){
    //     cout << transactions[i].id << " " << transactions[i].sender << " " << transactions[i].receiver << " " << transactions[i].sum << endl;
    // }
    return 0;
}