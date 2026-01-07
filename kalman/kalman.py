import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.spatial.transform import Rotation as R

class IMUDataProcessor:
    def __init__(self, dt):
        self.dt = dt  # 采样时间间隔
        
    def complementary_filter(self, ax, ay, az, gx, gy, gz, alpha=0.98):
        """
        互补滤波算法
        alpha: 滤波系数，控制加速度计和陀螺仪的权重
        """
        angles = np.zeros((len(ax), 3))  # [roll, pitch, yaw]
        
        # 初始化角度
        roll, pitch, yaw = 0, 0, 0
        
        for i in range(len(ax)):
            
            # 从加速度计计算俯仰和横滚角
            accel_pitch = np.arctan2(-ax[i], np.sqrt(ay[i]**2 + az[i]**2))
            accel_roll = np.arctan2(ay[i], az[i])
            
            # 陀螺仪积分
            gyro_pitch = pitch + gy[i] * self.dt
            gyro_roll = roll + gx[i] * self.dt
            gyro_yaw = yaw + gz[i] * self.dt
            
            # 互补滤波融合
            pitch = alpha * gyro_pitch + (1 - alpha) * accel_pitch
            roll = alpha * gyro_roll + (1 - alpha) * accel_roll
            yaw = gyro_yaw  # 没有磁力计，yaw会漂移
            
            angles[i] = [roll, pitch, yaw]
            
        return np.degrees(angles)  # 转换为角度
    
    def kalman_filter_imu(self, ax, ay, az, gx, gy, gz):
        """
        卡尔曼滤波算法处理IMU数据
        """
        n = len(ax)
        angles_kf = np.zeros((n, 3))
        
        # 状态向量: [roll, pitch, yaw, roll_bias, pitch_bias, yaw_bias]
        x = np.zeros(6)
        
        # 状态转移矩阵
        F = np.eye(6)
        F[0, 3] = -self.dt
        F[1, 4] = -self.dt
        F[2, 5] = -self.dt
        
        # 控制输入矩阵
        B = np.array([[self.dt, 0, 0],
                     [0, self.dt, 0],
                     [0, 0, self.dt],
                     [0, 0, 0],
                     [0, 0, 0],
                     [0, 0, 0]])
        
        # 观测矩阵 - 只观测角度
        H = np.array([[1, 0, 0, 0, 0, 0],
                     [0, 1, 0, 0, 0, 0],
                     [0, 0, 1, 0, 0, 0]])
        
        # 过程噪声协方差
        Q = np.eye(6) * 0.01
        
        # 观测噪声协方差
        R = np.eye(3) * 0.1
        
        # 误差协方差矩阵
        P = np.eye(6) * 0.1
        
        for i in range(n):
            # 预测步骤
            u = [gx[i], gy[i], gz[i]]  # 控制输入为陀螺仪数据
            x = F @ x + B @ u
            P = F @ P @ F.T + Q
            
            # 从加速度计计算观测值
            accel_pitch = np.arctan2(-ax[i], np.sqrt(ay[i]**2 + az[i]**2))
            accel_roll = np.arctan2(ay[i], az[i])
            
            # 使用上一时刻的yaw作为观测值（实际应用中需要磁力计）
            z = np.array([accel_roll, accel_pitch, x[2]])
            
            # 更新步骤
            y = z - H @ x  # 测量残差
            S = H @ P @ H.T + R
            K = P @ H.T @ np.linalg.inv(S)
            
            x = x + K @ y
            P = (np.eye(6) - K @ H) @ P
            
            angles_kf[i] = np.degrees(x[:3])
            
        return angles_kf

def generate_synthetic_imu_data(duration=10, dt=0.01):
    """生成模拟IMU数据用于测试"""
    t = np.arange(0, duration, dt)
    n = len(t)
    
    # 生成模拟角度轨迹
    roll_true = 10 * np.sin(2 * np.pi * 0.5 * t)
    pitch_true = 15 * np.sin(2 * np.pi * 0.3 * t)
    yaw_true = 5 * t
    
    # 生成陀螺仪数据（角速度）
    gyro_x = np.gradient(roll_true, t) * np.pi / 180  # 转换为弧度/秒
    gyro_y = np.gradient(pitch_true, t) * np.pi / 180
    gyro_z = np.gradient(yaw_true, t) * np.pi / 180
    
    # 添加噪声和偏置
    gyro_bias = np.array([0.01, -0.02, 0.005])
    gyro_data = np.column_stack([
        gyro_x + np.random.normal(0, 0.01, n) + gyro_bias[0],
        gyro_y + np.random.normal(0, 0.01, n) + gyro_bias[1],
        gyro_z + np.random.normal(0, 0.01, n) + gyro_bias[2]
    ])
    
    # 生成加速度计数据（假设只有重力）
    accel_data = []
    for i in range(n):
        rotation = R.from_euler('xyz', [roll_true[i], pitch_true[i], yaw_true[i]], degrees=True)
        # 重力向量在机体坐标系中的表示
        gravity_body = rotation.apply([0, 0, -9.81])
        accel_data.append(gravity_body + np.random.normal(0, 0.1, 3))
    
    accel_data = np.array(accel_data)
    
    # 创建DataFrame
    df = pd.DataFrame({
        'timestamp': t,
        'accel_x': accel_data[:, 0],
        'accel_y': accel_data[:, 1],
        'accel_z': accel_data[:, 2],
        'gyro_x': gyro_data[:, 0],
        'gyro_y': gyro_data[:, 1],
        'gyro_z': gyro_data[:, 2]
    })
    
    true_angles = np.column_stack([roll_true, pitch_true, yaw_true])
    
    return df, true_angles

