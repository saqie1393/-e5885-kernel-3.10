/* Copyright (C) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef CHG_ADC_VOLT_TEMP_CFG_H
#define CHG_ADC_VOLT_TEMP_CFG_H

#include "bsp_version.h"
/* VT数组的元素个数与chg_charge_cust.h中的保持一致，不可随意更改 */
#define BATT_VT_SIZE (116)
#define USB_VT_SIZE (75)

typedef struct
{
    int   temperature;
    int   voltage;
}TEMP2ADC;

typedef struct
{
    unsigned int product_type;
    TEMP2ADC  g_adc_batt_therm_map[BATT_VT_SIZE];
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    TEMP2ADC  g_adc_usb_therm_map[USB_VT_SIZE];
#endif
}chg_adc_vt_type;

static chg_adc_vt_type g_chg_adc_vt_all[] = 
{
    {
        .product_type = HW_VER_PRODUCT_E5783h_92a,
        {
            {-30,      1654},   /*vol to temp*/
            {-29,      1647},   /*vol to temp*/
            {-28,      1639},   /*vol to temp*/
            {-27,      1632},   /*vol to temp*/
            {-26,      1624},   /*vol to temp*/
            {-25,      1615},   /*vol to temp*/
            {-24,      1607},   /*vol to temp*/
            {-23,      1598},   /*vol to temp*/
            {-22,      1589},   /*vol to temp*/
            {-21,      1580},   /*vol to temp*/
            {-20,      1570},   /*vol to temp*/
            {-19,      1560},   /*vol to temp*/
            {-18,      1550},   /*vol to temp*/
            {-17,      1539},   /*vol to temp*/
            {-16,      1528},   /*vol to temp*/
            {-15,      1517},   /*vol to temp*/
            {-14,      1506},   /*vol to temp*/
            {-13,      1494},   /*vol to temp*/
            {-12,      1482},   /*vol to temp*/
            {-11,      1470},   /*vol to temp*/
            {-10,      1457},    /*vol to temp*/
            {-9,       1444},    /*vol to temp*/
            {-8,       1431},    /*vol to temp*/
            {-7,       1418},    /*vol to temp*/
            {-6,       1404},    /*vol to temp*/
            {-5,       1390},    /*vol to temp*/
            {-4,       1376},    /*vol to temp*/
            {-3,       1361},    /*vol to temp*/
            {-2,       1346},    /*vol to temp*/
            {-1,       1332},    /*vol to temp*/
            {0,        1316},    /*vol to temp*/
            {1,        1301},    /*vol to temp*/
            {2,        1286},    /*vol to temp*/
            {3,        1270},    /*vol to temp*/
            {4,        1254},    /*vol to temp*/
            {5,        1238},    /*vol to temp*/
            {6,        1222},    /*vol to temp*/
            {7,        1205},    /*vol to temp*/
            {8,        1189},    /*vol to temp*/
            {9,        1172},    /*vol to temp*/
            {10,       1155},    /*vol to temp*/
            {11,       1139},    /*vol to temp*/
            {12,       1122},    /*vol to temp*/
            {13,       1105},    /*vol to temp*/
            {14,       1088},    /*vol to temp*/
            {15,       1070},    /*vol to temp*/
            {16,       1053},    /*vol to temp*/
            {17,       1036},    /*vol to temp*/
            {18,       1019},    /*vol to temp*/
            {19,       1002},    /*vol to temp*/
            {20,       985},    /*vol to temp*/
            {21,       968},    /*vol to temp*/
            {22,       951},    /*vol to temp*/
            {23,       934},    /*vol to temp*/
            {24,       917},    /*vol to temp*/
            {25,       900},    /*vol to temp*/
            {26,       883},    /*vol to temp*/
            {27,       867},    /*vol to temp*/
            {28,       850},    /*vol to temp*/
            {29,       817},    /*vol to temp*/
            {30,       801},    /*vol to temp*/
            {31,       785},    /*vol to temp*/
            {32,       769},    /*vol to temp*/
            {33,       753},    /*vol to temp*/
            {34,       738},    /*vol to temp*/
            {35,       723},    /*vol to temp*/
            {37,       707},    /*vol to temp*/
            {39,       693},    /*vol to temp*/
            {40,       663},    /*vol to temp*/
            {41,       649},    /*vol to temp*/
            {42,       635},    /*vol to temp*/
            {43,       621},    /*vol to temp*/
            {44,       607},    /*vol to temp*/
            {45,       593},    /*vol to temp*/
            {46,       580},    /*vol to temp*/
            {47,       567},    /*vol to temp*/
            {48,       554},    /*vol to temp*/
            {49,       541},    /*vol to temp*/
            {50,       529},    /*vol to temp*/
            {51,       517},    /*vol to temp*/
            {52,       505},    /*vol to temp*/
            {53,       493},    /*vol to temp*/
            {54,       481},    /*vol to temp*/
            {55,       470},    /*vol to temp*/
            {56,       459},    /*vol to temp*/
            {56,       454},    /*vol to temp*/
            {57,       448},    /*vol to temp*/
            {58,       438},    /*vol to temp*/
            {59,       427},    /*vol to temp*/
            {60,       417},    /*vol to temp*/
            {60,       407},     /*vol to temp*/
            {61,       397},     /*vol to temp*/
            {62,       388},     /*vol to temp*/
            {63,       379},     /*vol to temp*/
            {64,       370},     /*vol to temp*/
            {65,       361},     /*vol to temp*/
            {66,       353},     /*vol to temp*/
            {67,       344},     /*vol to temp*/
            {68,       336},     /*vol to temp*/
            {69,       328},     /*vol to temp*/
            {70,       320},     /*vol to temp*/
            {71,       312},     /*vol to temp*/
            {72,       305},     /*vol to temp*/
            {73,       298},     /*vol to temp*/
            {74,       291},     /*vol to temp*/
            {75,       284},     /*vol to temp*/
            {76,       277},     /*vol to temp*/
            {77,       270},     /*vol to temp*/
            {78,       264},     /*vol to temp*/
            {79,       257},     /*vol to temp*/
            {80,       251},     /*vol to temp*/
            {81,       245},     /*vol to temp*/
            {82,       240},     /*vol to temp*/
            {83,       234},     /*vol to temp*/
            {84,       228},     /*vol to temp*/
            {85,       223},     /*vol to temp*/
        },
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
        {
            {-10,      1665},   /*vol to temp*/
            {0,        1575},   /*vol to temp*/
            {5,        1516},   /*vol to temp*/
            {10,       1448},    /*vol to temp*/
            {15,       1370},    /*vol to temp*/
            {20,       1285},    /*vol to temp*/
            {25,       1192},    /*vol to temp*/
            {30,       1095},    /*vol to temp*/
            {35,       996},    /*vol to temp*/
            {40,       897},    /*vol to temp*/
            {45,       801},    /*vol to temp*/
            {50,       710},    /*vol to temp*/
            {55,       624},    /*vol to temp*/
            {60,       546},    /*vol to temp*/
            {65,       476},    /*vol to temp*/
            {66,       463},    /*vol to temp*/
            {67,       450},    /*vol to temp*/
            {68,       437},    /*vol to temp*/
            {69,       425},    /*vol to temp*/
            {70,       413},    /*vol to temp*/
            {71,       401},    /*vol to temp*/
            {72,       390},    /*vol to temp*/
            {73,       379},    /*vol to temp*/
            {74,       368},    /*vol to temp*/
            {75,       358},    /*vol to temp*/
            {76,       347},    /*vol to temp*/
            {77,       337},    /*vol to temp*/
            {78,       328},    /*vol to temp*/
            {79,       318},    /*vol to temp*/
            {80,       309},     /*vol to temp*/
            {81,       300},     /*vol to temp*/
            {82,       292},     /*vol to temp*/
            {83,       283},     /*vol to temp*/
            {84,       275},     /*vol to temp*/
            {85,       267},     /*vol to temp*/
            {86,       259},     /*vol to temp*/
            {87,       252},     /*vol to temp*/
            {88,       244},     /*vol to temp*/
            {89,       237},     /*vol to temp*/
            {90,       231},     /*vol to temp*/
            {91,       224},     /*vol to temp*/
            {92,       217},     /*vol to temp*/
            {93,       211},     /*vol to temp*/
            {94,       205},     /*vol to temp*/
            {95,       199},     /*vol to temp*/
            {96,       193},     /*vol to temp*/
            {97,       188},     /*vol to temp*/
            {98,       182},     /*vol to temp*/
            {99,       177},     /*vol to temp*/
            {100,      172},     /*vol to temp*/
            {101,      167},     /*vol to temp*/
            {102,      162},     /*vol to temp*/
            {103,      158},     /*vol to temp*/
            {104,      153},     /*vol to temp*/
            {105,      149},     /*vol to temp*/
            {106,      145},     /*vol to temp*/
            {107,      141},     /*vol to temp*/
            {108,      137},     /*vol to temp*/
            {109,      133},     /*vol to temp*/
            {110,      129},     /*vol to temp*/
            {111,      125},     /*vol to temp*/
            {112,      122},     /*vol to temp*/
            {113,      118},     /*vol to temp*/
            {114,      115},     /*vol to temp*/
            {115,      112},     /*vol to temp*/
            {116,      109},     /*vol to temp*/
            {117,      106},     /*vol to temp*/
            {118,      103},     /*vol to temp*/
            {119,      100},     /*vol to temp*/
            {120,      98},     /*vol to temp*/
            {121,      95},     /*vol to temp*/
            {122,      92},     /*vol to temp*/
            {123,      90},     /*vol to temp*/
            {124,      87},     /*vol to temp*/
            {125,      85},     /*vol to temp*/
        },
#endif
    },
    /* 归一新产品在下面赋值  */
};

#endif
