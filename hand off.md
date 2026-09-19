# MacroDesk handoff

อัปเดตล่าสุด: 2026-09-18

## ภาพรวม

โปรเจกต์ Touch Control Deck สำหรับ `Waveshare ESP32-S3-Touch-LCD-7` ความละเอียด 800 × 480 ใช้ LVGL 8, Arduino ESP32 และ Native USB HID Keyboard สำหรับควบคุม Fusion 360 และ OrcaSlicer

Repository: <https://github.com/XCRYZER01/MacroDesk>

โฟลเดอร์เฟิร์มแวร์หลัก: `firmware/MacroDeckUI`

## UI ล่าสุด

![OrcaSlicer control deck](firmware/MacroDeckUI/assets/ui_orca_800x480.png)

- หน้า OrcaSlicer ใช้ grid 6 × 4 = 24 ช่อง ใส่ปุ่ม 23 ปุ่ม เหลือว่าง 1 ช่องล่างขวา
- กล่อง ACTIVE PRINTER เดิม (ซึ่งกดแล้วไม่ทำอะไร) ถูกวาดทับเป็นปุ่ม Slice Plate สีเขียวขนาดใหญ่ และช่องเดิมของ Slice ในตารางกลายเป็น Clone (Ctrl+K)
- กล่อง VIEW กับ DISPLAY ยุบรวมเป็นกล่องเดียว จัดเป็น 3 × 3 ใส่ปุ่มมุมมอง 8 ปุ่ม (Default, Top, Bottom, Front, Behind, Left, Right, Preview) hotspot วางตรงกับช่องที่วาดพอดี
- หน้า Fusion 360 ยังเป็น grid 5 × 4 แบบเดิม
- Header, sidebar, right panel และ bottom app bar อยู่พิกัดเดียวกันทั้งสองหน้า
- ลบวันที่และเวลาออกจากหัวจอทั้งสองหน้าแล้ว (เดิมเป็นตัวหนังสือในภาพ ไม่เคยเดิน) และลบไอคอน Wi-Fi ด้วย (บอร์ดไม่ได้ต่อ Wi-Fi จริง) เหลือแค่คำขวัญ ถ้าจะทำนาฬิกาจริง ต้องมีแหล่งเวลา: Wi-Fi + NTP (ไม่ต้องลงอะไรที่ PC) หรือ USB CDC + สคริปต์ฝั่ง PC
- แถบล่างเหลือ Fusion 360, Orca Slicer และ System (ลบ Bambu Studio ออกแล้ว System อยู่ x 337–454)
- แตะ `Orca Slicer` หรือ `Fusion 360` ที่แถบล่างเพื่อเปลี่ยนโปรไฟล์
- แตะปุ่ม `Move` บนหน้า Orca จะส่ง `M` แล้วเด้ง jog pad ขึ้นทับหน้าจอ (วาดด้วย LVGL ไม่ใช้ภาพ จึงไม่กินพื้นที่แฟลช) ปิดด้วยปุ่ม X มุมขวาบน หรือสลับโปรไฟล์

พิกัดสำหรับวาง hotspot:

| ส่วน | พิกัด |
|---|---|
| Grid หน้า Orca (6 × 4) | x = 134 + col × 80, y = 88 + row × 85, ขนาด 76 × 82 |
| Grid หน้า Fusion (5 × 4) | x = 133 + col × 97, y = 88 + row × 85, ขนาด 93 × 82 |
| Sidebar | x 4, y = 84 + i × 42, ขนาด 123 × 41 |
| Bottom bar | Fusion x 78, Orca x 208, System x 337 (กว้าง 118–128), y 432, สูง 40 |
| ปุ่ม Slice Plate (หน้า Orca) | x 620, y 88, ขนาด 168 × 63 |
| กล่อง VIEW รวม (3 × 3, หน้า Orca) | x = 626 + col × 54, y = 186 + row × 70, ขนาด 52 × 64 |

ไฟล์ภาพและข้อมูล:

- `firmware/MacroDeckUI/assets/ui_orca_800x480.png`
- `firmware/MacroDeckUI/ui_orca_rgb565.c`
- `firmware/MacroDeckUI/assets/ui_reference_800x480.png`
- `firmware/MacroDeckUI/ui_reference_rgb565.c`

