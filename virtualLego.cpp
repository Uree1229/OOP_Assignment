////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ��â�� Chang-hyeon Park, 
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

// window size
const int Width = 1024;
const int Height = 768;

// start button
bool start = false;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)



const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f} };
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.1   //blueball radius
#define B_RADIUS 0.2   //ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982
#define MAX_MOVE_BALL 100
#define BALL_SPEED 0.005 //ball speed

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
    float               center_x, center_y, center_z;
    float               m_radius;
    bool                blue_ball;                     //������ źȯ���� �����ϴ� ����
    float               m_velocity_x; // 1 or -1
    float               m_velocity_z; // tan 0 to tan pi/3
    bool                move; //

public:
    CSphere(void)                           // �� ��ü
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
        m_pSphereMesh = NULL;
        blue_ball = false;
        m_velocity_x = 1;
        m_velocity_z = 0;
        move = false;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
        
        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;

        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
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
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        m_pSphereMesh->DrawSubset(0);
    }

    bool hasIntersected(CSphere& ball)
    {
        // Insert your code here.

        return false;
    }

    void hitBy(CSphere& blue, CSphere& ball)
    {
        float distance = sqrt((ball.center_x - blue.center_x) * (ball.center_x - blue.center_x) + (ball.center_z - blue.center_z) * (ball.center_z - blue.center_z));
        float diff_rad = abs(ball.getRadius() - blue.getRadius());
        float sum_rad = ball.getRadius() + blue.getRadius();

        if (diff_rad <= distance && distance <= sum_rad)
        {
            exit(1);
        }
    }

    void move_Ball() {
        if (this->blue_ball == false && move == true) {
            D3DXVECTOR3 cord = this->getCenter();
           
            this->setCenter(cord.x + BALL_SPEED * this->m_velocity_x, float(M_RADIUS), cord.z + BALL_SPEED * this->m_velocity_z);
                
            
            }
            

        }

 public:   //setter

    void setove(bool move) {
        this->move = move;
    }
    void set_velocity() { // ������ ����
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(-60, 60);
        this->m_velocity_z = tan(dis(gen) % 60 / 180.0 * PI);

        std::uniform_int_distribution<int> dis1(-1, 1);
        this->m_velocity_x = dis1(gen);
       
    }
    void set_velocity_z(float m_velocity) {
        this->m_velocity_z = m_velocity;
    }
    void set_velocity_x(float dir) {
        this->m_velocity_x = dir;
    }
    void setBlue(bool sign) { this->blue_ball = sign; }               //�� ��ü�� źȯ���� ���� �� �����ϴ� setter
        
    void setPower(double vx, double vz)
    {
        this->m_velocity_x = vx;
        this->m_velocity_z = vz;
    }

    void setCenter(float x, float y, float z)
    {
        D3DXMATRIX m;
        center_x = x;   center_y = y;   center_z = z;
        D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }

    
  public: //getter

    bool getMove() {
        return this->move;
    }
    double getVelocity_X() { return this->m_velocity_x; }
    double getVelocity_Z() { return this->m_velocity_z; }
    bool getblueball() { return this->blue_ball; }
    float getRadius()
        const {
        if (this->blue_ball == true)
            return(float)(M_RADIUS);
        return (float)(B_RADIUS);
    }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }


    
    void ballUpdate(float timeDiff)
    {
     
        const float TIME_SCALE = 1.0;
        
        D3DXVECTOR3 cord = this->getCenter();
        double vx = abs(this->getVelocity_X());
        double vz = abs(this->getVelocity_Z());
        this->move_Ball();
     /*   if (vx > 0.01 || vz > 0.01)
        {
            float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
            float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

            //correction of position of ball
            // Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
            //tX, tZ �� ���̺��� ���������� ���� �Ÿ�
            if (tX >= (4.5 - M_RADIUS))                                    //���� ���� �ε����� �ݴ� �������� ƨ��� ���� ����
            {
                tX = 4.5 - M_RADIUS;
                setPower(-getVelocity_X() * 0.9, getVelocity_Z() * 0.9);
            }
            else if (tX <= (-4.5 + M_RADIUS))
            {
                tX = (-4.5 + M_RADIUS);
                setPower(-getVelocity_X() * 0.9, getVelocity_Z() * 0.9);
            }
            else if (tZ >= 3 - M_RADIUS)
            {
                tZ = 3 - M_RADIUS;
                setPower(getVelocity_X() * 0.9, -getVelocity_Z() * 0.9);
            }
            else if (tZ <= -3 + M_RADIUS)
            {
                tZ = -3 + M_RADIUS;
                setPower(getVelocity_X() * 0.9, -getVelocity_Z() * 0.9);
            }

            this->setCenter(tX, cord.y, tZ);
        }
        else
        {
            this->setPower(0, 0);
        }
        //this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
        double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
        if (rate < 0)
            rate = 0;
        this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
    */
    }




