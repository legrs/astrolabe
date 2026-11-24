#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector> //vectorは便利な動的な配列のようなもの

using namespace std;




///// これは描画にはつかってないよ







vector<string> split(const string &s, char delim) {//参照渡し s
    vector<string> elems;
    string item;
//    for (char ch: s) { // = for(int i=0;i<sizeof(s)/sizeof(string);i++) //sの各要素についてループ chには要素が代入される
    for(int i=1;i<s.size();i++){
        if (s[i] == delim) {//くぎりもじ
            elems.push_back(item); //vector末尾に追加
            item.clear(); //vectorで全要素を削除
        }
        else {
            item += s[i];
        }
    }
    elems.push_back(item);
    return elems;
}

const float lat;//latitude
//const float 
int main(){
    ifstream file("hygdata_v41.csv");
    string line;
    float mag;
    int counts[] = {0,0,0,0,0,0};
    string outs;
    getline(file, line);
    getline(file, line);
    for(int i=0;i<10000;i++){
        getline(file,line);
        try{
            mag = stof(split(line, ',')[13]);
        }catch(const std::invalid_argument& e){
            cout << "invalid argument "+ to_string(i)+" "+split(line, ',')[13] << endl;
        }catch(const std::out_of_range& e){
            cout << "out of range" << endl;
        }
        if(mag < 6.5){
            if(mag < 2){
                outs = ",b、";
                counts[0]++;
            }else if(mag < 3){ // 2
                outs = "O";
                counts[1]++;
            }else if(mag < 4){ // 3
                outs = ",●、";
                counts[2]++;
            }else if(mag < 5){ // 4
                outs = "-●";
                counts[3]++;
            }else if(mag < 6){ // 5
                outs = "●";
                counts[4]++;
            }else{   // 6等
                outs = ".";
                counts[5]++;
            }
            std::cout << outs << endl;        
        }
    }

    std::cout << " "+to_string(counts[0])+" "+to_string(counts[1])+" "+to_string(counts[2])+" "+to_string(counts[3])+" "+to_string(counts[4])+" "+to_string(counts[5]) << endl;
}

