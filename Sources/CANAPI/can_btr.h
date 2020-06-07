/*
 *  CAN Interface API, Version 3 (Bit-rate Conversion)
 *
 *  Copyright (C) 2017-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        CANAPI_Bitrates.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces (Bit-rate Conversion)
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 902 $
 *
 *  @defgroup    can_btr CAN Bit-rate Conversion
 *  @{
 */
#ifndef CAN_BTR_H_INCLUDED
#define CAN_BTR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  -----------------------------------------------
 */

#if (OPTION_CANAPI_COMPANIONS != 0)     // set it in the build environment!
#include "CANAPI_Types.h"               //   use CAN API V3 types and defines
#else                                   // otherwise:
#define CANBTR_STANDALONE_VARIANT       //   don't include CAN API V3 headers
#include <stdint.h>                     //   C99 header for sized integer types
#include <stdbool.h>                    //   C99 header for boolean type
#endif

/*  -----------  options  ------------------------------------------------
 */

/*#define CANBTR_PEAK_FREQUENCIES       // use only supported PCANBasic frequencies */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN 2.0 Predefined Bit-rates
 *  @brief Indexes to predefined bit-rates (CAN 2.0 only!)
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_INDEX_1M               (0)  /**< bit-rate: 1000 kbit/s */
#define BTR_INDEX_800K            (-1)  /**< bit-rate:  800 kbit/s */
#define BTR_INDEX_500K            (-2)  /**< bit-rate:  500 kbit/s */
#define BTR_INDEX_250K            (-3)  /**< bit-rate:  250 kbit/s */
#define BTR_INDEX_125K            (-4)  /**< bit-rate:  125 kbit/s */
#define BTR_INDEX_100K            (-5)  /**< bit-rate:  100 kbit/s */
#define BTR_INDEX_50K             (-6)  /**< bit-rate:   50 kbit/s */
#define BTR_INDEX_20K             (-7)  /**< bit-rate:   20 kbit/s */
#define BTR_INDEX_10K             (-8)  /**< bit-rate:   10 kbit/s */
#else
#define BTR_INDEX_1M              (CANBTR_INDEX_1M)
#define BTR_INDEX_800K            (CANBTR_INDEX_800K)
#define BTR_INDEX_500K            (CANBTR_INDEX_500K)
#define BTR_INDEX_250K            (CANBTR_INDEX_250K)
#define BTR_INDEX_125K            (CANBTR_INDEX_125K)
#define BTR_INDEX_100K            (CANBTR_INDEX_100K)
#define BTR_INDEX_50K             (CANBTR_INDEX_50K)
#define BTR_INDEX_20K             (CANBTR_INDEX_20K)
#define BTR_INDEX_10K             (CANBTR_INDEX_10K)
#endif
/** @} */

/** @name  CAN Controller Frequencies
 *  @brief Frequencies for calculation of the bit-rate
 *  @note  Usable frequencies depend on the microcontroller used
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_FREQ_80MHz       80000000   /**< frequency: 80 MHz */
#define BTR_FREQ_60MHz       60000000   /**< frequency: 60 MHz */
#define BTR_FREQ_40MHz       40000000   /**< frequency: 40 MHz */
#define BTR_FREQ_30MHz       30000000   /**< frequency: 30 MHz */
#define BTR_FREQ_24MHz       24000000   /**< frequency: 24 MHz */
#define BTR_FREQ_20MHz       20000000   /**< frequency: 20 MHz */
#define BTR_FREQ_SJA1000      8000000   /**< frequency:  8 MHz */
#else
#define BTR_FREQ_80MHz            (CANBTR_FREQ_80MHz)
#define BTR_FREQ_60MHz            (CANBTR_FREQ_60MHz)
#define BTR_FREQ_40MHz            (CANBTR_FREQ_40MHz)
#define BTR_FREQ_30MHz            (CANBTR_FREQ_30MHz)
#define BTR_FREQ_24MHz            (CANBTR_FREQ_24MHz)
#define BTR_FREQ_20MHz            (CANBTR_FREQ_20MHz)
#define BTR_FREQ_SJA1000          (CANBTR_FREQ_SJA1000)
#endif
/** @} */

