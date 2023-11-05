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

#ifndef _H_QCHASIS
#define _H_QCHASIS
#include"api.h"
#include"okapi/api.hpp"
#include"pros/apix.h"
#include"pal/gterm.h"
#include"lemlib/api.hpp"
#include<vector>
#include<string>
#include<memory>

/**
 * Choose whether to enable 6-motors mode
 *  define the macro below: enable
 *  otherwise: disable
*/
{tri_motors}#define QCHASIS_TRI



using namespace okapi;

using CTRL = okapi::ControllerDigital;
namespace QChasisConfig
{
const int LEFT_WHEEL_FRONT = {lwf};
const int LEFT_WHEEL_MID = {lwm};
const int LEFT_WHEEL_BACK = {lwb};
const int RIGHT_WHEEL_FRONT = {rwf};
const int RIGHT_WHEEL_MID = {rwm};
const int RIGHT_WHEEL_BACK = {rwb};

const int GYRO_PORT = {gyro};
const float TRACK_WIDTH = {track};
const float WHEEL_DIAMETER ={diameter};
const int WHEEL_RPM = {rpm} ;//360;  //36->84
const pros::motor_gearset_e_t GEAREST_BOX =  {gearnum};
const okapi::AbstractMotor::gearset OKAPI_GEAREST = {gearcolor};

const float LINEAR_KP ={pids_drive_kp};
const float LINEAR_KD ={pids_drive_kd};

const float ANGLE_KP ={pids_angle_kp};
const float ANGLE_KD ={pids_angle_kd};


const float SHOOT_DIST = {shoot_dist};
};


/** qchasis_drivemode */
typedef enum
{
  CALIB_DRIVE,  /**< activate caliberating and enter opcontrol mode*/
  ONLY_DRIVE,   /**< DO NOT CALIBERATE, directly enter opcontrol mode*/
  DIAGNO_DRIVE,  /**< activate caliberating and enter diagnostic mode*/
  NONE_MODE
}qchasis_drivemode;

class qchasis : std::enable_shared_from_this<qchasis>
{
private:
  //std::shared_ptr<okapi::ChassisController> drive_chasis;
  std::shared_ptr<lemlib::Chassis> auto_chasis;
  std::unique_ptr<pros::Task> printing_t;
  std::unique_ptr<pros::Task> async_action;
  std::shared_ptr<okapi::Controller> m_controller;
  lemlib::Pose m_goal= {0,0,0};
  std::shared_ptr<okapi::AsyncMotionProfileController> motion_profiles;

  float base_theta = {toward};

  bool fst_move = false;
  bool is_calib = false;
  bool m_is_arc = true;
  bool is_left_auto = true;

  bool need_calib = {need_calib};
  bool diagno = {diagno};

  float avail_goal_dist = QChasisConfig::SHOOT_DIST;

  pros::Motor left_front_motor {QChasisConfig::LEFT_WHEEL_FRONT, QChasisConfig::GEAREST_BOX, {revl1}}; // port 1, blue gearbox, not reversed
  pros::Motor left_mid_motor{QChasisConfig::LEFT_WHEEL_MID, QChasisConfig::GEAREST_BOX, {revl2}}; // port 2, green gearbox, not reversed
  pros::Motor left_back_motor{QChasisConfig::LEFT_WHEEL_BACK, QChasisConfig::GEAREST_BOX, {revl3}}; // port 2, green gearbox, not reversed
  pros::Motor right_front_motor{QChasisConfig::RIGHT_WHEEL_FRONT, QChasisConfig::GEAREST_BOX, {revr1}}; // port 1, blue gearbox, not reversed
  pros::Motor right_mid_motor{QChasisConfig::RIGHT_WHEEL_MID, QChasisConfig::GEAREST_BOX, {revr2}}; // port 2, green gearbox, not reversed
  pros::Motor right_back_motor{QChasisConfig::RIGHT_WHEEL_BACK, QChasisConfig::GEAREST_BOX, {revr3}}; // port 2, green gearbox, not reversed

#ifdef QCHASIS_TRI
  pros::MotorGroup left_side_motors{{left_front_motor,left_mid_motor, left_back_motor}};
  pros::MotorGroup right_side_motors{{right_front_motor,right_mid_motor, right_back_motor}};
#else
  pros::MotorGroup left_side_motors{{left_front_motor, left_back_motor}};
  pros::MotorGroup right_side_motors{{right_front_motor, right_back_motor}};
#endif
  pros::MotorGroup left_side_track{{left_front_motor, left_back_motor}};
  pros::MotorGroup right_side_track{{right_front_motor, right_back_motor}};
  pros::Imu inertial_sensor{QChasisConfig::GYRO_PORT}; // port 2
  lemlib::Drivetrain_t drivetrain {
    &left_side_motors, // left drivetrain motors
    &right_side_motors, // right drivetrain motors
    QChasisConfig::TRACK_WIDTH, // track width = 26cm
    QChasisConfig::WHEEL_DIAMETER, // wheel diameter (inch)
    QChasisConfig::WHEEL_RPM // wheel rpm (36->60)
  };
  lemlib::TrackingWheel track_vert_1{&left_side_track,QChasisConfig::WHEEL_DIAMETER,QChasisConfig::TRACK_WIDTH / 2.0f, QChasisConfig::WHEEL_RPM};
  lemlib::TrackingWheel track_vert_2{&right_side_track,QChasisConfig::WHEEL_DIAMETER,QChasisConfig::TRACK_WIDTH / 2.0f, QChasisConfig::WHEEL_RPM};

