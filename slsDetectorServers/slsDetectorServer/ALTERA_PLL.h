#pragma once

#include <unistd.h>     // usleep

/* Altera PLL DEFINES */

/** PLL Reconfiguration Registers */
//https://www.altera.com/documentation/mcn1424769382940.html
#define ALTERA_PLL_MODE_REG                     (0x00)

#define ALTERA_PLL_MODE_WT_RQUST_VAL            (0)
#define ALTERA_PLL_MODE_PLLNG_MD_VAL            (1)

#define ALTERA_PLL_STATUS_REG                   (0x01)
#define ALTERA_PLL_START_REG                    (0x02)
#define ALTERA_PLL_N_COUNTER_REG                (0x03)
#define ALTERA_PLL_M_COUNTER_REG                (0x04)
#define ALTERA_PLL_C_COUNTER_REG                (0x05)

#define ALTERA_PLL_C_COUNTER_LW_CNT_OFST        (0)
#define ALTERA_PLL_C_COUNTER_LW_CNT_MSK         (0x000000FF << ALTERA_PLL_C_COUNTER_LW_CNT_OFST)
#define ALTERA_PLL_C_COUNTER_HGH_CNT_OFST       (8)
#define ALTERA_PLL_C_COUNTER_HGH_CNT_MSK        (0x000000FF << ALTERA_PLL_C_COUNTER_HGH_CNT_OFST)
/* total_div = lw_cnt + hgh_cnt */
#define ALTERA_PLL_C_COUNTER_BYPSS_ENBL_OFST    (16)
#define ALTERA_PLL_C_COUNTER_BYPSS_ENBL_MSK     (0x00000001 << ALTERA_PLL_C_COUNTER_BYPSS_ENBL_OFST)
/* if bypss_enbl = 0, fout = f(vco)/total_div; else fout = f(vco) (c counter is bypassed) */
#define ALTERA_PLL_C_COUNTER_ODD_DVSN_OFST      (17)
#define ALTERA_PLL_C_COUNTER_ODD_DVSN_MSK       (0x00000001 << ALTERA_PLL_C_COUNTER_ODD_DVSN_OFST)
/** if odd_dvsn = 0 (even), duty cycle = hgh_cnt/ total_div; else duty cycle = (hgh_cnt - 0.5) / total_div */
#define ALTERA_PLL_C_COUNTER_SLCT_OFST          (18)
#define ALTERA_PLL_C_COUNTER_SLCT_MSK           (0x0000001F << ALTERA_PLL_C_COUNTER_SLCT_OFST)

#define ALTERA_PLL_PHASE_SHIFT_REG              (0x06)

#define ALTERA_PLL_SHIFT_NUM_SHIFTS_OFST        (0)
#define ALTERA_PLL_SHIFT_NUM_SHIFTS_MSK         (0x0000FFFF << ALTERA_PLL_SHIFT_NUM_SHIFTS_OFST)

#define ALTERA_PLL_SHIFT_CNT_SELECT_OFST        (16)
#define ALTERA_PLL_SHIFT_CNT_SELECT_MSK         (0x0000001F << ALTERA_PLL_SHIFT_CNT_SELECT_OFST)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C0_VAL        ((0x0 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C1_VAL        ((0x1 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C2_VAL        ((0x2 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C3_VAL        ((0x3 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C4_VAL        ((0x4 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C5_VAL        ((0x5 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C6_VAL        ((0x6 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C7_VAL        ((0x7 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C8_VAL        ((0x8 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C9_VAL        ((0x9 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C10_VAL       ((0x10 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C11_VAL       ((0x11 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C12_VAL       ((0x12 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C13_VAL       ((0x13 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C14_VAL       ((0x14 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C15_VAL       ((0x15 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C16_VAL       ((0x16 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)
#define ALTERA_PLL_SHIFT_CNT_SLCT_C17_VAL       ((0x17 << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK)