/** @name  CAN 2.0 and CAN FD Nominal Bit-rate Settings
 *  @brief Limits for nominal bit-rate settings
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_NOMINAL_BRP_MIN        1u  /**< min. bit-timing prescaler */
#define BTR_NOMINAL_BRP_MAX     1024u  /**< max. bit-timing prescaler */
#define BTR_NOMINAL_TSEG1_MIN      1u  /**< min. time segment 1 (before SP) */
#define BTR_NOMINAL_TSEG1_MAX    256u  /**< max. time segment 1 (before SP) */
#define BTR_NOMINAL_TSEG2_MIN      1u  /**< min. time segment 2 (after SP) */
#define BTR_NOMINAL_TSEG2_MAX    128u  /**< max. time segment 2 (after SP) */
#define BTR_NOMINAL_SJW_MIN        1u  /**< min. syncronization jump width */
#define BTR_NOMINAL_SJW_MAX      128u  /**< max. syncronization jump width */
#else
#define BTR_NOMINAL_BRP_MIN       (CANBTR_NOMINAL_BRP_MIN)
#define BTR_NOMINAL_BRP_MAX       (CANBTR_NOMINAL_BRP_MAX)
#define BTR_NOMINAL_TSEG1_MIN     (CANBTR_NOMINAL_TSEG1_MIN)
#define BTR_NOMINAL_TSEG1_MAX     (CANBTR_NOMINAL_TSEG1_MAX)
#define BTR_NOMINAL_TSEG2_MIN     (CANBTR_NOMINAL_TSEG2_MIN)
#define BTR_NOMINAL_TSEG2_MAX     (CANBTR_NOMINAL_TSEG2_MAX)
#define BTR_NOMINAL_SJW_MIN       (CANBTR_NOMINAL_SJW_MIN)
#define BTR_NOMINAL_SJW_MAX       (CANBTR_NOMINAL_SJW_MAX)
#endif
#define BTR_NOMINAL_SAM_MIN         0u  /**< single sampling (CAN 2.0 only) */
#define BTR_NOMINAL_SAM_MAX         1u  /**< triple sampling (CAN 2.0 only) */
#define BTR_NOMINAL_SP_MIN       0.05f  /**< tbd. */
#define BTR_NOMINAL_SP_MAX       1.00f  /**< tbd. */
#define BTR_NOMINAL_SPEED_MIN    1000.f /**< tbd. */
#define BTR_NOMINAL_SPEED_MAX 2000000.f /**< tbd. */
/** @} */

/** @name  CAN FD Data Bit-rate Settings
 *  @brief Limits for data bit-rate settings
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_DATA_BRP_MIN            1u  /**< min. baud rate prescaler */
#define BTR_DATA_BRP_MAX         1024u  /**< max. baud rate prescaler */
#define BTR_DATA_TSEG1_MIN          1u  /**< min. time segment 1 (before SP) */
#define BTR_DATA_TSEG1_MAX         32u  /**< max. time segment 1 (before SP) */
#define BTR_DATA_TSEG2_MIN          1u  /**< min. time segment 2 (after SP) */
#define BTR_DATA_TSEG2_MAX         16u  /**< max. time segment 2 (after SP) */
#define BTR_DATA_SJW_MIN            1u  /**< min. syncronization jump width */
#define BTR_DATA_SJW_MAX           16u  /**< max. syncronization jump width */
#else
#define BTR_DATA_BRP_MIN          (CANBTR_DATA_BRP_MIN)
#define BTR_DATA_BRP_MAX          (CANBTR_DATA_BRP_MAX)
#define BTR_DATA_TSEG1_MIN        (CANBTR_DATA_TSEG1_MIN)
#define BTR_DATA_TSEG1_MAX        (CANBTR_DATA_TSEG1_MAX)
#define BTR_DATA_TSEG2_MIN        (CANBTR_DATA_TSEG2_MIN)
#define BTR_DATA_TSEG2_MAX        (CANBTR_DATA_TSEG2_MAX)
#define BTR_DATA_SJW_MIN          (CANBTR_DATA_SJW_MIN)
#define BTR_DATA_SJW_MAX          (CANBTR_DATA_SJW_MAX)
#endif
#define BTR_DATA_SP_MIN           0.1f  /**< tbd. */
#define BTR_DATA_SP_MAX           1.0f  /**< tbd. */
#define BTR_DATA_SPEED_MIN     1000.0f  /**< tbd. */
#define BTR_DATA_SPEED_MAX 12000000.0f  /**< tbd. */
/** @} */

