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
#include "omp.h"
#include <iomanip>

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

const float version = 1;

// random funkcijos
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

// user'iu generavimas
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
    cout << "Successfully generated 1'000 users." << endl;
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

// tranzakciju generavimas
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
    cout << "Successfully generated 10'000 transactions." << endl;
}

// hex kodo uzrasymas little endian budu
string to_lil_endian(string big_end){
    int n=big_end.length();
    string lil_end="";
    if(n%2!=0){
        big_end += "0";
        ++n;
    }
    while(n > 0){
        lil_end += big_end[n-2];
        lil_end += big_end[n-1];
        n -= 2;
    }
    return lil_end;
}

// merkle tree nodes
struct node{
    string hash;
    node *left;
    node *right;
    node(string hash){
        this->hash = hash;
    }
};
// merkle root apskaiciuojanti funkcija
void merkle_root(vector<transaction> &transactions, string &root){
    vector<node*> blocks;
    for(int i=0; i<transactions.size(); ++i){
        blocks.push_back(new node(sha256(transactions[i].id)));
    }
    for(int i=0; i<blocks.size(); ++i){
        blocks[i]->left = NULL;
        blocks[i]->right = NULL;
    }
    vector<node*> nodes;
    int j=0, n;
    while(blocks.size() != 1){
        n=blocks.size();
        if(n%2 != 0){
            blocks.push_back(new node(blocks[n-1]->hash));
            blocks[n-1]->left = blocks[n-2]->left;
            blocks[n-1]->right = blocks[n-2]->right;
        }
        for(int i=0; i < blocks.size(); i+=2){
            nodes.push_back(new node(sha256(blocks[i]->hash + blocks[i+1]->hash)));
            nodes[j]->left = blocks[i];
            nodes[j]->right = blocks[i+1];
            j++;
        }
        blocks = nodes;
        nodes.clear();
        j=0;
    }
    root = blocks[0]->hash;
}

// ivykdomos transakcijos, jei siuntejas turi pakankama balansa
void executeTransactions(vector<transaction> &transactions, vector<user> &users){
    vector<user>::iterator it;
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
                    transactions.erase(transactions.begin()+i);
                    break;
                }
            }
        }
        --i;
    }
}

// spausdinami blokai
void outputBlocks(int height, queue &line, double time_taken){
    cout << "\n" << setw(32) << right << "Block height: " << height << endl;
    cout << setw(32) << right << "Previous block hash: " << line.prev_hash << endl;
    struct tm * timeinfo;
    timeinfo = localtime(&line.timestamp);

    cout << setw(32) << right << "Timestamp: " << asctime(timeinfo);
    cout << setw(32) << right << "Version: " << "0x" << line.version << endl;
    cout << setw(32) << right << "Merkleroot: " << line.merkleroot << endl;
    cout << setw(32) << right << "Nonce: " << line.nonce << endl;
    cout << setw(32) << right << "Difficulty target: " << line.dif_target << endl;
    cout << setw(32) << right << "Block hash: " << line.block_hash << endl;
    cout << setw(32) << right << "Block mined in: " << line.time << " s" << endl;
    cout << setw(32) << right << "Number of transactions: " << line.num_transact << endl;
    cout << setw(32) << right << "Thread that mined the block: " << line.num_thread << endl;
    cout << setw(32) << right << "Time since last block: " << time_taken << endl;
}

