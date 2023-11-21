////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <Windows.h>
#include <random>
#include <iostream>
#include <cmath>
#include <random>

IDirect3DDevice9* Device = NULL;
IDirect3DDevice9* EndingPage = NULL;
// window size
const int Width = 1024;
const int Height = 768;

// start button
bool temp_startpage = false;
bool start = false;                     // Game start control
static bool G_end = false;              // Game end control
static int G_event = 0;                 //
static float STATIC_BALL_SPEED = 0.007; //ball speed control
static int score = 0;                   // In game score
static int HP_count = 3;                //In game HP

const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.15   //blueball radius
#define B_RADIUS 0.1   //ball radius
#define B_DIAGONAL 0.15 // Box's Diagonal
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

#define PI 3.14159265

#define MAX_BALL_NUM 100 // max number of ball
#define MAX_ITEM_NUM 4 // max number of itembox

#define TIME_BALL_MOVE 4000 // ball's generating period
#define TIME_ITEM 8000      // item's generating period

#define FIXED_BALL_SPEED 0.001 // fixed speed


float angle_degree = 45.0f;             //angle element of dead event
float angle_radian = PI / 180.0;        //radian of angle

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
    float               center_x, center_y, center_z;
    float               m_radius;
    bool                blue_ball;                     //공인지 탄환인지 구분하는 변수
    float               m_velocity_x;                 // 1 or -1
    float               m_velocity_z;                 // tan 0 to tan pi/3 
    bool                move;                         // move control
    bool                isVisible;                    // visible control
    float               BALL_SPEED;                   // speed control : scalar
    // IDirect3DTexture9* m_pTexture;

public:
    CSphere(void)                           // 공 객체
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_pSphereMesh = NULL;
        blue_ball = false;
        move = false;
        isVisible = false;
        m_radius = 0;

        m_velocity_x = 1;
        m_velocity_z = 0;
        BALL_SPEED = FIXED_BALL_SPEED;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)return false;

        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;

        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))return false;
        return true;
    }

    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {

        if (NULL == pDevice || !getVisible()) return;

        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pSphereMesh->DrawSubset(0);
    }



    void hitBy(CSphere& blue, CSphere& ball) // blue ball & ball collison control 
    {

        float distance = sqrt((ball.center_x - blue.center_x) * (ball.center_x - blue.center_x) + (ball.center_z - blue.center_z) * (ball.center_z - blue.center_z));
        float diff_rad = abs(ball.getRadius() - blue.getRadius());
        float sum_rad = ball.getRadius() + blue.getRadius();

        if (ball.isVisible == true) {
            if (diff_rad <= distance && distance <= sum_rad)
            {
                ball.setVisible(false); // collison's ball distroy
                if (HP_count > 0) { //HP_count decrease
                    HP_count--;
                }
            }
        }

    }

    void move_Ball() {                                     //ball moving control is influenced in BALL_SPEED * Veclocity
        if (this->blue_ball == false && move == true) {
            D3DXVECTOR3 cord = this->getCenter();

            this->setCenter(cord.x + this->BALL_SPEED * this->m_velocity_x, float(M_RADIUS), cord.z + this->BALL_SPEED * this->m_velocity_z);


        }


    }

public:   //setter

    void setmove(bool move) { this->move = move; }
    void set_velocity() { // Velocity_x & z is generated using random variable
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(-60, 60);
        this->m_velocity_z = tan(dis(gen) % 60 / 180.0 * PI);

        std::uniform_int_distribution<int> dis1(0, 1);
        if (dis1(gen) == 0) this->m_velocity_x = -1;
        else { this->m_velocity_x = 1; }

    }
    void set_velocity_z(float m_velocity) { this->m_velocity_z = m_velocity; }
    void set_velocity_x(float dir) { this->m_velocity_x = dir; }
    void setBlue(bool sign) { this->blue_ball = sign; }               //공 객체에 탄환인지 공인 지 구분하는 setter   
    void setPower(double vx, double vz) { this->m_velocity_x = vx; this->m_velocity_z = vz; }
    void setCenter(float x, float y, float z)
    {
        D3DXMATRIX m;
        center_x = x;   center_y = y;   center_z = z;
        D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }
    void setLocalTransform(const D3DXMATRIX& mLocal) { this->m_mLocal = mLocal; }
    void setMove(bool move) { this->move = move; }
    void setVisible(bool visible) { this->isVisible = visible; }
    void setSpeed(float speed) { this->BALL_SPEED = speed; }