รูปแบบข้อมูลภาพ:

- เก็บเป็น RGB565 little-endian ตรงกับ `LV_COLOR_DEPTH 16` และ `LV_COLOR_16_SWAP 0` แล้ว `memcpy` ลง canvas ใน PSRAM ตอนบูต
- ไฟล์ `.c` เป็น `const uint16_t[]` หนึ่งบรรทัดต่อหนึ่งแถวพิกเซล (800 ค่าต่อบรรทัด) ภาพละ 768,000 ไบต์
- ถ้าแก้ PNG ต้อง regenerate ไฟล์ `.c` ใหม่ทุกครั้ง
- เดิมเก็บเป็น RGB888 (1,152,000 ไบต์ต่อภาพ) เปลี่ยนมาเป็น RGB565 เพื่อประหยัดพื้นที่ 768 KB ภาพบนจอไม่เปลี่ยน เพราะจอกับ LVGL เป็น 16 บิตอยู่แล้ว

## OrcaSlicer HID mapping ที่ตรวจแล้ว

อ้างอิงเอกสาร Keyboard Shortcuts ทางการ และ source code (`KBShortcutsDialog.cpp`, `GLCanvas3D.cpp`, `Gizmos/GLGizmo*.cpp`):
<https://github.com/OrcaSlicer/OrcaSlicer/wiki/keyboard-shortcuts/f70839b8ad12f7e404ac1ced8f5dfb7d4e3686e7>

Grid 6 × 4 เรียงตามลำดับการอ่าน:

| แถว | ปุ่ม | HID ที่ส่ง |
|---|---|---|
| 1 | New | Ctrl+N |
| 1 | Open | Ctrl+O |
| 1 | Save | Ctrl+S |
| 1 | Import | Ctrl+I |
| 1 | Arrange | A |
| 1 | Orient | Q |
| 2 | Instance + | + |
| 2 | Instance − | − |
| 2 | Move | M |
| 2 | Rotate | R |
| 2 | Scale | S |
| 2 | Lay Flat | F |
| 3 | Cut | C |
| 3 | Support (paint) | L |
| 3 | Seam (paint) | P |
| 3 | Fuzzy Skin | H |
| 3 | Color (paint) | N |
| 3 | Add Text | T |
| 4 | Measure | U |
| 4 | Undo | Ctrl+Z |
| 4 | Redo | Ctrl+Y |
| 4 | Delete | Delete |
| 4 | Clone | Ctrl+K |

Right panel:

| ปุ่ม | HID ที่ส่ง |
|---|---|
| Slice Plate (แทนกล่อง ACTIVE PRINTER เดิม) | Ctrl+R |
| Default | Ctrl+0 |
| Top | Ctrl+1 |
| Bottom | Ctrl+2 |
| Front | Ctrl+3 |
| Behind | Ctrl+4 |
| Left | Ctrl+5 |
| Right | Ctrl+6 |
| Preview | Tab |

ปุ่มที่เอาออกจากหน้า Orca: Preferences (Ctrl+P), Mesh Boolean (B), Print Plate (Ctrl+Shift+G)

ฟังก์ชันที่ OrcaSlicer ไม่มี shortcut จึงส่งจากบอร์ดไม่ได้ และตัดสินใจไม่ทำ: Add Plate, Split to Objects, Split to Parts, Variable Layer Height

## Fusion 360 HID mapping

แหล่งอ้างอิง: Product Design Online (Fusion hotkeys), CAD Forum (Fusion shortcuts) และหน้า Autodesk shortcuts

ตาราง 5 × 4 หน้าหลัก:

| ปุ่ม | HID ที่ส่ง |
|---|---|
| New Design / Open / Save | Ctrl+N / Ctrl+O / Ctrl+S |
| Undo / Redo | Ctrl+Z / Ctrl+Y |
| Line / Rectangle / Circle / Dimension | L / R / C / D |
| Extrude / Fillet / Move / Hole | E / F / M / H |
| Arc, Revolve, Chamfer, Shell, Combine, Pattern, Mirror | S search (ป้ายในภาพแก้เป็น `S search` แล้ว) |

