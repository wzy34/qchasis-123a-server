/**
 * This file is created by qiufuyu, TEAM 123A
 * 
 * This class provides an abstract layer for a whole complex chasis controlling process
 * using liberaries:
 *  -- lemlib
 *  -- okapi
 * 
 * Copyright: qiufuyu, TEAM 123A
 * 
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include"qchasis.h"
#include"qtimer.h"
using namespace okapi;

qchasis::qchasis(qchasis_drivemode mode)
{
    
    auto_chasis = std::make_shared<lemlib::Chassis>(drivetrain,lateralController,angularController,sensors);
    // if(is_calib)
    //     caliberate();  // Do caliberate and print time costs
    m_controller = std::make_shared<okapi::Controller>();
    if(mode == qchasis_drivemode::CALIB_DRIVE)
    {
        need_calib = true;
        diagno = false;
    }else if(mode == qchasis_drivemode::DIAGNO_DRIVE)
    {
        need_calib = true;
        diagno = true;
    }
    else if(mode == qchasis_drivemode::ONLY_DRIVE)
    {
        need_calib = false;
        diagno = false;
    }
    
    if(!diagno)
    {
        printing_t = std::make_unique<pros::Task>(([=]{
            while (1)
            {
                pros::lcd::set_text(7,"Powered By 123A QChasis!");
                pros::delay(1000);
                pros::lcd::set_text(7,"123A is the BEST!");
                pros::delay(1000);
                pros::lcd::set_text(7,"123A Will Win!");
                pros::delay(1000);
            }
            
        }));
    }else
        printing_t = nullptr;    
}
#define M pros::controller_id_e_t::E_CONTROLLER_MASTER
void qchasis::caliberate()
{
    if(!need_calib)
        return;
    pros::delay(5000);
    qtimer t;
    pros::lcd::set_text(0,"Auto Caliberating ... ");
    /**
     * To avoid failing to reset GYRO in Lemlib
     * We reset GYRO by hand before
    */
    m_controller->setText(2,0,"Please Wait");
    if(inertial_sensor.reset(true)!=1)
    {
        pros::c::controller_clear(M);
        pros::c::controller_set_text(M,0,0,"GYRO ERROR");
        if(errno == ENODEV)
        {
            pros::c::controller_set_text(M,1,0,"BAD PORT");
        }
        else if(errno == ENXIO)
        {
            pros::c::controller_set_text(M,1,0,"NOT A GYRO");
        }
        while (1)
        {
            pros::c::controller_rumble(M,".");
            pros::delay(200);
        }
        
    }
    auto_chasis->calibrate();
    //inertial_sensor.set_euler(pros::c::euler_s_t::)
    uint32_t sec = t.getTime() / 1000;
    //m_controller->rumble(".");
    pros::lcd::set_text(0,std::string("Caliberated in ")+std::to_string(sec)+std::string("(s)"));
    m_controller->setText(2,0,"CALIBERATED!");
    is_calib = true;
    inertial_sensor.set_rotation(base_theta);
}

void qchasis::tickUpdate(float multi)
{
    if(!is_calib && need_calib)
        return;
    if(diagno)
    {
        pros::lcd::set_background_color(255,0,0);
            auto pose = auto_chasis->getPose();
            pros::lcd::print(0,"DIAGNOSTIC MODE!");
            pros::lcd::print(1,"DO NOT DEPLOY IN GAME!");
            pros::lcd::print(3,"x:%lf",pose.x);
            pros::lcd::print(4,"y:%lf",pose.y);
            pros::lcd::print(5,"a:%lf",pose.theta);
            if(is_calib && need_calib)
            {
                if(m_controller->getDigital(CTRL::A))
                {
                    driveForward(61); // 24inch
                }else if(m_controller->getDigital(CTRL::B))
                {
                    driveDeltaTurn(45);
                }
                while(m_controller->getDigital(CTRL::A)||m_controller->getDigital(CTRL::B))
                    pros::delay(50);
            }else
            {
                m_controller->setText(2,0,"NOT CALIB!");
                m_controller->rumble("--");
                while (1)
                {
                    /* code */
                }
                
            }
        
    }
    else
    {
        auto pose = auto_chasis->getPose();
        pros::lcd::print(3,"x:%lf",pose.x);
        pros::lcd::print(4,"y:%lf",pose.y);
        pros::lcd::print(5,"a:%lf",pose.theta);
        if(m_controller->getAnalog(okapi::ControllerAnalog::leftX) 
        || m_controller->getAnalog(okapi::ControllerAnalog::leftY))
        {
            fst_move = true;
        }
        float power = m_controller->getAnalog(okapi::ControllerAnalog::leftY);
        float turn = m_controller->getAnalog(okapi::ControllerAnalog::leftX);
        if(abs(power)<0.1)
        {
            power=0;
        }
        if(abs(turn)<0.1)
        {
            turn=0;
        }
        float left = power + turn;
        float right = power - turn;
        left_side_motors.move(left*127*multi);
        right_side_motors.move(right*127*multi);
    }
    
}
void qchasis::setGyroHeading(double angle)
{
    inertial_sensor.set_rotation(angle);
}

