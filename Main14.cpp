#include<iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
using namespace std;
typedef struct point
{
    float x;
    float y;
}Point;
typedef struct size
{
    float length;
    float depth;
}Size;
typedef struct straightSection
{
    float x1, y1;
    float x2, y2;
}Intersection;
typedef struct rectangleCoordinates
{
    float x1, y1, x2, y2;
    float x3, y3, x4, y4;
}Coordinates;
typedef struct inforIDPoint
{
    int ID;
    float x, y;
}InformationIDPoint;
typedef struct inforIDEdge
{
    int ID1, ID2;
    float space;
}InformationIDEdge;

class Bai14
{
private:
    Point Rectangle[4];                 // tọa độ 4 đỉnh mảnh đất chia cho khoa/viện
    int M, N;                           // số khoa/viện ở chiều ngang và sâu
    Size Building;                      // chiều dài và sâu của 1 khoa/viện
    float d1, d2, d3;                   // độ rộng đường bao d1 và d2 theo d1, d3 là khoảng cách giữa các đường năm giữa đường giao cắt
    Point oldRectangleCoordinates;      // tọa độ hcn cũ 
    string firstLine, secondLine;       // 2 chuỗi nhập vào xác định in out 

    Size Area;                          // chiều dài và rộng của đất mỗi khoa/viện
    Size Space;                         // khoảng cách 2 đường bao dọc và ngang
    Size Land;                          // chiều dài và rộng của hcn cũ 
    vector<Point> Center;               // tọa độ tâm các khoa/viện
    Point newRectangle1[4];             // tọa độ 4 đỉnh đất mới(Rectangle1)
    Point newRectangle2[4];             // tọa độ 4 đỉnh đất mới(Rectangle2)
    vector<Coordinates> surroundedCoordinates; // tọa độ 4 đỉnh của vòng bao quanh khoa/viện
    vector<Intersection> intersection;  // tọa độ 2 mút đoạn giao cắt
    vector<Intersection> inIntersection;  // tọa độ 2 mút đoạn nằm giữa 2 giao cắt
    Point in[4], out[4];                // tọa độ 4 điểm vào và ra
    vector<InformationIDPoint> inforIDPoint; // stt và tọa độ các điểm gaio nhau
    vector<InformationIDEdge> inforIDEdge;  // ID các cạnh giao nhau
public:
    Bai14(Point r[], float m, float n, Size b, float de1, float de3, string f, string s) 
    {
        for (int i = 0; i < 4; ++i) {
            Rectangle[i] = r[i];
        }
        M = m;
        N = n;
        Building.length = b.length;
        Building.depth = b.depth;
        Center.resize(m * n);
        surroundedCoordinates.resize(m * n);
        d1 = de1;
        d2 = 2 * d1;
        firstLine = f;
        secondLine = s;
        d3 = de3;
    }
    void change() // đổi Rectangle->newRectangle2 
    {
        for(int i = 0; i < 4; i++){
            newRectangle2[i].x = Rectangle[i].x;
            newRectangle2[i].y = Rectangle[i].y;
        }
    }
    void swap(int &i, int &j) // đổi 2 giá trị
    {
        int temp = i;
        i = j;
        j = temp;
    }
    int findID(float x, float y) // tìm ID biết x và y
    {
        for(int i = 0; i < static_cast<int>(inforIDPoint.size()); i++){
            if(inforIDPoint[i].x == x && inforIDPoint[i].y == y){
                return i + 1;
            }
        }
        return -1;
    }

