/************************************************************

名称：卡尔曼滤波算法底层驱动
作者：ChengHaoTong
时间：2024/2/2

*************************************************************/

#ifndef KALMAN_H
#define KALMAN_H
#include <math.h>
#include "structconfig.h"

/*
以下两个函数是用户常用函数
void Kalman_Init(void);
void Kalman_Calculate(void);

用法:
【1】在主函数里,主循环之前初始化Kalman_Init();
(注意:必须先调用MPU6050_Init()才可以初始化Kalman.因为Kalman_Calculate()底层程序中使用了MPU6050_GetData();)
【2】在while(1)主循环里不断调用Kalman_Calculate()得到卡尔曼解算值
【3】在主函数里可直接使用pitch roll变量(这里已经声明为外部变量)
*/

void Kalman_Init(void);
void Kalman_Calculate(void);
extern float pitch,roll;

/*
以下是卡尔曼滤波算法的底层驱动程序
*/
typedef struct {
    float Q_angle;
    float Q_bias; 
    float R_measure;
    float angle;
    float bias; 
    float rate; 

    float P[2][2];
} KalmanFilter;

extern float gyroXrate,gyroYrate,accPitch,accRoll;

void Kalman_Init(void);
void Kalman_Calculate(void);
void KalmanFilter_Init(KalmanFilter *kf_pitch, KalmanFilter *kf_roll);
float KalmanFilter_Update(KalmanFilter *kf, float newAngle, float newRate, float dt);

#endif 
