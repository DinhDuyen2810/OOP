#if defined(__linux__)
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <random>
#include <lib/nlohmann/json.hpp>

#include "model/SocialForce.h"
#include "constant/Constant.h"
#include "renderer/Renderer.h"

using namespace std;
using namespace Constant;
using namespace Renderer;
using json = nlohmann::json;

// Global Variables
GLsizei winWidth = 1080; // Window width (16:10 ratio)
GLsizei winHeight = 660; // Window height (16:10 ratio)
SocialForce *socialForce;
float fps = 0; // Frames per second
int currTime = 0;
int startTime = 0;
bool animate = false; // Animate scene flag
float speedConsiderAsStop = 0.2;

json inputData;
std::map<std::string, std::vector<float>> mapData;
std::vector<json> juncDataList;
std::vector<float> juncData;
std::string juncName;
int juncIndex = 0;
float walkwayWidth;
int juncSizereal = 0;
std::string hallName;
struct Junction
{
    string name;
    int numOfH;
    float x, y;
    vector<float> HaLength = {0, 0, 0, 0}; // Trái, Dưới, Phải, Trên
};
int juncLost = -1;

int classificationType = 0;

std::vector<int> numOfPeople;
float minSpeed = -1;
float maxSpeed = -1;
int threshold = 0;

// Function Prototypes
void init();

void createWalls();

void createAgents();

void createAGVs();

void display();

void reshape(int width, int height);

void normalKey(unsigned char key, int xMousePos, int yMousePos);

void update();

int main(int argc, char **argv)
{
    inputData = Utility::readInputData("data/input.json");
    mapData = Utility::readMapData("data/map.txt");
    std::string input1;

    if ((int)inputData["runMode"]["value"] == 0)
    {
        do
        {
            cout << "Select the type of traffic you want to simulate" << endl;
            cout << "1. Hallway" << endl;
            cout << "2. Junction" << endl;
            cout << "Your choice: ";
            getline(cin, input1);
            if (input1 == "1")
            {
                walkwayWidth = (float)inputData["hallwayWidth"]["value"];
                float length1Side = ((float)inputData["hallwayLength"]["value"]) / 2;
                juncData = {length1Side, length1Side};
            }
            else if (input1 == "2")
            {
                do
                {
                    cout << "Please enter the junction you want to emulate" << endl;
                    cout << "(Press enter to randomly select a junction in the map)" << endl;
                    cout << "Your choice: ";
                    getline(cin, juncName);
                    if (juncName == "")
                    {
                        auto it = mapData.begin();
                        std::advance(it, Utility::randomInt(1, mapData.size() - 3));
                        std::string random_key = it->first;
                        juncName.assign(random_key);
                    }

                } while (mapData[juncName].size() < 3);
                juncData = mapData[juncName];
                walkwayWidth = mapData["walkwayWidth"][0];
                juncSizereal = juncData.size();
            }
        } while (input1 != "1" && input1 != "2");
    }
    else if ((int)inputData["runMode"]["value"] == 1){   
        juncDataList = Utility::convertMapData(mapData);
        float hallwayLength = juncDataList[juncIndex].items().begin().value();

        walkwayWidth = (float)inputData["hallwayWidth"]["value"];
        float length1Side = (hallwayLength) / 2;
        juncData = {length1Side, length1Side};
        juncSizereal = juncData.size();
    } else if ((int)inputData["runMode"]["value"] == 3){
        const char *fileName = "data/hospital.txt";
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
    }

    for(auto& temp : juncData){
        if(temp != 0){
            juncSizereal ++;
        }
    }
    float deviationParam = randomFloat(1 - (float)inputData["experimentalDeviation"]["value"] / 100,
     1 + (float)inputData["experimentalDeviation"]["value"] / 100);
    // Threshold people stopping at the corridor
    threshold = int(inputData["numOfAgents"]["value"]) * deviationParam * (float)(inputData["stopAtHallway"]["value"]) / 100;

    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA |
                        GLUT_DEPTH);         // Set display mode  Default mode used
    glutInitWindowSize(winWidth, winHeight); // Set window width and height
    glutInitWindowPosition(90, 90);          // Set window position
    glutCreateWindow(
        "Crowd Simulation using Social Force"); // Set window title and create
    // display window

    animate = true;
    startTime = currTime;
    if ((int)inputData["graphicsMode"]["value"] == 0)
    {
        glutHideWindow();
    }

    init();                   // Initialization
    glutDisplayFunc(display); // Send graphics to display window
    glutReshapeFunc(reshape); // Maintain aspect ratio when window first created,
    // resized and moved

    glutKeyboardFunc(normalKey);
    glutIdleFunc(update); // Continuously execute 'update()'
    glutMainLoop();       // Enter GLUT's main loop

    return 0;
}

