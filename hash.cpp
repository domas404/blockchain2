#include <string>
#include <vector>

using namespace std;

string to_nBase(int num, int n){
    string el = "0123456789abcdef";
    vector<int> liek;
    string fnum="";
    int z=0;
    int dal;
    do{
        dal = num/n;
        liek.push_back(num%n);
        num = dal;
        z++;
    } while(num>0);

    if(n==2 && liek.size()<8){
        for(int i=0; i<8-liek.size(); ++i){
            fnum += "0";
        }
    }
    for (int j=z-1; j>=0; j--)
        fnum += el[liek[j]];
    return fnum;
}

void hashfunc(string input, string &hex_val, vector<string> &bi_value){
    int tarp=0;
    hex_val="";
    int a=0;
    int n=input.length();
    if(input==""){
        while(hex_val.length()<64){
            for(int i=0; i<1; ++i){
                tarp = (tarp + a)%256;
                a += 50;
            }
            hex_val += to_nBase(tarp, 16);
            bi_value.push_back(to_nBase(tarp, 2));
        }
    }
    else{
        a = ((int(input[0])+256)/2)%256;
        while(hex_val.length()<64){
            for(int i=0; i<n; ++i){
                tarp = (a + i*(n-i) + i)%256;
                a += (int(input[i])+256)%256;
            }
            tarp = abs(tarp);
            hex_val += to_nBase(tarp, 16);
            bi_value.push_back(to_nBase(tarp, 2));
        }
    }
    if(hex_val.length()>64){
        hex_val.pop_back();
    }
    if(bi_value.size()>64){
        bi_value.pop_back();
    }
    // cout << "#" << hash_val << " hash length: " << hash_val.length() << endl;
    // return hash_val;
}
string hashfunc(string input){
    string hash_val="";
    int tarp=0;
    int a=0;
    int n=input.length();
    if(input==""){
        while(hash_val.length()<64){
            for(int i=0; i<1; ++i){
                tarp = (tarp + a)%256;
                a += 50;
            }
            hash_val += to_nBase(tarp, 16);
        }
    }
    else{
        a = ((int(input[0])+256)/2)%256;
        while(hash_val.length()<64){
            for(int i=0; i<n; ++i){
                tarp = (a + i*(n-i) + i)%256;
                a += (int(input[i])+256)%256;
            }
            tarp = abs(tarp);
            hash_val += to_nBase(tarp, 16);
        }
    }
    if(hash_val.length()>64){
        hash_val.pop_back();
    }
    // cout << "#" << hash_val << " hash length: " << hash_val.length() << endl;
    return hash_val;
}