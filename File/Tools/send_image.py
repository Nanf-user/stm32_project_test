"""
send_image.py — 将图片转为 RGB565 并通过串口发送到 STM32，写入 W25Q64

使用方法:
    pip install Pillow pyserial
    python send_image.py your_image.jpg

可选参数:
    --port COM3          串口号（默认 COM3）
    --baud 115200        波特率（默认 115200）
    --width 240          目标宽度（默认 240）
    --height 320         目标高度（默认 320）
    --addr 0             W25Q64 写入地址（默认 0）
    --raw out.bin        只生成 bin 文件，不发送串口
    --flip               上下翻转图片
"""

import struct
import sys
import time
import argparse

try:
    from PIL import Image
except ImportError:
    print("请先安装 Pillow:  pip install Pillow")
    sys.exit(1)


def image_to_rgb565(img_path, width, height, flip=False):
    """将图片转换为 RGB565 小端序字节流"""
    img = Image.open(img_path)

    # 翻转
    if flip:
        img = img.transpose(Image.FLIP_TOP_BOTTOM)

    # 缩放到目标尺寸
    img = img.resize((width, height), Image.LANCZOS)

    # 转为 RGB
    img = img.convert("RGB")

    # 逐像素转 RGB565（小端序：低字节在前）
    raw = bytearray()
    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            # RGB888 -> RGB565
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            # 小端序：低字节在前
            raw.append(rgb565 & 0xFF)         # low byte
            raw.append((rgb565 >> 8) & 0xFF)  # high byte

    return bytes(raw)


def build_packet(data, flash_addr=0):
    """
    构建协议包:
    Byte 0:     0xAA          (magic)
    Byte 1-4:   uint32 addr   (big-endian)
    Byte 5-7:   uint32 length (24-bit big-endian)
    Byte 8+:    raw data
    """
    length = len(data)
    if length > 0xFFFFFF:
        print(f"错误: 数据长度 {length} 超过 16MB 限制!")
        sys.exit(1)

    header = bytearray()
    header.append(0xAA)                                          # magic
    header += struct.pack(">I", flash_addr)                      # 4 bytes, big-endian addr
    header += struct.pack(">I", length)[1:4]                     # 3 bytes, big-endian length

    return bytes(header) + data


def send_via_serial(packet, port, baud):
    """通过串口直接发送数据（无握手机制，STM32 边收边擦边写）"""
    try:
        import serial
    except ImportError:
        print("请先安装 pyserial:  pip install pyserial")
        sys.exit(1)

    ser = serial.Serial(port, baud, timeout=1)
    total = len(packet)
    header_size = 8
    data_size = total - header_size
    chunk_size = 512

    print(f"连接 {port} @ {baud} bps")
    print(f"发送 {total} 字节 (头 8 字节 + 数据 {data_size} 字节)")
    print("-" * 40)

    # 直接发送协议头
    print("发送协议头...")
    ser.write(packet[:header_size])
    ser.flush()

    # 短暂等待 STM32 解析头部
    time.sleep(0.1)

    print("发送图片数据...")
    print("-" * 40)

    # 直接发送图片数据
    sent = 0
    start_time = time.time()

    while sent < data_size:
        end = min(sent + chunk_size, data_size)
        ser.write(packet[header_size + sent:header_size + end])
        ser.flush()
        sent = end

        # 进度条
        progress = sent / data_size * 100
        elapsed = time.time() - start_time
        speed = sent / elapsed / 1024 if elapsed > 0 else 0
        eta = (data_size - sent) / (sent / elapsed) if sent > 0 and elapsed > 0 else 0

        bar_len = 30
        filled = int(bar_len * sent / data_size)
        bar = "=" * filled + ">" + " " * (bar_len - filled - 1)

        print(f"\r  [{bar}] {progress:5.1f}%  {sent/1024:.1f}/{data_size/1024:.1f} KB  "
              f"{speed:.1f} KB/s  剩余 {eta:.0f}s", end="", flush=True)

    # 发送完成后，继续监听 STM32 返回的诊断信息（最多 10 秒）
    print(f"\n{'='*40}")
    elapsed = time.time() - start_time
    print(f"发送完成! 共 {data_size} 字节, 耗时 {elapsed:.1f} 秒")
    print(f"\n监听 STM32 返回信息 (10 秒)...")
    print("-" * 40)

    listen_start = time.time()
    stm32_msg = b""
    while time.time() - listen_start < 10:
        if ser.in_waiting > 0:
            byte = ser.read(1)
            stm32_msg += byte
            print(byte.decode('utf-8', errors='ignore'), end='', flush=True)
        else:
            time.sleep(0.01)

    if stm32_msg:
        print("\n" + "-" * 40)

    ser.close()


def main():
    parser = argparse.ArgumentParser(description="将图片转为 RGB565 发送到 STM32 W25Q64")
    parser.add_argument("image", help="image.bmp")
    parser.add_argument("--port", default="COM8", help="串口号 (默认 COM3)")
    parser.add_argument("--baud", type=int, default=115200, help="波特率 (默认 115200)")
    parser.add_argument("--width", type=int, default=240, help="目标宽度 (默认 240)")
    parser.add_argument("--height", type=int, default=320, help="目标高度 (默认 320)")
    parser.add_argument("--addr", type=int, default=0, help="W25Q64 地址 (默认 0)")
    parser.add_argument("--raw", default="", help="只导出 bin 文件，不发送串口")
    parser.add_argument("--flip", action="store_true", help="上下翻转图片")
    args = parser.parse_args()

    print(f"读取图片: {args.image}")
    data = image_to_rgb565(args.image, args.width, args.height, args.flip)
    print(f"转换完成: {args.width}x{args.height} = {len(data)} 字节 (RGB565)")
    print(f"前16字节: {' '.join(f'{b:02X}' for b in data[:16])}")

    if args.raw:
        # 只导出 bin 文件
        with open(args.raw, "wb") as f:
            f.write(data)
        print(f"已导出到: {args.raw}")
        return

    packet = build_packet(data, args.addr)
    print(f"协议包: {len(packet)} 字节 (8 字节头 + {len(data)} 字节数据)")
    print(f"头信息: addr=0x{args.addr:08X}, length={len(data)}")
    print()

    send_via_serial(packet, args.port, args.baud)


if __name__ == "__main__":
    main()
