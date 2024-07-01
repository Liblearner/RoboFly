/************************************************************

名称：卡尔曼滤波算法底层驱动
作者：ChengHaoTong
时间：2024/2/2

*************************************************************/

#include "stm32f10x.h"           // Device header
#include "Delay.h"
#include "mpu6050.h"
#include <math.h>
#include "Kalman.h"
#include "structconfig.h"

/*定义卡尔曼滤波所需要的基本变量*/
KalmanFilter kf_pitch; 														// 翻滚角
KalmanFilter kf_roll;													    // 俯仰角
int16_t AccX, AccY, AccZ, GYROX, GYROY, GYROZ;
float gyroXrate,gyroYrate,accPitch,accRoll;
float pitch,roll;

/*
@功能：卡尔曼滤波(底层端)驱动初始化
@参数：【不用了解】
@返回值：【不用了解】
*/
void KalmanFilter_Init(KalmanFilter *kf) {
    kf->Q_angle = 0.001f;
    kf->Q_bias = 0.003f;
    kf->R_measure = 0.03f;

    kf->angle = 0.0f;
    kf->bias = 0.0f; 

    kf->P[0][0] = 0.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 0.0f;
}

/*
@功能：卡尔曼滤波(底层端)更新参数
@参数：【不用了解】
@返回值：【不用了解】
*/
float KalmanFilter_Update(KalmanFilter *kf, float newAngle, float newRate, float dt) 
{
    kf->rate = newRate - kf->bias;
    kf->angle += dt * kf->rate;

    kf->P[0][0] += dt * (dt*kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    float S = kf->P[0][0] + kf->R_measure;
    float K[2];
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    float y = newAngle - kf->angle;
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;

    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];

    kf->P[0][0] -= K[0] * P00_temp;
    kf->P[0][1] -= K[0] * P01_temp;
    kf->P[1][0] -= K[1] * P00_temp;
    kf->P[1][1] -= K[1] * P01_temp;

    return kf->angle;
}

/*
@功能：卡尔曼滤波(用户端)初始化
@参数：无
@返回值：无
*/
void Kalman_Init()
{
	KalmanFilter_Init(&kf_pitch);
    KalmanFilter_Init(&kf_roll);
}

/*
@功能：卡尔曼滤波(用户端)计算
@参数：无
@返回值：无
*/
void Kalman_Calculate()
{
    //获取原始数据
    MPU6050_AccRead(&AccX, &AccY, &AccZ);
    MPU6050_GyroRead(&GYROX, &GYROY, &GYROZ);

    gyroXrate = GYROX / 131.0;
		gyroYrate = GYROY / 131.0;

		accPitch = atan2f(AccY, AccZ) * 180 / 3.14159265358979323846;
		accRoll = atan2f(AccX, AccZ) * 180 / 3.14159265358979323846;
		pitch = KalmanFilter_Update(&kf_pitch, accPitch, gyroYrate, 0.01);
		roll = KalmanFilter_Update(&kf_roll, accRoll, gyroXrate, 0.01);


/*
		MPU6050_GetData(&AccX, &AccY, &AccZ, &GYROX, &GYROY, &GYROZ);

		gyroXrate = GYROX / 131.0;
		gyroYrate = GYROY / 131.0;

		accPitch = atan2f(AccY, AccZ) * 180 / 3.14159265358979323846;
		accRoll = atan2f(AccX, AccZ) * 180 / 3.14159265358979323846;
		pitch = KalmanFilter_Update(&kf_pitch, accPitch, gyroYrate, 0.01);
		roll = KalmanFilter_Update(&kf_roll, accRoll, gyroXrate, 0.01);
        */
}
