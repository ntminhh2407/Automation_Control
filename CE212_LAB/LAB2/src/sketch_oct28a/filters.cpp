#include "filters.h"
#include <math.h>

// ===================== Moving Average ===================== //
void maReset(MovingAverage &f)
{
    for (uint8_t i = 0; i < MA_MAX_WINDOW; i++) f.buf[i] = 0.0f;
    f.index = 0;
    f.count = 0;
    f.sum   = 0.0f;
}

void maInit(MovingAverage &f, uint8_t window)
{
    if (window < 1) window = 1;
    if (window > MA_MAX_WINDOW) window = MA_MAX_WINDOW;
    f.window = window;
    maReset(f);
}

float maUpdate(MovingAverage &f, float x)
{
    if (f.count == f.window) {
        f.sum -= f.buf[f.index];      // bỏ mẫu cũ nhất
    } else {
        f.count++;                    // giai đoạn lấp đầy cửa sổ
    }
    f.buf[f.index] = x;
    f.sum += x;
    f.index = (f.index + 1) % f.window;
    return f.sum / f.count;           // chia count nên không bị lệch lúc đầu
}

// ===================== Complementary ===================== //
void cfReset(ComplementaryFilter &f)
{
    f.angle = 0.0f;
    f.initialized = false;
}

void cfInit(ComplementaryFilter &f, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    f.alpha = alpha;
    cfReset(f);
}

float cfUpdate(ComplementaryFilter &f, float gyroRate, float accelAngle, float dt)
{
    if (!f.initialized) {
        f.angle = accelAngle;         // khởi tạo bằng góc accel
        f.initialized = true;
        return f.angle;
    }
    f.angle = f.alpha * (f.angle + gyroRate * dt) + (1.0f - f.alpha) * accelAngle;
    return f.angle;
}

// ===================== Góc từ accel ===================== //
float accelRollDeg(float ay, float az)
{
    return atan2f(ay, az) * RAD_TO_DEG;
}

float accelPitchDeg(float ax, float ay, float az)
{
    return atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
}

// ===================== Attitude ===================== //
void attitudeInit(Attitude &a, uint8_t maWindow, float alpha)
{
    maInit(a.maAx, maWindow);
    maInit(a.maAy, maWindow);
    maInit(a.maAz, maWindow);
    cfInit(a.roll,  alpha);
    cfInit(a.pitch, alpha);
    a.lastMicros = 0;
    a.started = false;
}

void attitudeUpdate(Attitude &a, float ax, float ay, float az, float gx, float gy)
{
    uint32_t now = micros();
    if (!a.started) {                 // lần đầu chưa có dt
        a.lastMicros = now;
        a.started = true;
    }
    float dt = (now - a.lastMicros) * 1e-6f;
    a.lastMicros = now;
    if (dt <= 0.0f || dt > 0.5f) dt = 0.01f;   // chặn dt bất thường

    // 1) Lọc nhiễu accel bằng Moving Average
    float fx = maUpdate(a.maAx, ax);
    float fy = maUpdate(a.maAy, ay);
    float fz = maUpdate(a.maAz, az);

    // 2) Góc từ accel đã lọc
    float rollAcc  = accelRollDeg(fy, fz);
    float pitchAcc = accelPitchDeg(fx, fy, fz);

    // 3) Complementary: gyro (tích phân) + accel (chống trôi)
    cfUpdate(a.roll,  gx, rollAcc,  dt);
    cfUpdate(a.pitch, gy, pitchAcc, dt);
}