#pragma once

#include <stdint.h>
#include <stddef.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

typedef struct __attribute__((packed)) {
    uint16_t LowLimit;
    uint16_t BaseLow;
    uint8_t BaseMid;
    uint8_t Access;
    uint8_t Granularity;
    uint8_t BaseHigh;
} GDT_ENTRY;

typedef struct __attribute__((packed)) {
    uint16_t    Limit;
    size_t      Base;
} DESC_TBL;

typedef struct __attribute__((packed)) {
    uint16_t    BaseLow;
    uint16_t    Segment;
    uint8_t     ISTAndZero;
    uint8_t     SegmentType;
    uint16_t    BaseMid;
    uint32_t    BaseHigh;
    uint32_t    Reserved;
} IDT_GATE;

typedef struct __attribute__((packed)) {
    uint16_t    BaseLow;
    uint16_t    SegmentSelector;
    uint8_t     ISTAndZero;
    uint8_t     Misc;
    uint16_t    BaseHigh;
} IDT_ENTRY;

typedef struct __attribute__((packed)) {
    uint16_t    SegmentLimitLow;
    uint16_t    BaseLow;
    uint8_t     BaseMid1;
    uint8_t     SegmentType;
    uint8_t     SegmentLimitHigh;
    uint8_t     BaseMid2;
    uint32_t    BaseHigh;
    uint32_t    Reserved;
} TSS_ENTRY;

typedef struct __attribute__((packed)) {
    uint32_t    Reserved0;
    size_t      RSP0;
    size_t      RSP1;
    size_t      RSP2;

    size_t      Reserved12;

    size_t      IST1;
    size_t      IST2;
    size_t      IST3;
    size_t      IST4;
    size_t      IST5;
    size_t      IST6;
    size_t      IST7;

    size_t      Reserved34;
    uint16_t    Reserved5;

    uint16_t    IOMap;
} TSS64;

typedef struct __attribute__((packed)) {
    uint16_t    Length;
    size_t      Address;
} GDT;

typedef struct __attribute__((packed)) {
    uint16_t    Length;
    size_t      Address;
} IDT;