public: //getter

    float getSpeed() { return this->BALL_SPEED; }
    bool getMove() { return this->move; }
    double getVelocity_X() { return this->m_velocity_x; }
    double getVelocity_Z() { return this->m_velocity_z; }
    bool getblueball() { return this->blue_ball; }
    D3DXVECTOR3 getCenter(void) const { D3DXVECTOR3 org(center_x, center_y, center_z); return org; }
    float getRadius()
        const {
        if (this->blue_ball == true)
            return(float)(M_RADIUS);
        return (float)(B_RADIUS);
    }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    bool getVisible() { return isVisible; }

    void ballUpdate(float timeDiff) // ball update : using ball_move
    {

        const float TIME_SCALE = 0.5;

        D3DXVECTOR3 cord = this->getCenter();
        double vx = abs(this->getVelocity_X());
        double vz = abs(this->getVelocity_Z());
        this->move_Ball();

    }


private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9           m_mtrl;
    ID3DXMesh* m_pSphereMesh;

};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

    float               m_x;
    float               m_z;
    float               m_width;
    float               m_depth;
    float               m_height;
    int                 type; // 0 : left, 1: right, 2:up, 3: down
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;

        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;

        m_width = iwidth;
        m_depth = idepth;

        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }

    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pBoundMesh->DrawSubset(0);
    }

    bool hasIntersected(CSphere& ball)
    {
        D3DXVECTOR3 cord = ball.getCenter();
        float rad = ball.getRadius();
        float distance_x = abs(cord.x - this->m_x); //공의 x좌표-벽의 x좌표 의 절대값
        float distance_z = abs(cord.z - this->m_z); //공의 z좌표-벽의 z좌표 의 절대값

        if ((this->type == 0 && distance_x <= rad) || this->type == 1 && distance_x <= rad
            || this->type == 2 && distance_z <= rad || this->type == 3 && distance_z <= rad) { //left wall collision
            return true;
        }


        return false;
    }

    void hitBy(CSphere& ball) // ball & wall collison 1 : physical reflect
    {
        D3DXVECTOR3 cord = ball.getCenter();
        float rad = ball.getRadius();
        float distance_x = abs(cord.x - this->m_x);
        float distance_z = abs(cord.z - this->m_z);

        if (this->type == 0 && distance_x < rad) { //left wall collision
            ball.set_velocity_x(-1 * ball.getVelocity_X());
            ball.setCenter(this->m_x - 0.2, float(M_RADIUS), cord.z);

        }
        if (this->type == 1 && distance_x <= rad) { //right wall collision
            ball.set_velocity_x(-1 * ball.getVelocity_X());
            ball.setCenter(this->m_x + 0.2, float(M_RADIUS), cord.z);

        }
        if (this->type == 2 && distance_z <= rad) { //upper wall collision
            ball.set_velocity_z(-1 * ball.getVelocity_Z());
            ball.setCenter(cord.x, float(M_RADIUS), this->m_z + 0.2);

        }
        if (this->type == 3 && distance_z <= rad) { //down wall collision
            ball.set_velocity_z(-1 * ball.getVelocity_Z());
            ball.setCenter(cord.x, float(M_RADIUS), this->m_z - 0.2);
        }


    }
    void hitBy(CSphere& target, CSphere& ball) // ball & wall collison 2 : toward target = blue
    {
        D3DXVECTOR3 cordb = ball.getCenter();
        D3DXVECTOR3 cordt = target.getCenter();
        float radb = ball.getRadius();
        float distance_bx = abs(cordb.x - this->m_x);
        float distance_bz = abs(cordb.z - this->m_z);
        float radt = target.getRadius();
        float distance_tx = abs(cordt.x - this->m_x);
        float distance_tz = abs(cordt.z - this->m_z);

        // Calculate the direction from the ball to the target
        D3DXVECTOR3 direction = cordt - cordb;
        D3DXVec3Normalize(&direction, &direction);

        if (this->type == 0 && distance_bx < radb) { //left wall collision
            ball.set_velocity_x(direction.x);
            ball.set_velocity_z(direction.z);
            ball.setCenter(this->m_x - 0.2, float(M_RADIUS), cordb.z);
        }
        if (this->type == 1 && distance_bx <= radb) { //right wall collision
            ball.set_velocity_x(direction.x);
            ball.setCenter(this->m_x + 0.2, float(M_RADIUS), cordb.z);
        }
        if (this->type == 2 && distance_bz <= radb) { //upper wall collision
            ball.set_velocity_z(direction.z);
            ball.setCenter(cordb.x, float(M_RADIUS), this->m_z + 0.2);
        }
        if (this->type == 3 && distance_bz <= radb) { //down wall collision
            ball.set_velocity_z(direction.z);
            ball.setCenter(cordb.x, float(M_RADIUS), this->m_z - 0.2);
        }
    }

    void settype(int type) { // 0 : left, 1: right, 2:up, 3: down
        this->type = type;
    }
    void setPosition(float x, float y, float z)
    {
        D3DXMATRIX m;


        this->m_x = x;
        this->m_z = z;

        D3DXMatrixTranslation(&m, x, y, z);         //x,y,z의 좌표를 행렬로 변환
        setLocalTransform(m);                  //행렬대로 위치 변환
    }

    float getHeight(void) const { return M_HEIGHT; }