//kasami blokai
void mineBlock(vector<transaction> &transactions, vector<user> &users, vector<queue> &line, int &a, string &prev_hash, int &time_limit){
    auto pr = chrono::high_resolution_clock::now();
    vector<transaction> trans;   // transactions within a single block
    string merkleroot;
    string header_hash;
    int nonce=0;
    int dif_target=1000;

    a = rand()%(transactions.size()-100);
    trans.assign(transactions.begin()+a, transactions.begin()+a+100);
    executeTransactions(trans, users);
    merkle_root(trans, merkleroot);

    time_t timestamp;
    string difficulty = "00001"; // to_lil_endian(to_string(dif_target));

    string header = (
        to_lil_endian(to_nBase(int(version), 16)) +
        to_lil_endian(prev_hash) +
        to_lil_endian(merkleroot)
    );
    clock_t p, pb;
    p = clock();
    do{
        ++nonce;
        header_hash = sha256(
            header +
            to_lil_endian(to_nBase((int)time(nullptr), 16)) +
            to_lil_endian(to_nBase(dif_target, 16)) +
            to_lil_endian(to_nBase(nonce, 16))
        );
        pb = clock() - p;
    } while(difficulty.compare(header_hash) < 0 && pb/CLOCKS_PER_SEC < time_limit);

    auto pab = chrono::high_resolution_clock::now();
    double time_taken = chrono::duration_cast<chrono::nanoseconds>(pab - pr).count();
    time_taken *= 1e-9;

    if (pb/CLOCKS_PER_SEC < 5){
        line.push_back(queue(prev_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans, a,
        time_taken, trans.size(), header_hash, omp_get_thread_num()));
    }
    trans.clear();
}

// atrinkti blokai pridedami i grandine
void createBlock(vector<block> &blockchain, vector<transaction> &transactions, vector<user> &users){
    vector<queue> line;
    string prev_block_hash="0000000000000000000000000000000000000000000000000000000000000000";
    int time_limit = 5;

    auto pr = chrono::high_resolution_clock::now();
    omp_set_num_threads(5);
    #pragma omp parallel
    {
        int a;
        mineBlock(transactions, users, line, a, prev_block_hash, time_limit);
    }
    auto pab = chrono::high_resolution_clock::now();
    double time_taken = chrono::duration_cast<chrono::nanoseconds>(pab - pr).count();
    time_taken *= 1e-9;

    blockchain.push_back(block(line[0].prev_hash, line[0].timestamp, line[0].version, line[0].merkleroot, line[0].nonce, line[0].dif_target, line[0].transactions));
    outputBlocks(1, line[0], time_taken);

    transactions.erase(transactions.begin()+line[0].a, transactions.begin()+line[0].a+100);
    time_taken = 0;
    line.clear();
    
    for(int i=1; i<100; ++i){
        auto pr = chrono::high_resolution_clock::now();
        prev_block_hash = sha256(
            to_lil_endian(to_nBase((int)blockchain[i-1].version, 16)) +
            to_lil_endian(blockchain[i-1].prev_hash) +
            to_lil_endian(blockchain[i-1].merkleroot) +
            to_lil_endian(to_nBase((int)blockchain[i-1].timestamp, 16)) +
            to_lil_endian(to_nBase(blockchain[i-1].dif_target, 16)) +
            to_lil_endian(to_nBase(blockchain[i-1].nonce, 16))
        );
        #pragma omp parallel
        {
            int a; // iterator to locate which transactions were added to a block
            mineBlock(transactions, users, line, a, prev_block_hash, time_limit);
        }
        auto pab = chrono::high_resolution_clock::now();
        time_taken += chrono::duration_cast<chrono::nanoseconds>(pab - pr).count()*(1e-9);

        if(line.size()==0){
            if(time_limit==5)
                cout << "\nBlock " << i+1 << " was not found in 5 sec. Extending time to 10 sec..." << endl;
            
            else
                cout << "\nBlock " << i+1 << " was not found in 10 sec. Repeating..." << endl;

            time_limit = 10;
            --i;
            continue;
        }
        blockchain.push_back(block(line[0].prev_hash, line[0].timestamp, line[0].version, line[0].merkleroot, line[0].nonce, line[0].dif_target, line[0].transactions));
        outputBlocks(i+1, line[0], time_taken);

        transactions.erase(transactions.begin()+line[0].a, transactions.begin()+line[0].a+100);
        time_limit = 5;
        time_taken = 0;
        line.clear();
    }
}

int main(){
    vector<user> users;
    vector<transaction> transactions;
    vector<block> blockchain;
    blockchain.reserve(100);

    cout << "Generate new users? (y/n) ";
    string gen;
    cin >> gen;
    if (gen=="y")
        generateUsers();
    
    readUsers(users);
    generateTransactions(transactions, users);

    createBlock(blockchain, transactions, users);

    return 0;
}