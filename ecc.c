/*****************************************************************************************************************************
  Probabilistic ECC (Error Correction Code) Simulation Model
  
  This module simulates the behavioral impact of ECC in NAND flash memory.
  As the P/E cycle (erase_count) increases, the Bit Error Rate (BER) increases,
  causing read latency penalties (simulating "Read Retry" or "Soft Decision").
  
  Author: SSD Simulator ECC Extension
  Description: ECC simulation for realistic NAND flash behavior modeling
 *****************************************************************************************************************************/

#include <math.h>
#include "ecc.h"

/**
 * Calculate the Bit Error Rate (BER) based on the block's erase count
 * 
 * The BER increases exponentially with P/E cycles following the model:
 * BER = BASE_BER * e^(k * (erase_count / max_pe_cycles))
 * 
 * where k is calibrated such that BER reaches MAX_BER at max_pe_cycles.
 * Solving for k: MAX_BER = BASE_BER * e^k => k = ln(MAX_BER / BASE_BER)
 */
double calculate_ber(unsigned int erase_count, unsigned int max_pe_cycles)
{
    double ratio;
    double k;
    double ber;
    
    /* Prevent division by zero */
    if (max_pe_cycles == 0) {
        return ECC_BASE_BER;
    }
    
    /* Calculate the wear ratio (0.0 to 1.0+) */
    ratio = (double)erase_count / (double)max_pe_cycles;
    
    /* Cap the ratio at 1.0 to prevent extreme values */
    if (ratio > 1.0) {
        ratio = 1.0;
    }
    
    /* Calculate the exponential growth factor k */
    /* k = ln(MAX_BER / BASE_BER) ≈ ln(1e-3 / 1e-9) = ln(1e6) ≈ 13.8 */
    k = log(ECC_MAX_BER / ECC_BASE_BER);
    
    /* Calculate BER using exponential model */
    ber = ECC_BASE_BER * exp(k * ratio);
    
    /* Ensure BER doesn't exceed maximum */
    if (ber > ECC_MAX_BER) {
        ber = ECC_MAX_BER;
    }
    
    return ber;
}

/**
 * Calculate the read latency penalty based on the Bit Error Rate
 * 
 * Read latency penalties simulate real-world ECC behavior:
 * - Normal read: No penalty when BER is low
 * - Read retry: When BER exceeds threshold, retry with adjusted voltage thresholds
 * - Soft decision: When BER is very high, use soft decoding with multiple samples
 */
int64_t calculate_ecc_read_latency(double ber)
{
    int64_t latency_penalty = 0;
    
    if (ber < ECC_READ_RETRY_THRESHOLD) {
        /* Normal read - ECC can correct errors with hard decision decoding */
        latency_penalty = 0;
    }
    else if (ber < ECC_MULTI_RETRY_THRESHOLD) {
        /* Single read retry needed - adjust voltage threshold and re-read */
        latency_penalty = ECC_READ_RETRY_LATENCY;
    }
    else {
        /* Multiple retries + soft decision decoding required */
        /* Calculate number of retries based on BER severity */
        int num_retries = 2;
        double ber_range = ECC_MAX_BER - ECC_MULTI_RETRY_THRESHOLD;
        
        /* Guard against division by zero if thresholds are misconfigured */
        if (ber_range > 0) {
            num_retries = 2 + (int)((ber - ECC_MULTI_RETRY_THRESHOLD) / ber_range * 3);
        }
        
        /* Cap maximum retries */
        if (num_retries > 5) {
            num_retries = 5;
        }
        
        latency_penalty = num_retries * ECC_READ_RETRY_LATENCY + ECC_SOFT_DECISION_LATENCY;
    }
    
    return latency_penalty;
}

/**
 * Get the ECC operation type based on BER
 */
int get_ecc_operation_type(double ber)
{
    if (ber < ECC_READ_RETRY_THRESHOLD) {
        return ECC_READ_NORMAL;
    }
    else if (ber < ECC_MULTI_RETRY_THRESHOLD) {
        return ECC_READ_RETRY;
    }
    else {
        return ECC_READ_SOFT_DECISION;
    }
}

/**
 * Calculate the complete read latency including ECC overhead for a specific location
 * 
 * This function retrieves the block's erase count, calculates the BER,
 * and returns the total read time including any ECC-related penalties.
 */
int64_t calculate_total_read_latency(struct ssd_info *ssd, unsigned int channel, 
                                      unsigned int chip, unsigned int die, 
                                      unsigned int plane, unsigned int block,
                                      int64_t base_read_time)
{
    unsigned int erase_count;
    unsigned int max_pe_cycles;
    double ber;
    int64_t ecc_latency;
    
    /* Get the block's erase count */
    erase_count = ssd->channel_head[channel].chip_head[chip].die_head[die]
                     .plane_head[plane].blk_head[block].erase_count;
    
    /* Get the maximum P/E cycles from chip parameters */
    max_pe_cycles = ssd->channel_head[channel].chip_head[chip].ers_limit;
    
    /* If ers_limit is 0, use default value */
    if (max_pe_cycles == 0) {
        max_pe_cycles = ECC_MAX_PE_CYCLES;
    }
    
    /* Calculate BER based on erase count */
    ber = calculate_ber(erase_count, max_pe_cycles);
    
    /* Calculate ECC latency penalty */
    ecc_latency = calculate_ecc_read_latency(ber);
    
    return base_read_time + ecc_latency;
}