private:
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh* m_pBoundMesh;
};
// -----------------------------------------------------------------------------
// CBox class definition
// -----------------------------------------------------------------------------
class CBox {
private:
    float               m_x, m_z, m_width, m_depth, m_height;
    float               m_diagnoal;                              // diagnoal control
    bool                isVisible;                               // visible control
    int                 Item_type;                               // item type
    bool                item;                                    // item object or not
    bool                HP;                                      // HP object or not
    // IDirect3DTexture9* m_pTexture;

public:
    CBox(void)                           // 공 객체
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_pBoxMesh = NULL;
        m_diagnoal = 0;
        isVisible = false;
        m_width = 0;
        m_height = 0;
        m_depth = 0;
        HP = false;
    }
    ~CBox(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;

        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;


        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoxMesh, NULL)))
            return false;
        return true;
    }


    void destroy(void)
    {
        if (m_pBoxMesh != NULL) {
            m_pBoxMesh->Release();
            m_pBoxMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice || !getVisible())
            return;

        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pBoxMesh->DrawSubset(0);
    }

    void hitBy(CSphere& blue) // itembox & blue ball's collison
    {
        D3DXVECTOR3 cord = blue.getCenter();
        float distance = sqrt((this->getX() - cord.x) * (this->getX() - cord.x) + (this->getZ() - cord.z) * (this->getZ() - cord.z));
        float half_diagonal = sqrt(2 * (B_DIAGONAL * B_DIAGONAL)) / 2;

        if (this->getitem() == true && blue.getblueball() == true && this->getVisible() == true) {
            if (distance <= half_diagonal + blue.getRadius()) {

                switch (this->getitem_type()) {

                case 1: // ball Speed down
                    if (G_event == 0) {
                        STATIC_BALL_SPEED = STATIC_BALL_SPEED * 1 / 2;     // ball speed up
                        score += 50;                                     // score + 50
                        G_event = 1;                                     // event start
                    }
                    break;

                case 2: // heal
                    if (HP_count < 3) HP_count++;
                    G_event = 2;
                    break;
                }
                this->setVisible(false);
            }


        }
    }


public:   //setter

    void setPosition(float x, float y, float z)
    {
        D3DXMATRIX m;


        this->m_x = x;
        this->m_z = z;

        D3DXMatrixTranslation(&m, x, y, z);         //x,y,z의 좌표를 행렬로 변환
        setLocalTransform(m);                      //행렬대로 위치 변환
    }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    void setVisible(bool visible) { isVisible = visible; }
    void setDiagonal(float diagonal) { this->m_diagnoal = diagonal; }
    void setItem(bool item) { this->item = item; }
    void setItemType(int type) { this->Item_type = type; }
    void setHP(bool HP) { this->HP = HP; }

