#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

std::map<std::string, std::vector<float>> readHospitalData(const char *fileName)
{
	map<std::string, vector<float>> map;
	ifstream input(fileName);
	
	std::string delimiter = " ";
	std::string comma = ",";
	
	int lineNo = 1;
	int sumOfHos;

	for (std::string line; getline(input, line);)
	{
		vector<float> v;
		if (lineNo == 1)		//đọc số khoa của bv
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

int main()
{
	const char *fileName = "../../../data/hospital.txt"; 
    map<std::string, std::vector<float>> hospitalData = readHospitalData(fileName);
    
    for (const auto& pair : hospitalData) {
        const std::string& key = pair.first;
        const std::vector<float>& values = pair.second;

        std::cout << "Key: " << key << " | Values: ";
        for (const float& value : values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
	return 0;
}