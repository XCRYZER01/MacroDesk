#include "macro_deck_runtime.h"

#include <Arduino.h>
#include <FFat.h>
#include <esp_heap_caps.h>
#include <stdlib.h>
#include <string.h>

#include "macro_deck_ui.h"

namespace {

constexpr char kBundlePath[] = "/macrodesk.bin";
constexpr char kTempPath[] = "/macrodesk.tmp";
constexpr char kBackupPath[] = "/macrodesk.bak";
constexpr uint8_t kMagic[4] = {'M', 'D', 'B', '1'};
constexpr uint16_t kFormatVersion = 1;
constexpr size_t kHeaderSize = 16;
constexpr size_t kMaxBundleSize = 2 * 1024 * 1024;
constexpr size_t kImageBytes = 800 * 480 * sizeof(uint16_t);
constexpr size_t kProgressInterval = 4 * 1024;
constexpr uint8_t kMaxProfiles = 2;
constexpr uint8_t kMaxPages = 8;
constexpr uint16_t kMaxZones = 64;
constexpr uint8_t kMaxPageButtons = 24;

uint8_t *s_blob = nullptr;
size_t s_blob_size = 0;
macro_profile_t s_profiles[kMaxProfiles] = {};
macro_zone_t *s_zones[kMaxProfiles] = {};
macro_page_t *s_pages[kMaxProfiles] = {};
macro_button_t *s_page_buttons[kMaxProfiles][kMaxPages] = {};

uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    crc = ~crc;
    while (length-- != 0) {
        crc ^= *data++;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320UL & (uint32_t)-(int32_t)(crc & 1));
        }
    }
    return ~crc;
}

class Reader {
public:
    Reader(uint8_t *data, size_t size) : begin_(data), cursor_(data), end_(data + size) {}

    bool u8(uint8_t &value)
    {
        if (remaining() < 1) return false;
        value = *cursor_++;
        return true;
    }

    bool u16(uint16_t &value)
    {
        if (remaining() < 2) return false;
        value = read_u16_le(cursor_);
        cursor_ += 2;
        return true;
    }

    bool i16(lv_coord_t &value)
    {
        uint16_t raw;
        if (!u16(raw)) return false;
        value = (lv_coord_t)(int16_t)raw;
        return true;
    }

    bool u32(uint32_t &value)
    {
        if (remaining() < 4) return false;
        value = read_u32_le(cursor_);
        cursor_ += 4;
        return true;
    }

    bool string(const char *&value)
    {
        uint16_t length;
        if (!u16(length) || remaining() < (size_t)length + 1 || cursor_[length] != '\0') return false;
        value = reinterpret_cast<const char *>(cursor_);
        cursor_ += (size_t)length + 1;
        return true;
    }

    bool align4()
    {
        const size_t offset = (size_t)(cursor_ - begin_);
        const size_t padding = (4 - (offset & 3)) & 3;
        if (remaining() < padding) return false;
        cursor_ += padding;
        return true;
    }

    bool bytes(uint8_t *&value, size_t length)
    {
        if (remaining() < length) return false;
        value = cursor_;
        cursor_ += length;
        return true;
    }

    size_t remaining() const { return (size_t)(end_ - cursor_); }

private:
    uint8_t *begin_;
    uint8_t *cursor_;
    uint8_t *end_;
};

void release_runtime()
{
    for (uint8_t profile = 0; profile < kMaxProfiles; ++profile) {
        for (uint8_t page = 0; page < kMaxPages; ++page) {
            free(s_page_buttons[profile][page]);
            s_page_buttons[profile][page] = nullptr;
        }
        free(s_pages[profile]);
        free(s_zones[profile]);
        s_pages[profile] = nullptr;
        s_zones[profile] = nullptr;
    }
    free(s_blob);
    s_blob = nullptr;
    s_blob_size = 0;
    memset(s_profiles, 0, sizeof(s_profiles));
    macro_profiles = macro_builtin_profiles;
    macro_profile_count = macro_builtin_profile_count;
}

bool read_button(Reader &reader, macro_button_t &button)
{
    uint8_t action;
    if (!reader.u8(action) || !reader.u8(button.arg) || !reader.u32(button.accent) ||
        !reader.string(button.icon) || !reader.string(button.label) || !reader.string(button.keys)) {
        return false;
    }
    if (action > MACRO_ACTION_TEXT) return false;
    button.action = (macro_action_t)action;
    if (button.keys[0] == '\0') button.keys = nullptr;
    return true;
}