#define ALTERA_PLL_SHIFT_UP_DOWN_OFST           (21)
#define ALTERA_PLL_SHIFT_UP_DOWN_MSK            (0x00000001 << ALTERA_PLL_SHIFT_UP_DOWN_OFST)
#define ALTERA_PLL_SHIFT_UP_DOWN_NEG_VAL        ((0x0 << ALTERA_PLL_SHIFT_UP_DOWN_OFST) & ALTERA_PLL_SHIFT_UP_DOWN_MSK)
#define ALTERA_PLL_SHIFT_UP_DOWN_POS_VAL        ((0x1 << ALTERA_PLL_SHIFT_UP_DOWN_OFST) & ALTERA_PLL_SHIFT_UP_DOWN_MSK)

#define ALTERA_PLL_K_COUNTER_REG                (0x07)
#define ALTERA_PLL_BANDWIDTH_REG                (0x08)
#define ALTERA_PLL_CHARGEPUMP_REG               (0x09)
#define ALTERA_PLL_VCO_DIV_REG                  (0x1c)
#define ALTERA_PLL_MIF_REG                      (0x1f)


#define ALTERA_PLL_WAIT_TIME_US                 (10 * 1000)


uint32_t ALTERA_PLL_Cntrl_Reg = 0x0;
uint32_t ALTERA_PLL_Param_Reg = 0x0;
uint32_t ALTERA_PLL_Cntrl_RcnfgPrmtrRstMask = 0x0;
uint32_t ALTERA_PLL_Cntrl_WrPrmtrMask = 0x0;
uint32_t ALTERA_PLL_Cntrl_PLLRstMask = 0x0;
uint32_t ALTERA_PLL_Cntrl_AddrMask = 0x0;
int ALTERA_PLL_Cntrl_AddrOfst = 0;


/**
 * Set Defines
 * @param creg control register
 * @param preg parameter register
 * @param rprmsk reconfig parameter reset mask
 * @param wpmsk write parameter mask
 * @param prmsk pll reset mask
 * @param amsk address mask
 * @param aofst address offset
 */
void ALTERA_PLL_SetDefines(uint32_t creg, uint32_t preg, uint32_t rprmsk, uint32_t wpmsk, uint32_t prmsk, uint32_t amsk, int aofst) {
    ALTERA_PLL_Cntrl_Reg = creg;
    ALTERA_PLL_Param_Reg = preg;
    ALTERA_PLL_Cntrl_RcnfgPrmtrRstMask = rprmsk;
    ALTERA_PLL_Cntrl_WrPrmtrMask = wpmsk;
    ALTERA_PLL_Cntrl_PLLRstMask = prmsk;
    ALTERA_PLL_Cntrl_AddrMask = amsk;
    ALTERA_PLL_Cntrl_AddrOfst = aofst;
}

/**
 * Reset only PLL
 */
void ALTERA_PLL_ResetPLL () {
    FILE_LOG(logINFO, ("Resetting only PLL\n"));

    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) | ALTERA_PLL_Cntrl_PLLRstMask);
    usleep(ALTERA_PLL_WAIT_TIME_US);
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) & ~ALTERA_PLL_Cntrl_PLLRstMask);

}

/**
 * Reset PLL Reconfiguration and PLL
 */
void ALTERA_PLL_ResetPLLAndReconfiguration () {
    FILE_LOG(logINFO, ("Resetting PLL and Reconfiguration\n"));

    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) | ALTERA_PLL_Cntrl_RcnfgPrmtrRstMask | ALTERA_PLL_Cntrl_PLLRstMask);
    usleep(ALTERA_PLL_WAIT_TIME_US);
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) & ~ALTERA_PLL_Cntrl_RcnfgPrmtrRstMask & ~ALTERA_PLL_Cntrl_PLLRstMask);
}


/**
 * Set PLL Reconfig register
 * @param reg register
 * @param val value
 */
void ALTERA_PLL_SetPllReconfigReg(uint32_t reg, uint32_t val) {
    FILE_LOG(logINFO, ("Setting PLL Reconfig Reg\n"));

    // set parameter
    bus_w(ALTERA_PLL_Param_Reg, val);

    // set address
    bus_w(ALTERA_PLL_Cntrl_Reg, (reg << ALTERA_PLL_Cntrl_AddrOfst) & ALTERA_PLL_Cntrl_AddrMask);
    usleep(ALTERA_PLL_WAIT_TIME_US);

    //write parameter
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) | ALTERA_PLL_Cntrl_WrPrmtrMask);
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) & ~ALTERA_PLL_Cntrl_WrPrmtrMask);
    usleep(ALTERA_PLL_WAIT_TIME_US);
}

