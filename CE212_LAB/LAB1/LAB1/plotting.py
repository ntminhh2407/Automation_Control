import os
import signal
import time

import matplotlib.pyplot as plt
import serial

CONG_COM = "COM3"
TOC_DO = 115200
TEN_FILE = "data.csv"
SO_MAU_HIEN_THI = 1000
CHU_KY_VE = 0.05

DUONG_DAN_CSV = os.path.join(os.path.dirname(os.path.abspath(__file__)), TEN_FILE)

dung = False

def khi_nhan_ctrl_c(signum, frame):
    global dung
    dung = True

signal.signal(signal.SIGINT, khi_nhan_ctrl_c)

thoi_gian = []
gia_tri = []

ser = serial.Serial(CONG_COM, TOC_DO, timeout=0.05)
time.sleep(2)
ser.reset_input_buffer()

file_csv = open(DUONG_DAN_CSV, "w")
file_csv.write("thoi_gian_s,adc\n")
file_csv.flush()
print(f"Dữ liệu sẽ được lưu vào: {DUONG_DAN_CSV}")

plt.ion()
fig, ax = plt.subplots()
duong, = ax.plot([], [])
ax.set_xlabel("Thời gian (giây)")
ax.set_ylabel("Giá trị ADC (0 - 4095)")
ax.set_title("Dữ liệu cảm biến ESP32 (Ctrl+C để dừng)")
ax.set_xlim(0, 10)
ax.set_ylim(0, 4095)
ax.grid(True)
plt.pause(0.1)

print("Đang đọc dữ liệu... Nhấn Ctrl+C để dừng.")

bat_dau = time.time()
lan_ve_cuoi = 0
da_canh_bao = False

try:
    while not dung:
        while ser.in_waiting:
            dong = ser.readline().decode(errors="ignore").strip()

            if dong == "" or not dong[0].isdigit():
                continue

            phan = dong.split(",")
            if len(phan) != 2:
                continue

            try:
                t = int(phan[0]) / 1000
                v = int(phan[1])
            except ValueError:
                continue

            thoi_gian.append(t)
            gia_tri.append(v)
            file_csv.write(f"{t},{v}\n")   

        bay_gio = time.time()
        if bay_gio - lan_ve_cuoi >= CHU_KY_VE:
            lan_ve_cuoi = bay_gio
            file_csv.flush()

            if len(gia_tri) > 0:
                t_hien_thi = thoi_gian[-SO_MAU_HIEN_THI:]
                v_hien_thi = gia_tri[-SO_MAU_HIEN_THI:]
                duong.set_data(t_hien_thi, v_hien_thi)
                ax.set_xlim(t_hien_thi[0], max(t_hien_thi[-1], t_hien_thi[0] + 10))

            plt.pause(0.001)

            if not plt.fignum_exists(fig.number):
                break

            if len(gia_tri) == 0 and not da_canh_bao and bay_gio - bat_dau > 3:
                print("Chưa nhận được dữ liệu! Kiểm tra: đúng cổng COM, đã nạp code, baud = 115200.")
                da_canh_bao = True

finally:
    file_csv.close()
    ser.close()
    plt.close("all")
    print(f"Đã dừng. Lưu {len(gia_tri)} mẫu vào: {DUONG_DAN_CSV}")