void init()
{
    // General Light Intensity
    GLfloat gnrlAmbient[] = {
        0.8F, 0.8F, 0.8F,
        1.0}; // Ambient (R, G, B, A) light intensity of entire scene

    // Object Light Intensity
    GLfloat lghtDiffuse[] = {0.7F, 0.7F, 0.7F,
                             1.0}; // Diffuse (R, G, B, A) light intensity

    // Light Position
    GLfloat lghtPosition[] = {4.0, -4.0, 4.0, 0.0};

    glClearColor(1.0, 1.0, 1.0,
                 0.0);       // Set color used when color buffer cleared
    glShadeModel(GL_SMOOTH); // Set shading option

    // General Lighting
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gnrlAmbient);

    // Object Lighting
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lghtDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lghtPosition);

    glEnable(GL_DEPTH_TEST); // Enable hidden surface removal
    glEnable(GL_NORMALIZE);  // Normalize normal vector
    glEnable(GL_LIGHTING);   // Prepare OpenGL to perform lighting calculations
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHT0);

    glCullFace(GL_BACK);    // Specify face to be culled
    glEnable(GL_CULL_FACE); // Enable culling of specified face

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);

    srand(1604010629); // Seed to generate random numbers

    socialForce = new SocialForce;
    createWalls();
    createAgents();
    createAGVs();
}

