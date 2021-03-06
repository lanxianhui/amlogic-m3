/*
 * gf256mul.c
 *
 *  Created on: 2011-9-22
 *      Author: jerry.yu
 */



/*
    This file is part of the Crypto-avr-lib/microcrypt-lib.
    Copyright (C) 2008  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * \file     gf256mul.c
 * \email    daniel.otte@rub.de
 * \author   Daniel Otte
 * \date     2009-01-13
 * \license  GPLv3 or later
 *
 */

#include <linux/types.h>
#include "gf256mul.h"

uint8_t gf256mul(uint8_t a, uint8_t b, uint8_t reducer){
        uint8_t t,ret=0;
        while(a){
                if(a&1)
                        ret ^= b;
                t=a&0x80;
                b<<=1;
                if(t)
                        b^=reducer;
                a>>=1;
        }
        return ret;
}
