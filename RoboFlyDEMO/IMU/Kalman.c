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

/* 全局，六轴与解算角度 */
extern float accb[3];
extern FLOAT_ANGLE Gyr_rad;
extern FLOAT_ANGLE Att_Angle;

/*
@功能：卡尔曼滤波(底层端)驱动初始化
@参数：【不用了解】
@返回值：【不用了解】
*/
void KalmanFilter_Init(KalmanFilter *kf_pitch, KalmanFilter *kf_roll) {
    
	kf_roll->Q_angle = 0.001f;
    kf_roll->Q_bias = 0.003f;
    kf_roll->R_measure = 0.03f;

    kf_roll->angle = 0.0f;
    kf_roll->bias = 0.0f; 

    kf_roll->P[0][0] = 0.0f;
    kf_roll->P[0][1] = 0.0f;
    kf_roll->P[1][0] = 0.0f;
    kf_roll->P[1][1] = 0.0f;
	
	//实测发现pitch的噪声很大
	kf_pitch->Q_angle = 0.001f;
    kf_pitch->Q_bias = 0.003f;
    kf_pitch->R_measure = 10.0f;

    kf_pitch->angle = 0.0f;
    kf_pitch->bias = 0.0f; 

    kf_pitch->P[0][0] = 0.0f;
    kf_pitch->P[0][1] = 0.0f;
    kf_pitch->P[1][0] = 0.0f;
    kf_pitch->P[1][1] = 0.0f;
}

/*
@功能：卡尔曼滤波(底层端)更新参数
@参数：【不用了解】
@返回值：【不用了解】
*/
float KalmanFilter_Update(KalmanFilter *kf, float newAngle, float newRate, float dt) 
{
    //更新角度和角速度预测值
    kf->rate = newRate - kf->bias;
    kf->angle += dt * kf->rate;

    //预测更新误差协方差矩阵
    kf->P[0][0] += dt * (dt*kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    //计算卡尔曼增益，S为测量预测方差？
    float S = kf->P[0][0] + kf->R_measure;
    float K[2];
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    //测量值与预测值之间的残差，newAngle为测量值，angle是预测值
    float y = newAngle - kf->angle;

    //更新模型预测用的角度估计值与参数
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;
    
    //更新误差协方差矩阵
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
	KalmanFilter_Init(&kf_pitch, &kf_roll);
}

/*
@功能：卡尔曼滤波(用户端)计算
@参数：无
@返回值：无
*/
void Kalman_Calculate()
{
    // Gyro_rad的单位为弧度/秒
    // Gyr_rad单位为弧度/秒
    // gyroXrate = Gyr_rad.X / 131.0;
	// gyroYrate = Gyr_rad.Y / 131.0;
    //KF更新
	Att_Angle.pit = KalmanFilter_Update(&kf_pitch, Att_Angle.pit, Gyr_rad.X, 0.01);
	Att_Angle.rol = KalmanFilter_Update(&kf_roll,  Att_Angle.rol, Gyr_rad.Y, 0.01);
}
