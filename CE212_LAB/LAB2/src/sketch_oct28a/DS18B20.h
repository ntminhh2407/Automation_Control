#include <Arduino.h>

#define DS_DEFAULT_PIN          4       // GPIO nối chân DQ
#define DS_DEFAULT_RESOLUTION   12      // 9..12 bit, 12 bit = 0.0625 °C, đổi mất ~750 ms
#define DS_MAX_DEVICES          4       // Số DS18B20 tối đa quản lý trên 1 bus
#define DS_CONVERSION_TIMEOUT_MS 1000   // Thời gian chờ tối đa khi đọc kiểu blocking

// Low level //
// (không có, thư viện OneWire + DallasTemperature đã lo giao thức 1-Wire)

// High level //
// Khởi tạo bus, dò tìm cảm biến, đặt độ phân giải.
// Trả về true nếu tìm thấy ít nhất 1 cảm biến.
bool dsInit(uint8_t pin = DS_DEFAULT_PIN, uint8_t resolution = DS_DEFAULT_RESOLUTION);

uint8_t dsGetDeviceCount();                          // Số cảm biến tìm thấy
bool dsGetAddress(uint8_t index, uint8_t addr[8]);   // Mã ROM 64-bit của cảm biến thứ index

// Đọc kiểu không chặn (non-blocking): phù hợp khi loop() còn việc khác
bool dsStartConversion();                            // Ra lệnh đo cho tất cả cảm biến trên bus
bool dsIsConversionDone();                           // true khi đo xong
bool dsReadTemp(float &temp, uint8_t index = 0);     // °C, gọi sau khi đo xong

// Đọc kiểu chặn (blocking): đo rồi chờ xong mới trả về (~750 ms ở 12 bit)
bool dsReadTempBlocking(float &temp, uint8_t index = 0);