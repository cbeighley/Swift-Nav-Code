/*
 * Copyright (C) 2011-2013 Swift Navigation Inc.
 * Contact: Fergus Noble <fergus@swift-nav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

#include "../peripherals/spi.h"
#include "max2769.h"

/** \addtogroup board
 * \{ */

/** \defgroup max MAX2769
 * Interface to configure the MAX2769 RF Front-end.
 * \{ */

/** Write to one of the MAX2769 registers.
 *
 * \param addr MAX2769 register to write to.
 * \param data Data to write to register (28 bits).
 */
void max2769_write(u8 addr, u32 data)
{
  u32 write_word = ((data << 4) & 0xFFFFFFF0) | (addr & 0x0F);

  spi_slave_select(SPI_SLAVE_FRONTEND);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >> 24) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >> 16) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >>  8) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >>  0) & 0xFF);
  spi_slave_deselect();
}

/** Setup MAX2769 GPIOs and write default settings to MAX2769 registers. */
void max2769_setup(void)
{
  /* Setup MAX2769 PGM (PB8) - low */
  RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO8);
  gpio_clear(GPIOB, GPIO8);

  /* Setup MAX2769 NSHDN (PB9) - high */
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO9);
  gpio_set(GPIOB, GPIO9);

  /* Setup MAX2769 NIDLE (PB10) - high */
  gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO10);
  gpio_set(GPIOB, GPIO10);

  max2769_write(MAX2769_CONF1, MAX2769_CONF1_CHIPEN |
                MAX2769_CONF1_ILNA1(15) |
                MAX2769_CONF1_ILNA2(3) |
                MAX2769_CONF1_ILO(3) |
                MAX2769_CONF1_IMIX(3) |
//                MAX2769_CONF1_MIXPOLE_13MHZ |
                MAX2769_CONF1_MIXPOLE_36MHZ |
                MAX2769_CONF1_MIXEN |
                MAX2769_CONF1_ANTEN |
                //MAX2769_CONF1_FCEN(7) |  /* 1 MHz IF filter center freq. */
                //MAX2769_CONF1_FCEN(43) | /* 2 Mhz IF filter center freq. */
//                MAX2769_CONF1_FCEN(21) |   /* 4 Mhz IF filter center freq. */
//                MAX2769_CONF1_FCEN(42) | /* 8 Mhz IF filter center freq. */
                MAX2769_CONF1_FBW_18MHZ |
//                MAX2769_CONF1_FBW_8MHZ |
//                MAX2769_CONF1_FBW_4_2MHZ |
//                MAX2769_CONF1_FBW_2_5MHZ |
                MAX2769_CONF1_F3OR5_5 |
//                MAX2769_CONF1_FCENX_BP |
                MAX2769_CONF1_FGAIN_HIGH |
                MAX2769_CONF1_LNAMODE_GATED);

  max2769_write(MAX2769_CONF2,
                MAX2769_CONF2_IQEN |
                MAX2769_CONF2_GAINREF(170) |  /* optimal for 2 bits */
                //MAX2769_CONF2_GAINREF(82) | /* optimal for 3 bits */
                MAX2769_CONF2_AGCMODE_INDEP |
//                MAX2769_CONF2_AGCMODE_LOCKED |
                MAX2769_CONF2_FORMAT_SIGN_MAG |
//                MAX2769_CONF2_BITS_1 |
                MAX2769_CONF2_BITS_2 |
//                MAX2769_CONF2_BITS_3 |
                MAX2769_CONF2_DRVCFG_CMOS |
                MAX2769_CONF2_LOEN
                );

  max2769_write(MAX2769_CONF3,
                MAX2769_CONF3_RESERVED |
                MAX2769_CONF3_GAININ(58) |
                MAX2769_CONF3_FSLOWEN |
                MAX2769_CONF3_ADCEN |
                MAX2769_CONF3_DRVEN |
                MAX2769_CONF3_FOFSTEN |
                MAX2769_CONF3_FILTEN |
                MAX2769_CONF3_FHIPEN |
                MAX2769_CONF3_PGAIEN |
                MAX2769_CONF3_PGAQEN |
                /* STRM stuff was set before but its unused,
                 * can leave as zeros. */
                0
                );

  max2769_write(MAX2769_PLLCONF,
                MAX2769_PLLCONF_RESERVED |
                MAX2769_PLLCONF_VCOEN |
                MAX2769_PLLCONF_REFOUTEN |
//                MAX2769_PLLCONF_REFDIV_DIV_2 |  /* 8.184 MHz sample rate */
//                MAX2769_PLLCONF_REFDIV_DIV_NONE | /* 16.368 MHz sample rate */
                MAX2769_PLLCONF_REFDIV_2X |  /* 32*1.023 MHz sample rate */
                MAX2769_PLLCONF_IXTAL_BUFF_NORMAL |
                MAX2769_PLLCONF_XTALCAP(0b10000) |
                MAX2769_PLLCONF_LDMUX(0) |
                MAX2769_PLLCONF_ICP_1MA |
                MAX2769_PLLCONF_CPTEST(0) |
                MAX2769_PLLCONF_INTPLL
                );

  max2769_write(MAX2769_DIV,
//                MAX2769_DIV_NDIV(1554) |  /* -14 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1555) |  /* -15 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1548) |  /* -8 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1540) | /* 0 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1538) | /* 2 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1536) |  /* 4 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1532) |  /* 8 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1530) |  /* 10 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1528) |  /* 12 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1526) |  /* 14 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1525) |  /* 15 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(1558) | /* GLONASS 1602-1.023*1558 center */
                MAX2769_DIV_NDIV(1554) | /* GLONASS 1602-1.023*1554 center */
//                MAX2769_DIV_NDIV(1562) | /* GLONASS 1602-1.023*1562 center */
//                MAX2769_DIV_NDIV(1566) | /* GLONASS 1602-1.023*1566 center */
                MAX2769_DIV_RDIV(16)
                );

  max2769_write(MAX2769_FDIV,
                MAX2769_FDIV_RESERVED |
                MAX2769_FDIV_FDIV(0x80000)
                );

  max2769_write(MAX2769_CLK,
                MAX2769_CLK_L_CNT(1) |
                MAX2769_CLK_M_CNT(4095) |
                MAX2769_CLK_SERCLK
                );

}

/** \} */

/** \} */