  lemlib::OdomSensors_t sensors {
    nullptr,
    nullptr,
    nullptr, // horizontal tracking wheel 1
    nullptr, // we don't have a second tracking wheel, so we set it to nullptr
    &inertial_sensor // inertial sensor
  };
  lemlib::ChassisController_t lateralController {
    QChasisConfig::LINEAR_KP, // kP
    QChasisConfig::LINEAR_KD, // kD
    1, // smallErrorRange
    100, // smallErrorTimeout
    3, // largeErrorRange
    500, // largeErrorTimeout
    {pids_drive_ks} // slew rate
};
 
// turning PID
lemlib::ChassisController_t angularController {
    QChasisConfig::ANGLE_KP, // kP
    QChasisConfig::ANGLE_KD, // kD
    1, // smallErrorRange
    100, // smallErrorTimeout
    3, // largeErrorRange
    500, // largeErrorTimeout
    {pids_angle_ks} // slew rate
};
public:

    /**
     * @brief Initialize the qchasis
     * @attention Initialization does not include odometry's caliberate!
     * 
     */
    qchasis(qchasis_drivemode mode = qchasis_drivemode::NONE_MODE);

    /**
     * @brief Update function, called every 10ms
     * 
     * @param multi multiple of speed
     * 
    */
    void tickUpdate(float multi=1);

    void releaseMotors();
    
    /**
     * @brief  the Odometry system
     * 
     * @attention: If the controller SHAKED FOR A LONG TIME, it means `fail to reset the GYRO`!!!
     *           : IF the controller SHAKED JUST ONCE, it means `GYRO Rests OK` !!!
    */
    void caliberate();

    /**
     * @brief reset Gyro heading
     * @attention you should not use it manually, take `base_theta` instead
    */
    void setGyroHeading(double angle);

    std::shared_ptr<lemlib::Chassis> getAutoDriver();

    std::shared_ptr<okapi::Controller> getController();

    /**
     * @brief set the goal's coordinates
    */
    qchasis& addGoal(lemlib::Pose g);

    /**
     * @brief turn the chasis to the goal
    */
    qchasis& headToGoal(int timeout = 800);

    /**
     * @brief run a function async
    */
    void trigAsyncAction(std::function<void()> act);

    /**
     * @brief move forward for some time with specific percentage
     * 
     * @param time_sec running duration
     * @param pct motors' percentage
     * 
     * @attention You should not use this function unless you know what you are doing!
    */
    qchasis& driveTimedRun(float time_sec, int pct);

    /**
     * @brief move the chasis to a coordinate
     * 
     * @param dx x-coordinate
     * @param dy y-coordinate
     * @param timeout the most time it will taken to move to the target
     * @param is_abs is the coordinate a absolute coordinate
    */
    qchasis& driveMoveTo(float dx, float dy, double timeout = 800,bool is_abs = true, bool rev_heading = false);

    /**
     * @brief drive forward for a distance
     * @note unit: cm
    */
    qchasis& driveForward(float distance, int timeout= 800);
    qchasis& driveBackward(float distance, int timeout = 800);
    /**
     * @brief turn to a ABSOLUTE angle
     * @note unit: degree
    */
    qchasis& driveTurn(float angle, int timeout = 800);

    /**
     * @brief turn to an angle relate to the current orientation
     * @note unit: degree
    */
    qchasis& driveDeltaTurn(float delta, int timeout = 800);

    /**
     * @brief reset the current coordinate
    */
    qchasis& resetPose(float x,float y);

    qchasis& driveCurve(const char* name,float delta, int timeout);

    /**
     * @brief turn to a coordinate
    */
    qchasis& driveTurnTo(float x, float y, int timeout = 800,bool rev = false);

    /**
     * @brief delay for some while
     * @note unit: ms
     *       We highly recommend that call this function between any two motions
     *       for example:
     *       driveMoveTo(...,...)
     *       driveDelay(20)
     *       driveMoveTo(...,...)
    */
    qchasis& driveDelay(int timeout);


    /**
     * @brief get the distance between current to the goal setted before
    */
    float getGoalDistance();

    /**
     * @brief move automatically until the distance between the current to the goal
     *        is within a range
     * 
     * @param distance the target distance, -1 means use `avail_goal_dist`
     * @param timeout the maximum time to adjust
     * @param err_range allowing error range(unit: cm)
    */
    qchasis& driveGoalDistance(float distance = -1, int timeout=800, float err_range = 5);

    ~qchasis(){};
};


#endif
