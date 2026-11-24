#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
//   starplot
#define RADIUS 300
#define LATITUDE 36
#define LIMIT_MAG 5
#define MARKER_SIZE 4
#define HOSEI 0.7
#define SVG_STEP 5

using namespace std;

string output = "";


class Path{
    public:
        string st = "\
<path\n\
       style=\"fill:none;fill-opacity:1;stroke-width:0.3;stroke:#ff0000;stroke-opacity:1;stroke-miterlimit:4;stroke-dasharray:none\"\n\
       d=\"M\
    ";
        void plot(double &x , double &y){
            st +=  " " + to_string(x) + " " + to_string(y) + " L"; // 2.325 -23.23 L
        }
        void end(bool isClose){
            st.pop_back();
            if(isClose)
                st += "Z";
            st += "\
     \" \
\n />\n";
        }
};
void zh(double v[3]){ //r sita phi |-> x y z  配列vを書き換える（ポインタわたし）
    double r = v[0];
    v[0] = r * cos(v[1]) * cos(v[2]);
    v[1] = r * sin(v[1]) * cos(v[2]);
    v[2] = r * sin(v[2]);
}
void rt(double v[3] , double q){ // y軸回り左ネジでq rad まわす
    double x = v[0];
    v[0] = v[0] * cos(q) - v[2] * sin(q);
    v[1] = v[1];
    v[2] = x * sin(q) + v[2] * cos(q);
}
double toRad(double deg){
    return deg * M_PI / 180;
}
double toDeg(double rad){
    return rad * 180 / M_PI;
}
void stX(double v[3] ,double r){ //点を(x,y,z)=(0,0,-r)からのステレオ投影
    double dec = sqrt(pow(v[0],2)+pow(v[1],2));
    dec = v[2] / dec;
    dec = atan(dec);
    v[2] = v[2] + r;//tmp
    v[0] = (r * v[0]) / v[2];
    v[1] = (r * v[1]) / v[2];
    v[2] = 0;
    double d = v[0]*v[0] + v[1]*v[1];
    d = sqrt(d);
    d = (90 - toDeg(dec)) / d;
    d =  (d * RADIUS / (2 * (180 - LATITUDE)));
    d = d * HOSEI;
    d = 1 - HOSEI + d;
    v[0] = v[0] * d;
    v[1] = v[1] * d; 
}
void calX(double v[3] ,const double &r , double &sita , double &phi , const double &q){
    v[0] = r;
    v[1] = sita;
    v[2] = phi;

    zh(v);
    rt(v , q);
    stX(v , r);
}
double calStep(const double &r , double &x , double &y){
    double d = sqrt(x*x + y*y);
    double tmp = atan2(d , r);
    tmp = tmp - atan2(d-SVG_STEP , r);
    tmp = abs(tmp);
    return tmp;
}

