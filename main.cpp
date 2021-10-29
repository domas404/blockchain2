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
            // copy(transactions.begin(), transactions.end(), this->transactions.begin());
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

// transakciju hash'u eilutes hash'avimas
void merkle_hash(vector<transaction> &transactions, string &merkleroot){ 
    string tran="";              // continious string of all transactions within a block
    for(int i=0; i<transactions.size(); ++i)
        tran += transactions[i].id;

    merkleroot = sha256(tran);
}

// void merkleTree(vector<transaction> &transactions, string &merkleroot){
//     for(int i=0; i<transactions.size(); ++i){
        
//     }
// }

// ivykdomos transakcijos, jei siuntejas turi pakankama balansa
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

void outputBlocks(vector<block> &blockchain, string header_hash, double time_taken, int num_of_tr, int height){
    cout << "\nBlock height: " << height << endl;
    cout << "Previous hash: " << blockchain[height-1].prev_hash << endl;
    cout << "Timestamp: " << blockchain[height-1].timestamp << endl;
    cout << "Version: " << blockchain[height-1].version << endl;
    cout << "Merkleroot: " << blockchain[height-1].merkleroot << endl;
    cout << "Nonce: " << blockchain[height-1].nonce << endl;
    cout << "Difficulty target: " << blockchain[height-1].dif_target << endl;
    cout << "Block hash: " << header_hash << endl;
    cout << "Time spent mining: " << time_taken << " s" << endl;
    cout << "Number of transactions: " << num_of_tr << endl;
}

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
    merkle_hash(trans, merkleroot);

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
    // cout << omp_get_thread_num() << " " << header_hash << " " << time_taken << " " << nonce << " " << trans.size() << endl;
    trans.clear();
}