#return@包含9个列表的列表
def read_csv_to_arrays(file_path):
    """
    读取CSV文件,并将每列数据保存为numpy数组

    :param file_path: CSV文件路径
    :return: 包含9个numpy数组的列表(每列一个数组)
    """
    try:
        # 读取CSV文件
        df = pd.read_csv(file_path)
        
        # 检查列数是否为9
        if len(df.columns) != 9:
            raise ValueError("CSV文件列数不为9，请检查文件格式。")
        
        # 将每列转换为numpy数组
        columns = [df[col].values for col in df.columns]
        
        return columns
    
    except FileNotFoundError:
        print(f"错误：文件 '{file_path}' 未找到。")
        return None
    except Exception as e:
        print(f"读取文件时发生错误：{e}")
        return None

def plot_results(t, true_angles, comp_angles, kalman_angles):
    """绘制结果对比图"""
    fig, axes = plt.subplots(3, 1, figsize=(12, 10))
    
    angle_names = ['Roll', 'Pitch', 'Yaw']
    
    for i in range(3):
        axes[i].plot(t, true_angles[:, i], 'k-', label='True', linewidth=2)
        axes[i].plot(t, comp_angles[:, i], 'r--', label='Complementary', linewidth=1.5)
        axes[i].plot(t, kalman_angles[:, i], 'b:', label='Kalman', linewidth=1.5)
        axes[i].set_ylabel(f'{angle_names[i]} (degrees)')
        axes[i].legend()
        axes[i].grid(True)
    
    axes[2].set_xlabel('Time (s)')
    axes[0].set_title('IMU Attitude Estimation Comparison')
    plt.tight_layout()
    plt.show()


# 计算频谱的函数
def compute_spectrum(data, fs=1.0):
    n = len(data)
    fft_result = np.fft.fft(data)
    amplitude_spectrum = np.abs(fft_result) / n
    freq_axis = np.fft.fftfreq(n, d=1/fs)
    half_n = n // 2
    return freq_axis[:half_n], amplitude_spectrum[:half_n] * 2

# 计算方差的函数
def compute_variance(data, ddof=0):
    return np.var(data, ddof=ddof)

# 分析指定段的函数
def analyze_segment(data, start, end, fs=1.0):
    segment = data[start:end]
    freq, amp = compute_spectrum(segment, fs=fs)
    variance = compute_variance(segment)
    return freq, amp, variance



