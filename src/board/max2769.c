/*
 * Copyright (C) 2011-2014 Swift Navigation Inc.
 * Contact: Fergus Noble <fergus@swift-nav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>

#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

#include "../peripherals/spi.h"
#include "../settings.h"
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

u32 max2769_conf1;
u32 max2769_conf2;
u32 max2769_conf3;
u32 max2769_pllconf;
u32 max2769_div;
u32 max2769_fdiv;
u32 max2769_strm;
u32 max2769_clk;
u32 max2769_test1;
u32 max2769_test2;

/** Setup MAX2769 GPIOs and write default settings to MAX2769 registers. */
void max2769_configure(void)
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

  max2769_conf1 = MAX2769_CONF1_CHIPEN |
                  MAX2769_CONF1_ILNA1(15) |
                  MAX2769_CONF1_ILNA2(3) |
                  MAX2769_CONF1_ILO(3) |
                  MAX2769_CONF1_IMIX(3) |
                  MAX2769_CONF1_MIXPOLE_13MHZ |
                  MAX2769_CONF1_MIXEN |
                  MAX2769_CONF1_ANTEN |
                  //MAX2769_CONF1_FCEN(7) |  /* 1 MHz IF filter center freq. */
                  //MAX2769_CONF1_FCEN(43) | /* 2 Mhz IF filter center freq. */
                  MAX2769_CONF1_FCEN(21) |   /* 4 Mhz IF filter center freq. */
                  //MAX2769_CONF1_FCEN(42) | /* 8 Mhz IF filter center freq. */
                  MAX2769_CONF1_FBW_18MHZ |
                  MAX2769_CONF1_F3OR5_3 |
                  MAX2769_CONF1_FCENX_LP |
                  MAX2769_CONF1_FGAIN_HIGH |
                  MAX2769_CONF1_LNAMODE_LNA2;
  max2769_write(MAX2769_CONF1, max2769_conf1);

  max2769_conf2 = MAX2769_CONF2_IQEN |
                  MAX2769_CONF2_GAINREF(170) |  /* optimal for 2 bits */
                  //MAX2769_CONF2_GAINREF(82) | /* optimal for 3 bits */
                  MAX2769_CONF2_AGCMODE_INDEP |
                  MAX2769_CONF2_FORMAT_SIGN_MAG |
                  MAX2769_CONF2_BITS_2 |
                  MAX2769_CONF2_DRVCFG_CMOS |
                  MAX2769_CONF2_LOEN;
  max2769_write(MAX2769_CONF2, max2769_conf2);

  max2769_conf3 = MAX2769_CONF3_RESERVED |
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
                  0;
  max2769_write(MAX2769_CONF3, max2769_conf3);

  max2769_pllconf = MAX2769_PLLCONF_RESERVED |
                  MAX2769_PLLCONF_VCOEN |
                  MAX2769_PLLCONF_REFOUTEN |
                  //MAX2769_PLLCONF_REFDIV_DIV_2 |  /* 8.184 MHz sample rate */
                  MAX2769_PLLCONF_REFDIV_DIV_NONE | /* 16.368 MHz sample rate */
//                  MAX2769_PLLCONF_REFDIV_2X | /* 32.736  MHz sample rate */
                  MAX2769_PLLCONF_IXTAL_BUFF_NORMAL |
                  MAX2769_PLLCONF_XTALCAP(0b10000) |
                  MAX2769_PLLCONF_LDMUX(0) |
                  MAX2769_PLLCONF_ICP_1MA |
                  MAX2769_PLLCONF_CPTEST(0);
//                  MAX2769_PLLCONF_CPTEST(0) |
//                  MAX2769_PLLCONF_INTPLL;
  max2769_write(MAX2769_PLLCONF, max2769_pllconf);

  max2769_div =
//                MAX2769_DIV_NDIV(1536) | /* 2 * 1.023 MHz IF */
//                MAX2769_DIV_RDIV(16);
                MAX2769_DIV_NDIV(195) |  /* 4 * 1.023 MHz IF */
//                MAX2769_DIV_NDIV(95) |  /* 4 * 1.023 MHz IF */
                MAX2769_DIV_RDIV(4);
  max2769_write(MAX2769_DIV, max2769_div);

  max2769_fdiv = MAX2769_FDIV_RESERVED |
//                 MAX2769_FDIV_FDIV(0);
                 MAX2769_FDIV_FDIV(58982);
  max2769_write(MAX2769_FDIV, max2769_fdiv);

  max2769_clk = MAX2769_CLK_L_CNT(1) |
                MAX2769_CLK_M_CNT(218103) |
                MAX2769_CLK_SERCLK;
  max2769_write(MAX2769_CLK, max2769_clk);

}

antenna_type_t antenna = AUTO;

bool antenna_changed(struct setting *s, const char *val)
{
  if (s->type->from_string(s->type->priv, s->addr, s->len, val))
  {
    max2769_conf1 &= MAX2769_CONF1_LNAMODE_MASK;

    switch (antenna) {
    default:
    case AUTO:
      max2769_conf1 |= MAX2769_CONF1_LNAMODE_GATED;
      break;
    case PATCH:
      max2769_conf1 |= MAX2769_CONF1_LNAMODE_LNA1;
      break;
    case EXTERNAL:
      max2769_conf1 |= MAX2769_CONF1_LNAMODE_LNA2;
      break;
    }
    max2769_write(MAX2769_CONF1, max2769_conf1);
    printf("Antenna changed to: %s\n", val);

    return true;
  }
  return false;
}

/* Setup user settings hooks etc. */
void max2769_setup(void)
{
  static const char const *antenna_enum[] = {"Auto", "Patch", "External", NULL};
  static struct setting_type antenna_setting;
  int TYPE_ANTENNA = settings_type_register_enum(antenna_enum, &antenna_setting);
  SETTING_NOTIFY("frontend", "antenna_selection", antenna, TYPE_ANTENNA, antenna_changed);
}

/** \} */

/** \} */

