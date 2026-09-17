# Waveshare ESP32-S3 Touch LCD 7 — Control Deck Project

## Hardware

- **Board:** Waveshare ESP32-S3-Touch-LCD-7
- **Display:** 7" IPS
- **Resolution:** 800 × 480
- **Touch:** Capacitive Touch, รองรับ Multi-touch
- **MCU:** ESP32-S3
- **Flash:** 16 MB
- **PSRAM:** 8 MB
- **Current COM Port:** `COM13`

Documentation:
https://docs.waveshare.com/ESP32-S3-Touch-LCD-7

---

## Project Goal

ทำอุปกรณ์ **Touch Control Deck / Macro Keyboard** สำหรับควบคุมโปรแกรม เช่น

- Fusion 360
- OrcaSlicer
- Bambu Studio
- โปรแกรมอื่น ๆ ในอนาคต

โครงสร้างระบบ:

```text
Touch LCD
   ↓
LVGL UI
   ↓
Macro / Profile Manager
   ↓
USB HID Keyboard
   ↓
Windows PC
   ↓
Fusion 360 / OrcaSlicer / Bambu Studio
```

---

## USB ที่ต้องใช้

บอร์ดนี้มี USB มากกว่าหนึ่งส่วน

### USB-UART

ใช้สำหรับ:

- Flash firmware
- Serial Monitor
- Debug

ตอนนี้ Windows มองเห็นเป็น:

```text
COM13
```

### Native USB ของ ESP32-S3

ใช้สำหรับทำ:

- USB HID Keyboard
- Macro Keyboard
- Control Deck

เป้าหมายคือให้ Windows มองอุปกรณ์เป็น **Keyboard HID** ไม่ใช่แค่ COM Port

---

## UI Concept

ใช้จอแบบ Landscape:

```text
800 × 480
```

ตัวอย่าง Profile:

### Fusion 360

- Sketch
- Line
- Rectangle
- Circle
- Dimension
- Trim
- Extrude
- Hole
- Fillet
- Chamfer
- Pattern
- Undo
- Redo
- Save

### OrcaSlicer

- Move
- Rotate
- Scale
- Lay Flat
- Cut
- Support Paint
- Auto Orient
- Slice
- Preview
- Send Print
- Undo
- Redo

สามารถสลับ Profile จากหน้า UI ได้

```text
Fusion 360 | OrcaSlicer | Bambu Studio | System
```

---

## Development Plan

ไม่ควรเริ่มจาก UI เต็มทันที

ให้ทดสอบเป็นขั้นตอน:

### Test 1 — Display

Flash โปรแกรมทดสอบและตรวจว่า:

- จอแสดงผลได้
- Resolution ถูกต้อง
- สีและ orientation ถูกต้อง

### Test 2 — Touch

ตรวจว่า:

- Touch Controller ทำงาน
- ตำแหน่ง X/Y ถูกต้อง
- กดปุ่ม LVGL ได้

### Test 3 — Basic UI

สร้างปุ่มบนจอ 1 ปุ่ม เช่น:

```text
UNDO
```

ตรวจว่า Event จาก Touch เข้า firmware ได้

### Test 4 — USB HID

เมื่อกดปุ่ม `UNDO`

ESP32-S3 ส่ง:

```text
Ctrl + Z
```

ไปยัง Windows

ถ้าผ่านขั้นนี้ แสดงว่า Macro Deck หลักทำงานแล้ว

### Test 5 — Application Profiles

เพิ่ม Profile:

1. Fusion 360
2. OrcaSlicer
3. Bambu Studio

พร้อม Mapping Shortcut ของแต่ละโปรแกรม

### Test 6 — Full Control Deck UI

เพิ่ม:

- Icon
- Page switching
- Context menu
- Macro หลายคำสั่ง
- Settings
- Profile editor
- Rotary Encoder (ถ้าต้องการ)

---

## Recommended Software Stack

### UI

```text
LVGL
```

### Firmware

เลือกได้ระหว่าง:

```text
Arduino ESP32
```

หรือ

```text
ESP-IDF
```

สำหรับการเริ่มต้น แนะนำ Arduino ก่อน เพราะทดสอบ Hardware และ USB HID ได้เร็วกว่า

---

## First Milestone

เป้าหมายแรก:

```text
Touch Button
     ↓
ESP32-S3
     ↓
USB HID
     ↓
Ctrl + Z
     ↓
Windows
```

ถ้าขั้นนี้ทำงานได้ Project Control Deck ถือว่าผ่าน Proof of Concept แล้ว