def main():
    dt = 0.01
    # datas = read_csv_to_arrays("E:\Project\Robotics\RoboFly\kalman\DATA(2025.10.7-20.25.42).csv")
    datas = read_csv_to_arrays("E:\Project\Robotics\RoboFly\kalman\DATA(2025.10.7-20.12.17).csv")
    # datas = read_csv_to_arrays("E:\Project\Robotics\RoboFly\kalman\DATA(2025.10.7-19.53.52).csv")
    
    accx = datas[0]
    accy = datas[1]
    accz = datas[2]

    gyrx = datas[3]
    gyry = datas[4]
    gyrz = datas[5]

    roll = datas[6]
    pitch = datas[7]
    yaw = datas[8]

    '''
    原始数据展示
    '''
    plt.figure(1)
    # plt.figure(figsize=(10, 6))
    plt.plot(accx, label='accx', color='blue')
    plt.plot(accy, label='accy', color='red', linestyle='--')
    plt.plot(accz, label='accz', color='green', linestyle='-.')
    plt.title('Acc xyz')
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')
    plt.legend()
    plt.grid(True)

    plt.figure(2)
    # plt.figure(figsize=(10, 6))
    plt.plot(gyrx, label='gyrx', color='blue')
    plt.plot(gyry, label='gyry', color='red', linestyle='--')
    plt.plot(gyrz, label='gyrz', color='green', linestyle='-.')
    plt.title('Gyr xyz')
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')
    plt.legend()
    plt.grid(True)

    plt.figure(3)
    # plt.figure(figsize=(10, 6))
    plt.plot(roll, label='roll', color='blue')
    plt.plot(pitch, label='pitch', color='red', linestyle='--')
    plt.plot(yaw, label='yaw', color='green', linestyle='-.')
    plt.title('roll picth yaw')
    plt.xlabel('X-axis')
    plt.ylabel('Y-axis')
    plt.legend()
    plt.grid(True)

    '''
    频谱分析
    '''
    # start_idx, end_idx = 1, 4000
    # freq, amp, var = analyze_segment(accx, start_idx, end_idx, fs=100)

    # # 打印结果
    # print(f"段 {start_idx}~{end_idx} 的方差:", var)
    # print("频谱前5个频率点:", freq[:5])
    # print("频谱前5个幅值点:", amp[:5])
    # plt.figure(4)
    # # plt.figure(figsize=(10, 4))
    # plt.plot(freq, amp)
    # plt.title("Segment Spectrum accx")
    # plt.xlabel("Frequency (Hz)")
    # plt.ylabel("Amplitude")
    # plt.grid(True)

    # # 频谱分析
    # freq1, amp1, var1 = analyze_segment(accy, start_idx, end_idx, fs=100)

    # # 打印结果
    # print(f"段 {start_idx}~{end_idx} 的方差:", var1)
    # print("频谱前5个频率点:", freq1[:5])
    # print("频谱前5个幅值点:", amp1[:5])
    # plt.figure(5)
    # # plt.figure(figsize=(10, 4))
    # plt.plot(freq1, amp1)
    # plt.title("Segment Spectrum accy")
    # plt.xlabel("Frequency (Hz)")
    # plt.ylabel("Amplitude")
    # plt.grid(True)

    # freq2, amp2, var2 = analyze_segment(accz, start_idx, end_idx, fs=100)

    # # 打印结果
    # print(f"段 {start_idx}~{end_idx} 的方差:", var2)
    # print("频谱前5个频率点:", freq2[:5])
    # print("频谱前5个幅值点:", amp2[:5])
    # plt.figure(6)
    # # plt.figure(figsize=(10, 4))
    # plt.plot(freq2, amp2)
    # plt.title("Segment Spectrum accz")
    # plt.xlabel("Frequency (Hz)")
    # plt.ylabel("Amplitude")
    # plt.grid(True)
    # plt.show()

    #    # 初始化处理器
    # processor = IMUDataProcessor(dt)
    
    # # 使用互补滤波
    # print("应用互补滤波...")
    # comp_angles = processor.complementary_filter(accel_data, gyro_data, alpha=0.98)
    
    # # 使用卡尔曼滤波
    # print("应用卡尔曼滤波...")
    # kalman_angles = processor.kalman_filter_imu(accel_data, gyro_data)
    
    # # 绘制结果
    # t = df['timestamp'].values
    # plot_results(t, true_angles, comp_angles, kalman_angles)


       # 初始化处理器
    processor = IMUDataProcessor(dt)
    
    # 使用互补滤波
    print("应用互补滤波...")
    comp_angles = processor.complementary_filter(accx, accy, accz, gyrx, gyry, gyrz, alpha=0.98)
    
    # 使用卡尔曼滤波
    print("应用卡尔曼滤波...")
    kalman_angles = processor.kalman_filter_imu(accx, accy, accz, gyrx, gyry, gyrz)
    
    # 绘制结果
    t = np.arange(0, len(accx) * dt, dt)[:len(accx)]  # 创建时间序列
    # 将roll, pitch, yaw组合成正确的格式
    true_angles = np.column_stack([roll, pitch, yaw])
    plot_results(t, true_angles, comp_angles, kalman_angles)
    
    # # 保存结果
    # result_df = pd.DataFrame({
    #     'timestamp': t,
    #     'roll_true': true_angles[:, 0],
    #     'pitch_true': true_angles[:, 1],
    #     'yaw_true': true_angles[:, 2],
    #     'roll_comp': comp_angles[:, 0],
    #     'pitch_comp': comp_angles[:, 1],
    #     'yaw_comp': comp_angles[:, 2],
    #     'roll_kalman': kalman_angles[:, 0],
    #     'pitch_kalman': kalman_angles[:, 1],
    #     'yaw_kalman': kalman_angles[:, 2]
    # })
    
    # result_df.to_csv('imu_attitude_results.csv', index=False)
    # print("结果已保存到 imu_attitude_results.csv")

if __name__ == "__main__":
    main()
 
