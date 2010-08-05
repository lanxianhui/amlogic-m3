/*
 * AMLOGIC Audio/Video streaming port driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef VMJPEG_MC_H
#define VMJPEG_MC_H 

static const u32 vmjpeg_mc[] __attribute__ ((aligned (UCODE_ALIGN))) =
{
0x00000000,0x00000000,0x06801801,0x06800000,0x0d000001,0x07400040,
0x0680c000,0x060c2900,0x06800000,0x060c0800,0x080c0002,0x06bfbfc0,
0x06030400,0x0809c900,0x0809c800,0x0c00f000,0x00000000,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x0649c00c,
0x07c0118c,0x0649c10c,0x0c780980,0x00400000,0x00000000,0x00000000,
0x00000000,0x00000000,0x0cc00000,0x00400000,0x0cc00000,0x00400000,
0x0cc00000,0x00400000,0x0cc00000,0x00400000,0x0cc00000,0x00400000,
0x0cc00000,0x00400000,0x0cc00000,0x00400000,0x0cc00000,0x00400000,
0x0c780640,0x064c0007,0x0cc00000,0x00400000,0x0cc00000,0x00400000,
0x0cc00000,0x00400000,0x00000000,0x00000000,0x0cc00000,0x00400000,
0x00000000,0x00000000,0x0cc00000,0x00400000,0x04807410,0x0aafc010,
0x0649c810,0x0c00e040,0x00800000,0x0c7ffec0,0x00400000,0x0c7ffdc0,
0x080c0000,0x0be0c807,0x0bef4047,0x0c7ffcc0,0x09c09147,0x060c0009,
0x0c00e580,0x0680020f,0x040001cf,0x0aaf7fc7,0x0aa27607,0x0aa0f647,
0x0c7823c0,0x00000000,0x06800007,0x07c00c07,0x080c0800,0x0c7856c0,
0x00000000,0x0c00e240,0x0680020f,0x0400020f,0x0a633fc8,0x06800048,
0x07c00c08,0x06bfffc8,0x07c01048,0x06800008,0x07c01008,0x0c00dfc0,
0x0680020f,0x040001cf,0x0c7ffa40,0x00000000,0x0c785280,0x00000000,
0x07800c48,0x0aa08a08,0x0c7810c0,0x0c00dd40,0x0680040f,0x040001cf,
0x04402208,0x07c00c48,0x0aa08007,0x0c780f00,0x0c00db80,0x0680040f,
0x040001cf,0x04402208,0x07c00c48,0x0aa08007,0x0c780d40,0x069b5a89,
0x0c00d980,0x0680040f,0x040001cf,0x04402208,0x07c00c48,0x0a42e247,
0x0c00d800,0x0680040f,0x040001cf,0x04402208,0x07c00c48,0x069c19c9,
0x0a808247,0x069c1889,0x0a803247,0x0c780900,0x00000000,0x06800049,
0x0c780100,0x07c01209,0x06800009,0x07c01209,0x0c00d400,0x0680040f,
0x040001cf,0x0c00d340,0x0680040f,0x040001cf,0x0c00d280,0x0680040f,
0x040001cf,0x0c00d1c0,0x0680040f,0x040001cf,0x0c00d100,0x0680040f,
0x040001cf,0x0440a208,0x07c00c48,0x0a63c007,0x068001c7,0x0c00cf40,
0x0680040f,0x040001cf,0x04402208,0x07c00c48,0x0a620007,0x068001c7,
0x078011c7,0x0aa141c7,0x06800187,0x06800009,0x0c781280,0x07c01209,
0x07c011c7,0x0c7811c0,0x00000000,0x07800c49,0x0a614009,0x00000000,
0x06800009,0x0c783e40,0x07c00c09,0x07800c08,0x09808048,0x07c00c08,
0x0ba6d049,0x00000000,0x0c00c880,0x0680020f,0x0400020f,0x07c00c88,
0x06800007,0x06c00047,0x060e0207,0x0be283c8,0x06800808,0x0c00c640,
0x0680040f,0x040001cf,0x0d07ff48,0x060e0207,0x04441249,0x0c7ffbc0,
0x07c00c49,0x0c00c440,0x0680040f,0x040001cf,0x0d07ff48,0x060e0307,
0x04441249,0x0c7ff9c0,0x07c00c49,0x0aa0c009,0x0c00afc0,0x060c2009,
0x0c783580,0x00000000,0x06800007,0x0c7834c0,0x07c00c07,0x07800c08,
0x0a2e8048,0x048f8207,0x0a617408,0x07800c08,0x0a2e0208,0x0c785e40,
0x00000000,0x0c00be40,0x0680040f,0x0400020f,0x04402208,0x07c00c48,
0x0aac0007,0x0aa63007,0x0aa37747,0x046db207,0x0c57f0c0,0x046c4207,
0x0c503300,0x046da207,0x0c504c00,0x046e1207,0x0c57dc80,0x00000000,
0x07800c48,0x0c782dc0,0x00000000,0x07800c49,0x0aa0c089,0x0c7ff680,
0x00000000,0x0c00b840,0x0680040f,0x0400020f,0x07c01048,0x0c782b40,
0x07c01008,0x0811010c,0x0c00b680,0x0680020f,0x040001cf,0x0c00b5c0,
0x0680040f,0x0400020f,0x0c00b500,0x0680040f,0x040001cf,0x0649c209,
0x05804249,0x0aa18009,0x0b005248,0x00000000,0x06800009,0x0c7826c0,
0x07c00c09,0x078011c9,0x0a220109,0x0649c209,0x05804249,0x0aa3c009,
0x0a80d209,0x06800009,0x0c780300,0x00000000,0x0649c209,0x05804249,
0x05801249,0x0a406209,0x06800009,0x078011c9,0x0a20c049,0x068000c9,
0x06800089,0x07c011c9,0x07c004c7,0x07c00508,0x06800009,0x07c005c9,
0x07c00589,0x07c00549,0x07800989,0x0480f249,0x05404249,0x0609c309,
0x07800c49,0x0400f1c7,0x058041c7,0x065a4807,0x065a4708,0x065a4812,
0x0a4fd487,0x00000000,0x064c4612,0x0b403487,0x00000000,0x04001208,
0x064c4207,0x064c400a,0x02407287,0x040081c7,0x0e000207,0x02412292,
0x00000000,0x00000000,0x0f0001c0,0x0f010200,0x097081c8,0x02007487,
0x064c4308,0x02407207,0x07c01487,0x058101c7,0x07c014c7,0x078004c7,
0x07800508,0x040071c7,0x058031c7,0x060c0907,0x07c00607,0x07c002c7,
0x04007208,0x05803208,0x07c00308,0x0c00a140,0x0680020f,0x040001cf,
0x0aa180c7,0x00000000,0x06800007,0x07c00c07,0x0c781380,0x00000000,
0x0aa0c3c9,0x0c781140,0x00000000,0x0c009e40,0x0680020f,0x040001cf,
0x07c00cc7,0x0c009d40,0x0680010f,0x040001cf,0x07c00d07,0x0aa28047,
0x06800049,0x0a600087,0x06800089,0x078002c8,0x04001208,0x05801208,
0x07c002c8,0x05401208,0x07c00608,0x0c0099c0,0x0680010f,0x040001cf,
0x07c00d47,0x0aa1c047,0x0a600087,0x07800308,0x04001208,0x05801208,
0x07c00308,0x05401249,0x07c00409,0x0c0096c0,0x0680020f,0x040001cf,
0x07c00d87,0x0c0095c0,0x0680020f,0x040001cf,0x07c00dc7,0x0c0094c0,
0x0680010f,0x040001cf,0x07c00e07,0x0c0093c0,0x0680010f,0x040001cf,
0x07c00e47,0x0c0092c0,0x0680020f,0x040001cf,0x07c00e87,0x0c0091c0,
0x0680020f,0x040001cf,0x07c00ec7,0x0c0090c0,0x0680010f,0x040001cf,
0x07c00f07,0x0c008fc0,0x0680010f,0x040001cf,0x07c00f47,0x0c008ec0,
0x0680020f,0x040001cf,0x07c00f87,0x0c780140,0x0aa10009,0x060c2009,
0x0c007ac0,0x00000000,0x0c780080,0x00000000,0x07800c07,0x0a220047,
0x080c0800,0x0c008b40,0x0680020f,0x040001cf,0x0a6f7fc7,0x0c7fa4c0,
0x00000000,0x080c0002,0x0c7fa000,0x00000000,0x0c7818c0,0x00000000,
0x07800c09,0x09809089,0x07c00c09,0x07800c49,0x0bae8449,0x00000000,
0x0c008700,0x0680020f,0x0400020f,0x04401249,0x0a20bc08,0x0c780900,
0x0480f208,0x0aa0c008,0x0aa14048,0x0c780000,0x06801804,0x0c780100,
0x06801c06,0x06802404,0x06802806,0x06800005,0x06800007,0x0680040a,
0x07400187,0x04001186,0x0c008200,0x0680020f,0x0400020f,0x02007207,
0x02005205,0x05401145,0x0340b285,0x058022cb,0x0740010b,0x0d07fd4a,
0x04001104,0x04410249,0x0b83b1c9,0x024091c9,0x0c007e80,0x0680020f,
0x0400020f,0x07400188,0x0d07ff07,0x04001186,0x0c7ff580,0x0480f208,
0x0aa14008,0x0aa1c048,0x06800007,0x0c7fefc0,0x07c00c07,0x06800004,
0x0c780100,0x06803406,0x06800044,0x06805806,0x060c2104,0x06800005,
0x06800007,0x0680040a,0x0c007900,0x0680020f,0x0400020f,0x02007207,
0x0ba0c08a,0x00000000,0x060c2607,0x02005205,0x05401145,0x0340b285,
0x058022cb,0x0ba0c08a,0x00000000,0x060c220b,0x0d07fc8a,0x00000000,
0x04410249,0x0b8121c9,0x024091c9,0x0b220047,0x05801107,0x0c0073c0,
0x0680040f,0x0400020f,0x07400188,0x0d07ff04,0x04001186,0x0a21c047,
0x00000000,0x0c0071c0,0x0680020f,0x0400020f,0x05408208,0x07400188,
0x0c7fe900,0x0aa0c009,0x0c005dc0,0x060c2009,0x0c7fe380,0x00000000,
0x07800c09,0x098090c9,0x07c00c09,0x07801209,0x0aa0c049,0x06800009,
0x06801009,0x060c0809,0x07800c49,0x0c006cc0,0x0680020f,0x0400020f,
0x04401249,0x0b2140c8,0x00000000,0x06800007,0x0c7fdf00,0x07c00c07,
0x07c00108,0x06800146,0x0c006a00,0x0680020f,0x040001cf,0x07800d8a,
0x07800cc4,0x0a808107,0x07800405,0x07800e8a,0x07800dc4,0x0a804107,
0x06810045,0x06814045,0x07800f8a,0x0958414a,0x07400185,0x04001186,
0x0c006600,0x0680020f,0x040001cf,0x07400187,0x04402249,0x0d07fac8,
0x04001186,0x0c006440,0x0680040f,0x040001cf,0x0c006380,0x0680020f,
0x040001cf,0x04403249,0x0aa0c009,0x0c005000,0x060c2009,0x080c0a10,
0x080c2c01,0x06800004,0x07c00004,0x07c00044,0x080c2a00,0x06800004,
0x098047c4,0x060c2d04,0x098043c4,0x060c2f04,0x06800004,0x07c00084,
0x07800084,0x07800146,0x0aa1c004,0x07800185,0x078001c6,0x0aa10044,
0x07800205,0x07800246,0x07800285,0x09184106,0x0aa0c004,0x0680050a,
0x0680070a,0x09104106,0x0ba0c104,0x060c2a04,0x04403104,0x07c00484,
0x09003106,0x07c00444,0x09084105,0x0aa0c004,0x06801806,0x06802406,
0x07c00386,0x09004105,0x060c2104,0x0aa0c004,0x06803404,0x06805804,
0x07c003c4,0x06800004,0x07c00344,0x07800384,0x064c2a07,0x04004144,
0x07000146,0x0b82c187,0x04006144,0x07000146,0x0b823187,0x04007144,
0x07000146,0x0b838187,0x068001c6,0x04008144,0x07000146,0x0b834187,
0x06800206,0x04009144,0x07000146,0x0b830187,0x06800246,0x0400a144,
0x07000146,0x0b82c187,0x06800286,0x0400b144,0x07000146,0x0b828187,
0x068002c6,0x0400c144,0x07000146,0x0b824187,0x06800306,0x0400d144,
0x07000146,0x0b820187,0x06800346,0x0400e144,0x07000146,0x0b81c187,
0x06800386,0x0c780680,0x068003c6,0x04005144,0x07000146,0x0b816187,
0x06800146,0x0c780500,0x06800186,0x04002144,0x07000146,0x0b807187,
0x04003144,0x07000146,0x0b80d187,0x068000c6,0x0c7802c0,0x06800106,
0x04001144,0x07000146,0x0b407187,0x06800086,0x07000106,0x0b404187,
0x06800046,0x0c780080,0x06800006,0x04001206,0x0c004580,0x040003c8,
0x040001cf,0x0aa20006,0x02005184,0x04401145,0x07000145,0x068003c8,
0x02408188,0x03805205,0x02407147,0x04010104,0x02006184,0x07000186,
0x020061c6,0x04010104,0x02006184,0x07000186,0x064c2404,0x00000000,
0x0a20c3c4,0x0c7fff40,0x00000000,0x060c2306,0x060e000a,0x078003c9,
0x064c2503,0x0a2fe003,0x0a211003,0x09005103,0x0c780240,0x00000000,
0x09127203,0x02008209,0x07000208,0x060c2708,0x0f8001c4,0x0c7ffd40,
0x080c2440,0x078005c4,0x0be14044,0x00000000,0x04c01104,0x0c002b40,
0x07c005c4,0x06800045,0x07800007,0x07800048,0x07800484,0x0a670004,
0x07800d06,0x0aa18046,0x07800346,0x054011c7,0x04801186,0x02c07187,
0x0c780240,0x07800d46,0x0aa3c046,0x07800346,0x05401208,0x04801186,
0x09461146,0x02c08188,0x0c780240,0x07800d46,0x0aa1c046,0x00000000,
0x05401208,0x07800346,0x09021186,0x09461146,0x02c08188,0x0649c306,
0x09104186,0x0c780340,0x094c4146,0x0649c306,0x07800d0a,0x0a60c04a,
0x09142246,0x04001249,0x09502149,0x07800d4a,0x0a60c04a,0x09102246,
0x04001249,0x094c2149,0x0aa0c007,0x0c780140,0x09482144,0x09805045,
0x0c780080,0x09482144,0x06491e06,0x0befc146,0x00000000,0x06091c07,
0x06091d08,0x06091b05,0x07800606,0x0aa14004,0x07800d08,0x0aa0c048,
0x00000000,0x05801186,0x040011c7,0x0b807187,0x09805285,0x06491e06,
0x0befc146,0x00000000,0x06091c07,0x06091b05,0x080c2480,0x07800344,
0x04001104,0x07800445,0x0b403144,0x0c7fd180,0x07c00344,0x07800084,
0x04001104,0x07800105,0x0b403144,0x0c7fc7c0,0x07c00084,0x07800004,
0x04001104,0x078002c5,0x0b813144,0x07c00004,0x06800004,0x07c00004,
0x07800044,0x04001104,0x0c001b80,0x00000000,0x07800547,0x07800d46,
0x02007187,0x07c00547,0x07800305,0x0b40f144,0x07c00044,0x0ba10207,
0x00000000,0x06800004,0x07c00544,0x07801004,0x0aa0c004,0x0aa10044,
0x04401104,0x0c7fc040,0x07c01004,0x07801044,0x0c7f94c0,0x07c01004,
0x06800104,0x07c005c4,0x064c2404,0x0befc3c4,0x00000000,0x0803ffa5,
0x06491e04,0x0befc184,0x00000000,0x08091007,0x00000000,0x00000000,
0x08091000,0x080c0800,0x06800004,0x07c00c04,0x0480710c,0x0780098d,
0x0480134d,0x0946110d,0x078011cd,0x0480734d,0x0948310d,0x07801488,
0x078014c7,0x09610207,0x0609ca08,0x0609c904,0x081f7401,0x0c000ec0,
0x00000000,0x0649c904,0x0a6f4004,0x00000000,0x07801184,0x04403104,
0x07c01184,0x0580330c,0x0c000c80,0x00000000,0x0aaf800c,0x00000000,
0x0649c204,0x0480f104,0x07c00984,0x0c7f86c0,0x00000000,0x064c2908,
0x09808288,0x060c2908,0x00000000,0x0c000940,0x00000000,0x064c2908,
0x09141208,0x0a6f0008,0x0cc00000,0x00000000,0x048071cc,0x044011c7,
0x054011c7,0x06801288,0x020081c8,0x07000207,0x04001208,0x07000208,
0x09610207,0x06092d08,0x06802a87,0x06092107,0x08091600,0x08091700,
0x078004c7,0x06091407,0x078004c7,0x06091207,0x07800507,0x06091307,
0x0649c308,0x09102208,0x03807207,0x06091507,0x06a04007,0x0649c308,
0x048ff208,0x02c07207,0x0cc00000,0x06091107,0x0649c80d,0x0480734d,
0x0aa1c00d,0x0780118e,0x0340d38d,0x02c0c34c,0x0400338e,0x07c0118e,
0x0809c800,0x0cc00000,0x00000000,0x0649c400,0x05810040,0x09010000,
0x07c01281,0x07c012c0,0x0649c500,0x05810040,0x09010000,0x07c01301,
0x07c01340,0x0649c600,0x05810040,0x09010000,0x07c01381,0x07c013c0,
0x0649c700,0x05810040,0x09010000,0x07c01401,0x0cc00000,0x07c01440,
0x0c07f800,0x00000000,0x064c4711,0x0b2f6011,0x00000000,0x0cc00000,
0x0f8003cf,0x0609ce28,0x0c07f600,0x00000000,0x0649ce28,0x0a6f4028,
0x00000000,0x0cc00000,0x00000000};
#endif /* VMJPEG_MC_H */