public: //getter

    const float getDiagonal(void) const { return this->m_diagnoal; }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    const bool getVisible(void) const { return this->isVisible; }
    const int getitem_type(void)const { return this->Item_type; }
    const bool getitem(void) const { return this->item; }
    const float getX(void)const { return this->m_x; }
    const float getZ(void)const { return this->m_z; }
    const int getHP(void) const { return this->HP; }

private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9           m_mtrl;
    ID3DXMesh* m_pBoxMesh;

};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;

        m_bound._center = lit.Position;
        m_bound._radius = radius;

        m_lit.Type = lit.Type;
        m_lit.Diffuse = lit.Diffuse;
        m_lit.Specular = lit.Specular;
        m_lit.Ambient = lit.Ambient;
        m_lit.Position = lit.Position;
        m_lit.Direction = lit.Direction;
        m_lit.Range = lit.Range;
        m_lit.Falloff = lit.Falloff;
        m_lit.Attenuation0 = lit.Attenuation0;
        m_lit.Attenuation1 = lit.Attenuation1;
        m_lit.Attenuation2 = lit.Attenuation2;
        m_lit.Theta = lit.Theta;
        m_lit.Phi = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;

        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;

        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh* m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// Global variables
// -----------------------------------------------------------------------------
CWall   g_legoPlane;
CWall   g_legowall[4];
CSphere   g_sphere[MAX_BALL_NUM];
CSphere end_ball[8];
CSphere   g_target_blueball;
CLight   g_light;

CBox health[3];
CBox item[50]; // test Box

float spherePos[MAX_BALL_NUM][2];
float sitemPos[MAX_ITEM_NUM][2];

static int G_time = 0;
static int G_time_item = 0;
static int G_num = 0;
static int G_num_item = 0;
double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------



