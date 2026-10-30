#pragma once
#include <Arduino.h>

// ===================== Moving Average ===================== //
#define MA_MAX_WINDOW 32          // kích thước cửa sổ tối đa

struct MovingAverage {
    float   buf[MA_MAX_WINDOW];   // bộ đệm vòng
    uint8_t window;               // cửa sổ đang dùng (1..MA_MAX_WINDOW)
    uint8_t index;                // vị trí ghi tiếp theo
    uint8_t count;                // số mẫu đã có (tối đa = window)
    float   sum;                  // tổng các mẫu trong cửa sổ
};

void  maInit(MovingAverage &f, uint8_t window);
void  maReset(MovingAverage &f);
float maUpdate(MovingAverage &f, float x);   // nhập mẫu mới, trả về trung bình

// ===================== Complementary ===================== //
struct ComplementaryFilter {
    float alpha;        // 0..1, càng gần 1 càng tin gyro (thường 0.96 ~ 0.99)
    float angle;        // góc ước lượng (độ)
    bool  initialized;  // mẫu đầu tiên lấy góc từ accel
};

void  cfInit(ComplementaryFilter &f, float alpha);
void  cfReset(ComplementaryFilter &f);
// gyroRate: °/s, accelAngle: độ, dt: giây
float cfUpdate(ComplementaryFilter &f, float gyroRate, float accelAngle, float dt);

// ===================== Góc từ accel ===================== //
float accelRollDeg(float ay, float az);
float accelPitchDeg(float ax, float ay, float az);

// ===================== Gộp thành bộ ước lượng tư thế ===================== //
struct Attitude {
    MovingAverage       maAx, maAy, maAz;   // lọc nhiễu accel
    ComplementaryFilter roll, pitch;
    uint32_t            lastMicros;
    bool                started;
};

void attitudeInit(Attitude &a, uint8_t maWindow = 5, float alpha = 0.98f);
// ax,ay,az (g), gx,gy (°/s). dt tự tính bằng micros().
void attitudeUpdate(Attitude &a, float ax, float ay, float az, float gx, float gy);