/** @name  SJA1000 Bit-rate Settings (CAN 2.0 only)
 *  @brief Limits for bit-rate settings of the SJA1000 CAN controller
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_SJA1000_BRP_MIN         1u  /**< min. baud rate prescaler */
#define BTR_SJA1000_BRP_MAX        64u  /**< max. baud rate prescaler */
#define BTR_SJA1000_TSEG1_MIN       1u  /**< min. time segment 1 (before SP) */
#define BTR_SJA1000_TSEG1_MAX      16u  /**< max. time segment 1 (before SP) */
#define BTR_SJA1000_TSEG2_MIN       1u  /**< min. time segment 2 (after SP) */
#define BTR_SJA1000_TSEG2_MAX       8u  /**< max. time segment 2 (after SP) */
#define BTR_SJA1000_SJW_MIN         1u  /**< min. syncronization jump width */
#define BTR_SJA1000_SJW_MAX         4u  /**< max. syncronization jump width */
#define BTR_SJA1000_SAM_MIN         0u  /**< single: the bus is sampled once */
#define BTR_SJA1000_SAM_MAX         1u  /**< triple: the bus is sampled three times */
#else
#define BTR_SJA1000_BRP_MIN       (CANBTR_SJA1000_BRP_MIN)
#define BTR_SJA1000_BRP_MAX       (CANBTR_SJA1000_BRP_MAX)
#define BTR_SJA1000_TSEG1_MIN     (CANBTR_SJA1000_TSEG1_MIN)
#define BTR_SJA1000_TSEG1_MAX     (CANBTR_SJA1000_TSEG1_MAX)
#define BTR_SJA1000_TSEG2_MIN     (CANBTR_SJA1000_TSEG2_MIN)
#define BTR_SJA1000_TSEG2_MAX     (CANBTR_SJA1000_TSEG2_MAX)
#define BTR_SJA1000_SJW_MIN       (CANBTR_SJA1000_SJW_MIN)
#define BTR_SJA1000_SJW_MAX       (CANBTR_SJA1000_SJW_MAX)
#define BTR_SJA1000_SAM_MIN       (CANBTR_SJA1000_SAM_MIN)
#define BTR_SJA1000_SAM_MAX       (CANBTR_SJA1000_SAM_MAX)
#endif
#define BTR_SJA1000_SP_MIN       0.20f  /**< tbd. */
#define BTR_SJA1000_SP_MAX       0.95f  /**< tbd. */
#define BTR_SJA1000_SPEED_MIN    1000.f /**< tbd. */
#define BTR_SJA1000_SPEED_MAX 1000000.f /**< tbd. */
 /** @} */

/** @name  Error Codes
 *  @brief Possible error codes
 *  @note  tbd.
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTRERR_NOERROR              (0) /**< no error! */
#define BTRERR_BAUDRATE           (-91) /**< illegal baudrate */
#define BTRERR_NULLPTR            (-94) /**< null-pointer assignment */
#define BTRERR_NOTSUPP            (-98) /**< not supported */
#define BTRERR_FATAL              (-99) /**< other errors */
#else
#define BTRERR_NOERROR            (CANERR_NOERROR)
#define BTRERR_BAUDRATE           (CANERR_BAUDRATE)
#define BTRERR_NULLPTR            (CANERR_NULLPTR)
#define BTRERR_NOTSUPP            (CANERR_NOTSUPP)
#define BTRERR_FATAL              (CANERR_FATAL)
#endif
/** @} */

/** @name  Other Stuff
 *  @brief This and that
 *  @{ */
#ifdef CANBTR_STANDALONE_VARIANT
#define BTR_STRING_LENGTH          256  /**< recommended size of the string buffer */
#else
#define BTR_STRING_LENGTH         (CANPROP_MAX_BUFFER_SIZE)
#endif
 /** @} */

/*  -----------  types  --------------------------------------------------
 */

/** @brief       CAN Bit-rate Settings (nominal and data):
 */