void createWalls()
{
    Wall *wall;

    vector<float> coors = Utility::getWallCoordinates(walkwayWidth, juncData);

    if (juncData.size() == 2)
    {
        // Upper Wall
        wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
        socialForce->addWall(wall);
        // Lower Wall
        wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
        socialForce->addWall(wall);
    }
    else
    {
        if ((int)inputData["runMode"]["value"] == 3){
            // Upper Wall
            if(juncData[0] != 0 && juncData[2] != 0){
                if(juncData[1] == 0){
                    wall = new Wall(coors[0], coors[1], coors[6], coors[7]);
                    socialForce->addWall(wall);
                } else {
                    wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
                    socialForce->addWall(wall);
                    wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
                    socialForce->addWall(wall); 
                }
            } else if(juncData[0] == 0 && juncData[2] != 0){
                wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
                socialForce->addWall(wall); 
            } else if(juncData[0] != 0 && juncData[2] == 0){
                wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
                socialForce->addWall(wall); 
            }
            // Lower Wall
            if(juncData[0] != 0 && juncData[2] != 0){
                if(juncData[3] == 0){
                    wall = new Wall(coors[8], coors[9], coors[14], coors[15]);
                    socialForce->addWall(wall);
                } else {
                    wall = new Wall(coors[8], coors[9], coors[10], coors[11]);
                    socialForce->addWall(wall);
                    wall = new Wall(coors[12], coors[13], coors[14], coors[15]);
                    socialForce->addWall(wall);     
                }
            } else if(juncData[0] == 0 && juncData[2] != 0){
                wall = new Wall(coors[12], coors[13], coors[14], coors[15]);
                socialForce->addWall(wall); 
            } else if(juncData[0] != 0 && juncData[2] == 0){
                wall = new Wall(coors[8], coors[9], coors[10], coors[11]);
                socialForce->addWall(wall); 
            }
            // Left Wall
            if(juncData[1] != 0 && juncData[3] != 0){
                if(juncData[0] == 0){
                    wall = new Wall(coors[16], coors[17], coors[22], coors[23]);
                    socialForce->addWall(wall);
                } else {
                    wall = new Wall(coors[16], coors[17], coors[18], coors[19]);
                    socialForce->addWall(wall);
                    wall = new Wall(coors[20], coors[21], coors[22], coors[23]);
                    socialForce->addWall(wall);     
                }
            } else if(juncData[1] == 0 && juncData[3] != 0){
                if(juncData[0] == 0){
                    wall = new Wall(coors[18], coors[19], coors[20], coors[21]);
                    socialForce->addWall(wall); 
                }
                wall = new Wall(coors[20], coors[21], coors[22], coors[23]);
                socialForce->addWall(wall); 
            } else if(juncData[1] != 0 && juncData[3] == 0){
                wall = new Wall(coors[16], coors[17], coors[18], coors[19]);
                socialForce->addWall(wall); 
                if(juncData[0] == 0){
                    wall = new Wall(coors[18], coors[19], coors[20], coors[21]);
                    socialForce->addWall(wall); 
                }
            }
            // Right Wall
            if(juncData[1] != 0 && juncData[3] != 0){
                if(juncData[2] == 0){
                    wall = new Wall(coors[24], coors[25], coors[30], coors[31]);
                    socialForce->addWall(wall);
                } else {
                    wall = new Wall(coors[24], coors[25], coors[26], coors[27]);
                    socialForce->addWall(wall);
                    wall = new Wall(coors[28], coors[29], coors[30], coors[31]);
                    socialForce->addWall(wall);     
                }
            } else if(juncData[1] == 0 && juncData[3] != 0){
                if(juncData[2] == 0){
                    wall = new Wall(coors[26], coors[27], coors[28], coors[29]);
                    socialForce->addWall(wall); 
                }
                wall = new Wall(coors[28], coors[29], coors[30], coors[31]);
                socialForce->addWall(wall); 
            } else if(juncData[1] != 0 && juncData[3] == 0){
                wall = new Wall(coors[24], coors[25], coors[26], coors[27]);
                socialForce->addWall(wall); 
                if(juncData[2] == 0){
                    wall = new Wall(coors[26], coors[27], coors[28], coors[29]);
                    socialForce->addWall(wall); 
                }
            }
        } else {
            // Upper Wall
            if (juncData.size() == 4)
            {
                wall = new Wall(coors[0], coors[1], coors[2], coors[3]);
                socialForce->addWall(wall);

                wall = new Wall(coors[4], coors[5], coors[6], coors[7]);
                socialForce->addWall(wall);
            }
            else if (juncData.size() == 3)
            {
                wall = new Wall(coors[0], coors[1], coors[6], coors[7]);
                socialForce->addWall(wall);
            }

            // Lower Wall
            wall = new Wall(coors[8], coors[9], coors[10], coors[11]);
            socialForce->addWall(wall);

            wall = new Wall(coors[12], coors[13], coors[14], coors[15]);
            socialForce->addWall(wall);

            // Left Wall
            if (juncData.size() == 4)
            {
                wall = new Wall(coors[16], coors[17], coors[18], coors[19]);
                socialForce->addWall(wall);
            }

            wall = new Wall(coors[20], coors[21], coors[22], coors[23]);
            socialForce->addWall(wall);

            // Right Wall
            if (juncData.size() == 4)
            {
                wall = new Wall(coors[24], coors[25], coors[26], coors[27]);
                socialForce->addWall(wall);
            }

            wall = new Wall(coors[28], coors[29], coors[30], coors[31]);
            socialForce->addWall(wall);
        }
    }
}