// kasami blokai
void createBlock(vector<transaction> &transactions, vector<user> &users){
    
    auto pr = chrono::high_resolution_clock::now();
    vector<block> blockchain;
    vector<block> line;
    float version = 1;
    string prev_hash="0000000000000000000000000000000000000000000000000000000000000000"; // prev hash for genesis block
    int dif_target=100;
    int nonce;
    string merkleroot;       // transactions hash
    vector<transaction> trans;   // transactions within a single block
    double time_taken;
    string header_hash;
    int a;  // random 100 vien po kitos einancios transakcijos
    string header_line1, header_line2;
    string prev_block_hash;

    omp_set_num_threads(5);
    #pragma omp parallel
    {
        time_t timestamp;
        string difficulty = to_lil_endian(to_string(dif_target));
        nonce=0;
        string header="";
        header_hash="";        

        // first block

        a = rand()%(transactions.size()-100);
        trans.assign(transactions.begin()+a, transactions.begin()+a+100);
        executeTransactions(trans, users);
        merkle_hash(trans, merkleroot);
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
        } while(difficulty.compare(header_hash) < 0);
        line.push_back(block(prev_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        cout << header_hash << " " << omp_get_thread_num() << endl;
    }
    blockchain.push_back(line[0]);
    // blockchain.push_back(block(prev_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));

    auto pab = chrono::high_resolution_clock::now();
    time_taken = chrono::duration_cast<chrono::nanoseconds>(pab - pr).count();
    time_taken *= 1e-9;
    
    outputBlocks(blockchain, header_hash, time_taken, trans.size(), 1);
    header_hash = "";
    nonce=0;
    trans.clear();
    transactions.erase(transactions.begin()+a, transactions.begin()+a+100);
    

    // for(int i=1; i<100; ++i){
    //     // ++nonce;
    //     auto pr = chrono::high_resolution_clock::now();
    //     a = rand()%(transactions.size()-100);
    //     trans.assign(transactions.begin()+a, transactions.begin()+a+100);
    //     executeTransactions(trans, users);
    //     merkle_hash(trans, merkleroot);

    //     // mining blocks
    //     prev_block_hash = sha256(
    //         to_lil_endian(to_nBase((int)blockchain[i-1].version, 16)) +
    //         to_lil_endian(blockchain[i-1].prev_hash) +
    //         to_lil_endian(blockchain[i-1].merkleroot) +
    //         to_lil_endian(to_nBase((int)blockchain[i-1].timestamp, 16)) +
    //         to_lil_endian(to_nBase(blockchain[i-1].dif_target, 16)) +
    //         to_lil_endian(to_nBase(blockchain[i-1].nonce, 16))
    //     );
        
    //     header = (
    //         to_lil_endian(to_nBase(int(version), 16)) +
    //         to_lil_endian(prev_block_hash) +
    //         to_lil_endian(merkleroot)
    //     );
        
    //     do{
    //         ++nonce;
    //         header_hash = sha256(
    //             header +
    //             to_lil_endian(to_nBase((int)time(nullptr), 16)) +
    //             to_lil_endian(to_nBase(dif_target, 16)) +
    //             to_lil_endian(to_nBase(nonce, 16))
    //         );
    //         header_hash = sha256(header_line1);
    //     } while(difficulty.compare(header_hash) < 0);
        
    //     blockchain.push_back(block(prev_block_hash, time(nullptr), version, merkleroot, nonce, dif_target, trans));
        
    //     auto pab = chrono::high_resolution_clock::now();
    //     time_taken = chrono::duration_cast<chrono::nanoseconds>(pab - pr).count();
    //     time_taken *= 1e-9;

    //     outputBlocks(blockchain, header_hash, time_taken, trans.size(), i+1);

    //     header_hash = "";
    //     nonce=0;
    //     trans.clear();
    //     transactions.erase(transactions.begin()+a, transactions.begin()+a+100);
    // }
}

void outputUsers(string fileName, vector<user> &users){
    ofstream out(fileName);
    vector<user>::iterator it=users.begin();
    for(it; it!=users.end(); ++it){
        out << (*it).getName() << " " << (*it).getBalance() << endl;
    }
    out.close();
}

// omp_lock_t simple_lock;

int main(){
    // clock_t p, pb;
    // int sum=0, i=0;
    // p = clock();
    // do{
    //     ++i;
    //     sum+=i;
    //     pb = clock()-p;
    // } while(sum < 1000000 && pb/CLOCKS_PER_SEC < 3);
    // cout << sum << " " << i << " " << ((float)pb)/CLOCKS_PER_SEC << endl;
    // return 0;

    vector<user> users;
    vector<transaction> transactions;
    vector<block> blockchain;
    vector<queue> line;

    cout << line.size() << endl;

    int time_limit = 5;
    blockchain.reserve(100);
    string prev_block_hash="0000000000000000000000000000000000000000000000000000000000000000";

    cout << "Generate new users? (y/n) ";
    string gen;
    cin >> gen;
    if (gen=="y")
        generateUsers();
    
    readUsers(users);
    generateTransactions(transactions, users);

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
    outputBlocks(blockchain, line[0].block_hash, line[0].time, line[0].num_transact, 1);

    cout << "Thread that mined the block: " << line[0].num_thread << endl;
    cout << "Time since last block: " << time_taken << endl;

    transactions.erase(transactions.begin()+line[0].a, transactions.begin()+line[0].a+100);
    time_taken = 0;
    line.clear();
    
    for(int i=1; i<100; ++i){
        auto pr = chrono::high_resolution_clock::now();
        #pragma omp parallel
        {
            int a;
            prev_block_hash = sha256(
                to_lil_endian(to_nBase((int)blockchain[i-1].version, 16)) +
                to_lil_endian(blockchain[i-1].prev_hash) +
                to_lil_endian(blockchain[i-1].merkleroot) +
                to_lil_endian(to_nBase((int)blockchain[i-1].timestamp, 16)) +
                to_lil_endian(to_nBase(blockchain[i-1].dif_target, 16)) +
                to_lil_endian(to_nBase(blockchain[i-1].nonce, 16))
            );
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
        outputBlocks(blockchain, line[0].block_hash, line[0].time, line[0].num_transact, i+1);
        cout << "Thread that mined the block: " << line[0].num_thread << endl;
        cout << "Time since last block: " << time_taken << endl;
        transactions.erase(transactions.begin()+line[0].a, transactions.begin()+line[0].a+100);
        time_limit = 5;
        time_taken = 0;
        line.clear();
    }

    return 0;

    // #pragma omp parallel sections
    //     #pragma omp section
    // omp_set_num_threads(4);
    //         cout << omp_get_num_threads() << endl;
    
    // cout << "Generate new users? (y/n) ";
    // string gen;
    // cin >> gen;
    // if (gen=="y")
    //     generateUsers();
    
    // readUsers(users);
    // generateTransactions(transactions, users);

    // omp_init_lock(&simple_lock);
    // #pragma omp parallel num_threads(6)
    //     {
    //        int tid = omp_get_thread_num();
            //createBlock(transactions, users);
            
    //    }
    // createBlock(transactions, users);

    return 0;
}