S search = กด `S` เปิดช่องค้นหาคำสั่ง รอ 400 ms พิมพ์ชื่อคำสั่ง รอ 300 ms แล้วกด Enter ใช้ได้กับ Fusion ภาษาอังกฤษเท่านั้น ชื่อที่ใช้ค้นอยู่ใน `fusion_search_term()` ระหว่างส่ง UI ของบอร์ดจะค้างประมาณ 0.7 วินาที

แถบขวา:

| ปุ่ม | HID ที่ส่ง |
|---|---|
| ACTIVE WORKSPACE (กล่องทั้งกล่อง) | Ctrl+] สลับ workspace ถัดไป |
| Fit | F6 (ผู้ใช้ยืนยันบน Fusion แล้ว ป้ายในภาพแก้เป็น F6) |
| Home | ยังไม่ส่ง ยังไม่พบปุ่มเริ่มต้นของ Fusion (ป้ายในภาพเขียน Ctrl + Home ซึ่งยังไม่ยืนยัน) |
| Visibility (แทน Zoom) | V |
| Full Screen (แทน Pan) | Ctrl+Shift+F |
| 4 Views (แทน Orbit) | Shift+1 |
| Shaded / Hidden / Wireframe | Ctrl+4 / Ctrl+5 / Ctrl+7 (ป้ายในภาพแก้ให้ตรงแล้ว) |

Sidebar หน้าย่อย (วาดด้วย LVGL แบบเดียวกับ Orca, ไฮไลต์ส้ม Home ในภาพถูกลบและใช้ `s_fusion_nav_marker` แทน):

| หน้า | ปุ่ม |
|---|---|
| Home | ตาราง 5 × 4 หลัก |
| Sketch | Line L, Rectangle R, Circle C, Arc (S), Dimension D, Trim T, Offset O, Project P, Construction X, Sketch Fillet (S) |
| Solid | Extrude E, Press Pull Q, Fillet F, Hole H, Move M, Joint J, Revolve/Chamfer/Shell/Combine/Pattern/Mirror (S) |
| Surface | Patch, Stitch, Unstitch, Thicken, Trim (S ทั้งหมด) |
| Mesh | Insert Mesh, Convert Mesh, Reduce, Remesh (S ทั้งหมด) |
| Sheet Metal | Flange, Unfold, Refold Faces, Create Flat Pattern, Sheet Metal Rules (S ทั้งหมด) |
| Tools | Measure I, Appearance A, Visibility V, Repeat Last (Space), Section Analysis (S), Interference (S) |
| Settings | Browser Ctrl+Alt+B, Data Panel Ctrl+Alt+P, ViewCube Ctrl+Alt+V, Full Screen Ctrl+Shift+F, Reset Layout Ctrl+Alt+R, 4 Views Shift+1 |

## Sidebar หน้าย่อย (หน้า Orca)

แตะ sidebar แล้วเปิดหน้าย่อยทับพื้นที่ตาราง (131, 84, 486 × 344) วาดด้วย LVGL และแตะ Prepare เพื่อกลับตารางหลัก ไอคอนเฟืองมุมขวาบนเปิดหน้า Settings

| หน้า | ปุ่ม (HID) |
|---|---|
| Prepare | ตาราง 6 × 4 หลัก (ภาพพื้นหลัง) |
| Modify | Clone Ctrl+K, Instance + `+`, Instance − `-`, Mesh Boolean B, Assembly Y, Printable V, Select All Ctrl+A, Deselect Esc, Copy Ctrl+C, Paste Ctrl+V, Cut Ctrl+X, Delete All Ctrl+D |
| View | Zoom In I, Zoom Out O, Sidebar Shift+Tab, Prep/Preview Tab และปุ่มสีส้มที่ใช้ได้เฉพาะใน Preview: One Layer L, G-code C, Layer Up/Down, Move Back/Fwd (ลูกศร), Start Home, End End |
| Support | Support Paint L, Brim Ears E, Seam Paint P, Fuzzy Skin H |
| Filament | Filament 1–9 (กดเลข, เลข 1 Orca รอ 0.5 วินาทีก่อนใช้), Color Paint N |
| Printer | Print Plate Ctrl+Shift+G, Export G-code Ctrl+G, Slice Ctrl+R, Save Ctrl+S, Save As Ctrl+Shift+S |
| Tools | Measure U, Assembly Y, Add Text T, Shortcuts ?, 3Dconnexion Ctrl+M |
| Settings | Preferences Ctrl+P, Switch Tab Ctrl+Tab, Jog Step (ตั้งค่าของบอร์ด ใช้ค่าเดียวกับ jog pad) |