bool parse_bundle(uint8_t *blob, size_t size)
{
    if (size < kHeaderSize || memcmp(blob, kMagic, sizeof(kMagic)) != 0 ||
        read_u16_le(blob + 4) != kFormatVersion || blob[6] == 0 || blob[6] > kMaxProfiles ||
        read_u32_le(blob + 8) != size ||
        crc32_update(0, blob + kHeaderSize, size - kHeaderSize) != read_u32_le(blob + 12)) {
        return false;
    }

    const uint8_t profile_count = blob[6];
    Reader reader(blob + kHeaderSize, size - kHeaderSize);
    for (uint8_t profile_index = 0; profile_index < profile_count; ++profile_index) {
        macro_profile_t &profile = s_profiles[profile_index];
        if (!reader.string(profile.name) || !reader.u32(profile.highlight_fill) ||
            !reader.u32(profile.highlight_border) || !reader.align4()) return false;

        uint32_t image_size;
        uint8_t *image_data;
        if (!reader.u32(image_size) || image_size != kImageBytes || !reader.bytes(image_data, image_size)) return false;
        profile.image = reinterpret_cast<const uint16_t *>(image_data);

        uint16_t zone_count;
        if (!reader.u16(zone_count) || zone_count == 0 || zone_count > kMaxZones) return false;
        s_zones[profile_index] = (macro_zone_t *)calloc(zone_count, sizeof(macro_zone_t));
        if (s_zones[profile_index] == nullptr) return false;
        profile.zones = s_zones[profile_index];
        profile.zone_count = zone_count;
        for (uint16_t zone_index = 0; zone_index < zone_count; ++zone_index) {
            macro_zone_t &zone = s_zones[profile_index][zone_index];
            if (!reader.i16(zone.x) || !reader.i16(zone.y) || !reader.i16(zone.w) || !reader.i16(zone.h) ||
                !read_button(reader, zone.button)) return false;
        }

        uint8_t page_count;
        if (!reader.u8(page_count) || page_count == 0 || page_count > kMaxPages) return false;
        s_pages[profile_index] = (macro_page_t *)calloc(page_count, sizeof(macro_page_t));
        if (s_pages[profile_index] == nullptr) return false;
        profile.pages = s_pages[profile_index];
        profile.page_count = page_count;
        for (uint8_t page_index = 0; page_index < page_count; ++page_index) {
            macro_page_t &page = s_pages[profile_index][page_index];
            uint8_t button_count;
            if (!reader.string(page.title) || !reader.string(page.hint) || !reader.u8(page.cols) ||
                !reader.u8(button_count) || button_count > kMaxPageButtons) return false;
            page.count = button_count;
            if (button_count == 0) continue;
            s_page_buttons[profile_index][page_index] =
                (macro_button_t *)calloc(button_count, sizeof(macro_button_t));
            if (s_page_buttons[profile_index][page_index] == nullptr) return false;
            page.buttons = s_page_buttons[profile_index][page_index];
            for (uint8_t button_index = 0; button_index < button_count; ++button_index) {
                if (!read_button(reader, s_page_buttons[profile_index][page_index][button_index])) return false;
            }
        }
    }
    if (reader.remaining() != 0) return false;
    macro_profiles = s_profiles;
    macro_profile_count = profile_count;
    return true;
}

bool load_bundle()
{
    File file = FFat.open(kBundlePath, FILE_READ);
    if (!file) return false;
    const size_t size = file.size();
    if (size < kHeaderSize || size > kMaxBundleSize) {
        file.close();
        return false;
    }
    uint8_t *blob = (uint8_t *)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (blob == nullptr || file.read(blob, size) != size) {
        free(blob);
        file.close();
        return false;
    }
    file.close();
    s_blob = blob;
    s_blob_size = size;
    if (!parse_bundle(blob, size)) {
        release_runtime();
        return false;
    }
    return true;
}

bool validate_file(const char *path)
{
    File file = FFat.open(path, FILE_READ);
    if (!file || file.size() < kHeaderSize || file.size() > kMaxBundleSize) return false;
    uint8_t header[kHeaderSize];
    if (file.read(header, sizeof(header)) != sizeof(header) || memcmp(header, kMagic, sizeof(kMagic)) != 0 ||
        read_u16_le(header + 4) != kFormatVersion || read_u32_le(header + 8) != file.size()) {
        file.close();
        return false;
    }
    uint8_t chunk[1024];
    uint32_t crc = 0;
    while (file.available()) {
        const size_t count = file.read(chunk, sizeof(chunk));
        crc = crc32_update(crc, chunk, count);
    }
    file.close();
    return crc == read_u32_le(header + 12);
}

enum class ReceiveState { Idle, Receiving };
ReceiveState s_receive_state = ReceiveState::Idle;
File s_receive_file;
size_t s_receive_size = 0;
size_t s_receive_count = 0;
uint32_t s_receive_expected_crc = 0;
uint32_t s_receive_crc = 0;
size_t s_next_progress = kProgressInterval;
uint32_t s_receive_last_ms = 0;
uint8_t s_receive_chunk[1024];
char s_line[96] = {};
size_t s_line_length = 0;

void receive_error(const char *message)
{
    if (s_receive_file) s_receive_file.close();
    FFat.remove(kTempPath);
    s_receive_state = ReceiveState::Idle;
    Serial.printf("MDERR %s\n", message);
}

