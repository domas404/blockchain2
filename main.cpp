#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "hash.cpp"

using namespace std;
using hrClock = chrono::high_resolution_clock;

class blockchain{
    string prev_hash;
    double timestamp;
    int version;
    string merkleroot;
    string nonce;
    string dif_target;
    vector<string> transactions;
};

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
void generateUsers(vector<user> &users){
    string name="user";
    for(int i=0; i<1000; i++){
        name += to_string(i+1);
        users.push_back(user(name, randomBalance(), generateKey()));
        name="user";
    }
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
        hashed = hashfunc(id);
        transactions.push_back(transaction(hashed, users[sender].public_key, users[receiver].public_key, balance));
        id = "";
    }
}

int main(){
    vector<user> users;
    vector<transaction> transactions;
    generateUsers(users);
    generateTransactions(transactions, users);

    for(int i=0; i<50; ++i){
        cout << transactions[i].id << " " << transactions[i].sender << " " << transactions[i].receiver << " " << transactions[i].sum << endl;
    }
    return 0;
}