/**
 * Write Phase Shift
 * @param phase phase shift
 * @param clkIndex clock index
 * @param pos 1 if up down direction of shift is positive, else 0
 */
void ALTERA_PLL_SetPhaseShift(int32_t phase, int clkIndex, int pos) {
    FILE_LOG(logINFO, ("\tWriting PLL Phase Shift\n"));
    uint32_t value = (((phase << ALTERA_PLL_SHIFT_NUM_SHIFTS_OFST) & ALTERA_PLL_SHIFT_NUM_SHIFTS_MSK) |
             ((clkIndex << ALTERA_PLL_SHIFT_CNT_SELECT_OFST) & ALTERA_PLL_SHIFT_CNT_SELECT_MSK) |
             (pos ? ALTERA_PLL_SHIFT_UP_DOWN_POS_VAL : ALTERA_PLL_SHIFT_UP_DOWN_NEG_VAL));

    FILE_LOG(logDEBUG1, ("C%d phase word:0x%08x\n", clkIndex, value));

    // write phase shift
    ALTERA_PLL_SetPllReconfigReg(ALTERA_PLL_PHASE_SHIFT_REG, value);
    usleep(ALTERA_PLL_WAIT_TIME_US);
}

/**
 * Set PLL mode register to polling mode
 */
void ALTERA_PLL_SetModePolling() {
    FILE_LOG(logINFO, ("\tSetting Polling Mode\n"));
    ALTERA_PLL_SetPllReconfigReg(ALTERA_PLL_MODE_REG, ALTERA_PLL_MODE_PLLNG_MD_VAL);
    usleep(ALTERA_PLL_WAIT_TIME_US);
}

/**
 * Calculate and write output frequency
 * @param clkIndex clock index
 * @param pllVCOFreqMhz PLL VCO Frequency in Mhz
 * @param value frequency to set to
 * @param frequency set
 */
int ALTERA_PLL_SetOuputFrequency (int clkIndex, int pllVCOFreqMhz, int value) {
    FILE_LOG(logINFO, ("\tC%d: Setting output frequency\n"));

    // calculate output frequency
    uint32_t total_div =  pllVCOFreqMhz / value;

    // assume 50% duty cycle
    uint32_t low_count = total_div / 2;
    uint32_t high_count = low_count;
    uint32_t odd_division = 0;

    // odd division
    if (total_div > (2 * low_count)) {
        ++high_count;
        odd_division = 1;
    }
    FILE_LOG(logINFO, ("\tC%d: Low:%d, High:%d, Odd:%d\n", clkIndex, low_count, high_count, odd_division));

    // command to set output frequency
    uint32_t val = (((low_count << ALTERA_PLL_C_COUNTER_LW_CNT_OFST) & ALTERA_PLL_C_COUNTER_LW_CNT_MSK) |
            ((high_count << ALTERA_PLL_C_COUNTER_HGH_CNT_OFST) & ALTERA_PLL_C_COUNTER_HGH_CNT_MSK) |
            ((odd_division << ALTERA_PLL_C_COUNTER_ODD_DVSN_OFST) & ALTERA_PLL_C_COUNTER_ODD_DVSN_MSK) |
            ((clkIndex << ALTERA_PLL_C_COUNTER_SLCT_OFST) & ALTERA_PLL_C_COUNTER_SLCT_MSK));
    FILE_LOG(logDEBUG1, ("C%d word:0x%08x\n", clkIndex, val));

    // write frequency (post-scale output counter C)
    ALTERA_PLL_SetPllReconfigReg(ALTERA_PLL_C_COUNTER_REG, val);
    usleep(ALTERA_PLL_WAIT_TIME_US);

    // reset only PLL
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) | ALTERA_PLL_Cntrl_PLLRstMask);
    usleep(ALTERA_PLL_WAIT_TIME_US);
    bus_w(ALTERA_PLL_Cntrl_Reg, bus_r(ALTERA_PLL_Cntrl_Reg) & ~ALTERA_PLL_Cntrl_PLLRstMask);

    return (pllVCOFreqMhz / (low_count + high_count));
}

