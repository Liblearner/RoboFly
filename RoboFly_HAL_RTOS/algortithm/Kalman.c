/************************************************************

���ƣ��������˲��㷨�ײ�����
���ߣ�ChengHaoTong
ʱ�䣺2024/2/2

*************************************************************/

#include "stm32f1xx_hal.h"           // Device header
//#include "Delay.h"
#include "mpu6050.h"
#include <math.h>
#include "Kalman.h"
#include "structconfig.h"

/*���忨�����˲�����Ҫ�Ļ�������*/
KalmanFilter kf_pitch; 														// ������
KalmanFilter kf_roll;													    // ������
int16_t AccX, AccY, AccZ, GYROX, GYROY, GYROZ;
float gyroXrate,gyroYrate,accPitch,accRoll;

/* ȫ�֣����������Ƕ� */
extern float accb[3];
extern FLOAT_XYZ Gyr_rad;
extern FLOAT_ANGLE Att_Angle;

/*
@���ܣ��������˲�(�ײ��)������ʼ��
@�������������˽⡿
@����ֵ���������˽⡿
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
	
	//ʵ�ⷢ��pitch�������ܴ�
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
@���ܣ��������˲�(�ײ��)���²���
@�������������˽⡿
@����ֵ���������˽⡿
*/
float KalmanFilter_Update(KalmanFilter *kf, float newAngle, float newRate, float dt) 
{
    //���½ǶȺͽ��ٶ�Ԥ��ֵ
    kf->rate = newRate - kf->bias;
    kf->angle += dt * kf->rate;

    //Ԥ��������Э�������
    kf->P[0][0] += dt * (dt*kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    //���㿨�������棬SΪ����Ԥ�ⷽ�
    float S = kf->P[0][0] + kf->R_measure;
    float K[2];
    K[0] = kf->P[0][0] / S;
    K[1] = kf->P[1][0] / S;

    //����ֵ��Ԥ��ֵ֮��ĲвnewAngleΪ����ֵ��angle��Ԥ��ֵ
    float y = newAngle - kf->angle;

    //����ģ��Ԥ���õĽǶȹ���ֵ�����
    kf->angle += K[0] * y;
    kf->bias += K[1] * y;
    
    //�������Э�������
    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];

    kf->P[0][0] -= K[0] * P00_temp;
    kf->P[0][1] -= K[0] * P01_temp;
    kf->P[1][0] -= K[1] * P00_temp;
    kf->P[1][1] -= K[1] * P01_temp;

    return kf->angle;
}

/*
@���ܣ��������˲�(�û���)��ʼ��
@��������
@����ֵ����
*/
void Kalman_Init()
{
	KalmanFilter_Init(&kf_pitch, &kf_roll);
}

/*
@���ܣ��������˲�(�û���)����
@��������
@����ֵ����
*/
void Kalman_Calculate()
{
    // Gyro_rad�ĵ�λΪ����/��
    // Gyr_rad��λΪ����/��
    // gyroXrate = Gyr_rad.X / 131.0;
	// gyroYrate = Gyr_rad.Y / 131.0;
    //KF����
	Att_Angle.pit = KalmanFilter_Update(&kf_pitch, Att_Angle.pit, Gyr_rad.X, 0.01);
	Att_Angle.rol = KalmanFilter_Update(&kf_roll,  Att_Angle.rol, Gyr_rad.Y, 0.01);
}
