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
    string id;
    string sender;
    string receiver;
    double sum;
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

string generateKey(){
    string key="";
    int tarp=0;
    while(key.length()<64){
        key += to_nBase(randomKey(), 16);
    }
    if(key.length() > 16) key.pop_back();
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

int main(){
    vector<user> users;
    generateUsers(users);
    // vector<user>::iterator it=users.begin();
    // for(it; it!=users.end(); ++it){
    //     cout << (*it).getName() << " " << (*it).getBalance() << " " << (*it).public_key << endl;
    // }
    return 0;
}