void setAgentsFlow(Agent *agent, float desiredSpeed, float maxSpeed, float minSpeed, int caseJump)
{
    // if (socialForce->getCrowdSize() < threshold)
    // {
    //     agent->setStopAtCorridor(true);
    // }

    int codeSrc = 0;
    int codeDes = 0;
    int juncType = juncSizereal;
    // xác định hình dạng của ngã rẽ hoặc ngã 3
    if (juncData.size() == 4){
        if(juncData[0] == 0){
            if(juncData[1] == 0){
                juncLost = 4;   
            } else if(juncData[3] == 0){
                juncLost = 7;
            } else {
                juncLost = 0;
            }
        } else if(juncData[1] == 0){
            if(juncData[0] == 0){
                juncLost = 4;
            } else if(juncData[2] == 0){
                juncLost = 5;
            } else {
                juncLost = 1;
            }
        } else if(juncData[2] == 0){
            if(juncData[1] == 0){
                juncLost = 5;
            } else if(juncData[3] == 0){
                juncLost = 6;
            } else {
                juncLost = 2;
            }
        } else if(juncData[3] == 0){
            if(juncData[2] == 0){
                juncLost = 6;
            } else if(juncData[0] == 0){
                juncLost = 7;
            } else {
                juncLost = 3;
            }
        }
    }

    if (juncType == 4)
    {
        if (caseJump < 3)
        {
            codeSrc = 0; // Go from Left to Right
        }
        else if (caseJump < 6)
        {
            codeSrc = 1; // Go from Right to Left
        }
        else if (caseJump < 9)
        {
            codeSrc = 2; // Go from Top to Bottom
        }
        else
        {
            codeSrc = 3; // Go from Bottom to Top
        }
    }
    else if (juncType == 3)
    {
        if(juncLost == 0){
            if (caseJump < 6)
            {
                codeSrc = 1;
                if (caseJump % 2 == 0)
                {
                    codeDes = 2;
                }
                else
                {
                    codeDes = 3;
                }
            }
            else if (caseJump < 12)
            {
                codeSrc = 2;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 2;
                }
            }
            else if (caseJump < 18)
            {
                codeSrc = 3;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 3;
                }
            }
        }
        if(juncLost == 1){
            if (caseJump < 6)
            {
                codeSrc = 0;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 2;
                }
            }
            else if (caseJump < 12)
            {
                codeSrc = 1;
                if (caseJump % 2 == 0)
                {
                    codeDes = 1;
                }
                else
                {
                    codeDes = 2;
                }
            }
            else if (caseJump < 18)
            {
                codeSrc = 3;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 1;
                }
            }
        }
        if(juncLost == 2){
            if (caseJump < 6)
            {
                codeSrc = 0;
                if (caseJump % 2 == 0)
                {
                    codeDes = 2;
                }
                else
                {
                    codeDes = 3;
                }
            }
            else if (caseJump < 12)
            {
                codeSrc = 2;
                if (caseJump % 2 == 0)
                {
                    codeDes = 1;
                }
                else
                {
                    codeDes = 2;
                }
            }
            else if (caseJump < 18)
            {
                codeSrc = 3;
                if (caseJump % 2 == 0)
                {
                    codeDes = 1;
                }
                else
                {
                    codeDes = 3;
                }
            }
        }
        if(juncLost == 3){
            if (caseJump < 6)
            {
                codeSrc = 0;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 3;
                }
            }
            else if (caseJump < 12)
            {
                codeSrc = 1;
                if (caseJump % 2 == 0)
                {
                    codeDes = 1;
                }
                else
                {
                    codeDes = 3;
                }
            }
            else if (caseJump < 18)
            {
                codeSrc = 2;
                if (caseJump % 2 == 0)
                {
                    codeDes = 0;
                }
                else
                {
                    codeDes = 1;
                }
            }
        }
    }
    else if (juncType == 2)
    {
        if(juncLost == -1){
            if (caseJump < 3)
            {
                codeSrc = 0;
            } else {
                codeSrc = 1;
            }   
        } else if(juncLost == 4){
            if (caseJump < 3)
            {
                codeSrc = 1;
            } else {
                codeSrc = 3;
            }   
        } else if(juncLost == 5){
            if (caseJump < 3)
            {
                codeSrc = 0;
            } else {
                codeSrc = 3;
            }   
        } else if(juncLost == 6){
            if (caseJump < 3)
            {
                codeSrc = 0;
            } else {
                codeSrc = 2;
            }   
        } else if(juncLost == 7){
            if (caseJump < 3)
            {
                codeSrc = 1;
            } else {
                codeSrc = 2;
            }   
        }

    }

    vector<float> position = Utility::getPedesSource(
        codeSrc,
        (float)inputData["totalCrowdLength"]["value"],
        (float)inputData["headCrowdLength"]["value"],
        (float)inputData["crowdWidth"]["value"],
        walkwayWidth, juncData);
    vector<float> desList;

    if (juncType == 4 || juncType == 2)
    {
        desList = Utility::getPedesDestination(codeSrc, caseJump % 3, walkwayWidth, juncData, agent->getStopAtCorridor());
    }
    else if (juncType == 3)
    {
        desList = Utility::getPedesDestination(codeDes, caseJump % 3, walkwayWidth, juncData, agent->getStopAtCorridor());
    }

    agent->setPosition(position[0], position[1]);
    if (juncType == 3 && codeSrc != codeDes)
    {
        agent->setPath(randomFloat(-walkwayWidth / 2, walkwayWidth / 2), randomFloat(-walkwayWidth / 2, walkwayWidth / 2), 2.0);
    } 
    agent->setPath(desList[0], desList[1], desList[2]);
    agent->setDestination(desList[0], desList[1]);
    agent->setDesiredSpeed(desiredSpeed);
    std::vector<float> color = getPedesColor(maxSpeed, minSpeed, agent->getDesiredSpeed(), classificationType);
    agent->setColor(color[0], color[1], color[2]);
    socialForce->addAgent(agent);
}