void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
    g_target_blueball.setVisible(false);
    int i;

    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    // create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;         //판 색상
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

    // create walls and set the position. note that there are four walls
    if (false == g_legowall[0].create(Device, -1, -1, 9, 0.1f, 0.1f, d3d::DARKRED)) return false;
    g_legowall[0].setPosition(0.0f, 0.12f, 3.06f); g_legowall[0].settype(3); //down

    if (false == g_legowall[1].create(Device, -1, -1, 9, 0.1f, 0.1f, d3d::DARKRED)) return false;
    g_legowall[1].setPosition(0.0f, 0.12f, -3.06f); g_legowall[1].settype(2); //upper

    if (false == g_legowall[2].create(Device, -1, -1, 0.1f, 0.1f, 6.24f, d3d::DARKRED)) return false;
    g_legowall[2].setPosition(4.56f, 0.12f, 0.0f); g_legowall[2].settype(0); //left

    if (false == g_legowall[3].create(Device, -1, -1, 0.1f, 0.1f, 6.24f, d3d::DARKRED)) return false;
    g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f); g_legowall[3].settype(1);

    // create four balls and set the position

    std::uniform_real_distribution<float> disw(-3.0f, 3.0f);
    std::uniform_real_distribution<float> dish(-4.0f, 4.0f);
    std::uniform_int_distribution<int>dis(1, 2);
    for (i = 0; i < MAX_BALL_NUM; i++) {                                                   //기존의 미리 만들어진 객체 4개에 대해 위치 랜덤으로 생성하도록 구현했지만
        if (false == g_sphere[i].create(Device, sphereColor[i])) return false;         //시간초 마다 객체를 동적으로 생성하고 제거하도록 해야 한다.

        std::random_device rd;

        std::mt19937 gen(rd());
        float width = disw(gen);
        float height = dish(gen);

        g_sphere[i].setBlue(false);
        spherePos[i][0] = height;
        spherePos[i][1] = width;
    }
    for (i = 0; i < MAX_ITEM_NUM; i++) {                                                   //기존의 미리 만들어진 객체 4개에 대해 위치 랜덤으로 생성하도록 구현했지만
        std::random_device rd;

        std::mt19937 gen(rd());
        float width = disw(gen);
        float height = dish(gen);
        int type = dis(gen);

        sitemPos[i][0] = height;
        sitemPos[i][1] = width;

        if (type == 1) {
            if (false == item[i].create(Device, (float)B_DIAGONAL, (float)B_DIAGONAL, (float)B_DIAGONAL, d3d::BLACK)) return false;
            item[i].setPosition(sitemPos[i][0], 0.0f, sitemPos[i][1]); item[i].setItem(true); item[i].setItemType(type);
        }
        else if (type == 2) {
            if (false == item[i].create(Device, (float)B_DIAGONAL, (float)B_DIAGONAL, (float)B_DIAGONAL, d3d::RED)) return false;
            item[i].setPosition(sitemPos[i][0], 0.0f, sitemPos[i][1]); item[i].setItem(true); item[i].setItemType(type);
        }


    }

    for (i = 0; i < MAX_BALL_NUM; i++) {
        if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
        g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
        g_sphere[i].setPower(0, 0);
        g_sphere[i].set_velocity();
    }

    //create ending event
    for (i = 0; i < 8; i++)
    {
        if (false == end_ball[i].create(Device, d3d::MAGENTA)) return false;
        end_ball[i].setCenter(0, 0, 0);
        end_ball[i].setPower(0, 0);
        end_ball[i].set_velocity_x(3 * cos(i * angle_degree * angle_radian));
        end_ball[i].set_velocity_z(3 * sin(i * angle_degree * angle_radian));
    }


    //create Box
    for (int i = 0; i < HP_count; i++) {
        if (false == health[i].create(Device, 4 * (float)B_DIAGONAL, (float)B_DIAGONAL, (float)B_DIAGONAL, d3d::DARKRED)) return false;         //판 색상
        health[i].setPosition(3 - (float)i, 0.0f, -3.5f); health[i].setVisible(true); health[i].setHP(true);

    }


    // create blue ball for set m_velocity
    g_target_blueball.setBlue(true);
    if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
    g_target_blueball.setCenter(.0f, (float)M_RADIUS, .0f);
    g_target_blueball.setVisible(false);

    // light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type = D3DLIGHT_POINT;
    lit.Diffuse = d3d::WHITE * 7.0f;
    lit.Specular = d3d::WHITE * 1.9f;
    lit.Ambient = d3d::WHITE * 1.9f;
    lit.Position = D3DXVECTOR3(0.0f, 10.0f, 0.0f);
    lit.Range = 100.0f;

    lit.Attenuation0 = 0.f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;

    // Position and aim the camera.
    D3DXVECTOR3 pos(0.0f, 10.0f, 0.1f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
    D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
    Device->SetTransform(D3DTS_VIEW, &g_mView);

    // Set the projection matrix.
    D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
        (float)Width / (float)Height, 1.0f, 100.0f);
    Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    g_light.setLight(Device, g_mWorld);
    return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
    for (int i = 0; i < 4; i++) {
        g_legowall[i].destroy();
    }
    destroyAllLegoBlock();
    g_light.destroy();
}
HDC hdc = GetDC(NULL);

struct D3DVERTEX
{
    FLOAT x, y, z, rhw;
    DWORD color;
};