    bool checkEdge(float y1, float y2, float x) // kiểm tra cạnh đấy có đi qua Building không
    {
        for(int j = 0; j < M; j++){
            if(x < surroundedCoordinates[j].x2 && x > surroundedCoordinates[j].x1){
                for(int i = 0; i < N; i++){
                    if(y1 == surroundedCoordinates[i * M].y1 &&
                    y2 == surroundedCoordinates[i * M].y3){
                        return false;
                    }
                }
            }
        }
        return true;
    }
    bool checkColumn1(int i) // kiểm tra điểm i có nằm trong cột bên phải của luồng dọc ngoài không
    {   
        if(inforIDPoint[i - 1].x == surroundedCoordinates[0].x1 || 
           inforIDPoint[i - 1].x == newRectangle1[1].x){
            return true;
        }
        return false;
    }
    bool checkColumn2(int i) // kiểm tra điểm i có nằm trong cột bên trái của luồng dọc không
    {
        for(int j = 0; j < M; j++){
            if(inforIDPoint[i - 1].x == surroundedCoordinates[j].x2){
                return true;
            }
        }
        return false;
    }
    bool checkColumn3(int i) // kiểm tra i có nằm trong cột bên phải của luồng dọc trong
    {
        for(int j = 0; j < M - 1; j++){
            if(inforIDPoint[i - 1].x == surroundedCoordinates[j + 1].x1){
                return true;
            }
        }
        return false;
    }
    bool checkSwapColumn1(int k, int h) // kiểm tra i có nằm trên cạnh trái của đường bao ko
    {
        float x = inforIDPoint[k - 1].x;
        float maxy = max(inforIDPoint[k - 1].y, inforIDPoint[h - 1].y);
        float miny = min(inforIDPoint[k - 1].y, inforIDPoint[h - 1].y);
        if(x == newRectangle1[1].x){
            return true;
        }
        for(int i = 0; i < N; i++){
            for(int j = 0; j < M; j++){
                if(x == surroundedCoordinates[i * M + j].x1){
                    if(miny >= surroundedCoordinates[i * M + j].y1 && 
                    maxy <= surroundedCoordinates[i * M + j].y4){
                        return true;
                    }
                }
            }
        }
        return false;
    }
    bool checkSwapColumn2(int k) // kiểm tra i nằm cách surrounded.x1 lẻ hay chẵn
    {
        float x = inforIDPoint[k - 1].x;
        for(int i = 0; i < M; i++){
            if(x > surroundedCoordinates[i].x1 && x < surroundedCoordinates[i].x2){
                int delta = (int)(inforIDPoint[k - 1].x - surroundedCoordinates[i].x1 / d3);
                if(delta % 2 == 1){
                    return true;
                }
            }
        } 
        return false;
    }
    bool checkSwapRow1(int k) // Kiểm tra i cách newRetangle[0].x chẵn hay lẻ
    {
        if(inforIDPoint[k - 1].y == newRectangle1[3].y){
            return true;
        }
        int miny = findID(inforIDPoint[k - 1].x, newRectangle1[0].y);
        if((k - miny) % 2 == 1){
            return true;
        }
        return false;
    }
    bool checkSwapRow2(int k) // kiểm tre i có trùng với surrounded.y2 hay ko
    {
        int y = inforIDPoint[k - 1].y;
        if(y == newRectangle1[3].y){
            return true;
        }
        for(int j = 0; j < N; j++){
            if(y == surroundedCoordinates[j * M].y2){
                return true;
            }
        }
        return false;
    }

    void setCoordinates(Point point[], string s) // đọc tọa độ nhập từ string và lưu 
    {
        stringstream ss(s.substr(2));
            vector<float> numbers;
            char ch; 
            while (ss >> ch) {
                float number;
                if (isdigit(ch) || ch == '.') { 
                    ss.putback(ch); 
                    ss >> number; 
                    numbers.push_back(number);
                }
            }
            point[3].y = numbers.back();
            numbers.pop_back();
            point[3].x = numbers.back();
            numbers.pop_back();
            point[1].y = numbers.back();
            numbers.pop_back();
            point[1].x = numbers.back();
            numbers.pop_back();
    }