std::shared_ptr<lemlib::Chassis> qchasis::getAutoDriver()
{
    return auto_chasis;
}
std::shared_ptr<okapi::Controller> qchasis::getController()
{
    return m_controller;
}

qchasis& qchasis::addGoal(lemlib::Pose g)
{
    m_goal = g;
    return (*this);
}

qchasis& qchasis::headToGoal(int timeout)
{

    auto_chasis->turnTo(m_goal.x,m_goal.y,timeout);
    return (*this);
}

void qchasis::trigAsyncAction(std::function<void()> act)
{
    async_action = nullptr;
    async_action = std::make_unique<pros::Task>(act);
}

qchasis &qchasis::driveTimedRun(float time_sec, int pct)
{
    auto_chasis->timedRun(time_sec,pct);
    //auto_chasis->brake();
    return (*this);
}

qchasis &qchasis::driveMoveTo(float dx, float dy, double timeout, bool is_abs,bool rev_head)
{
    if(is_abs)
    {
        auto_chasis->moveTo(dx,dy,(int)timeout,200.0,false,rev_head);
    }else
    {
        lemlib::Pose o = auto_chasis->getPose();
        auto_chasis->moveTo(dx+o.x,dy+o.y,(int)timeout,200.0,false,rev_head);
    }
    //auto_chasis->brake();
    return (*this);
}
#define IN2CM(x) (x/0.3937007874)
#define CM2IN(x) (x*0.3937007874)
#define DEG2RAD(x) (3.1415926f / 180.0f * x)
qchasis &qchasis::driveForward(float distance, int timeout)
{
    distance = CM2IN(distance);
    float theta = auto_chasis->getPose().theta;
    theta = DEG2RAD(theta);
    lemlib::Pose o = getAutoDriver()->getPose();
    float dx = std::sin(theta) * distance;
    float dy = std::cos(theta) * distance;
    getAutoDriver()->moveTo(o.x + dx, o.y + dy,timeout);
    //auto_chasis->brake();

    return (*this);
}

qchasis &qchasis::driveTurn(float angle, int timeout)
{
    auto_chasis->turnAngle(angle,timeout);
    //auto_chasis->brake();

    return (*this);
}

qchasis &qchasis::driveDeltaTurn(float delta, int timeout)
{
    auto_chasis->turnAngle(auto_chasis->getPose().theta + delta,timeout);
    //auto_chasis->brake();

    return (*this);
}

qchasis &qchasis::resetPose(float x, float y)
{
    auto_chasis->setPose(x,y,auto_chasis->getPose().theta);
    return (*this);
}

qchasis &qchasis::driveCurve(const char* name, float delta, int timeout)
{
    auto_chasis->follow(name,timeout,delta,false,200);
    return (*this);
}

qchasis &qchasis::driveTurnTo(float x, float y, int timeout,bool rev)
{
    auto_chasis->turnTo(x,y,timeout,rev);
    //auto_chasis->brake();
    return (*this);
}

qchasis &qchasis::driveDelay(int timeout)
{
    pros::delay(timeout);
    return (*this);
}

float qchasis::getGoalDistance()
{   
    lemlib::Pose p = auto_chasis->getPose();
    return IN2CM(std::sqrt(std::pow(m_goal.x - p.x,2) + std::pow(m_goal.y - p.y,2)));
}

qchasis &qchasis::driveGoalDistance(float distance, int timeout, float err_range)
{
    if(distance == -1)
        distance = avail_goal_dist;
    float now = getGoalDistance();
    float err = now -distance;
    if(std::abs(err) <= err_range)
        return (*this);
    driveForward(err,timeout);
    driveDelay(20);
    return (*this);
}