void DrawStartPage()
{
    Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
    Device->BeginScene();

    RECT rectTitle{ 250, 250, 750, 300 };
    RECT rectMessage{ 200, 350, 800, 450 };

    D3DVERTEX vertices[] =
    {
        { 200.0f, 350.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(230,230,230) },
        { 800.0f, 350.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(230,230,230) },
        { 800.0f, 450.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(140,140,140) },
        { 200.0f, 450.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(140,140,140) }
    };

    Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(D3DVERTEX));

    LPD3DXFONT pTitleFont = NULL;
    HRESULT hr = D3DXCreateFont(Device, 70, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Copperplate Gothic Bold", &pTitleFont);
    pTitleFont->DrawText(NULL, "Bullet Hell", -1, &rectTitle, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(128, 0, 0));

    LPD3DXFONT pFont = NULL;
    D3DXCreateFont(Device, 40, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Copperplate Gothic Light", &pFont);
    pFont->DrawText(NULL, "Press Space to Start", -1, &rectMessage, DT_CENTER | DT_VCENTER, D3DCOLOR_XRGB(0, 0, 0));

    Device->EndScene();
    Device->Present(0, 0, 0, 0);

    pFont->Release();
    pFont = nullptr;
    pTitleFont->Release();
    pTitleFont = nullptr;
}

// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
int k = 0;

bool Display(float timeDelta)
{
    int i = 0;
    int j = 0;

    D3DXVECTOR3 coord3d = g_target_blueball.getCenter();        //fetch coordination of blue ball

    WCHAR word[1024];
    int num = 100;

    if (Device)
    {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();


        // update the position of each ball. during update, check whether each ball hit by walls.
        for (i = 0; i < MAX_BALL_NUM; i++) {
            g_sphere[i].ballUpdate(timeDelta);
            for (j = 0; j < 4; j++) {
                if (g_legowall[j].hasIntersected(g_sphere[i]) && g_sphere[i].getVisible()==true)
                {
                    score += 100;
                    if (score % 3 == 0) {
                        g_legowall[j].hitBy(g_target_blueball, g_sphere[i]); ;
                    }
                    else {
                        g_legowall[j].hitBy(g_sphere[i]); ;

                    }
                }
            }
        }

        // check whether any two balls hit together and update the m_velocity of balls
        for (i = 0; i < MAX_BALL_NUM; i++) {
            g_sphere[i].hitBy(g_target_blueball, g_sphere[i]);
        }
        for (i = 0; i < MAX_ITEM_NUM; i++) {
            item[i].hitBy(g_target_blueball);
            item[i].draw(Device, g_mWorld);
        }
        g_legoPlane.draw(Device, g_mWorld);
        for (i = 0; i < 4; i++) {
            g_legowall[i].draw(Device, g_mWorld);
        }

        // show the health bar
        if (HP_count == 3) {
            health[0].setHP(true);
            health[1].setHP(true);
            health[2].setHP(true);
        }
        else if (HP_count == 2) {
            health[0].setHP(true);
            health[1].setHP(true);
            health[2].setHP(false);
        }
        else if (HP_count == 1) {
            health[0].setHP(true);
            health[1].setHP(false);
            health[2].setHP(false);
        }
        else if (HP_count == 0) {
            health[0].setHP(false);
            health[1].setHP(false);
            health[2].setHP(false);
        }

        //show or hide the item
        for (i = 0; i < 3; i++) {

            if (health[i].getHP() == false) {
                health[i].setVisible(false);
            }
            else {
                health[i].setVisible(true);
            }
            health[i].draw(Device, g_mWorld);
        }

        //if HP_count==0
        if (HP_count == 0) {
            for (int i = 0; i < 8; i++)
            {
                end_ball[i].setVisible(true);
                end_ball[i].setMove(true);
                end_ball[i].ballUpdate(timeDelta);

                end_ball[i].draw(Device, g_mWorld);
            }
            G_end = true;
        }
        else
        {
            for (int i = 0; i < 8; i++)
                end_ball[i].setCenter(coord3d.x, (float)B_RADIUS, coord3d.z);
        }

        // draw ball
        for (i = 0; i < MAX_BALL_NUM; i++) {
            g_sphere[i].draw(Device, g_mWorld);
            g_sphere[i].setSpeed(STATIC_BALL_SPEED);

        }
        
        //draw startpage
        if (!temp_startpage) DrawStartPage();


        if (start) {
            // draw plane, walls, and spheres
            if (G_end == true) {                    // G_end == true => HP = 0
                for (i = 0; i < MAX_BALL_NUM; i++) {
                    g_sphere[i].setMove(false);
                    
                }
                g_target_blueball.setMove(false);
                g_target_blueball.setVisible(false);
            }
            else {
                //draw balls every certain period of time
                G_time++;                            // time count ++

                if (G_time > TIME_BALL_MOVE) { // init G_time & add new ball => repeat MAX_BALL_NUM
                    G_time = 0;
                    g_sphere[G_num].setVisible(true);
                    g_sphere[G_num].setMove(true);
                    if (G_num < MAX_BALL_NUM) { G_num++; }
                }
            }

            // draw items every certain period of time
            if (G_num_item == 0) {
                item[G_num_item].setVisible(true);
                G_num_item = 1;
            }
            if (G_event != 0) {

                G_time_item++;

                if (G_time_item > TIME_ITEM) {
                    G_time_item = 0;
                    if (G_event == 1) STATIC_BALL_SPEED = FIXED_BALL_SPEED * (1 + 3 * G_num_item / 100);
                    G_event = 0;
                    item[G_num_item].setVisible(true);
                    if (G_num_item < MAX_ITEM_NUM) { G_num_item++; }
                }

            }

        }

        g_target_blueball.draw(Device, g_mWorld);
        g_light.draw(Device);

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);

        // print the score on the window
        if (temp_startpage)
        {
            //score and set blueball visible
            wsprintfW(word, L"SCORE : %d", score);
            TextOutW(hdc, 100, 100, word, wcslen(word));
        }

    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool wire = false;
    static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

    int rectLeft = Width / 2;
    int rectRight = Width / 2;
    int rectTop = Height / 2;
    int rectBottom = Height / 2;

    switch (msg) {
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        break;
    }
    case WM_KEYDOWN:                  //버튼 입력 변수
    {
        switch (wParam) {
        case VK_ESCAPE:               //ESC 누르면 나감
            DestroyWindow(hwnd);
            break;
        case VK_RETURN:               //엔터 키 누르면
            if (NULL != Device) {
                wire = !wire;
                Device->SetRenderState(D3DRS_FILLMODE,
                    (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
            }
            break;
        case VK_SPACE:
            if (!temp_startpage)
            {
                DrawStartPage();
                temp_startpage = true;
            }
            //g_target_blueball.setBlue()
            g_target_blueball.setmove(true);
            start = !start;                  //스페이스 바 입력 시 커서 사라짐 ( 파란 공이 움직일 수  있도록 하는 조건 )
            ShowCursor(!start);
            if (!g_target_blueball.getVisible() == true)
                g_target_blueball.setVisible(true);
            break;
        case VK_F1:

            for (int i = 0; i < MAX_BALL_NUM; i++) {
                if (g_sphere[i].getMove() == false) g_sphere[i].setmove(true);
                else { g_sphere[i].setmove(false); }
            }
            break;


        }

        break;
    }

    case WM_MOUSEMOVE:
    {
        int new_x = LOWORD(lParam);
        int new_y = HIWORD(lParam);
        float dx;
        float dy;

        if (start == true && g_target_blueball.getMove() == true)               //파란 공이 움직이도록 하는 코드
        {
            dx = (old_x - new_x);
            dy = (old_y - new_y);
            dy = (old_y - new_y);
            D3DXVECTOR3 coord3d = g_target_blueball.getCenter();      //파란 공 현재 좌표 얻기

            if (coord3d.x >= (4.5 - M_RADIUS))                     //파란 공 벽에 닿으면 반대편 벽으로 이동하는 if 코드
            {
                coord3d.x = -4.5 + 2 * M_RADIUS;
                g_target_blueball.setCenter(coord3d.x, (float)M_RADIUS, coord3d.z);
            }
            else if (coord3d.x <= (-4.5 + M_RADIUS))
            {
                coord3d.x = 4.5 - 2 * M_RADIUS;
                g_target_blueball.setCenter(coord3d.x, (float)M_RADIUS, coord3d.z);
            }
            else if (coord3d.z >= 3 - M_RADIUS)
            {
                coord3d.z = -3 + 2 * M_RADIUS;
                g_target_blueball.setCenter(coord3d.x, (float)M_RADIUS, coord3d.z);
            }
            else if (coord3d.z <= -3 + M_RADIUS)
            {
                coord3d.z = 3 - 2 * M_RADIUS;
                g_target_blueball.setCenter(coord3d.x, (float)M_RADIUS, coord3d.z);
            }

            else
                g_target_blueball.setCenter(coord3d.x + dx * 0.01f, (float)M_RADIUS, coord3d.z - dy * 0.01f);      //현재 좌표에 마우스 이동 변화량 더하여 위치 변환

        }
        old_x = new_x;
        old_y = new_y;

        break;
    }
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE prevInstance,
    PSTR cmdLine,
    int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));

    if (!d3d::InitD3D(hinstance,
        Width, Height, true, D3DDEVTYPE_HAL, &Device))
    {
        ::MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }

    if (!Setup())
    {
        ::MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }
    DrawStartPage();
    d3d::EnterMsgLoop(Display);

    Cleanup();

    Device->Release();

    return 0;
}
