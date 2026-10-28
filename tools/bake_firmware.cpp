// bake_firmware.cpp
// Extracts or replaces Huawei firmware partitions using QHuaweiFlasher's ptable code.
//
// Usage:
//   bake_firmware extract ORIGINAL.bin System.bin APP.bin
//   bake_firmware replace ORIGINAL.bin System.bin APP.bin OUTPUT.bin

#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "ptable.h"
#include "signver.h"
// Standard CRC16 for Huawei partitions
extern "C" unsigned short crc16(uint8_t* buf, int len) {
    unsigned short crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}
ptable_list* ptable = nullptr;
int32_t signlen = -1;
char signver_hash[100] = "";

void set_modified() {}
void fdirlist(int) {}
QComboBox* pselector = nullptr;

static std::vector<uint8_t> read_file(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error(std::string("Cannot open: ") + path);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),
                                std::istreambuf_iterator<char>());
}

static void write_partition(uint32_t code, const char* output) {
    int idx = -1;
    for (int i = 0; i < ptable->index(); ++i) {
        if (ptable->code(i) == code) {
            if (idx != -1)
                throw std::runtime_error("Duplicate partition code: " +
                                         std::to_string(code));
            idx = i;
        }
    }
    if (idx < 0)
        throw std::runtime_error("Partition code not found: " +
                                 std::to_string(code));

    const uint32_t size = ptable->psize(idx);
    const uint8_t* data = ptable->iptr(idx);
    if (!data || size == 0)
        throw std::runtime_error("Partition has no data: " + std::to_string(code));

    std::ofstream out(output, std::ios::binary);
    if (!out) throw std::runtime_error(std::string("Cannot create: ") + output);
    out.write(reinterpret_cast<const char*>(data), size);
    if (!out) throw std::runtime_error(std::string("Write failed: ") + output);

    std::cout << "Extracted partition 0x" << std::hex << code << std::dec
              << " -> " << output << " (" << size << " bytes)\n";
}

static void replace_partition(uint32_t code, const char* path) {
    int idx = -1;
    for (int i = 0; i < ptable->index(); ++i) {
        if (ptable->code(i) == code) {
            if (idx != -1)
                throw std::runtime_error("Duplicate partition code: " +
                                         std::to_string(code));
            idx = i;
        }
    }
    if (idx < 0)
        throw std::runtime_error("Partition code not found: " +
                                 std::to_string(code));

    std::vector<uint8_t> data = read_file(path);
    if (data.empty()) throw std::runtime_error(std::string("Empty file: ") + path);
    if (data.size() > UINT32_MAX)
        throw std::runtime_error(std::string("Partition too large: ") + path);

    std::cout << "Replacing partition 0x" << std::hex << code << std::dec
              << " (" << reinterpret_cast<const char*>(ptable->name(idx))
              << ") with " << data.size() << " bytes\n";
    ptable->replace(idx, data.data(), static_cast<uint32_t>(data.size()));
}

static void write_rebuilt(const char* original, const char* output,
                          const std::vector<bool>& was_compressed) {
    FILE* out = std::fopen(output, "wb");
    if (!out) throw std::runtime_error(std::string("Cannot create output: ") + output);

    FILE* src = std::fopen(original, "rb");
    if (!src) {
        std::fclose(out);
        throw std::runtime_error("Cannot reopen original firmware");
    }
    uint8_t prefix[0x5c];
    if (std::fread(prefix, 1, sizeof(prefix), src) != sizeof(prefix)) {
        std::fclose(src);
        std::fclose(out);
        throw std::runtime_error("Original firmware too short for prefix");
    }
    std::fclose(src);

    if (std::fwrite(prefix, 1, sizeof(prefix), out) != sizeof(prefix)) {
        std::fclose(out);
        throw std::runtime_error("Failed to write output prefix");
    }

    for (int i = 0; i < ptable->index(); ++i)
        ptable->save_part(i, out, was_compressed[i]);

    if (std::fclose(out) != 0)
        throw std::runtime_error("Failed to finalize output");
    std::cout << "Wrote " << output << "\n";
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " extract ORIGINAL.bin System.bin APP.bin\n"
                  << "  " << argv[0] << " replace ORIGINAL.bin System.bin APP.bin OUTPUT.bin\n";
        return 2;
    }

    const std::string mode = argv[1];
    const bool extract = mode == "extract";
    const bool replace = mode == "replace";
    if ((!extract && !replace) || (extract && argc != 5) ||
        (replace && argc != 6)) {
        std::cerr << "Invalid arguments. Use extract or replace mode.\n";
        return 2;
    }

    const char* original = argv[2];
    try {
        FILE* in = std::fopen(original, "rb");
        if (!in) throw std::runtime_error(std::string("Cannot open firmware: ") + original);

        ptable = new ptable_list();
        ptable->findparts(in);
        std::fclose(in);

        if (ptable->index() < 2)
            throw std::runtime_error("No valid firmware partitions detected");

        if (extract) {
            write_partition(0x590000, argv[3]); // System
            write_partition(0x5a0000, argv[4]); // APP
        } else {
            std::vector<bool> was_compressed(ptable->index());
            for (int i = 0; i < ptable->index(); ++i)
                was_compressed[i] = (ptable->zsize(i) != 0);

            replace_partition(0x590000, argv[3]);
            replace_partition(0x5a0000, argv[4]);
            write_rebuilt(original, argv[5], was_compressed);
        }

        delete ptable;
        ptable = nullptr;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        delete ptable;
        ptable = nullptr;
        return 1;
    }
}