vector<string> split(const string &s, char delim) {//参照渡し s
    vector<string> elems;
    string item;
//    for (char ch: s) { // = for(int i=0;i<sizeof(s)/sizeof(string);i++) //sの各要素についてループ chには要素が代入される
    for(int i=0;i<s.size();i++){
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
void plotStar(int grade , double x , double y){
    output = "\n\
    <use xlink:href=\"#grade"
    + to_string(grade) +
    "\" x=\""
    + to_string(x) +
    "\" y=\""
    + to_string(y) +
    "\" />\
    ";
}
//極座標版
void stP(double v[] , const double &r ){
    double sita = v[0];
    double phi = v[1];
    v[0] = r * cos(sita) * tan( (M_PI - (2*phi)) / 4 );
    v[1] = r * sin(sita) * tan( (M_PI - (2*phi)) / 4 );
}
void calP(double v[] , const double &r , double &ra , double &dec){
                v[0] = ra;
                v[1] = dec;
                stP(v,r);
                double d = v[0]*v[0] + v[1]*v[1];
                d = sqrt(d);
                d = (90 - toDeg(dec)) / d;
                d =  (d * RADIUS / (2 * (180 - LATITUDE)));
                d = d * HOSEI;
                d = 1 - HOSEI + d;
                v[0] = v[0] * d;
                v[1] = v[1] * d; 
                //cout << "ra " << toDeg(ra) << "dec " << toDeg(dec) << endl;
}
// 単位hをわたせ
double calSunPos(double M){
        double E, tmp, f;
        float a,b,c;
        const float e = 0.01672;

        M = M / 8765.81256;
        M = M * 2 * M_PI ;//平均近点角rad
        cout << toDeg(M) << "\n";
        a = (M / (1 - e));
        b = (M + e);
        c = ((M + M_PI * e)/(1 + e));
        //どれが一番小さいか（安定して収束させるためのE0を求める
            if(a < b){
                if(a < c){
                    E = a;
                }else{
                    E = c;
                }
            }else{ // b < a
                if(b < c){
                    E = b;
                }else{
                    E = c;
                }
            }
        for(int i=0; i<5; i++){
            tmp = E - (e * sin(E)) -M;//分母
            tmp = tmp /( 1 - e * cos(E));//分子
            tmp = E - tmp;//
            if(tmp == E){
                E = tmp;
                break;
            }
            E = tmp;
        } 
        tmp = cos(E); 
        f = (tmp - e)/(1 - e * tmp);
        f = acos(f);
        cout << toDeg(f) << "\n";

        //cosなので半分越えてたら反転
        if( M > M_PI){
            f = -f;
        }
        //軌道のかたむき
        f += toRad( 102.950 );
        return f;
}

int main(){

    const double q = toRad(90 - LATITUDE);
    const double r =  RADIUS / (2 * tan( (M_PI + (2 * q))/4 ));
    ofstream outputfile("starMap.svg");
    outputfile << "\
<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n\
<!-- Created by legrs with c++ -->\n\
\n\
<svg\n\
   width=\"210mm\"\n\
   height=\"297mm\"\n\
   viewBox=\"0 0 210 297\"\n\
   version=\"1.1\"\n\
   id=\"svg103863\"\n\
   xmlns=\"http://www.w3.org/2000/svg\"\n\
   xmlns:svg=\"http://www.w3.org/2000/svg\">\n\
  <defs>\n\
  <g\n\
     id=\"grade1\">\n\
    <circle\n\
       style=\"fill:#ffffff;fill-opacity:1;stroke:#000000;stroke-width:" + to_string(0.17*MARKER_SIZE) + ";stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g0\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.43*MARKER_SIZE) + "\" />\n\
    <circle\n\
       style=\"fill:#000000;fill-opacity:1;stroke:none;stroke-width:0;stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g1\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.125*MARKER_SIZE) + "\" />\n\
  </g>\n\
  \n\
  \n\
  <g\n\
     id=\"grade2\">\n\
    <circle\n\
       style=\"fill:#ffffff;fill-opacity:1;stroke:#000000;stroke-width:" + to_string(0.13*MARKER_SIZE) + ";stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g2\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.4*MARKER_SIZE) + "\" />\n\
  </g>  \n\
\n\
\n\
  <g\n\
     id=\"grade3\">\n\
    <circle\n\
       style=\"fill:#000000;fill-opacity:1;stroke:none;stroke-width:0;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g3\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.3*MARKER_SIZE) + "\" />\n\
  </g>\n\
\n\
  <g\n\
     id=\"grade4\">\n\
    <circle\n\
       style=\"fill:#000000;fill-opacity:1;stroke:none;stroke-width:0;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g4\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.2*MARKER_SIZE) + "\" />\n\
  </g>\n\
\n\
  <g\n\
     id=\"grade5\">\n\
    <circle\n\
       style=\"fill:#000000;fill-opacity:1;stroke:none;stroke-width:0;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g5\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(0.1*MARKER_SIZE) + "\" />\n\
  </g>\n\
  </defs>\n\
";


    //radius
    outputfile << "\
    <circle\n\
       style=\"fill:#fff4db;fill-opacity:1;stroke:#000000;stroke-width:0;stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g0\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"" + to_string(RADIUS/2) + "\" />\n\
    ";
    outputfile << "\
    <circle\n\
       style=\"fill:none;fill-opacity:1;stroke:#000000;stroke-width:0stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       id=\"g0\"\n\
       cx=\"0\"\n\
       cy=\"0\"\n\
       r=\"1.5\" />\n\
    ";
    outputfile << "\
    <line x1=\"" + to_string(RADIUS/-2) +"\" y1=\"0\" x2=\"" + to_string(RADIUS/2) + "\" y2=\"0\" stroke=\"#000\" style=\"stroke-width:0.3;\"/>\n";
    outputfile << "\
    <line x1=\"0\" y1=\"" + to_string(RADIUS/-2) +"\" x2=\"0\" y2=\"" + to_string(RADIUS/2) + "\" stroke=\"#000\"  style=\"stroke-width:0.3;\"/>\n";
    double v[3];
    double sita , phi;
    double tmp;
    phi = 0;
    sita = 0;

    // for debug
    /*
    sita = 0;
    phi = 0;
    calP(v,r,sita,sita);
    outputfile << "\
    <circle\n\
       style=\"fill:#000000;fill-opacity:1;stroke:#000000;stroke-width:0stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"\n\
       cx=\"" <<v[0]<< "\"\n\
       cy=\"" <<v[1]<< "\"\n\
       r=\"1\" />\n\
    ";
    */
    Path ecliptic;
    while ( sita < 2 * M_PI){  //黄道
        v[0] = r;
        v[1] = sita;
        v[2] = phi;

        zh(v);
        rt(v , toRad(23.4));

        tmp = v[0]; //90度かいてん
        v[0] = v[1];
        v[1] = tmp;

        stX(v , r);
        cout << " r " << r<< " sita " << toDeg(sita)<< " phi " << toDeg(phi) << " x " <<v[0]<< "y " << v[1]<< endl;
        ecliptic.plot(v[0],v[1]);
        sita += calStep(r,v[0],v[1]);
    }
    ecliptic.end(true);
    outputfile << ecliptic.st;
    const int dateQ[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    double time = -120; //1/5で近日点 5*24 = 120
    double sita2,phi1;
    cout << "ここから\n";
    for( int i=0; i<12; i++){
        if( time < 0 ){
            time += 8765.81256;
        }else if(8765.81256 < time){
            time -=8765.81256;
        }
        sita = calSunPos(time);
        v[0] = r;
        v[1] = sita;
        v[2] = 0;

        zh(v);
        tmp = v[1];
        v[1] = tmp * cos(toRad(23.4));
        v[2] = tmp * sin(toRad(23.4));

        stX(v, r);

        //cout <<"time "<< time << " r " << r<< " sita " << toDeg(sita)<< " sita2 " << toDeg(sita2)<< " phi1 " << toDeg(phi1) << " x " <<v[0]<< "y " << v[1]<< endl;
        
        float scaling = 1.1;
        outputfile << "\
        <line x1=\"" + to_string(v[0]*scaling) +"\" y1=\"" + to_string(v[1]*scaling) +"\" x2=\"" + to_string(v[0]/scaling) + "\" y2=\"" + to_string(v[1]/scaling) +"\" stroke=\"#000\" style=\"stroke-width:0.3;stroke:#ff0000;\"/>\n";
        time += dateQ[i]*24;


    }
    
    ifstream file("./hygdata_v41.csv");
    string line;
    float mag;
    double ra,dec;
    int counts[] = {0,0,0,0,0,0};
    stringstream tmpOut;
    getline(file, line);
    getline(file, line);
    //cout << "q " << toDeg(q)<< "r " << r << endl;
    for(int i=0;i<118217;i++){
        getline(file,line);
        try{
            mag = stof(split(line, ',')[13]);//13 mag 23 ra 24 dec
            ra = stof(split(line, ',')[23]);
            dec = stof(split(line, ',')[24]);
        }catch(const std::invalid_argument& e){
            cout << "invalid argument "+ to_string(i)+" "+split(line, ',')[13] << endl;
        }catch(const std::out_of_range& e){
            cout << "out of range" << endl;
        }
        if(split(line, ',')[1] == "113963")
            cout << toDeg(ra) << " " << toDeg(dec) << endl;
        if(mag <= LIMIT_MAG  &&  -q < dec){
            if(mag < 1.5){// 1
                calP(v , r , ra , dec);
                tmpOut << "1," + to_string(v[0]) +","+ to_string(v[1]) + "\n";
                plotStar(1,v[0],v[1]);
                outputfile << output;
                counts[0]++;
            }else if(mag < 2.5){ // 2
                calP(v , r , ra , dec);
                tmpOut << "2," + to_string(v[0]) +","+ to_string(v[1]) + "\n";
                plotStar(2,v[0],v[1]);
                outputfile << output;
                counts[1]++;
            }else if(mag < 3.5){ // 3
                calP(v , r , ra , dec);
                plotStar(3,v[0],v[1]);

                outputfile << output;
                counts[2]++;
            }else if(mag < 4.5){ // 4
                calP(v , r , ra , dec);
                plotStar(4,v[0],v[1]);

                outputfile << output;
                counts[3]++;
            }else if(mag < 5.5){ // 5
                calP(v , r , ra , dec);
                plotStar(5,v[0],v[1]);

                outputfile << output;
                counts[4]++;
            }else{   // 6等
                calP(v , r , ra , dec);
                plotStar(5,v[0],v[1]);

                outputfile << output;
                counts[5]++;
            }
        }
    }
    std::cout << " "+to_string(counts[0])+" "+to_string(counts[1])+" "+to_string(counts[2])+" "+to_string(counts[3])+" "+to_string(counts[4])+" "+to_string(counts[5]) << endl;
    
    string grade;
    for(int i=0;i<(counts[0] + counts[1] - 1);i++){ //plot二回目
        getline(tmpOut,line);
        grade = split(line , ',')[0];
        v[0] = stof(split(line , ',')[1]);
        v[1] = stof(split(line , ',')[2]);
        if(grade == "1"){
            plotStar(1,v[0],v[1]);
            outputfile << output;
        }else{
            plotStar(2,v[0],v[1]);
            outputfile << output;
        }
    }








    outputfile << "\n\
</svg>\
";
    outputfile.close();
     return 0;
}