void createAgents()
{
    Agent *agent;
    
    float deviationParam = randomFloat(1 - (float)inputData["experimentalDeviation"]["value"] / 100,
     1 + (float)inputData["experimentalDeviation"]["value"] / 100);
    // cout << "Deviation: "<< deviationParam <<" - Num agents: "<< int(int(inputData["numOfAgents"]["value"]) * deviationParam) << endl;
    
    
    numOfPeople = Utility::getNumPedesInFlow(juncSizereal, int(int(inputData["numOfAgents"]["value"]) * deviationParam));
    vector<double> velocityList = Utility::getPedesVelocity(classificationType, inputData, deviationParam);
    if (classificationType == 0)
    {
        minSpeed = 0.52;
        maxSpeed = 2.28;
    }
    else
    {
        minSpeed = velocityList[0];
        maxSpeed = velocityList[velocityList.size() - 1];
    }

    auto rng = std::default_random_engine{};
    std::shuffle(velocityList.begin(), velocityList.end(), rng);

    int pedesCount = 0;

    // test

    // for (int temp = 0; temp < 3; temp++)
    // {
    //     agent = new Agent;
    //     // setAgentsFlow(agent, 1, maxSpeed, minSpeed, Point3f(randomFloat(-3.0, -2.0), randomFloat(9.0, 10.0), 0.0), Point3f(randomFloat(-3.2, -2.8), randomFloat(-2.2, -1.8), 0.0));
    //     agent->setPosition(randomFloat(-3.0, -2.0), randomFloat(9.0, 10.0));
    //     // // float x = randomFloat(-13.3F, -6.0);
    //     // // float y = randomFloat(-2.0, 2.0);
    //     float x = randomFloat(-3.2, -2.8);
    //     float y = randomFloat(-2.2, -1.8);
    //     agent->setPath(x, y, 2);
    //     agent->setDestination(x, y);
    //     // // agent->setPath(randomFloat(22.0, 25.0), randomFloat(-3.0, -2.0), 1.0);
    //     agent->setDesiredSpeed(1);
    //     agent->setStopAtCorridor(true);
    //     std::vector<float> color = Utility::getPedesColor(maxSpeed, minSpeed, agent->getDesiredSpeed(), classificationType);
    //     agent->setColor(color[0], color[1], color[2]);
    //     socialForce->addAgent(agent);
    // }

    // test

    if (juncSizereal == 2)
    {
        for (int idx = 0; idx < 6; idx++)
        {
            for (int temp = 0; temp < numOfPeople[idx]; temp++)
            {
                agent = new Agent;
                setAgentsFlow(agent, velocityList[pedesCount], maxSpeed, minSpeed, idx);
                pedesCount = pedesCount + 1;
            }
        }
    }
    else if (juncSizereal == 3)
    {
        for (int idx = 0; idx < 18; idx++)
        {
            for (int temp = 0; temp < numOfPeople[idx]; temp++)
            {
                agent = new Agent;
                setAgentsFlow(agent, velocityList[pedesCount], maxSpeed, minSpeed, idx);
                pedesCount = pedesCount + 1;
            }
        }
    }
    else if (juncSizereal == 4)
    {
        for (int idx = 0; idx < 12; idx++)
        {
            for (int temp = 0; temp < numOfPeople[idx]; temp++)
            {
                agent = new Agent;
                setAgentsFlow(agent, velocityList[pedesCount], maxSpeed, minSpeed, idx);
                pedesCount = pedesCount + 1;
            }
        }
    }
}