    void calculateInforIDPoint() // tính các tọa độ giao nhau
    {
        inforIDPoint = vector<InformationIDPoint>();
        InformationIDPoint temp;
        for(int i = 0; i <= 2 * M; i++){
            if(i % 2 == 0){
                if(i == 0){
                    calculateInforIDPoint1(inforIDPoint);
                } else if(i == 2 * M){
                    calculateInforIDPoint2(inforIDPoint);
                } else {
                    calculateInforIDPoint3(inforIDPoint, i / 2 - 1);
                }
            } else {
                calculateInforIDPoint4(inforIDPoint, (i - 1) / 2);
            }
        }
        temp.x = out[1].x;
        temp.y = min(out[1].y, out[3].y);
        inforIDPoint.push_back(temp);
        temp.y = max(out[1].y, out[3].y);
        inforIDPoint.push_back(temp);
        temp.x = in[1].x;
        temp.y = min(in[1].y, in[3].y);
        inforIDPoint.push_back(temp);
        temp.y = max(in[1].y, in[3].y);
        inforIDPoint.push_back(temp);
    }
    void calculateInforIDPoint1(vector<InformationIDPoint>& inforIDPoint)
    {
        InformationIDPoint temp;
        float begin = newRectangle1[0].y;
        float end = newRectangle1[3].y;
        temp.x = newRectangle1[0].x;
        int countTheNumber = 0;
        do{
            float t = begin;
            while(t <= end){
                while(t < surroundedCoordinates[0].y1){
                    temp.y = t;
                    inforIDPoint.push_back(temp);
                    t += d3;
                }
                for(int i = 0; i < N; i++){
                    t = surroundedCoordinates[i * M].y1;
                    while(t < surroundedCoordinates[i * M].y4){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                    if(i == N - 1){
                        break;
                    }
                    t = surroundedCoordinates[i * M].y4;
                    while(t < surroundedCoordinates[(i + 1) * M].y1){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                }
                t = surroundedCoordinates[N * M - 1].y4;
                while(t <= newRectangle1[3].y){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                }
            }
            temp.x = surroundedCoordinates[0].x1;
            countTheNumber++;
        } while(countTheNumber <= 1);
        
    }
    void calculateInforIDPoint2(vector<InformationIDPoint>& inforIDPoint)
    {
        InformationIDPoint temp;
        float begin = newRectangle1[1].y;
        float end = newRectangle1[2].y;
        temp.x = surroundedCoordinates[M - 1].x2;
        int countTheNumber = 0;
        do{
            float t = begin;
            while(t <= end){
                while(t < surroundedCoordinates[0].y1){
                    temp.y = t;
                    inforIDPoint.push_back(temp);
                    t += d3;
                }
                for(int i = 0; i < N; i++){
                    t = surroundedCoordinates[i * M].y1;
                    while(t < surroundedCoordinates[i * M].y4){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                    if(i == N - 1){
                        break;
                    }
                    t = surroundedCoordinates[i * M].y4;
                    while(t < surroundedCoordinates[(i + 1) * M].y1){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                }
                t = surroundedCoordinates[N * M - 1].y4;
                while(t <= newRectangle1[3].y){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                }
            }
            temp.x = newRectangle1[1].x;
            countTheNumber++;
        } while(countTheNumber <= 1); 
    }
    void calculateInforIDPoint3(vector<InformationIDPoint>& inforIDPoint, int value)
    {
        InformationIDPoint temp;
        float begin = newRectangle1[1].y;
        float end = newRectangle1[2].y;
        temp.x = surroundedCoordinates[value].x2;
        int countTheNumber = 0;
        do{
            float t = begin;
            while(t <= end){
                while(t < surroundedCoordinates[0].y1){
                    temp.y = t;
                    inforIDPoint.push_back(temp);
                    t += d3;
                }
                for(int i = 0; i < N; i++){
                    t = surroundedCoordinates[i * M].y1;
                    while(t < surroundedCoordinates[i * M].y4){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                    if(i == N - 1){
                        break;
                    }
                    t = surroundedCoordinates[i * M].y4;
                    while(t < surroundedCoordinates[(i + 1) * M].y1){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                    }
                }
                t = surroundedCoordinates[N * M - 1].y4;
                while(t <= newRectangle1[3].y){
                        temp.y = t;
                        inforIDPoint.push_back(temp);
                        t += d3;
                }
            }
            temp.x = surroundedCoordinates[value + 1].x1;
            countTheNumber++;
        } while(countTheNumber <= 1);
    }
    void calculateInforIDPoint4(vector<InformationIDPoint>& inforIDPoint, int value)
    {
        InformationIDPoint temp;
        float end = surroundedCoordinates[value].x2;
        float t = surroundedCoordinates[value].x1;
        while(t + d3 < end){
            t += d3;
            temp.x = t;
            for(int i = 0; i <= N; i++){
                if(i == 0){
                    temp.y = newRectangle1[0].y;
                    inforIDPoint.push_back(temp);
                    temp.y = surroundedCoordinates[0].y1;
                    inforIDPoint.push_back(temp);
                } else if(i == N){
                    temp.y = surroundedCoordinates[M * N - 1].y4;
                    inforIDPoint.push_back(temp);
                    temp.y = newRectangle1[3].y;
                    inforIDPoint.push_back(temp);                    
                } else {
                    temp.y = surroundedCoordinates[(i - 1) * M + value].y4;
                    inforIDPoint.push_back(temp);
                    temp.y = surroundedCoordinates[i * M + value].y1;
                    inforIDPoint.push_back(temp); 
                }
            }
        }
    }
    void calculateInforIDEdgeInOut(vector<InformationIDEdge>& inforIDEdge) // tính cạnh in out
    {
        InformationIDEdge temp;
        temp.space = d2;
        temp.ID2 = static_cast<int>(inforIDPoint.size()) - 3;
        temp.ID1 = findID(inforIDPoint[temp.ID2 - 1].x - d2, inforIDPoint[temp.ID2 - 1].y);
        inforIDEdge.push_back(temp);
        temp.ID1 = static_cast<int>(inforIDPoint.size()) - 1;
        temp.ID2 = findID(inforIDPoint[temp.ID1 - 1].x + d2, inforIDPoint[temp.ID1 - 1].y);
        inforIDEdge.push_back(temp);
        temp.ID1 = static_cast<int>(inforIDPoint.size());
        temp.ID2 = findID(inforIDPoint[temp.ID1 - 1].x + d2, inforIDPoint[temp.ID1 - 1].y);
        inforIDEdge.push_back(temp);
        temp.ID2 = static_cast<int>(inforIDPoint.size()) - 2;
        temp.ID1 = findID(inforIDPoint[temp.ID2 - 1].x - d2, inforIDPoint[temp.ID2 - 1].y);
        inforIDEdge.push_back(temp);
    }
    void calculateInforIDEdge() // tính các cạnh
    {
        inforIDEdge = vector<InformationIDEdge>();
        InformationIDEdge temp;
        for(int i = 0; i < static_cast<int>(inforIDPoint.size()) - 4; i++){
            if(inforIDPoint[i].x == inforIDPoint[i + 1].x){
                if(checkEdge(inforIDPoint[i].y, inforIDPoint[i + 1].y, inforIDPoint[i].x)){
                    temp.ID1 = i + 1;
                    temp.ID2 = i + 2;
                    temp.space = inforIDPoint[i + 1].y - inforIDPoint[i].y;
                    if(checkSwapColumn1(temp.ID1, temp.ID2) || checkSwapColumn2(temp.ID1)){
                        swap(temp.ID1, temp.ID2);
                    }
                    inforIDEdge.push_back(temp);
                }    
            }
        }
        calculateInforIDEdgeInOut(inforIDEdge);
        int begin = findID(surroundedCoordinates[0].x1, newRectangle1[0].y);
        for(int i = begin; i < static_cast<int>(inforIDPoint.size()) - 3; i++){
            if(checkColumn3(i)){
                temp.ID1 = i;
                temp.space = Space.length;
                temp.ID2 = findID(inforIDPoint[i - 1].x - temp.space, inforIDPoint[i - 1].y);
                if(checkSwapRow1(temp.ID1)){
                    swap(temp.ID1, temp.ID2);
                }
                inforIDEdge.push_back(temp);
            } else if(checkColumn2(i)){
                temp.ID1 = i;
                float tp = surroundedCoordinates[0].x2 - surroundedCoordinates[0].x1;
                temp.space = tp - floor(tp / d3) * d3;
                if(temp.space == 0){
                    temp.space = d3;
                }
                temp.ID2 = findID(inforIDPoint[i - 1].x - temp.space, inforIDPoint[i - 1].y);
                if(temp.ID2 != -1){
                    if(checkSwapRow2(temp.ID1)){
                        swap(temp.ID1, temp.ID2);
                    }
                    inforIDEdge.push_back(temp);
                }
            } else if(checkColumn1(i)){
                temp.ID1 = i;
                temp.space = d1 + Space.length / 2;
                temp.ID2 = findID(inforIDPoint[i - 1].x - temp.space, inforIDPoint[i - 1].y);          
                if(checkSwapRow1(temp.ID1)){
                    swap(temp.ID1, temp.ID2);
                }
                inforIDEdge.push_back(temp);
            } else {
                temp.ID1 = i;
                temp.space = d3;
                temp.ID2 = findID(inforIDPoint[i - 1].x - temp.space, inforIDPoint[i - 1].y);            
                if(checkSwapRow2(temp.ID1)){
                    swap(temp.ID1, temp.ID2);
                }
                inforIDEdge.push_back(temp);
            }
        }
    }
    void calculateNewLandCoordinates() // tính Rectangle biết Rectangle2(tính tâm và size của Rectangle)
    {
        oldRectangleCoordinates.x = (newRectangle2[1].x + newRectangle2[0]. x) / 2;
        oldRectangleCoordinates.y = (newRectangle2[2].y + newRectangle2[1]. y) / 2;
        Rectangle[0].x = oldRectangleCoordinates.x - (newRectangle2[1].x - newRectangle2[0]. x) / 2 + (d1 + d2);
        Rectangle[0].y = oldRectangleCoordinates.y - (newRectangle2[2].y - newRectangle2[1]. y) / 2 + (d1 + d2);
        Rectangle[1].x = oldRectangleCoordinates.x + (newRectangle2[1].x - newRectangle2[0]. x) / 2 - (d1 + d2);
        Rectangle[1].y = oldRectangleCoordinates.y - (newRectangle2[2].y - newRectangle2[1]. y) / 2 + (d1 + d2);
        Rectangle[2].x = oldRectangleCoordinates.x + (newRectangle2[1].x - newRectangle2[0]. x) / 2 - (d1 + d2);
        Rectangle[2].y = oldRectangleCoordinates.y + (newRectangle2[2].y - newRectangle2[1]. y) / 2 - (d1 + d2);
        Rectangle[3].x = oldRectangleCoordinates.x - (newRectangle2[1].x - newRectangle2[0]. x) / 2 + (d1 + d2);
        Rectangle[3].y = oldRectangleCoordinates.y + (newRectangle2[2].y - newRectangle2[1]. y) / 2 - (d1 + d2);
        Land.length = (Rectangle[1].x - Rectangle[0]. x);
        Land.depth = (Rectangle[2].y - Rectangle[1]. y);
    }
    void calculateArea() // tính diện tích đất chia đều cho các khoa/viện
    {
        Area.length = (Rectangle[1].x - Rectangle[0]. x) / M;
        Area.depth = (Rectangle[2].y - Rectangle[1]. y) / N;
    }
    void calculateCenterCoordinates() // tính tọa độ tâm các khoa/viện 
    {
        for(int i = 0; i < M * N; i++){
            Center[i].x = Rectangle[0].x + (i % M) * Area.length + Area.length / 2;
            Center[i].y = Rectangle[0].y + (i / M) * Area.depth + Area.depth / 2;
        }
    }
    void calculateSpace() // tính khoảng cách 2 đường bao dọc và ngang
    {
        Space.length = 2 * ((Area.length - Building.length) / 2 - d1);
        Space.depth = 2 * ((Area.depth - Building.depth) / 2 - d1);
    }
    void calculateSmallSurroundedCoordinates() // tính tọa độ vòng bao các khoa/viện
    {
        for(int i = 0; i < M * N; i++){
        surroundedCoordinates[i].x1 =  Center[i].x - Building.length / 2 - d1;
        surroundedCoordinates[i].y1 =  Center[i].y - Building.depth / 2 - d1;
        surroundedCoordinates[i].x2 =  Center[i].x + Building.length / 2 + d1;
        surroundedCoordinates[i].y2 =  Center[i].y - Building.depth / 2 - d1;
        surroundedCoordinates[i].x3 =  Center[i].x + Building.length / 2 + d1;
        surroundedCoordinates[i].y3 =  Center[i].y + Building.depth / 2 + d1;
        surroundedCoordinates[i].x4 =  Center[i].x - Building.length / 2 - d1;
        surroundedCoordinates[i].y4 =  Center[i].y + Building.depth / 2 + d1;
        }
    }
    void calculatenewRectangle1() // tính tọa độ newRectangle1
    {
        newRectangle1[0].x = oldRectangleCoordinates.x - (Land.length + 2 * d1) / 2;
        newRectangle1[0].y = oldRectangleCoordinates.y - (Land.depth + 2 * d1) / 2;
        newRectangle1[1].x = oldRectangleCoordinates.x + (Land.length + 2 * d1) / 2;
        newRectangle1[1].y = oldRectangleCoordinates.y - (Land.depth + 2 * d1) / 2;
        newRectangle1[2].x = oldRectangleCoordinates.x + (Land.length + 2 * d1) / 2;
        newRectangle1[2].y = oldRectangleCoordinates.y + (Land.depth + 2 * d1) / 2;
        newRectangle1[3].x = oldRectangleCoordinates.x - (Land.length + 2 * d1) / 2;
        newRectangle1[3].y = oldRectangleCoordinates.y + (Land.depth + 2 * d1) / 2;
    }
    void calculateInOut() // tính tọa độ vào ra từ chuỗi và còn lại
    {
        if(firstLine[0] == 'o'){
            string s = firstLine;
            firstLine = secondLine;
            secondLine = s;
        }
        setCoordinates(in, firstLine);
        setCoordinates(out, secondLine);
        in[0].x = in[2].x = newRectangle1[0].x;
        in[0].y = in[1].y;
        in[2].y = in[3].y;
        out[0].x = out[2].x = newRectangle1[1].x;
        out[0].y = out[1].y;
        out[2].y = out[3].y;
    }

    void printinforIDPoint()
    {
        ofstream outFile("input.txt", ios::out | ios::trunc);        
        if (!outFile) {
            cerr << "Khong mo duoc input" << endl;
            return;
        }
        for(int i = 0; i < static_cast<int>(inforIDPoint.size()); i++){
            inforIDPoint[i].ID = i + 1;
            cout << "n " << inforIDPoint[i].ID << " 0 # " << inforIDPoint[i].x << "," << inforIDPoint[i].y << endl;  
            outFile << "n " << inforIDPoint[i].ID << " 0 # " << inforIDPoint[i].x << "," << inforIDPoint[i].y << endl;  
        }

        outFile.close(); // Đóng file sau khi ghi xong
    }
    void printinforIDEdge()
    {
        ofstream outFile("input.txt", ios::app); // Mở file để ghi tiếp vào cuối file
        if (!outFile) {
            cerr << "Khong mo duoc input" << endl;
            return;
        }
        for(int i = 0; i < static_cast<int>(inforIDEdge.size()); i++){
            cout << "a " << inforIDEdge[i].ID1 << " " << inforIDEdge[i].ID2 
                << " 0 INF " << inforIDEdge[i].space << endl;
            outFile << "a " << inforIDEdge[i].ID1 << " " << inforIDEdge[i].ID2 
                    << " 0 INF " << inforIDEdge[i].space << endl;
        }

        outFile.close(); // Đóng file sau khi ghi xong
    }

};

int main()
{
    Point Rectangle[4];
    int M, N;
    Size Building;
    float d1, d3;
    string firstLine;
    string secondLine;

    {Rectangle[0].x = 0; Rectangle[0].y = 0; Rectangle[1].x = 30; Rectangle[1].y = 0;
    Rectangle[2].x = 30; Rectangle[2].y = 24; Rectangle[3].x = 0; Rectangle[3].y = 24;
    M = 4;
    N = 3;
    Building.length = 2;
    Building.depth = 2;
    d1 = 1;
    d3 = 1;
    firstLine = "i 0,11 0,13";
    secondLine = "o 30,11 30,13";}

    /* for(int i = 0; i < 4; i++){
        cin >> Rectangle[i].x >> Rectangle[i].y;
    }// Nhập tọa độ 4 đỉnh mảnh đất
    cin >> M >> N;  // Nhập số khoa/viện theo chiều ngang và dọc
    cin >> Building.length >> Building.depth;  // Nhập độ dài và rộng của công trình khoa/viện 
    cin >> d1; // Nhập độ rộng của đường bao
    cin >> d3; // nhập khoảng cách các đưỡng giữa các đường giao cắt
    cin >> firstLine;
    cin >> secondLine;
    */

    Bai14* myMain = new Bai14(Rectangle, M, N, Building, d1, d3, firstLine, secondLine);
    myMain->change();
    myMain->calculateNewLandCoordinates();
    myMain->calculateArea();
    myMain->calculateCenterCoordinates();
    myMain->calculateSmallSurroundedCoordinates();
    myMain->calculateSpace();
    myMain->calculatenewRectangle1();
    myMain->calculateInOut();
    myMain->calculateInforIDPoint();
    myMain->printinforIDPoint();
    myMain->calculateInforIDEdge();
    myMain->printinforIDEdge();
}

