/*! @file dspl_priv.h
 * @brief Private module definition for the Blackfin DSP runtime library.
 * 
 */
#ifndef DSPL_PRIV_H_
#define DSPL_PRIV_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/*! @brief The object struct of the camera module */
struct OSC_DSPL {
    /*! @brief Dummy to have at least one member in this struct. */
    int dummy; 
};

/*********************************************************************//*!
 * @brief Rearrange real data array in bit reverse order
 * 
 * This function is  needed to perform a RFFT
 * 
 * @param in real array to be rearranged in bit reversed order
 * @param size of the input array to be rearranged
 * @param out Real output array containing the values of the input array
 *  in bit reversed order
*//*********************************************************************/
void OscDsplRBitrev(const fract16 in[], fract16 out[], int size);

/*********************************************************************//*!
 * @brief Rearrange complex data array in bit reverse order
 * 
 * This function is needed to perform a CFFT or IFFT
 * 
 * @param in complex array to be rearranged in bit reversed order
 * @param size of the input array to be rearranged
 * @param out Complex output array containing the values of the input array
 *  in bit reversed order
*//*********************************************************************/
void OscDsplCBitrev(const complex_fract16 in[], complex_fract16 out[], int size);

/*********************************************************************//*!
 * @brief Check if a fract16 array need scaling
 * 
 * This function is needed to perform a RFFT. It checks if any value
 * is bigger than 0.5 or smaller than -0.5.
 * 
 * @param butterflyfr16 fract16 array to be checked
 * @param fft_size size of the input array
 * @return  Returns 1 if the array needs scaling, else 0 
*//*********************************************************************/
int OscDsplRDscale( fract16 butterflyfr16[], int fft_size);

/*********************************************************************//*!
 * @brief Check if a complex_fract16 array need scaling
 * 
 * This function is needed to perform a RFFT. It checks if any value
 * is bigger than 0.5 or smaller than -0.5.
 * 
 * @param butterflycfr16 complex_fract16 array to be checked
 * @param fft_size size of the input array
 * @return  Returns 1 if the array needs scaling, else 0 
*//*********************************************************************/
int OscDsplCDscale( complex_fract16 butterflycfr16[], int fft_size);

/*********************************************************************//*!
 * @brief Saturate a fract64 value to fract32
 * 
 * This function is needed for the var_fr16 function
 * 
 * @param in fract64 input value 
 * @return  Returns saturated value
*//*********************************************************************/
long long int OscDsplSatFr64(long long int in);

#endif /*DSPL_PRIV_H_*/