void createAGVs()
{
    AGV *agv = NULL;
    vector<int> array;

    // test
    // agv = new AGV();
    // vector<Point3f> route = Utility::getRouteAGV(juncData.size(), 0, 2, walkwayWidth, juncData);
    // agv->setDirection(0, 2);
    // agv->setPosition(route[0].x, route[0].y);

    // for (Agent *agent : socialForce->getCrowd())
    // {
    //     if (agent->getPosition().distance(agv->getPosition()) < 0.5F)
    //     {
    //         do
    //         {
    //             agent->setPosition(agent->getPosition().x - 0.1F, agent->getPosition().y - 0.1F);
    //         } while (agent->getPosition().distance(agv->getPosition()) < 0.5F);
    //     }
    // }

    // agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
    // agv->setDesiredSpeed(0.6F);
    // agv->setAcceleration(inputData[9]);
    // agv->setDistance((float)inputData[10]);
    // for (int i = 1; i < route.size(); i++)
    // {
    //     agv->setPath(route[i].x, route[i].y, 1.0);
    //     std::cout << route[i] << endl;
    // }
    // socialForce->addAGV(agv);

    // test
    if ((int)inputData["runMode"]["value"] == 0)
    {
        for (int i = 0; i < juncData.size(); i++)
        {
            if (juncData.size() == 4)
            {
                array = {0, 1, 2};
            }
            else if (juncData.size() == 3)
            {
                if (i == 0)
                {
                    array = {1, 2};
                }
                else if (i == 1)
                {
                    array = {0, 2};
                }
                else
                {
                    array = {0, 1};
                }
            }
            else if (juncData.size() == 2)
            {
                array = {1};
            }

            for (int j : array)
            {
                agv = new AGV();
                vector<Point3f> route = Utility::getRouteAGV(i, j, walkwayWidth, juncData, juncLost);
                agv->setDirection(i, j);
                agv->setPosition(route[0].x, route[0].y);

                /* // for (Agent *agent : socialForce->getCrowd())
                // {
                //     if (agent->getPosition().distance(agv->getPosition()) < 0.5F)
                //     {
                //         do
                //         {
                //             agent->setPosition(agent->getPosition().x - 0.1F, agent->getPosition().y - 0.1F);
                //         } while (agent->getPosition().distance(agv->getPosition()) < 0.5F);
                //     }
                // } */

                agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
                agv->setDesiredSpeed(0.6F);
                agv->setAcceleration(inputData["acceleration"]["value"]);
                agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
                for (int i = 1; i < route.size(); i++)
                {
                    agv->setPath(route[i].x, route[i].y, 1.0);
                }
                socialForce->addAGV(agv);
            }
        }
    } else if ((int)inputData["runMode"]["value"] == 1){
        int numOfHallway = juncDataList.size();
        int numRunPerHallway = (int)inputData["noRunPerHallway"]["value"];
        int juncIndexTemp = 0;
        float hallwayLength = juncDataList[juncIndexTemp].items().begin().value();
        cout << "*****=> " << juncDataList[juncIndex].items().begin().key() << ": " << hallwayLength << endl;
        float length1Side = (hallwayLength) / 2;
        vector<float> juncDataTemp = {length1Side, length1Side};
        int numAGVPerRun = 1;
        if ((int)inputData["runConcurrently"]["value"] == 1)
        {
            numAGVPerRun = 2;
        }
        for (int i = 0; i < numOfHallway * numRunPerHallway; i++)
        {
            for (int j = 0; j < numAGVPerRun; j++)
            {
                agv = new AGV();
                vector<Point3f> route = Utility::getRouteAGV(j, 1, walkwayWidth, juncDataTemp, juncLost); // Just need the source
                agv->setDirection(j, 1);
                agv->setPosition(route[0].x, route[0].y);

                agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
                agv->setDesiredSpeed((float)inputData["agvDesiredSpeed"]["value"]);
                agv->setAcceleration(inputData["acceleration"]["value"]);
                agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
                for (int i = 1; i < route.size(); i++)
                {
                    agv->setPath(route[i].x, route[i].y, 1.0);
                }
                socialForce->addAGV(agv);

                int marker = numRunPerHallway * (juncIndexTemp + 1) - 1;
                if ((int)inputData["runConcurrently"]["value"] == 1)
                {
                    marker = numRunPerHallway * 2 * (juncIndexTemp + 1) - 1;
                }
                if (agv->getId() == marker)
                {
                    juncIndexTemp = juncIndexTemp + 1;
                    if (juncIndexTemp == juncDataList.size())
                    {
                        juncIndexTemp = 0;
                    }
                    hallwayLength = juncDataList[juncIndexTemp].items().begin().value();
                    length1Side = (hallwayLength) / 2;
                    juncDataTemp = {length1Side, length1Side};
                }
            }
        }
    } else if ((int)inputData["runMode"]["value"] == 3){
        for (int i = 0; i < juncSizereal; i++)
        {
            if (juncSizereal == 4)
            {
                array = {0, 1, 2};
            }
            else if (juncSizereal == 3)
            {
                if (i == 0)
                {
                    array = {1, 2};
                }
                else if (i == 1)
                {
                    array = {0, 2};
                }
                else
                {
                    array = {0, 1};
                }
            }
            else if (juncSizereal == 2)
            {
                array = {1};
            }

            for (int j : array)
            {
                agv = new AGV();
                vector<Point3f> route = Utility::getRouteAGV(i, j, walkwayWidth, juncData, juncLost);
                agv->setDirection(i, j);
                agv->setPosition(route[0].x, route[0].y);
                agv->setDestination(route[route.size() - 1].x, route[route.size() - 1].y);
                agv->setDesiredSpeed(0.6F);
                agv->setAcceleration(inputData["acceleration"]["value"]);
                agv->setThresholdDisToPedes((float)inputData["thresDistance"]["value"]);
                for (int i = 1; i < route.size(); i++)
                {
                    agv->setPath(route[i].x, route[i].y, 1.0);
                }
                socialForce->addAGV(agv);
            }
        }
    }
}