#ifdef CANBTR_STANDALONE_VARIANT
typedef union btr_bitrate_t_ {
    int32_t index;                      /**< index to predefined bit-rate (<= 0) */
    struct {                            /*   bit-timing register: */
        int32_t frequency;              /**<   clock domain (frequency in [Hz]) */
        struct {                        /*     nominal bus speed: */
            uint16_t brp;               /**<     bit-rate prescaler */
            uint16_t tseg1;             /**<     TSEG1 segment */
            uint16_t tseg2;             /**<     TSEG2 segment */
            uint16_t sjw;               /**<     synchronization jump width */
            uint8_t  sam;               /**<     number of samples (SJA1000) */
        } nominal;                      /**<   nominal bus speed */
#ifndef OPTION_CAN_2_0_ONLY
        struct {                        /*     data bus speed: */
            uint16_t brp;               /**<     bit-rate prescaler */
            uint16_t tseg1;             /**<     TSEG1 segment */
            uint16_t tseg2;             /**<     TSEG2 segment */
            uint16_t sjw;               /**<     synchronization jump width */
        } data;                         /**<   data bus speed */
#endif
    } btr;                              /**< bit-timing register */
} btr_bitrate_t;
#else
typedef can_bitrate_t btr_bitrate_t;    /* CAN API V3 bit-rate settings */
#endif
/** @brief       CAN Transmission Rate (nominal and data):
 */
#ifdef CANBTR_STANDALONE_VARIANT
typedef struct btr_speed_t {
    struct {                            /*   nominal bus speed: */
#ifndef OPTION_CAN_2_0_ONLY
        bool  fdoe;                     /**<   CAN FD operation enabled */
#endif
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
    } nominal;                          /**< nominal bus speed */
#ifndef OPTION_CAN_2_0_ONLY
    struct {                            /*   data bus speed: */
        bool  brse;                     /**<   bit-rate switch enabled */
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
    } data;                             /**< data bus speed */
#endif
} btr_speed_t;
#else
typedef can_speed_t btr_speed_t;        /* CAN API V3 bus speed */
#endif
/** @brief       CAN 2.0 Indexes to predefined bit-rates
 */
typedef int32_t btr_index_t;

/** @brief       CAN Bit-rate settings as zero-terminated string
 *  @note        Comma-sepatated key-value string as defined by PEAK's
 *               PCANBasic API and used in the MacCAN project.
 */
typedef char* btr_string_t;

/** @brief       SJA1000 Bit-timing register (BTR0 & BTR1)
 */
typedef uint16_t btr_sja1000_t;


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       ...
 *
 *  @param[in]   bitrate -
 *  @param[in]   fdoe    -
 *  @param[in]   brse    -
 *  @param[out]  speed   -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_bitrate2speed(const btr_bitrate_t *bitrate, bool fdoe, bool brse, btr_speed_t *speed);


/** @brief       ...
 *
 *  @note        Sorry, not realized yet!
 *
 *  @param[in]   speed   -
 *  @param[out]  bitrate -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_speed2bitrate(const btr_speed_t *speed, btr_bitrate_t *bitrate);


/** @brief       ...
 *
 *  @param[in]   index   -
 *  @param[out]  bitrate -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_index2bitrate(const btr_index_t index, btr_bitrate_t *bitrate);


/** @brief       ...
 *
 *  @note        Sorry, not realized yet!
 *
 *  @param[in]   bitrate -
 *  @param[out]  index   -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_bitrate2index(const btr_bitrate_t *bitrate, btr_index_t *index);


/** @brief       ...
 *
 *  @param[in]   string  -
 *  @param[out]  bitrate -
 *  @param[out]  brse    -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_string2bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *brse);


/** @brief       ...
 *
 *  @param[in]   bitrate -
 *  @param[in]   brse    -
 *  @param[out]  string  -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_bitrate2string(const btr_bitrate_t *bitrate, bool brse, btr_string_t string);


/** @brief       ...
 *
 *  @param[in]   btr0btr1 -
 *  @param[out]  bitrate  -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_sja10002bitrate(const btr_sja1000_t btr0btr1, btr_bitrate_t *bitrate);


/** @brief       ...
 *
 *  @note        Sorry, not realized yet!
 *
 *  @param[in]   bitrate  -
 *  @param[out]  btr0btr1 -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_bitrate2sja1000(const btr_bitrate_t *bitrate, btr_sja1000_t *btr0btr1);


/** @brief       ...
 *
 *  @param[in]   index    -
 *  @param[out]  btr0btr1 -
 *
 *  @returns     0 if successful, or a negative value on error.
 */
int btr_index2sja1000(const btr_index_t index, btr_sja1000_t *btr0btr1);


#ifdef __cplusplus
}
#endif
#endif /* CAN_BTR_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