void finish_receive()
{
    s_receive_file.close();
    if (s_receive_crc != s_receive_expected_crc || !validate_file(kTempPath)) {
        receive_error("checksum-or-format");
        return;
    }
    FFat.remove(kBackupPath);
    const bool had_bundle = FFat.exists(kBundlePath);
    if (had_bundle && !FFat.rename(kBundlePath, kBackupPath)) {
        receive_error("backup-failed");
        return;
    }
    if (!FFat.rename(kTempPath, kBundlePath)) {
        if (had_bundle) FFat.rename(kBackupPath, kBundlePath);
        receive_error("rename-failed");
        return;
    }
    FFat.remove(kBackupPath);
    s_receive_state = ReceiveState::Idle;
    Serial.println("MDOK saved-restarting");
    Serial.flush();
    delay(150);
    ESP.restart();
}

void handle_command(const char *line)
{
    unsigned long size = 0;
    unsigned long crc = 0;
    if (sscanf(line, "MDUP %lu %lx", &size, &crc) == 2) {
        if (size < kHeaderSize || size > kMaxBundleSize) {
            Serial.println("MDERR invalid-size");
            return;
        }
        FFat.remove(kTempPath);
        s_receive_file = FFat.open(kTempPath, FILE_WRITE);
        if (!s_receive_file) {
            Serial.println("MDERR open-failed");
            return;
        }
        s_receive_size = (size_t)size;
        s_receive_count = 0;
        s_receive_expected_crc = (uint32_t)crc;
        s_receive_crc = 0;
        s_next_progress = kProgressInterval;
        s_receive_last_ms = millis();
        s_receive_state = ReceiveState::Receiving;
        Serial.printf("MDREADY %u\n", (unsigned)s_receive_size);
    } else if (strcmp(line, "MDINFO") == 0) {
        Serial.printf("MDINFO %s %u\n", FFat.exists(kBundlePath) ? "runtime" : "builtin",
                      (unsigned)macro_profile_count);
    } else if (strcmp(line, "MDERASE") == 0) {
        const bool removed = !FFat.exists(kBundlePath) || FFat.remove(kBundlePath);
        Serial.println(removed ? "MDOK erased-restarting" : "MDERR erase-failed");
        if (removed) {
            Serial.flush();
            delay(150);
            ESP.restart();
        }
    }
}

}  // namespace

bool macro_deck_runtime_begin(void)
{
    if (!FFat.begin(true)) {
        Serial.println("MacroDesk FAT mount failed; using built-in profiles");
        return false;
    }
    Serial.printf("MacroDesk FAT ready: %u / %u bytes used\n",
                  (unsigned)FFat.usedBytes(), (unsigned)FFat.totalBytes());
    if (!FFat.exists(kBundlePath) && FFat.exists(kBackupPath)) {
        if (FFat.rename(kBackupPath, kBundlePath)) Serial.println("MacroDesk runtime backup restored");
    } else if (FFat.exists(kBundlePath) && FFat.exists(kBackupPath)) {
        FFat.remove(kBackupPath);
    }
    if (!FFat.exists(kBundlePath)) {
        Serial.println("MacroDesk runtime profile not found; using built-in profiles");
        return true;
    }
    if (!load_bundle()) {
        Serial.println("MacroDesk runtime profile invalid; using built-in profiles");
        return false;
    }
    Serial.printf("MacroDesk runtime profile loaded: %u profiles, %u bytes\n",
                  (unsigned)macro_profile_count, (unsigned)s_blob_size);
    return true;
}

void macro_deck_runtime_poll(void)
{
    if (s_receive_state == ReceiveState::Receiving) {
        size_t available = (size_t)Serial.available();
        if (available == 0) {
            if (millis() - s_receive_last_ms > 10000) {
                if (s_receive_file) s_receive_file.close();
                FFat.remove(kTempPath);
                s_receive_state = ReceiveState::Idle;
                Serial.printf("MDERR receive-timeout %u/%u\n",
                              (unsigned)s_receive_count, (unsigned)s_receive_size);
            }
            return;
        }
        const size_t remaining = s_receive_size - s_receive_count;
        size_t count = available < sizeof(s_receive_chunk) ? available : sizeof(s_receive_chunk);
        if (count > remaining) count = remaining;
        count = Serial.readBytes(s_receive_chunk, count);
        if (count == 0 || s_receive_file.write(s_receive_chunk, count) != count) {
            receive_error("write-failed");
            return;
        }
        s_receive_last_ms = millis();
        s_receive_crc = crc32_update(s_receive_crc, s_receive_chunk, count);
        s_receive_count += count;
        if (s_receive_count >= s_next_progress && s_receive_count < s_receive_size) {
            Serial.printf("MDPROGRESS %u %u\n", (unsigned)s_receive_count, (unsigned)s_receive_size);
            s_next_progress += kProgressInterval;
        }
        if (s_receive_count == s_receive_size) finish_receive();
        return;
    }

    while (Serial.available()) {
        const char value = (char)Serial.read();
        if (value == '\r') continue;
        if (value == '\n') {
            s_line[s_line_length] = '\0';
            if (s_line_length != 0) handle_command(s_line);
            s_line_length = 0;
        } else if (s_line_length + 1 < sizeof(s_line)) {
            s_line[s_line_length++] = value;
        } else {
            s_line_length = 0;
        }
    }
}
