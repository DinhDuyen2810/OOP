#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>
#include <bits/stdc++.h>

using namespace std;
//đọc dữ liệu từ hospital.txt
std::map<std::string, std::vector<float>> readHospitalData(const char *fileName)
{
    map<std::string, vector<float>> map;
    ifstream input(fileName);

    if (!input.is_open()) {
        cerr << "Error: Could not open file " << fileName << endl;
        return map; // Return empty map if file cannot be opened
    }

    std::string delimiter = " ";
    std::string comma = ",";

    int lineNo = 1;
    int sumOfHos;

    for (std::string line; getline(input, line);)
    {
        vector<float> v;
        if (lineNo == 1)        //đọc số khoa của bv
        {
            v.push_back(stof(line));
            sumOfHos = stof(line);
            map["NumofDepartment"] = v;
        }
        else if ((lineNo >= 2) && (lineNo <= sumOfHos + 1))
        {
            size_t pos = 0;
            size_t pos2 = 0;
            std::string token;
            std::string token2;
            int count = 0;
            std::string DeName;

            //đọc tọa độ và lưu tên khoa vào DeName
            while ((pos = line.find(delimiter)) != std::string::npos)
            {
                token = line.substr(0,pos);
                if (count < 2)
                {
                    while ((pos2 = token.find(comma)) != std::string::npos)
                    {
                    token2 = token.substr(0,pos2);
                    v.push_back(stof(token2));
                    token.erase(0,pos2 + comma.length());
                    }
                    v.push_back(stof(token));
                }

                if (count == 2)
                {
                    v.push_back(stof(token));
                }

                line.erase(0,pos + delimiter.length());
                count++;
            }
            DeName.assign(line);
            map[DeName] = v;
        }
        else
        {
            size_t pos = 0;
            size_t pos2 = 0;
            std::string token;
            std::string token2;

            //đọc tọa độ và lưu tên khoa vào DeName
            while ((pos = line.find(delimiter)) != std::string::npos)
            {
                token = line.substr(0,pos);
                while ((pos2 = token.find(comma)) != std::string::npos)
                {
                    token2 = token.substr(0,pos2);
                    v.push_back(stof(token2));
                    token.erase(0,pos2 + comma.length());
                }
                v.push_back(stof(token));
                line.erase(0,pos + delimiter.length());
            }

            while ((pos = line.find(comma)) != std::string::npos)
                {
                    token = line.substr(0,pos);
                    v.push_back(stof(token));
                    line.erase(0,pos + comma.length());
                }
            v.push_back(stof(line));
            if (lineNo == sumOfHos + 2)
            map["PosofA"] = v;
            if (lineNo == sumOfHos + 3)
            map["PosofStartAGV"] = v;
            if (lineNo == sumOfHos + 4)
            map["PosofEndAGV"] = v;
            if (lineNo == sumOfHos + 5)
            map["PosofHos"] = v;
        }
        lineNo++;
    }
    return map;
}

// Kiểm tra xem tồn tại giao lộ chưa
bool check(map<string, vector<float>>& junctions, float x, float y) {
    for (const auto& junc : junctions) {
        if (junc.second[1] == x && junc.second[2] == y) {
            return true;
        }
    }
    return false;
}

// Tăng numOfH
void add(map<string, vector<float>>& junctions, float x, float y) {
    for (auto& junc : junctions) {
        if (junc.second[1] == x && junc.second[2] == y) {
            junc.second[0]++;
            return;
        }
    }
}

// Nếu chưa tồn tại giao lộ thì thêm, tồn tại rồi thì tăng hành lang giao
void addWard(map<string, vector<float>>& junctions, float x, float y) {
    if (check(junctions, x, y)) {
        add(junctions, x, y);
    } else {
        vector<float> Junc(7, 0.0); // numOfH, x, y, HaLength (4 values)
        Junc[0] = 2;
        Junc[1] = x;
        Junc[2] = y;
        string name = "J" + to_string(junctions.size());
        junctions[name] = Junc;
    }
}

// kiểm tra b có nằm giữa không
bool isBetween(float a, float b, float c) {
    return (a <= b && b <= c) || (c <= b && b <= a);
}