/* Hàm display thực hiện việc xóa và chuẩn bị bộ đệm cho khung hình mới, thiết lập vị trí và
hướng nhìn của camera, vẽ các đối tượng trong cảnh (agents, AGVs, walls), 
hiển thị thông tin lên màn hình và cuối cùng hoán đổi bộ đệm để hiển thị khung hình mới. */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffer
    glLoadIdentity();             // Initialize modelview matrix to identity matrix

    // Camera
    gluLookAt(0.0, 0.0, 18.0, // Position
              0.0, 0.0, 0.0,  // Look-at point
              0.0, 1.0, 0.0); // Up-vector

    glPushMatrix();
    glScalef(1.0, 1.0, 1.0);

    drawAgents(socialForce);
    drawAGVs(socialForce, juncData, (int)inputData["runConcurrently"]["value"], (int)inputData["runMode"]["value"]);
    drawWalls(socialForce);
    glPopMatrix();

    showInformation(socialForce, fps, animate, currTime, startTime, classificationType, winWidth, winHeight);

    glutSwapBuffers();
}
/* Hàm reshape trong đoạn mã này có nhiệm vụ thiết lập lại chế độ hiển thị khi cửa sổ thay đổi kích thước.
Điều này bao gồm việc thiết lập lại viewport và ma trận chiếu. */
void reshape(int width, int height)
{
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Initialize projection matrix to identity matrix
    gluPerspective(65.0, static_cast<GLfloat>(width) / height, 1.0,
                   100.0); // Create matrix for symmetric perspective-view frustum

    glMatrixMode(GL_MODELVIEW);

    winWidth = width;
    winHeight = height;
}
/* Hàm này cung cấp các thao tác cơ bản để điều khiển hoạt ảnh và thoát chương trình thông qua bàn phím. */
void normalKey(unsigned char key, int xMousePos, int yMousePos)
{
    switch (key)
    {
    case 'a': // Animate or inanimate scene
        animate = (!animate) ? true : false;
        startTime = currTime;
        break;

    case 27: // ASCII character for Esc key
        delete socialForce;
        socialForce = 0;

        exit(0); // Terminate program
        break;
    }
}

