/*****************************************************************************************************************************
  Probabilistic ECC (Error Correction Code) Simulation Model
  
  This module simulates the behavioral impact of ECC in NAND flash memory.
  As the P/E cycle (erase_count) increases, the Bit Error Rate (BER) increases,
  causing read latency penalties (simulating "Read Retry" or "Soft Decision").
  
  Author: SSD Simulator ECC Extension
  Description: ECC simulation for realistic NAND flash behavior modeling
 *****************************************************************************************************************************/

#ifndef ECC_H
#define ECC_H

#include "initialize.h"

/*
 * ECC Model Constants
 * These values are based on typical NAND flash characteristics
 */

/* Maximum P/E cycles for typical MLC NAND (can be configured) */
#define ECC_MAX_PE_CYCLES 3000

/* Base BER at beginning of life (very low error rate) */
#define ECC_BASE_BER 1e-9

/* Maximum BER at end of life (high error rate requiring read retries) */
#define ECC_MAX_BER 1e-3

/* BER threshold for triggering read retry (soft decision decoding) */
#define ECC_READ_RETRY_THRESHOLD 1e-5

/* BER threshold for triggering multiple read retries */
#define ECC_MULTI_RETRY_THRESHOLD 1e-4

/* Read retry latency penalty in nanoseconds (typical: 25us per retry) */
#define ECC_READ_RETRY_LATENCY 25000

/* Soft decision decoding latency penalty in nanoseconds (typical: additional 50us) */
#define ECC_SOFT_DECISION_LATENCY 50000

/*
 * ECC Operation Types
 */
#define ECC_READ_NORMAL 0
#define ECC_READ_RETRY 1
#define ECC_READ_SOFT_DECISION 2

/*
 * Function Declarations
 */

/**
 * Calculate the Bit Error Rate (BER) based on the block's erase count
 * 
 * @param erase_count: The number of P/E cycles the block has undergone
 * @param max_pe_cycles: Maximum rated P/E cycles for the NAND type
 * @return: The calculated BER as a double value
 * 
 * The BER increases exponentially with P/E cycles following the model:
 * BER = BASE_BER * e^(k * erase_count / max_pe_cycles)
 * where k is calibrated such that BER reaches MAX_BER at end of life
 */
double calculate_ber(unsigned int erase_count, unsigned int max_pe_cycles);

/**
 * Calculate the read latency penalty based on the Bit Error Rate
 * 
 * @param ber: The Bit Error Rate for the target block
 * @return: Additional read latency in nanoseconds
 * 
 * Read latency penalties are applied as follows:
 * - BER < READ_RETRY_THRESHOLD: Normal read, no penalty
 * - BER >= READ_RETRY_THRESHOLD and < MULTI_RETRY_THRESHOLD: Single read retry
 * - BER >= MULTI_RETRY_THRESHOLD: Multiple retries + soft decision decoding
 */
int64_t calculate_ecc_read_latency(double ber);

/**
 * Get the ECC operation type based on BER
 * 
 * @param ber: The Bit Error Rate
 * @return: ECC operation type (ECC_READ_NORMAL, ECC_READ_RETRY, or ECC_READ_SOFT_DECISION)
 */
int get_ecc_operation_type(double ber);

/**
 * Calculate the complete read latency including ECC overhead for a specific location
 * 
 * @param ssd: Pointer to the SSD info structure
 * @param channel: Channel number
 * @param chip: Chip number
 * @param die: Die number
 * @param plane: Plane number
 * @param block: Block number
 * @param base_read_time: Base read time without ECC overhead
 * @return: Total read time including ECC overhead
 * 
 * This is a convenience function that combines calculate_ber and calculate_ecc_read_latency
 */
int64_t calculate_total_read_latency(struct ssd_info *ssd, unsigned int channel, 
                                      unsigned int chip, unsigned int die, 
                                      unsigned int plane, unsigned int block,
                                      int64_t base_read_time);

#endif /* ECC_H */
