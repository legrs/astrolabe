#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

#define LATITUDE 36 //°
#define DEC_STEP 5 //°
#define RA_STEP 10 //°
#define SVG_STEP 5 //mm
#define RADIUS 140
#define HOSEI 0

using namespace std;

double toDeg(const double &rad){
    return rad * 180 / M_PI;
}
double toRad(const double &deg){
    return deg * M_PI / 180;
}
class Path{
    public:
        string st = "\
<path\
       style=\"fill:none;fill-opacity:0.511506;stroke-width:0.1;stroke:#000000;stroke-opacity:1;stroke-miterlimit:4;stroke-dasharray:none\"\
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
id=\"path979\" />";
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
void st(double v[3] ,double r){ //半径rの球場の点を(x,y,z)=(0,0,-1)からのステレオ投影
    v[2] = v[2] + r;//tmp
    v[0] = (r * v[0]) / v[2];
    v[1] = (r * v[1]) / v[2];
    v[2] = 0;
}
void cal(double v[3] ,const double &r , double &sita , double phi , const double &q){
    v[0] = r;
    v[1] = sita;
    v[2] = phi;

    zh(v);
    rt(v , q);
    phi = asin(v[2]/r);//回転後のphi
    st(v , r);
    double d = sqrt(v[0]*v[0] + v[1]*v[1]);
    d = (90 - toDeg(phi)) / d;
    d =  (d * RADIUS / (2 * (180 - LATITUDE)));
    d = d * HOSEI;
    d = 1 - HOSEI + d;
    v[0] = v[0] * d;
    v[1] = v[1] * d; 

}
double calStep(const double &r , double &x , double &y){
    double d = sqrt(x*x + y*y);
    double tmp = atan2(d , r);
    tmp = tmp - atan2(d-SVG_STEP , r);
    tmp = abs(tmp);
    return tmp;
}

int main(){
    
    ofstream outputfile("line.svg");
    outputfile <<  "\
<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\
<!-- Created with Inkscape (http://www.inkscape.org/) -->\
\
<svg\
   width=\"120mm\"\
   height=\"120mm\"\
   viewBox=\"0 0 120 120\"\
   version=\"1.1\"\
   id=\"svg5\"\
   inkscape:version=\"1.1.2 (0a00cf5339, 2022-02-04)\"\
   sodipodi:docname=\"line.svg\"\
   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\
   xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\
   xmlns=\"http://www.w3.org/2000/svg\"\
   xmlns:svg=\"http://www.w3.org/2000/svg\">\
  <defs\
     id=\"defs2\" />\
  <g\
     inkscape:label=\"Layer 1\"\
     inkscape:groupmode=\"layer\"\
     id=\"layer1\">\
     ";

    const double q = (M_PI / 2) - toRad(LATITUDE);
    const double r =  RADIUS / (2 * tan( (M_PI + 2 * toRad(23.4))/4 ));
    const double Cq = cos(q);
    const double Sq = sin(q);
    const double stepL = 1; //mm
    const double stepD = toRad(DEC_STEP) ; 
    const double stepR = toRad(RA_STEP);
    //const double A = cos(toRad(LATITUDE)) / sin(toRad(LATITUDE));
    const double A = cos(q) / sin(q);
    const double B = - tan(q);
    double v[3]; //  r ,    sita ,   phi 
    //           =   v[0]   v[1]     v[2]
    double p , sita , phi;
    
        cout <<" RADIUS " << RADIUS << " LATITUDE " << LATITUDE << " q " << toDeg(q) << " r " << r << " z " << v[2] << endl;


    v[0] = r;

    Path circle;
    sita = 0;
    while( sita < 2 * M_PI ){
        v[0] = RADIUS/2;
        v[1] = sita;
        v[2] = 0;
        zh(v);
        circle.plot(v[0] , v[1]);
        sita += 0.05;
    }
    circle.end(true);
    outputfile << circle.st;



    //sita = 0; phi = toRad(-170);
        v[0] = r;
        v[1] = sita;
        v[2] = phi;
        zh(v);
        cout <<" sita " << toDeg(sita) << " phi " << toDeg(phi) << " x " << v[0] << " y " << v[1]<< " z " << v[2] << endl;


    sita = 0;
    phi = M_PI /2;//天頂から
    double end;
    if(B < -1){
        end = -M_PI/2;
    }else{
        end = -2 * q;
    }
    while( end <= phi){
        if( 0 <= phi || phi <= -(M_PI - (2 * q))){ //天球上半分
            Path path;
            sita = 0;
            while( sita < 2 * M_PI ){
                cal(v , r , sita , phi , q);
                path.plot(v[0] , v[1]);
                sita += calStep(r , v[0] , v[1]);
            }
            path.end(true);
            outputfile << path.st;
        }else{
            Path path;
            p = A * sin(phi);
            p = 1 + p;
            p = acos(p/cos(phi));

            p = p - toRad(1);//少しはみ出させる
            cout << "phi " << toDeg(phi) << " p " << toDeg(p)<< " B " << B << endl;
            if( B < sin(phi) ){
                cout << "hoge" << endl;
                sita = M_PI - p;
                while( 0 < sita ){
                    cal(v , r , sita , phi , q);
                    path.plot(v[0] , v[1]);
                    sita -= calStep(r , v[0] , v[1]);
                }
                sita = 2 * M_PI;
                while(  M_PI + p < sita ){
                    cal(v , r , sita , phi , q);
                    path.plot(v[0] , v[1]);
                    sita -= calStep(r , v[0] , v[1]);
                }
            }else if( sin(phi) <= B ){
                cout << "phi" << endl;
                sita = p;
                while( 0 < sita ){
                    cal(v , r , sita , phi , q);
                    path.plot(v[0] , v[1]);
                    sita -= calStep(r , v[0] , v[1]);
                }
                sita = 2 * M_PI;
                while( (2 * M_PI) - p < sita ){
                    cal(v , r , sita , phi , q);
                    path.plot(v[0] , v[1]);
                    sita -= calStep(r , v[0] , v[1]);
                }
            }
            path.end(false);
            outputfile << path.st;
        }         

        //薄明線のために
        if( -16 < toDeg(phi) && toDeg(phi) < -14){
            phi = toRad(-18);
            cout << "aaaaaaaa" << endl;
        }else if(-19 < toDeg(phi) && toDeg(phi) < -17){
            phi = toRad(-20);
        }else{
            phi -= stepD;
        }
        cout << "aaaaaaaa "<<toRad(-12)<<" "<<toRad(-16) << " "<< toDeg(phi) << endl;

    }
    

    sita = M_PI / 2;
    phi = 0;
    const double a = r * cos((M_PI/2) - q);
    double ad , s , sita2;
    while( sita < 6 * M_PI / 4 ){
        if(M_PI - sita != 0) //ゼロ除算防止
            ad = a / cos(M_PI - sita); // 切り取る線分までの距離
        if( 0 < ad && ad <= r ){//切り取られるかどううあ
            s = acos(ad/r);
        }else{
            s = 0;
        }
        if( sita <= M_PI ){//sitaの逆がわ
            sita2 = sita + M_PI;
        }else{
            sita2 = sita - M_PI;
        }
        phi = -toRad(LATITUDE) + s;
        cout <<" sita " << toDeg(sita) << " phi " << toDeg(phi) << " a " << a<< " ad " << ad << endl;
        Path path;

        while( phi < M_PI / 2 ){
            cal(v , r , sita , phi , q);
            path.plot(v[0] , v[1]);
            phi += calStep(r , v[0] , v[1]);
        }
        phi = M_PI / 2;
         if( toRad(LATITUDE) + s > M_PI / 2 ){ //だい4象限まででおわり
             end = -M_PI + toRad(LATITUDE) + s;
             cout << "dai4" << endl;
         }else{
             -M_PI / 2;
             cout << "dai3" << endl;
         }
        while( end < phi ){
            cal(v , r , sita2 , phi , q);
            path.plot(v[0] , v[1]);
            phi -= calStep(r , v[0] , v[1]);
        }
        phi = -M_PI / 2;
        while( phi <= -toRad(LATITUDE) - s ){
            cal(v , r , sita , phi , q);
            path.plot(v[0] , v[1]);
            phi += calStep(r , v[0] , v[1]);
        }

        if( 0 < ad && ad <= r ){//切り取られるかどううあ
            path.end(false);
        }else{
            path.end(true);
        }
        outputfile << path.st;
        sita += toRad(RA_STEP);
    }

    outputfile << "\
  </g>\
</svg>\
";
    outputfile.close();
    return 0;
}