void update()
{
    int frameTime;       // Store time in milliseconds
    static int prevTime; // Stores time in milliseconds

    currTime = glutGet(GLUT_ELAPSED_TIME); // Get time in milliseconds since 'glutInit()' called
    frameTime = currTime - prevTime;
    prevTime = currTime;

    int count_agents = 0, count_agvs = 0;

    std::vector<Agent *> agents = socialForce->getCrowd();
    for (Agent *agent : agents)
    {
        Point3f src = agent->getPosition();
        Point3f des = agent->getDestination();

        if (Utility::isPositionErr(src, walkwayWidth, juncData.size(), socialForce->getAGVs()))
        {
            socialForce->removeAgent(agent->getId());
            continue;
        }

        if (agent->getVelocity().length() < LOWER_SPEED_LIMIT + 0.2 &&
            agent->getMinDistanceToWalls(socialForce->getWalls(), src, agent->getRadius()) < 0.2 &&
            (agent->interDes).size() == 0)
        {
            Point3f intermediateDes = Utility::getIntermediateDes(src, walkwayWidth, walkwayWidth);

            (agent->interDes).push_back(intermediateDes);
            agent->setPath(intermediateDes.x, intermediateDes.y, 1.0);
            agent->setPath(des.x, des.y, 1.0);
        }

        if ((agent->interDes).size() > 0)
        {
            float distanceToInterDes = src.distance((agent->interDes).front());
            if (distanceToInterDes <= 1)
            {
                (agent->interDes).clear();
            }
        }

        float distanceToTarget = src.distance(des);
        if (distanceToTarget <= 1 || isnan(distanceToTarget))
        {
            agent->setIsMoving(false);
            if (!agent->getStopAtCorridor())
            {
                socialForce->removeAgent(agent->getId());
            }
            count_agents = count_agents + 1;
        }
    }

    std::vector<AGV *> agvs = socialForce->getAGVs();
    for (AGV *agv : agvs)
    {
        if (agv->getCollisionStartTime() == 0 && agv->getVelocity().length() < speedConsiderAsStop && agv->getIsMoving())
        {
            agv->setCollisionStartTime(glutGet(GLUT_ELAPSED_TIME));
            // cout << "- Start collision: " << convertTime(agv->getCollisionStartTime()) << endl;
        }

        if (agv->getCollisionStartTime() != 0 && agv->getVelocity().length() > speedConsiderAsStop && agv->getIsMoving())
        {
            agv->setTotalStopTime(agv->getTotalStopTime() + glutGet(GLUT_ELAPSED_TIME) - agv->getCollisionStartTime());
            // cout << "- Stop collision: " << convertTime(glutGet(GLUT_ELAPSED_TIME)) << endl;
            // cout << "=> Total collision: " << convertTime(agv->getTotalStopTime()) << endl;
            agv->setCollisionStartTime(0);
        }

        Point3f src = agv->getPosition();
        Point3f des = agv->getDestination();

        float distance = src.distance(des);
        if (distance <= 1 || isnan(distance))
        {
            if (agv->getIsMoving())
            {
                agv->setTravelingTime(glutGet(GLUT_ELAPSED_TIME) - agv->getTravelingTime());
                agv->setIsMoving(false);

                int numAGVCompleted = getNumAGVCompleted(socialForce->getAGVs());

                int marker = (int)inputData["noRunPerHallway"]["value"];
                if ((int)inputData["runConcurrently"]["value"] == 1)
                {
                    marker = (int)inputData["noRunPerHallway"]["value"] * 2;
                    if (numAGVCompleted % 2 == 0)
                    {
                        socialForce->removeCrowd();
                        createAgents();
                    }
                }
                else
                {
                    socialForce->removeCrowd();
                    createAgents();
                }

                if (numAGVCompleted > 0 && numAGVCompleted % marker == 0)
                {
                    juncIndex = juncIndex + 1;
                    if (juncIndex == juncDataList.size())
                    {
                        juncIndex = 0;
                    }
                    socialForce->removeWalls();
                    float hallwayLength = juncDataList[juncIndex].items().begin().value();
                    if (numAGVCompleted + 1 <= agvs.size())
                    {
                        cout << "*****=> " << juncDataList[juncIndex].items().begin().key() << ": " << hallwayLength << endl;
                    }
                    float length1Side = (hallwayLength) / 2;
                    juncData = {length1Side, length1Side};
                    createWalls();
                    // cout << agv->getId() << " - Remove and re-create agent" << endl;
                }
            }
            count_agvs = count_agvs + 1;
        }
    }
    if (count_agvs == agvs.size())
    {
        int totalRunningTime = currTime - startTime;
        Utility::writeResult(
            "data/end.txt", juncName, inputData["graphicsMode"]["value"], agvs,
            juncDataList,
            (int)inputData["runConcurrently"]["value"],
            (int)inputData["runMode"]["value"],
            (int)inputData["noRunPerHallway"]["value"],
            totalRunningTime);

        std::cout << "Maximum speed: " << maxSpeed << " - Minimum speed: " << minSpeed << endl;
        std::cout << "Finish in: " << Utility::convertTime(totalRunningTime) << endl;
        delete socialForce;
        socialForce = 0;

        exit(0); // Terminate program
    }

    if (animate)
    {
        socialForce->moveCrowd(static_cast<float>(frameTime) / 1000); // Perform calculations and move agents
        socialForce->moveAGVs(static_cast<float>(frameTime) / 1000);
    }
    computeFPS(&fps);
    glutPostRedisplay();
    glutIdleFunc(update); // Continuously execute 'update()'
}
