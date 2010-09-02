/*
 * TVIN signal format table
 *
 * Author: Lin Xu <lin.xu@amlogic.com>
 *         Bobby Yang <bo.yang@amlogic.com>
 *
 * Copyright (C) 2010 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "tvin_global.h"
#include "tvin_format_table.h"


const struct tvin_format_s tvin_fmt_tbl[TVIN_SIG_FMT_HDMI_MAX + 1] =
{
//   H_Active V_Active H_cnt Hs_cnt H_Total V_Total Hs_Front Hs_Width Vs_Front Vs_Width Hs_Polarity             Vs_Polarity             Scan_Mode                   Pixel_Clk(Khz/10)
    {   0,       0,       0,   0,      0,      0,      0,      0,      0,       0,      TVIN_SYNC_POL_NULL    , TVIN_SYNC_POL_NULL    , TVIN_SCAN_MODE_NULL       ,     0}, // TVIN_SIG_FMT_NULL
//VGA
    { 512,     384,    4085, 204,    640,    407,     16,     32,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  1567}, // TVIN_SIG_FMT_VGA_512X384P_60D147,
    { 560,     384,    4085, 279,    704,    407,     16,     48,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  1723}, // TVIN_SIG_FMT_VGA_560X384P_60D147,
    { 640,     200,    6369, 391,    912,    262,     94,     56,     25,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  1432}, // TVIN_SIG_FMT_VGA_640X200P_59D924,
    { 640,     350,    2641, 203,    832,    445,     32,     64,     32,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3150}, // TVIN_SIG_FMT_VGA_640X350P_85D080,
    { 640,     400,    3178, 381,    800,    525,     16,     96,     50,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2517}, // TVIN_SIG_FMT_VGA_640X400P_59D940,
    { 640,     400,    2641, 203,    832,    445,     32,     64,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3150}, // TVIN_SIG_FMT_VGA_640X400P_85D080,
    { 640,     400,    3301, 461,    824,    508,      3,    115,     42,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2496}, // TVIN_SIG_FMT_VGA_640X400P_59D638,
    { 640,     400,    4029, 304,    848,    440,     64,     64,      7,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2105}, // TVIN_SIG_FMT_VGA_640X400P_56D416,
    { 640,     480,    6356, 473,    780,    525,     18,     58,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  1227}, // TVIN_SIG_FMT_VGA_640X480I_29D970,
    { 640,     480,    2859, 204,    896,    525,     80,     64,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3134}, // TVIN_SIG_FMT_VGA_640X480P_66D619,
    { 640,     480,    2857, 212,    864,    525,     64,     64,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3024}, // TVIN_SIG_FMT_VGA_640X480P_66D667,
    { 640,     480,    3178, 381,    800,    525,     16,     96,     10,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2517}, // TVIN_SIG_FMT_VGA_640X480P_59D940,
    { 640,     480,    3175, 381,    800,    525,     16,     96,     10,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2520}, // TVIN_SIG_FMT_VGA_640X480P_60D000,
    { 640,     480,    2641, 127,    832,    520,     24,     40,      9,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3150}, // TVIN_SIG_FMT_VGA_640X480P_72D809,
    { 640,     480,    2667, 203,    840,    500,     16,     64,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3150}, // TVIN_SIG_FMT_VGA_640X480P_75D000_A,
    { 640,     480,    2311, 156,    832,    509,     56,     56,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3600}, // TVIN_SIG_FMT_VGA_640X480P_85D008,
    { 640,     480,    3301, 440,    826,    508,      3,    110,      4,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2502}, // TVIN_SIG_FMT_VGA_640X480P_59D638,
    { 640,     480,    2540, 305,    800,    525,     16,     96,     11,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3150}, // TVIN_SIG_FMT_VGA_640X480P_75D000_B,
    { 640,     870,    1452, 140,    832,    918,     32,     80,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5728}, // TVIN_SIG_FMT_VGA_640X870P_75D000,
    { 720,     350,    3178, 381,    900,    449,     18,    108,     37,       2,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2832}, // TVIN_SIG_FMT_VGA_720X350P_70D086,
    { 720,     400,    2637, 203,    936,    446,     36,     72,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3550}, // TVIN_SIG_FMT_VGA_720X400P_85D039,
    { 720,     400,    3178, 381,    900,    449,     18,    108,     12,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2832}, // TVIN_SIG_FMT_VGA_720X400P_70D086,
    { 720,     400,    2535, 304,    900,    449,     18,    108,     21,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3550}, // TVIN_SIG_FMT_VGA_720X400P_87D849,
    { 720,     400,    3178, 381,    900,    525,     18,    108,     50,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2832}, // TVIN_SIG_FMT_VGA_720X400P_59D940,
    { 720,     480,    3178, 381,    900,    525,     18,    108,     10,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2832}, // TVIN_SIG_FMT_VGA_720X480P_59D940,
    { 752,     484,    6356, 468,    910,    525,     22,     67,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  1432}, // TVIN_SIG_FMT_VGA_752X484I_29D970,
    { 768,     574,    6400, 468,    944,    625,     22,     69,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  1475}, // TVIN_SIG_FMT_VGA_768X574I_25D000,
    { 800,     600,    2844, 200,   1024,    625,     24,     72,      1,       2,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  3600}, // TVIN_SIG_FMT_VGA_800X600P_56D250,
    { 800,     600,    2640, 320,   1056,    628,     40,    128,      1,       4,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  4000}, // TVIN_SIG_FMT_VGA_800X600P_60D317,
    { 800,     600,    2080, 240,   1040,    666,     56,    120,     37,       6,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5000}, // TVIN_SIG_FMT_VGA_800X600P_72D188,
    { 800,     600,    2133, 162,   1056,    625,     16,     80,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  4950}, // TVIN_SIG_FMT_VGA_800X600P_75D000,
    { 800,     600,    1863, 114,   1048,    631,     32,     64,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5625}, // TVIN_SIG_FMT_VGA_800X600P_85D061,
    { 832,     624,    2036, 175,   1120,    654,     32,     96,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5500}, // TVIN_SIG_FMT_VGA_832X624P_75D087,
    { 848,     480,    2327, 171,   1088,    507,     40,     80,      3,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  4675}, // TVIN_SIG_FMT_VGA_848X480P_84D751,
    {1024,     768,    2075, 125,   1328,    813,     80,     80,     11,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  6400}, // TVIN_SIG_FMT_VGA_1024X768P_59D278,
    {1024,     768,    1660, 120,   1328,    804,     32,     96,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8000}, // TVIN_SIG_FMT_VGA_1024X768P_74D927,
    {1024,     768,    2815, 392,   1264,    817,      8,    176,      0,       4,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  4490}, // TVIN_SIG_FMT_VGA_1024X768I_43D479,
    {1024,     768,    2068, 209,   1344,    806,     24,    136,      3,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  6500}, // TVIN_SIG_FMT_VGA_1024X768P_60D004,
    {1024,     768,    1771, 181,   1328,    806,     24,    136,      3,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7500}, // TVIN_SIG_FMT_VGA_1024X768P_70D069,
    {1024,     768,    1666, 122,   1312,    800,     16,     96,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7875}, // TVIN_SIG_FMT_VGA_1024X768P_75D029,
    {1024,     768,    1456, 102,   1376,    808,     48,     96,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  9450}, // TVIN_SIG_FMT_VGA_1024X768P_84D997,
    {1024,     768,    2096, 200,   1344,    795,     64,    128,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  6411}, // TVIN_SIG_FMT_VGA_1024X768P_60D000,
    {1024,     768,    1589, 151,   1344,    840,     64,    128,      4,       4,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8459}, // TVIN_SIG_FMT_VGA_1024X768P_74D925,
    {1024,     768,    1660, 120,   1328,    803,     16,     96,      6,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8000}, // TVIN_SIG_FMT_VGA_1024X768P_75D020,
    {1024,     768,    1777, 355,   1368,    804,     18,    273,      0,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7700}, // TVIN_SIG_FMT_VGA_1024X768P_70D008,
    {1024,     768,    1637, 372,   1408,    806,      8,    320,      0,       8,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8600}, // TVIN_SIG_FMT_VGA_1024X768P_75D782,
    {1024,     768,    1612, 152,   1360,    805,     32,    128,      2,       4,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8437}, // TVIN_SIG_FMT_VGA_1024X768P_77D069,
    {1024,     768,    1728, 181,   1296,    806,     24,    136,      3,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7500}, // TVIN_SIG_FMT_VGA_1024X768P_71D799,
    {1024,    1024,    1578, 179,   1408,   1056,     21,    160,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8921}, // TVIN_SIG_FMT_VGA_1024X1024P_60D000,
    {1053,     754,    2824, 400,   1286,    815,     14,    182,      0,       4,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  4554}, // TVIN_SIG_FMT_VGA_1053X754I_43D453,
    {1056,     768,    2809, 386,   1280,    819,      7,    176,      0,       4,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  4557}, // TVIN_SIG_FMT_VGA_1056X768I_43D470,
    {1120,     750,    3043, 176,   1456,    821,    112,     84,      8,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  4784}, // TVIN_SIG_FMT_VGA_1120X750I_40D021,
    {1152,     864,    1566, 102,   1480,    912,     32,     96,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  9450}, // TVIN_SIG_FMT_VGA_1152X864P_70D012,
    {1152,     864,    1481, 119,   1600,    900,     64,    128,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_VGA_1152X864P_75D000,
    {1152,     864,    1297, 105,   1576,    907,     64,    128,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 12150}, // TVIN_SIG_FMT_VGA_1152X864P_84D999,
    {1152,     870,    1456, 128,   1456,    915,     32,    128,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10000}, // TVIN_SIG_FMT_VGA_1152X870P_75D062,
    {1152,     900,    1618, 138,   1504,    937,     29,    128,      2,       4,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  9294}, // TVIN_SIG_FMT_VGA_1152X900P_65D950,
    {1152,     900,    1617, 135,   1528,    937,     40,    128,      2,       4,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  9450}, // TVIN_SIG_FMT_VGA_1152X900P_66D004,
    {1152,     900,    1394, 91,1    472,    943,     16,     96,      2,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10556}, // TVIN_SIG_FMT_VGA_1152X900P_76D047,
    {1152,     900,    1393, 119,   1504,    943,     32,    128,      2,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_VGA_1152X900P_76D149,
    {1244,     842,    3810, 275,   1524,    875,     36,    110,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  4001}, // TVIN_SIG_FMT_VGA_1244X842I_30D000,
    {1280,     768,    2110, 47,1    440,    790,     48,     32,      3,       7,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  6825}, // TVIN_SIG_FMT_VGA_1280X768P_59D995,
    {1280,     768,    1659, 125,   1696,    805,     80,    128,      3,       7,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10225}, // TVIN_SIG_FMT_VGA_1280X768P_74D893,
    {1280,     768,    1457, 116,   1712,    809,     80,    136,      3,       7,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 11750}, // TVIN_SIG_FMT_VGA_1280X768P_84D837,
    {1280,     960,    1667, 104,   1800,   1000,     96,    112,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_VGA_1280X960P_60D000,
    {1280,     960,    1333, 114,   1680,   1000,     32,    144,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 12600}, // TVIN_SIG_FMT_VGA_1280X960P_75D000,
    {1280,     960,    1164, 108,   1728,   1011,     64,    160,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_VGA_1280X960P_85D002,
    {1280,    1024,    2154, 102,   1696,   1069,     64,     80,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7875}, // TVIN_SIG_FMT_VGA_1280X1024I_43D436,
    {1280,    1024,    1563, 104,   1688,   1066,     48,    112,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_VGA_1280X1024P_60D020,
    {1280,    1024,    1250, 107,   1688,   1066,     16,    144,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 13500}, // TVIN_SIG_FMT_VGA_1280X1024P_75D025,
    {1280,    1024,    1097, 102,   1728,   1072,     64,    160,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 15750}, // TVIN_SIG_FMT_VGA_1280X1024P_85D024,
    {1280,    1024,    1579, 170,   1708,   1056,     44,    184,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10818}, // TVIN_SIG_FMT_VGA_1280X1024P_59D979,
    {1280,    1024,    1280, 142,   1728,   1085,     64,    192,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 13500}, // TVIN_SIG_FMT_VGA_1280X1024P_72D005,
    {1280,    1024,    1578, 179,   1760,   1056,     26,    200,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 11152}, // TVIN_SIG_FMT_VGA_1280X1024P_60D002,
    {1280,    1024,    1413, 133,   1696,   1056,     32,    160,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 12000}, // TVIN_SIG_FMT_VGA_1280X1024P_67D003,
    {1280,    1024,    1268, 107,   1712,   1064,     32,    144,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 13500}, // TVIN_SIG_FMT_VGA_1280X1024P_74D112,
    {1280,    1024,    1231,  71,   1724,   1066,     32,    100,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14000}, // TVIN_SIG_FMT_VGA_1280X1024P_76D179,
    {1280,    1024,    1394,  96,   1632,   1075,     16,    112,      6,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 11705}, // TVIN_SIG_FMT_VGA_1280X1024P_66D718,
    {1280,    1024,    1395, 108,   1648,   1075,     24,    128,      2,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 11812}, // TVIN_SIG_FMT_VGA_1280X1024P_66D677,
    {1280,    1024,    1233,  47,   1664,   1066,     32,     64,      2,       8,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 13500}, // TVIN_SIG_FMT_VGA_1280X1024P_76D107,
    {1280,    1024,    1565, 104,   1688,   1065,     48,    112,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10786}, // TVIN_SIG_FMT_VGA_1280X1024P_59D996,
    {1360,     768,    2096, 160,   1776,    798,     72,    136,      3,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  8475}, // TVIN_SIG_FMT_VGA_1360X768P_59D799,
    {1360,    1024,    1771, 357,   1824,   1097,     24,    368,      0,       8,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED , 10300}, // TVIN_SIG_FMT_VGA_1360X1024I_51D476,
    {1440,    1080,    1491, 117,   1936,   1118,     96,    152,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 12987}, // TVIN_SIG_FMT_VGA_1440X1080P_60D000,
    {1600,    1200,    1600, 142,   2160,   1301,     48,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED , 13500}, // TVIN_SIG_FMT_VGA_1600X1200I_48D040,
    {1600,    1200,    1333, 119,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 16200}, // TVIN_SIG_FMT_VGA_1600X1200P_60D000,
    {1600,    1200,    1231, 109,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 17550}, // TVIN_SIG_FMT_VGA_1600X1200P_65D000,
    {1600,    1200,    1143, 102,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 18900}, // TVIN_SIG_FMT_VGA_1600X1200P_70D000,
    {1600,    1200,    1067,  95,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 20250}, // TVIN_SIG_FMT_VGA_1600X1200P_75D000,
    {1600,    1200,    1000,  89,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 21600}, // TVIN_SIG_FMT_VGA_1600X1200P_80D000,
    {1600,    1200,     941,  84,   2160,   1250,     64,    192,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 22950}, // TVIN_SIG_FMT_VGA_1600X1200P_85D000,
    {1600,    1280,    1120, 128,   2240,   1334,      0,    256,      0,      10,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 20000}, // TVIN_SIG_FMT_VGA_1600X1280P_66D931,
    {1680,    1080,    1491, 122,   2256,   1118,    104,    184,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 15133}, // TVIN_SIG_FMT_VGA_1680X1080P_60D000,
    {1792,    1344,    1196,  98,   2448,   1394,    128,    200,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 20475}, // TVIN_SIG_FMT_VGA_1792X1344P_60D000,
    {1792,    1344,     941,  83,   2456,   1417,     96,    216,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 26100}, // TVIN_SIG_FMT_VGA_1792X1344P_74D997,
    {1856,    1392,    1158, 103,   2528,   1439,     96,    224,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 21825}, // TVIN_SIG_FMT_VGA_1856X1392P_59D995,
    {1856,    1392,     889,  78,   2560,   1500,    128,    224,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 28800}, // TVIN_SIG_FMT_VGA_1856X1392P_75D000,
    {1868,    1200,    1064,  86,   2560,   1253,    136,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 24058}, // TVIN_SIG_FMT_VGA_1868X1200P_75D000,
    {1920,    1080,    1491, 120,   2576,   1118,    120,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 17280}, // TVIN_SIG_FMT_VGA_1920X1080P_60D000,
    {1920,    1080,    1182,  94,   2608,   1128,    136,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 22064}, // TVIN_SIG_FMT_VGA_1920X1080P_75D000,
    {1920,    1080,    1037,  82,   2624,   1134,    144,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 25293}, // TVIN_SIG_FMT_VGA_1920X1080P_85D000,
    {1920,    1200,     933,  74,   2624,   1262,    144,    208,      3,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 28125}, // TVIN_SIG_FMT_VGA_1920X1200P_84D932,
    {1920,    1200,    1064,  84,   2624,   1253,    144,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 24659}, // TVIN_SIG_FMT_VGA_1920X1200P_75D000,
    {1920,    1200,     934,  74,   2640,   1260,    152,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 28274}, // TVIN_SIG_FMT_VGA_1920X1200P_85D000,
    {1920,    1234,    1035,  82,   2624,   1288,    144,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 25348}, // TVIN_SIG_FMT_VGA_1920X1234P_75D000,
    {1920,    1234,     908,  72,   2640,   1296,    152,    208,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 29082}, // TVIN_SIG_FMT_VGA_1920X1234P_85D000,
    {1920,    1440,    1111,  89,   2600,   1500,    128,    208,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 23400}, // TVIN_SIG_FMT_VGA_1920X1440P_60D000,
    {1920,    1440,     889,  75,   2640,   1500,    144,    224,      1,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 29700}, // TVIN_SIG_FMT_VGA_1920X1440P_75D000,
    {2048,    1536,    1044,  90,   2504,   1597,     48,    216,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 23993}, // TVIN_SIG_FMT_VGA_2048X1536P_60D000_A,
    {2048,    1536,     830,  68,   2656,   1606,     64,    216,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 31991}, // TVIN_SIG_FMT_VGA_2048X1536P_75D000,
    {2048,    1536,    1049,  84,   2800,   1589,    152,    224,      1,       3,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 26695}, // TVIN_SIG_FMT_VGA_2048X1536P_60D000_B,
    {2048,    2048,     788,  90,   2816,   2114,     42,    320,      3,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 35723}, // TVIN_SIG_FMT_VGA_2048X2048P_60D008,
    {   0,       0,       0,   0,      0,      0,      0,      0,      0,       0,      TVIN_SYNC_POL_NULL    , TVIN_SYNC_POL_NULL    , TVIN_SCAN_MODE_NULL       ,     0}, // TVIN_SIG_FMT_VGA_MAX,
//Component format
    { 720,     480,    3175, 459,    858,    525,     16,     62,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  1350}, // TVIN_SIG_FMT_COMPONENT_480P_60D000,
    { 720,     240,    6356, 229,    858,    263,     19,     62,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  2703}, // TVIN_SIG_FMT_COMPONENT_480I_59D940,
    { 720,     576,    3200, 467,    864,    625,     12,     64,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  1350}, // TVIN_SIG_FMT_COMPONENT_576P_50D000,
    { 720,     288,    6400, 237,    864,    313,     12,     63,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  2700}, // TVIN_SIG_FMT_COMPONENT_576I_50D000,
    {1280,     720,    2224,  54,   1650,    750,    110,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7418}, // TVIN_SIG_FMT_COMPONENT_720P_59D940,
    {1280,     720,    2667,  54,   1980,    750,    440,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_COMPONENT_720P_50D000,
    {1920,    1080,    3707,  59,   2750,   1125,    638,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7418}, // TVIN_SIG_FMT_COMPONENT_1080P_23D976,
    {1920,    1080,    3704,  59,   2750,   1125,    638,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_COMPONENT_1080P_24D000,
    {1920,    1080,    3556,  59,   2640,   1125,    528,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_COMPONENT_1080P_25D000,
    {1920,    1080,    2963,  30,   2200,   1125,     88,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_COMPONENT_1080P_30D000,
    {1920,    1080,    1778,  59,   2640,   1125,    528,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_COMPONENT_1080P_50D000,
    {1920,    1080,    1481,  59,   2200,   1125,     88,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_COMPONENT_1080P_60D000,
    {1920,     540,    2966,  30,   2200,    563,     88,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED , 14850}, // TVIN_SIG_FMT_COMPONENT_1080I_29D970,
    {1920,     540,    3707,  59,   2750,    563,    638,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7418}, // TVIN_SIG_FMT_COMPONENT_1080I_47D952,
    {1920,     540,    3704,  59,   2750,    563,    638,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7418}, // TVIN_SIG_FMT_COMPONENT_1080I_48D000,
    {1920,     540,    3556,  59,   2640,    563,    528,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7425}, // TVIN_SIG_FMT_COMPONENT_1080I_50D000_A,
    {1920,     540,    3200,  59,   2304,    625,     32,    168,     23,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  7425}, // TVIN_SIG_FMT_COMPONENT_1080I_50D000_B,
    {1920,     540,    3200, 233,   2376,    625,    147,     66,      4,       1,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7200}, // TVIN_SIG_FMT_COMPONENT_1080I_50D000_C,
    {1920,     540,    2963,  89,   2200,    563,     88,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7425}, // TVIN_SIG_FMT_COMPONENT_1080I_60D000,
    {   0,       0,       0,   0,      0,      0,      0,      0,      0,       0,      TVIN_SYNC_POL_NULL    , TVIN_SYNC_POL_NULL    , TVIN_SCAN_MODE_NULL       ,     0}, // TVIN_SIG_FMT_COMPONENT_MAX,
//HDMI(CEA-861-E)
    { 640,     480,       0,   0,    800,    525,     16,     96,     10,       2,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2518}, // TVIN_SIG_FMT_HDMI_640x480P_60Hz,
    { 720,     480,       0,   0,    858,    525,     16,     62,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2700}, // TVIN_SIG_FMT_HDMI_720x480P_60Hz,
    {1280,     720,       0,   0,   1650,    750,    110,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1280x720P_60Hz,
    {1920,     540,       0,   0,   2200,    563,     88,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7425}, // TVIN_SIG_FMT_HDMI_1920x1080I_60Hz,
    {1440,     240,       0,   0,   1716,    263,     38,    124,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  2700}, // TVIN_SIG_FMT_HDMI_1440x480I_60Hz,
    {1440,     240,       0,   0,   1716,    262,     38,    124,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2700}, // TVIN_SIG_FMT_HDMI_1440x240P_60Hz,
    {2880,     240,       0,   0,   3432,    263,     76,    248,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  5400}, // TVIN_SIG_FMT_HDMI_2880x480I_60Hz,
    {2880,     240,       0,   0,   3432,    262,     76,    248,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_2880x240P_60Hz,
    {1440,     480,       0,   0,   1716,    525,     32,    124,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_1440x480P_60Hz,
    {1920,    1080,       0,   0,   2200,   1125,     88,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_HDMI_1920x1080P_60Hz,
    { 720,     576,       0,   0,    864,    625,     12,     64,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2700}, // TVIN_SIG_FMT_HDMI_720x576P_50Hz,
    {1280,     720,       0,   0,   1980,    750,    440,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1280x720P_50Hz,
    {1920,     540,       0,   0,   2640,    563,    528,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED ,  7425}, // TVIN_SIG_FMT_HDMI_1920x1080I_50Hz_A,
    {1440,     288,       0,   0,   1728,    313,     24,    126,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  2700}, // TVIN_SIG_FMT_HDMI_1440x576I_50Hz,
    {1440,     288,       0,   0,   1728,    312,     24,    126,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  2700}, // TVIN_SIG_FMT_HDMI_1440x288P_50Hz,
    {2880,     288,       0,   0,   3456,    313,     48,    252,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  5400}, // TVIN_SIG_FMT_HDMI_2880x576I_50Hz,
    {2880,     288,       0,   0,   3456,    312,     48,    252,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_2880x288P_50Hz,
    {1440,     576,       0,   0,   1728,    625,     24,    128,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_1440x576P_50Hz,
    {1920,    1080,       0,   0,   2640,   1125,    528,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_HDMI_1920x1080P_50Hz,
    {1920,    1080,       0,   0,   2750,   1125,    638,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1920x1080P_24Hz,
    {1920,    1080,       0,   0,   2640,   1125,    528,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1920x1080P_25Hz,
    {1920,    1080,       0,   0,   2200,   1125,     88,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1920x1080P_30Hz,
    {2880,     480,       0,   0,   3432,    525,     64,    248,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_HDMI_2880x480P_60Hz,
    {2880,     576,       0,   0,   3456,    625,     48,    256,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_HDMI_2880x576P_60Hz,
    {1920,     540,       0,   0,   2304,    625,     32,    168,     23,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  7200}, // TVIN_SIG_FMT_HDMI_1920x1080I_50Hz_B,
    {1920,     540,       0,   0,   2640,    563,    528,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED , 14850}, // TVIN_SIG_FMT_HDMI_1920x1080I_100Hz,
    {1280,     720,       0,   0,   1980,    750,    440,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_HDMI_1280x720P_100Hz,
    { 720,     576,       0,   0,    864,    625,     12,     64,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_720x576P_100Hz,
    {1440,     288,       0,   0,   1728,    313,     24,     12,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  5400}, // TVIN_SIG_FMT_HDMI_1440x576I_100Hz,
    {1920,     540,       0,   0,   2200,    563,     88,     44,      2,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_INTERLACED , 14850}, // TVIN_SIG_FMT_HDMI_1920x1080I_120Hz,
    {1280,     720,       0,   0,   1650,    750,    110,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 14850}, // TVIN_SIG_FMT_HDMI_1280x720P_120Hz,
    { 720,     480,       0,   0,    858,    525,     16,     62,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5400}, // TVIN_SIG_FMT_HDMI_720x480P_120Hz,
    {1440,     240,       0,   0,   1716,    263,     38,     12,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED ,  5400}, // TVIN_SIG_FMT_HDMI_1440x480I_120Hz,
    { 720,     576,       0,   0,    864,    625,     13,     64,      5,       5,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_HDMI_720x576P_200Hz,
    {1440,     288,       0,   0,   1728,    313,     24,     12,      2,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED , 10800}, // TVIN_SIG_FMT_HDMI_1440x576I_200Hz,
    { 720,     480,       0,   0,    858,    525,     16,     62,      9,       6,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_PROGRESSIVE, 10800}, // TVIN_SIG_FMT_HDMI_720x480P_240Hz,
    {1440,     240,       0,   0,   1716,    263,     38,     12,      4,       3,      TVIN_SYNC_POL_NEGATIVE, TVIN_SYNC_POL_NEGATIVE, TVIN_SCAN_MODE_INTERLACED , 10800}, // TVIN_SIG_FMT_HDMI_1440x480I_240Hz,
    {1280,     720,       0,   0,   3300,    750,   1760,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  5940}, // TVIN_SIG_FMT_HDMI_1280x720P_24Hz, 60
    {1280,     720,       0,   0,   3960,    750,   2420,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1280x720P_25Hz,
    {1280,     720,       0,   0,   3300,    750,   1760,     40,      5,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE,  7425}, // TVIN_SIG_FMT_HDMI_1280x720P_30Hz,
    {1920,    1080,       0,   0,   2200,   1125,     88,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 29700}, // TVIN_SIG_FMT_HDMI_1920x1080P_120Hz,
    {1920,    1080,       0,   0,   2640,   1125,    528,     44,      4,       5,      TVIN_SYNC_POL_POSITIVE, TVIN_SYNC_POL_POSITIVE, TVIN_SCAN_MODE_PROGRESSIVE, 29700}, // TVIN_SIG_FMT_HDMI_1920x1080P_100Hz,
    {   0,       0,       0,   0,      0,      0,      0,      0,      0,       0,      TVIN_SYNC_POL_NULL    , TVIN_SYNC_POL_NULL    , TVIN_SCAN_MODE_NULL       ,     0}, // TVIN_SIG_FMT_HDMI_MAX,
};