- ปุ่มสีส้มในหน้า View ถ้ากดตอนอยู่ Prepare จะไปเรียกคำสั่งอื่น (L = Support Paint, C = Cut, ลูกศร = ขยับชิ้นงาน)
- `LV_MEM_SIZE` ของ LVGL มีแค่ 48 KB จึงสร้างหน้าย่อยตอนเปิดและลบหน้าเดิมทิ้ง ให้มีอยู่ในหน่วยความจำทีละหน้า วัดจริง: หน้า Modify/View ใช้ heap 81% (เหลือ 9.7 KB), Prepare 59% สลับไปมาหลายรอบแล้วค่าไม่เปลี่ยน ไม่มีหน่วยความจำรั่ว
- ไฮไลต์ของ sidebar วาดด้วย LVGL (``s_nav_marker``) และย้ายตามหน้าที่เปิด ส่วนไฮไลต์ Prepare ที่เคยอยู่ในภาพถูกลบออกแล้ว
- Serial จะพิมพ์ ``LVGL heap: …`` ทุกครั้งที่สลับหน้า ใช้ตรวจหน่วยความจำได้

## Jog pad

กด `Move` บนหน้า Orca แล้ว jog pad เด้งขึ้น ขนาด 328 × 300 ที่ตำแหน่ง (236, 96)

| ปุ่ม | HID ที่ส่ง | ผลใน Orca |
|---|---|---|
| Y + | Arrow Up | ขยับ +Y |
| Y − | Arrow Down | ขยับ −Y |
| X + | Arrow Right | ขยับ +X |
| X − | Arrow Left | ขยับ −X |
| step | ไม่ส่ง | สลับระยะ 10 mm / 1 mm (โหมด 1 mm เติม Shift ให้ลูกศร) |
| X | ไม่ส่ง | ปิด jog pad |

- ปุ่มลูกศรกดค้างได้ ใช้ `LV_EVENT_LONG_PRESSED_REPEAT` ส่งซ้ำ
- ต้องเลือกโมเดลใน Orca ก่อน และโฟกัสต้องอยู่ที่หน้าต่าง 3D
- ขยับได้แค่แกน X และ Y เพราะ Orca ไม่มีปุ่มลัดสำหรับแกน Z รวมถึงหมุนและย่อขยาย
- ถ้าอยากคุมแกน Z หรือหมุน ต้องเพิ่ม HID Mouse (composite keyboard + mouse) แล้วทำโซนลากนิ้วเป็น trackpad ลาก gizmo

## USB HID สำคัญ

บอร์ดมี USB Type-C สองช่อง:

- `USB TO UART` ใช้แฟลชและ Serial Monitor ปัจจุบันเป็น `COM13`
- `USB Type-C / Native USB` ใช้ส่ง HID Keyboard ไปยัง Windows

GPIO19/20 แชร์ระหว่าง USB และ CAN ผ่าน CH422G ดังนั้นเฟิร์มแวร์ต้องสั่ง `EXIO5 LOW` หลัง `board->begin()` ก่อน `USB.begin()` หากไม่ทำ Windows จะไม่เห็น HID แม้ Serial จะแจ้งว่า TinyUSB เริ่มแล้ว

ข้อความบูตที่ถูกต้อง:

```text
USB HID keyboard ready (EXIO5 LOW)
Macro Deck UI ready
```

## Build และ Flash

Arduino CLI:

```powershell
$cli = 'C:\Users\User\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
$fqbn = 'esp32:esp32:waveshare_esp32_s3_touch_lcd_7:PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,USBMode=default,CDCOnBoot=default,UploadMode=default'

& $cli compile --fqbn $fqbn --warnings none --export-binaries 'D:\Macro Dash\firmware\MacroDeckUI'
& $cli upload --port COM13 --fqbn $fqbn --input-dir 'D:\Macro Dash\firmware\MacroDeckUI\build\esp32.esp32.waveshare_esp32_s3_touch_lcd_7'
& $cli monitor --port COM13 --config baudrate=115200
```