// tính khoảng cách 2 điểm
float calculateDistance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// kiểm tra 2 giao lộ có cắt khoa/viện không
bool isLineThroughWard(const map<string, vector<float>>& wardCoordinates, float x1, float y1, float x2, float y2) {
    for (const auto& ward : wardCoordinates) {
        float wardX1 = ward.second[0];
        float wardY1 = ward.second[1];
        float wardX2 = ward.second[2];
        float wardY3 = ward.second[5];
        if (x2 == x1){
            if(y1 > y2){
                swap(y1, y2);
            }
            if(wardX1 <= x1 && x1 < wardX2 && y1 < wardY1 && wardY1 < y2 && y1 < wardY3 && wardY3 <= y2){
                return true;
            }
        } else if(y2 == y1){
            if (x1 > x2)
            {
                swap(x1, x2);
            }
            
            if(wardY1 <= y1 && y1 < wardY3 && x1 < wardX1 && wardX1 < x2 && x1 < wardX2 && wardX2 <= x2){
                return true;
            }
        }
    }
    return false;
}

// Tính khoảng cách từng giao lộ với nhau
void calculateDistances(map<string, vector<float>>& junctions, map<string, float>& hallways,
                        const map<string, vector<float>>& wardCoordinates, float walkwayWidth) {
    vector<pair<string, string>> checkedConnections;

    for (auto& j1 : junctions) {
        for (auto& j2 : junctions) {
            if (j1.first != j2.first &&
                find(checkedConnections.begin(), checkedConnections.end(), make_pair(j1.first, j2.first)) == checkedConnections.end() &&
                find(checkedConnections.begin(), checkedConnections.end(), make_pair(j2.first, j1.first)) == checkedConnections.end()) {

                float x1 = j1.second[1];
                float y1 = j1.second[2];
                float x2 = j2.second[1];
                float y2 = j2.second[2];

                if (x1 == x2 || y1 == y2) {
                    bool isBlocked = false;
                    for (const auto& j3 : junctions) {
                        if (j3.first != j1.first && j3.first != j2.first) {
                            float x3 = j3.second[1];
                            float y3 = j3.second[2];
                            if ((x1 == x2 && x1 == x3 && isBetween(y1, y3, y2)) || (y1 == y2 && y1 == y3 && isBetween(x1, x3, x2))) {
                                isBlocked = true;
                                break;
                            }
                        }
                    }
                    if (!isBlocked) {
                        if (!isLineThroughWard(wardCoordinates, x1, y1, x2, y2)) {
                            float distance = calculateDistance(x1, y1, x2, y2) - walkwayWidth;
                            string nameH = "H" + to_string(hallways.size());
                            hallways[nameH] = distance;
                            checkedConnections.push_back(make_pair(j1.first, j2.first));

                            // Cập nhật khoảng cách trong vector HaLength của mỗi giao lộ
                            if (x1 == x2) {
                                if (y1 < y2) {
                                    if (j1.second[6] == 0 || distance < j1.second[6])
                                        j1.second[6] = distance; // Trên
                                    if (j2.second[4] == 0 || distance < j2.second[4])
                                        j2.second[4] = distance; // Dưới
                                } else {
                                    if (j1.second[4] == 0 || distance < j1.second[4])
                                        j1.second[4] = distance; // Dưới
                                    if (j2.second[6] == 0 || distance < j2.second[6])
                                        j2.second[6] = distance; // Trên
                                }
                            } else if (y1 == y2) {
                                if (x1 < x2) {
                                    if (j1.second[5] == 0 || distance < j1.second[5])
                                        j1.second[5] = distance; // Phải
                                    if (j2.second[3] == 0 || distance < j2.second[3])
                                        j2.second[3] = distance; // Trái
                                } else {
                                    if (j1.second[3] == 0 || distance < j1.second[3])
                                        j1.second[3] = distance; // Trái
                                    if (j2.second[5] == 0 || distance < j2.second[5])
                                        j2.second[5] = distance; // Phải
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


int randomInt(int from, int to)
{
    static std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> distr(from, to);
    return distr(gen);
}
 
std::map<std::string, float> convertJuncData(const std::map<std::string, std::vector<float>>& mapData) {
    std::map<std::string, float> data;
    for (const auto& elem : mapData) {
        for (int i = 3; i <= 6; i++) { // Indices 3 to 6 for HaLength in vector<float>
            data[elem.first + "_" + std::to_string(i-3)] = elem.second[i];
        }
    }
    return data;
}

std::map<std::string, std::vector<float>> mapData;
std::map<std::string, float> juncDataList;
std::vector<float> juncData;
std::string juncName;
std::string hallName;

int main()
{
    const char *fileName = "../../data/hospital.txt";
    std::map<std::string, std::vector<float>> hospitalData = readHospitalData(fileName);
    std::map<std::string, std::vector<float>> WardData;
    std::map<std::string, std::vector<float>> WardCoordinates;
    // lọc các khoa viện
    for (const auto& pair : hospitalData) {
        if(pair.first.length() < 2){
            WardData[pair.first] = pair.second;
        }
    }

    WardData["A"] = {hospitalData["PosofA"][0], hospitalData["PosofA"][1], hospitalData["PosofA"][4],
     hospitalData["PosofA"][5], hospitalData["PosofA"][6] - hospitalData["PosofA"][2]};
    float walkwayWidth = 10e8;
     // tính toán tọa độ từng khoa/viện
    for (auto& ward : WardData) {
        float x1 = ward.second[0] - ward.second[4] / 2.0;
        float y1 = ward.second[1];
        float x2 = ward.second[0] + ward.second[4] / 2.0;
        float y2 = ward.second[1];
        float x3 = ward.second[0] + ward.second[4] / 2.0;
        float y3 = ward.second[3];
        float x4 = ward.second[0] - ward.second[4] / 2.0;
        float y4 = ward.second[3];
        if(walkwayWidth > x1){
            walkwayWidth = x1;
        }
        WardCoordinates[ward.first] = {x1, y1, x2, y2, x3, y3, x4, y4};
    }
    // tìm các Junction
    map<string, vector<float>> junctions;
    map<string, float> hallways;
    for (auto& ward : WardCoordinates) {
        float x1 = ward.second[0] - walkwayWidth;
        float y1 = ward.second[1] - walkwayWidth;
        float x2 = ward.second[2];
        float y2 = ward.second[3] - walkwayWidth;
        float x3 = ward.second[4];
        float y3 = ward.second[5];
        float x4 = ward.second[6] - walkwayWidth;
        float y4 = ward.second[7];
        addWard(junctions, x1, y1);
        addWard(junctions, x2, y2);
        addWard(junctions, x3, y3);
        addWard(junctions, x4, y4);
    }

    calculateDistances(junctions, hallways, WardCoordinates, walkwayWidth);
//-----------------------------------------------------------------------------------//
    std::string input1;
    do{
        cout << "Select the type of traffic you want to simulate" << endl;
        cout << "1. Hallway" << endl;
        cout << "2. Junction" << endl;
        cout << "Your choice: ";
        getline(cin, input1);
        if (input1 == "1")
        {
            do
            {
                cout << "Please enter the hallway you want to emulate" ;
                cout << "(from H0 to H" << hallways.size() - 1 << ")" << endl;
                cout << "(Press enter to randomly select a hallway in the map)" << endl;
                cout << "Your choice: ";
                getline(cin, hallName);
                if (hallName == "")
                {
                    auto it = hallways.begin();
                    std::advance(it, randomInt(0, hallways.size() - 1));
                    std::string random_key = it->first;
                    hallName.assign(random_key);
                }

            } while (hallways.count(hallName) == 0);
            cout << hallName << " : " << endl;
            float length1Side = (float)hallways[hallName] / 2;
            juncData = {length1Side, length1Side};
        }
        else if (input1 == "2")
        {
            do
            {
                cout << "Please enter the junction you want to emulate";
                cout << "(from J0 to J" << junctions.size() - 1 << ")" << endl;
                cout << "(Press enter to randomly select a junction in the map)" << endl;
                cout << "Your choice: ";
                getline(cin, juncName);
                if (juncName == "")
                {
                    auto it = junctions.begin();
                    std::advance(it, randomInt(0, junctions.size() - 1));
                    std::string random_key = it->first;
                    juncName.assign(random_key);
                }

            } while (junctions[juncName].size() < 4);
            cout << juncName << " : " << endl;            
            juncData = {junctions[juncName][3], junctions[juncName][4], junctions[juncName][5], junctions[juncName][6]};
            juncDataList = convertJuncData(junctions);
        }
    } while (input1 != "1" && input1 != "2"); 


    cout << "walkwayWidth: " << walkwayWidth << endl;  
    cout << "juncData: ";
    for (const float &data : juncData)
    {
        cout << data << " ";
    }
    cout << endl;
    cout << "juncDataList: " << endl;
    int count = 0;
    for (const auto &pair : juncDataList)
    {
        cout << pair.first << ": " << pair.second;
        if(count == 7){
            cout << endl;
            count = 0;
        } else {
            cout << "  ";
            count++;
        }
    }
    
    return 0;
}