private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
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
        // Insert your code here.
        return false;
    }

    void hitBy(CSphere& ball)
    {
        D3DXVECTOR3 cord = ball.getCenter();
        float rad = ball.getRadius();
        float distance_x = abs(cord.x - this->m_x);
        float distance_z = abs(cord.z - this->m_z);
        //if (this->type == 0 && ball.getblueball() == true && ) {

        //}

        if (this->type == 0 && distance_x < rad) { //left wall collision
            ball.set_velocity_x(-1 * ball.getVelocity_X());
            ball.setCenter(this->m_x-0.2,float(M_RADIUS), cord.z);
            
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
    void settype(int type) { // 0 : left, 1: right, 2:up, 3: down
        this->type = type;
    }
    void setPosition(float x, float y, float z)
    {
        D3DXMATRIX m;
        this->m_x = x;
        this->m_z = z;

        D3DXMatrixTranslation(&m, x, y, z);         //x,y,z�� ��ǥ�� ��ķ� ��ȯ
        setLocalTransform(m);                  //��Ĵ�� ��ġ ��ȯ
    }

    float getHeight(void) const { return M_HEIGHT; }



private:
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh* m_pBoundMesh;
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
// -----------------------------------------------------------------------------
CWall   g_legoPlane;


CWall   g_legowall[4];
CSphere   g_sphere[4];
//CSphere g_test;
CSphere   g_target_blueball;
CLight   g_light;

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
    int i;

    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    // create plane and set the position
    if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;         //�� ����
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

    std::uniform_int_distribution<int> int_distribution(1, 4);               //��ġ ���� ���� ���� ����
    std::uniform_real_distribution<float> distribution_r(5.0, 6.0);
    std::uniform_real_distribution<float> distribution_l(-6.0, -5.0);
    std::uniform_real_distribution<float> distribution_u(4.0, 5.0);
    std::uniform_real_distribution<float> distribution_d(-5.0, -4.0);

    /*for (i = 0; i < 4; i++) {                                                   //������ �̸� ������� ��ü 4���� ���� ��ġ �������� �����ϵ��� ����������
        if (false == g_sphere[i].create(Device, sphereColor[i])) return false;         //�ð��� ���� ��ü�� �������� �����ϰ� �����ϵ��� �ؾ� �Ѵ�.

        std::random_device rd;

        std::mt19937 gen(rd());
        int random_int = int_distribution(gen);
        float right_x = distribution_r(gen);
        float left_x = distribution_l(gen);
        float up_y = distribution_u(gen);
        float down_y = distribution_d(gen);

        g_sphere[i].setBlue(false);

        if (random_int == 1)
            g_sphere[i].setCenter(right_x, (float)M_RADIUS, up_y);
        else if (random_int == 2)
            g_sphere[i].setCenter(left_x, (float)M_RADIUS, up_y);
        else if (random_int == 3)
            g_sphere[i].setCenter(right_x, (float)M_RADIUS, down_y);
        else
            g_sphere[i].setCenter(left_x, (float)M_RADIUS, down_y);
        g_sphere[i].setPower(0, 0);
    }*/
    for (i = 0; i < 4; i++) {
        if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
        g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
        g_sphere[i].setPower(0, 0);
        g_sphere[i].set_velocity();
   
    }
    /*if (false == g_test.create(Device, d3d::BLUE)) return false;
    g_test.setCenter(.0f, (float)M_RADIUS, 2.0f);
    g_test.set_velocity_x(3);
    g_test.set_velocity_z(0);
    */
    // create blue ball for set m_velocity
    g_target_blueball.setBlue(true); //true�� �����ؾ���
    if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
    g_target_blueball.setCenter(.0f, (float)M_RADIUS, .0f);

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


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
    int i = 0;
    int j = 0;

    if (Device)
    {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        // update the position of each ball. during update, check whether each ball hit by walls.
        for (i = 0; i < 4; i++) {
            g_sphere[i].ballUpdate(timeDelta);
                      
            for (j = 0; j < 4; j++) { g_legowall[i].hitBy(g_sphere[j]); }
          //  g_legowall[i].hitBy(g_test);
        }

        // check whether any two balls hit together and update the m_velocity of balls
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {


                if (i >= j) { continue; }
                g_sphere[i].hitBy(g_target_blueball,g_sphere[i]);
            }
        }

        // draw plane, walls, and spheres
        g_legoPlane.draw(Device, g_mWorld);
        for (i = 0; i < 4; i++) {
            g_legowall[i].draw(Device, g_mWorld);
            g_sphere[i].draw(Device, g_mWorld);
        }
        g_target_blueball.draw(Device, g_mWorld);
        //g_test.draw(Device, g_mWorld);
        g_light.draw(Device);

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
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
    case WM_KEYDOWN:                  //��ư �Է� ����
    {
        switch (wParam) {
        case VK_ESCAPE:               //ESC ������ ����
            ::DestroyWindow(hwnd);
            break;
        case VK_RETURN:               //���� Ű ������
            if (NULL != Device) {
                wire = !wire;
                Device->SetRenderState(D3DRS_FILLMODE,
                    (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
            }
            break;
        case VK_SPACE:               //�����̽� �� ������
            /*                                                //�����̽� �� ������ �Ͼ� ���� �����̴� �ڵ� �ּ�ó��
            D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
            D3DXVECTOR3   whitepos = g_sphere[3].getCenter();
            double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
               pow(targetpos.z - whitepos.z, 2)));      // �⺻ 1 ��и�
            if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }   //4 ��и�
            if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 ��и�
            if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 ��и�
            double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
            g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));
            */
            g_target_blueball.setove(true);
            start = !start;                  //�����̽� �� �Է� �� Ŀ�� ����� ( �Ķ� ���� ������ ��  �ֵ��� �ϴ� ���� )
            ShowCursor(!start);
            break;
        case VK_F1:
            //D3DXVECTOR3 coord3d[3];
            
            for (int i = 0; i < 4; i++) {
                if(g_sphere[i].getMove()== false) g_sphere[i].setove(true);
                else { g_sphere[i].setove(false); }
            }
            break;
            
        /*case VK_F2:
            //D3DXVECTOR3 coor;
            g_test.move_Ball();
            break;*/
        }
 
        break;
    }

    case WM_MOUSEMOVE:
    {
        int new_x = LOWORD(lParam);
        int new_y = HIWORD(lParam);
        float dx;
        float dy;
        if (MK_RBUTTON)
        {

        }
        /*
           if (LOWORD(wParam) & MK_LBUTTON) {            //���� Ŭ�� �Է� �� ���̺� ȸ���ϴ� �ڵ� �ּ� ó��

               if (isReset) {
                   isReset = false;
               } else {
                   D3DXVECTOR3 vDist;
                   D3DXVECTOR3 vTrans;
                   D3DXMATRIX mTrans;
                   D3DXMATRIX mX;
                   D3DXMATRIX mY;

                   switch (move) {
                   case WORLD_MOVE:
                       dx = (old_x - new_x) * 0.01f;
                       dy = (old_y - new_y) * 0.01f;
                       D3DXMatrixRotationY(&mX, dx);
                       D3DXMatrixRotationX(&mY, dy);
                       g_mWorld = g_mWorld * mX * mY;

                       break;
                   }
               }

               old_x = new_x;
               old_y = new_y;

           } else {
               isReset = true;

           if (LOWORD(wParam) & MK_RBUTTON) {
              dx = (old_x - new_x);// * 0.01f;
              dy = (old_y - new_y);// * 0.01f;

              D3DXVECTOR3 coord3d=g_target_blueball.getCenter();
              g_target_blueball.setCenter(coord3d.x+dx*(-0.007f),coord3d.y,coord3d.z+dy*0.007f );
           }
           old_x = new_x;
           old_y = new_y;

               move = WORLD_MOVE;
           }
        */


        if (start == true&& g_target_blueball.getMove() == true)               //�Ķ� ���� �����̵��� �ϴ� �ڵ�
        {
            dx = (old_x - new_x);
            dy = (old_y - new_y);
            dy = (old_y - new_y);
            D3DXVECTOR3 coord3d = g_target_blueball.getCenter();      //�Ķ� �� ���� ��ǥ ���

            if (coord3d.x >= (4.5 - M_RADIUS))                     //�Ķ� �� ���� ������ �ݴ��� ������ �̵��ϴ� if �ڵ�
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
                g_target_blueball.setCenter(coord3d.x + dx * 0.01f, (float)M_RADIUS, coord3d.z - dy * 0.01f);      //���� ��ǥ�� ���콺 �̵� ��ȭ�� ���Ͽ� ��ġ ��ȯ

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

    d3d::EnterMsgLoop(Display);

    Cleanup();

    Device->Release();

    return 0;
}