ผล compile ล่าสุด (2026-09-18, RGB565 + grid 6 × 4 + jog pad + Slice/Clone + กล่อง VIEW รวม + sidebar หน้าย่อย):

```text
Sketch uses 2238729 bytes (71%) of program storage space.
Global variables use 89352 bytes (27%) of dynamic memory.
```

ผล upload ล่าสุด (2026-09-18): `Hash of data verified.` และบูตขึ้นครบทั้งสองบรรทัด

## พื้นที่แฟลช

- Partition ปัจจุบัน `app3M_fat9M_16MB` เพดานโค้ด 3 MB (3,145,728 ไบต์)
- ใช้อยู่ 2.23 MB (70%) โดยเป็นข้อมูลภาพ 2 หน้า 1.54 MB และโค้ดจริงประมาณ 630 KB
- เพิ่มภาพได้อีกประมาณ 1 หน้า ถ้าจะเพิ่มมากกว่านั้น เลือกได้: partition `all_app` (4 MB APP ไม่มี OTA), custom partition (8 MB APP) หรือย้ายภาพไปเก็บใน FATFS 9.9 MB แล้วโหลดเข้า PSRAM ตอนบูต

## งานถัดไป

1. Fusion: หาปุ่มของ Home view (ตอนนี้ยังไม่ผูก)
2. สร้างหน้าโปรไฟล์ System
3. หมุน/ย่อขยาย/ขยับแกน Z จากจอ (ต้องเพิ่ม HID Mouse หรือโปรแกรมฝั่ง PC)
4. ใช้ช่องว่างที่เหลือ 1 ช่องในตาราง Orca

## สถานะปัจจุบัน

- เฟิร์มแวร์บนบอร์ด: แฟลช build ล่าสุด (RGB565 + grid 6 × 4 + jog pad + Slice Plate + Clone + กล่อง VIEW รวม + ลบ Bambu Studio) เมื่อ 2026-09-18 ผ่าน COM13 บูตผ่าน
- ทดสอบการแตะ: ปุ่มมุมมองทั้ง 8 ปุ่ม (Default…Preview) ขึ้น `Touch action: Orca View …` ครบตามลำดับ
- ทดสอบกับ OrcaSlicer จริงผ่านพอร์ต Native USB แล้ว ผ่าน (2026-09-18) ปุ่มส่งถึง Orca ได้
- สีบนจอหลังเปลี่ยนเป็น RGB565: ตรวจด้วยตาแล้ว ถูกต้อง แดง/น้ำเงินไม่สลับ
- ใช้งานจริงต้องเสียบช่อง Native USB (ส่ง HID) ส่วนช่อง UART ใช้แฟลชและดู log เท่านั้น เสียบทั้งสองช่องพร้อมกันได้
- Native USB mux fix (`EXIO5 LOW`): implement แล้วและใช้งานได้
- COM13: USB-Enhanced-SERIAL CH343
- Fusion 360: ผูกปุ่มครบทุกหน้าแล้ว (2026-09-18) ทดสอบการแตะผ่าน Serial ผ่านทุกปุ่ม ผู้ใช้ยืนยันกับ Fusion จริงแล้วว่า F6 = Fit และปุ่ม S search พิมพ์ชื่อคำสั่งแล้วเรียกคำสั่งได้ เหลือ Home ที่ยังไม่ผูก
- System: มีปุ่มใน bottom bar แต่ยังไม่มีหน้า profile
- Sidebar Orca: ครบทั้ง 8 หน้า แฟลชและทดสอบสลับหน้าแล้ว (2026-09-18)
- ไอคอนใหม่ 6 ตัว (Instance ±, Support, Fuzzy Skin, Color, Measure) วาดขึ้นเองด้วย SVG แล้วเรนเดอร์ด้วย Edge headless ไม่ได้นำไอคอนของ OrcaSlicer มาใช้ เพื่อเลี่ยงเงื่อนไขสัญญาอนุญาต AGPL

## Git

Commit ล่าสุดที่ push แล้ว (ดู hash ด้วย `git log -1`):

```text
feat: expand Orca deck to 6x4 grid with jog pad, RGB565 images
```

ก่อนแก้ไขอะไร ให้ตรวจ `git status` ก่อนทุกครั้ง ห้ามลบหรือ reset การแก้ไขใน worktree โดยไม่ตรวจก่อน