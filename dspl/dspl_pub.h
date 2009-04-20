/*	Oscar, a hardware abstraction framework for the LeanXcam and IndXcam.
	Copyright (C) 2008 Supercomputing Systems AG
	
	This library is free software; you can redistribute it and/or modify it
	under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2.1 of the License, or (at
	your option) any later version.
	
	This library is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
	General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*! @file dspl_pub.h
 * @brief API definition for the Blackfin DSP realtime module
 * 
 */
#ifndef DSPL_PUB_H_
#define DSPL_PUB_H_

#include "oscar_error.h"
#ifdef OSC_HOST
	#include "oscar_types_host.h"
	#include "oscar_host.h"
#else
	#include "oscar_types_target.h"
	#include "oscar_target.h"
#endif /* OSC_HOST */


#ifdef OSC_TARGET
/* Include the header files of the DSP runtime library
 * (part of the toolchain)*/
#include <filter.h>
#include <fract.h>
#include <fract_complex.h>
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/* For the host, we need to manually make the type definitions */
typedef int16 fract16;
typedef int32 fract32;
typedef int32 fract2x16;

typedef struct complex_fract16 {
	fract16 re;
	fract16 im;
} complex_fract16;

#endif /* OSC_HOST */

/*=========================== API functions ============================*/

/*********************************************************************//*!
 * @brief Constructor
 * 
 * @param hFw Pointer to the handle of the framework.
 * @return SUCCESS or an appropriate error code otherwise
 *//*********************************************************************/
OSC_ERR OscDsplCreate(void *hFw);

/*********************************************************************//*!
 * @brief Destructor
 * 
 * @param hFw Pointer to the handle of the framework.
 *//*********************************************************************/
void OscDsplDestroy(void *hFw);

/*********************************************************************//*!
 * @brief Convert fract16 to float.
 * 
 * @param n Fract16 number
 * @return Float equivalent of the supplied number.
 *//*********************************************************************/
float OscDsplFr16ToFloat(fract16 n);

/*********************************************************************//*!
 * @brief Convert float to fract16 (saturating)
 * 
 * @param n float number
 * @return Fract16 equivalent of the supplied float.
 *//*********************************************************************/
fract16 OscDsplFloatToFr16(float n);


#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 high_of_fr2x16(fract2x16 x);
#define OscDsplHigh_of_fr2x16 high_of_fr2x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Get high part of fract2x16 packet type.
 * 
 * Equivalent to high_of_fr2x16 from the ADI DSP library.
 * 
 * @param x Packet type.
 * @return The high part.
 *//*********************************************************************/
fract16 OscDsplHigh_of_fr2x16(fract2x16 x);
#endif /* OSC_HOST */


#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 low_of_fr2x16(fract2x16 x);
#define OscDsplLow_of_fr2x16 low_of_fr2x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Get low part of fract2x16 packet type.
 * 
 * Equivalent to low_of_fr2x16 from the ADI DSP library.
 * 
 * @param x Packet type.
 * @return The high part.
 *//*********************************************************************/
fract16 OscDsplLow_of_fr2x16(fract2x16 x);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 shl_fr1x16(fract16 x, int y);
#define OscDsplShl_fr1x16 shl_fr1x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Arithmetically shift left src variable by shft places.
 * 
 * Equivalent to shl_fr1x16 from the ADI DSP library.
 * The empty bits are zero-filled. If shft is negative, the sift is to
 * the right by abs(shft) places with sign extension.
 * 
 * @param x Source data
 * @param y	Number of places to shift
 * @return The high part.
 *//*********************************************************************/
fract16 OscDsplShl_fr1x16(fract16 x, int y);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract2x16 shl_fr2x16(fract2x16 x, int y);
#define OscDsplShl_fr2x16 shl_fr2x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Arithmetically shift left packed fr16 by shft places.
 * 
 * Equivalent to shl_fr2x16 from the ADI DSP library.
 * 
 * Arithmetically shifts both fract16s in the fract2x16 left by _y places,and 
 * returns the packed result.  The empty bits are zero filled.  If shft is 
 * negative, the shift is to the right by abs(shft) places with sign extension.
 * 
 * @param x Source data
 * @param y	Number of places to shift
 * @return The high part.
 *//*********************************************************************/
fract2x16 OscDsplShl_fr2x16(fract2x16 x, int y);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract2x16 compose_fr2x16(fract16 h, fract16 l);
#define OscDsplCompose_fr2x16 compose_fr2x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Compose fract2x16 packet type.
 * 
 * Equivalent to compose_fr2x16 from the ADI DSP library.
 * 
 * @param h High part.
 * @param l Low part.
 * @return The packed type.
 *//*********************************************************************/
fract2x16 OscDsplCompose_fr2x16(fract16 h, fract16 l);
#endif /* OSC_HOST */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Transform a fract32 number into a fract16 number
 * 
 * Saturates and rounds the same way as the dsp
 * (when saving an accumulator result into a fract16 data register)
 * 
 * Only for host
 * 
 * @param multfr32 Fract32 number to be saved.
 * @return The saved fract16 value.
 *//*********************************************************************/
fract16 OscDsplTransfr32fr16(fract32 multfr32);
#endif /* OSC_HOST */


#ifdef OSC_TARGET
fract16 mult_fr1x16(fract16 a,fract16 b);
#define OscDsplMultFr16 mult_fr1x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Multiplies two fract16 numbers
 * 
 * Multiplies two fract16 numbers and truncates the result. No rounding
 * takes place!
 * @see OscDsplMultRfr16
 * 
 *  Only for host
 * 
 * @param a Fract16 number.
 * @param b Fract16 number.
 * @return The resulting fract16 value.
 *//*********************************************************************/
fract16 OscDsplMultFr16(fract16 a, fract16 b);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
fract16 multr_fr1x16(fract16 a,fract16 b);
#define OscDsplMultRFr16 multr_fr1x16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Multiplies two fract16 numbers
 * 
 * Multiplies two fract16 numbers, then rounds and saturates
 * the result the same way as the DSP.
 * @see OscDsplMultfr16
 * 
 *  Only for host
 * 
 * @param a Fract16 number.
 * @param b Fract16 number.
 * @return The resulting fract16 value.
 *//*********************************************************************/
fract16 OscDsplMultRFr16(fract16 a, fract16 b);
#endif /* OSC_HOST */


#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _sin_fr16(fract16 x);
#define OscDspl_sin_fr16 _sin_fr16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Sin(x)
 * 
 * Equivalent to sin_fr16 from the ADI DSP library.
 * 
 * @param x frac16 number.
 * @return An approximation of sin(x)
 *//*********************************************************************/
fract16 OscDspl_sin_fr16(fract16 x);
#endif /* OSC_HOST */


#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _cos_fr16(fract16 x);
#define OscDspl_cos_fr16 _cos_fr16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Cos(x)
 * 
 * Equivalent to cos_fr16 from the ADI DSP library.
 * 
 * @param x Frac16 number.
 * @return An approximation of cos(x)
 *//*********************************************************************/
fract16 OscDspl_cos_fr16(fract16 x);
#endif /* OSC_HOST */


#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _mean_fr16(const fract16 x[], int lenght);
#define OscDspl_mean_fr16 _mean_fr16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Calculates the mean of a fract16 vector
 * 
 * Equivalent to mean_fr16 from the ADI DSP library.
 * 
 * @param x Fract16 array with input values
 * @param length of input array
 * @return The mean of the input values
 *//*********************************************************************/
fract16 OscDspl_mean_fr16(const fract16 x[],int  length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
complex_fract16 _cadd_fr16 ( complex_fract16 a, complex_fract16 b );
#define OscDspl_cadd_fr16 _cadd_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex_fract16 adder
 * 
 * Adds two complexfract16 numbers and round the result
 * the same way as the dsp
 * 
 * @param a Complex_fract16 number.
 * @param b Complex_fract16 number
 * @return Result of the addition
 *//*********************************************************************/
complex_fract16 OscDspl_cadd_fr16(complex_fract16 a, complex_fract16 b );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
complex_fract16 _csub_fr16 ( complex_fract16 a, complex_fract16 b );
#define OscDspl_csub_fr16 _csub_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex_fract16 subtracter
 * 
 * Subtracts two complexfract16 numbers and round the result
 * the same way as the dsp
 * 
 * @param a Complex_fract16 number.
 * @param b Complex_fract16 number
 * @return Result of the subtraction
 *//*********************************************************************/
complex_fract16 OscDspl_csub_fr16(complex_fract16 a, complex_fract16 b );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
complex_fract16 _cdiv_fr16 ( complex_fract16 a, complex_fract16 b );
#define OscDspl_cdiv_fr16 _cdiv_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex_fract16 division
 * 
 * Divides two complex_fract16 numbers and round the result
 * the same way as the dsp
 * 
 * @param a Fract16 number.
 * @param b Fract16 number
 * @return Result of the division
 *//*********************************************************************/
complex_fract16 OscDspl_cdiv_fr16(complex_fract16 a, complex_fract16 b);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
complex_fract16 _cmlt_fr16 ( complex_fract16 a, complex_fract16 b );
#define OscDspl_cmlt_fr16 _cmlt_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex_fract16 multiplicator
 * 
 * Multiplies two complex_fract16 numbers and round the result
 * the same way as the dsp
 * 
 * @param a Fract16 number.
 * @param b Fract16 number
 * @return Result of the multiplication
 *//*********************************************************************/
complex_fract16 OscDspl_cmlt_fr16(complex_fract16 a, complex_fract16 b );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library
 * 
 *  ATTENTION: The function Negate(x) does not exist in the library and
 *  therefore this function is not working in the dsp*/
complex_fract16 _conj_fr16 ( complex_fract16 a);
#define OscDspl_conj_fr16 _conj_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex_fract16 conjugator
 * 
 * Conjugates a complexfract16 number
 * 
 * @param a Complex_fract16 number.
 * @return Conjugated result
 *//*********************************************************************/
complex_fract16 OscDspl_conj_fr16(complex_fract16 a);

#endif /* OSC_HOST */



#define __USE_FAST_LOOKUP__  1

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
void _twidfftrad2_fr16(complex_fract16 w[],int n);
#define OscDspl_twidfftrad2_fr16 _twidfftrad2_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST

/*********************************************************************//*!
 * @brief Twiddle table generator
 * 
 * Equivalent to twiddlerad2_fr16 from the ADI DSP library.
 * 
 * @param w Pointer to the output twiddle table array
 * @param n FFT size
 *//*********************************************************************/
void OscDspl_twidfftrad2_fr16(complex_fract16 w[], int n );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
void  rfft_fr16( const fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );
#define OscDspl_rfft_fr16 rfft_fr16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Real fast fourier transformation
 * 
 * Equivalent to rfft_fr16 from the ADI DSP library.
 * 
 * This FFT uses the decimation-in-time FFT algorithm, for more details
 * please read the following webpage: oscar/dspl/FFT/fft.html
 * 
 * The FFT consists of differt stages:
 * 
 * - The input array is rearranged in bit reverse order
 * 
 * - Not the official first stage starts: calculate the butterfly. As there is
 * no twiddle factor but 0 and -1, this stage is calculated on its own
 *      - Number of groups : fft_size/2
 *      - Number of butterflies: 1
 * 
 * - The remaining stages are iterativ and have the following algorithm:
 *      - Number of groups : fft_size/4
 *      - Number of butterflies: 2
 *      - While (group > 0)
 *      - {
 *          - For each group calculate all butterflies
 *          - Number of groups : groups / 2
 *          - Number of butterflies:  butterflies * 2
 *      - }
 * 
 * 
 * 
 * stride = (FFT-size for which the twiddle array has been created) / FFT-size
 * 
 * For static scaling set scaling = 1, for dynamic: scaling = 2
 * for no scaling (only saturating): scaling = 3
 * 
 * 
 * @param in  Array of input data with real values
 * @param twiddle Twiddle table array
 * @param stride Defines how the twiddle table should be read
 * @param fft_size Size of the fft array
 * @param scaling Scaling method to be used
 * @param out Output data array with complex values
 * @param block_exponent Outputs the number it times the data has been scaled
 *//*********************************************************************/
void  OscDspl_rfft_fr16( const fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
void  cfft_fr16( const complex_fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );
#define OscDspl_cfft_fr16 cfft_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex fast fourier transformation
 * 
 * Equivalent to cfft_fr16 from the ADI DSP library.
 * 
 * @see OscDsplRfftFr16 for more details about the algorithm
 * 
 * stride = (FFT-size for which the twiddle array has been created)/FFT-size
 * 
 * For static scaling set scaling = 1, for dynamic: scaling = 2
 * for no scaling (only saturating): scaling = 3
 * 
 * @param in  Array of input data with complex values
 * @param twiddle Twiddle table array
 * @param stride Defines how the twiddle table should be read
 * @param fft_size Size of the fft array
 * @param scaling Scaling method to be used
 * @param out Output data array with complex values
 * @param block_exponent Outputs the number it times the data has been scaled
 *//*********************************************************************/
void  OscDspl_cfft_fr16( const complex_fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
void  ifft_fr16( const complex_fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );
#define OscDspl_ifft_fr16 ifft_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Inverse fast fourier transformation
 * 
 * Equivalent to ifft_fr16 from the ADI DSP library.
	
 * @see OscDsplRfftFr16 for more details about the algorithm
 * 
 * stride = (FFT-size for which the twiddle array has been created)/FFT-size
 * 
 * For static scaling set scaling = 1, for dynamic: scaling = 2
 * for no scaling (only saturating): scaling = 3
 * 
 * @param in  Array of input data with complex values
 * @param twiddle Twiddle table array
 * @param stride Defines how the twiddle table should be read
 * @param fft_size Size of the fft array
 * @param scaling Scaling method to be used
 * @param out Output data array with complex values
 * @param block_exponent Outputs the number it times the data has been scaled
 *//*********************************************************************/
void  OscDspl_ifft_fr16( const complex_fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling );

#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _vecmax_fr16(const fract16 vec[], int length);
#define OscDspl_vecmax_fr16 _vecmax_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Find maximum vector value
 * 
 * @param vec  Input array
 * @param length Array length
 * @return Maximum value of the array
 *//*********************************************************************/
fract16 OscDspl_vecmax_fr16(const fract16 vec[], int length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
int _vecmaxloc_fr16(const fract16 vec[], int length);
#define OscDspl_vecmaxloc_fr16 _vecmaxloc_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Find maximum vector value location
 * 
 * @param vec  Input array
 * @param length Array length
 * @return Maximum value location in the array
 *//*********************************************************************/
int OscDspl_vecmaxloc_fr16(const fract16 vec[], int length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _vecmin_fr16(const fract16 vec[], int length);
#define OscDspl_vecmin_fr16 _vecmin_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Find minimun vector value
 * 
 * @param vec  Input array
 * @param length Array length
 * @return Minimum value of the array
 *//*********************************************************************/
fract16 OscDspl_vecmin_fr16(const fract16 vec[], int length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _vecminloc_fr16(const fract16 vec[], int length);
#define OscDspl_vecminloc_fr16 _vecminloc_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Find minimum vector value location
 * 
 * @param vec  Input array
 * @param length Array length
 * @return Minimum value location in the array
 *//*********************************************************************/
fract16 OscDspl_vecminloc_fr16(const fract16 vec[], int length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _var_fr16(const fract16 sample[], int length);
#define OscDspl_var_fr16 _var_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Find maximum vector value
 * 
 * @param sample  Input array
 * @param length Array length
 * @return Maximum value of the array
 *//*********************************************************************/
fract16 OscDspl_var_fr16(const fract16 sample[], int length);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
void _histogram_fr16(   const fract16 samples[],
							int histogram[],
							fract16 max_sample,
							fract16 min_sample,
							int sample_length,
							int bin_count);
#define OscDspl_histogram_fr16 _histogram_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Calculate histogram of a vector
 * 
 * @param samples  Input vector
 * @param max_sample Maximum value of the input vector
 * @param min_sample Minimum value of the input vector
 * @param sample_length Input vector length
 * @param bin_count Length of histogram vector = number of bins
 * @param histogram Output array containing the histogram data
 *//*********************************************************************/
void OscDspl_histogram_fr16(    const fract16 samples[],
							int histogram[],
							fract16 max_sample,
							fract16 min_sample,
							int sample_length,
							int bin_count);
#endif /* OSC_HOST */
#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
fract16 _sqrt_fr16(fract16 x);
#define OscDspl_sqrt_fr16 _sqrt_fr16
#endif /* OSC_TARGET */

#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Calculate the square root
 * 
 * @param x  Input value
 * @return Square root of the input value
 *//*********************************************************************/
fract16 OscDspl_sqrt_fr16(fract16 x);
#endif /* OSC_HOST */

#ifdef OSC_TARGET
/*! @brief Target only: Redirect the call to the DSP runtime library */
#define OscDspl_cabs_fr16 cabs_fr16
#endif /* OSC_TARGET */
#ifdef OSC_HOST
/*********************************************************************//*!
 * @brief Complex absolute
 * 
 * Equivalent to cabs_fr16 from the ADI DSP library.
 * 
 * @param c Complex number.
 * @return The absolute of the complex number (a real value).
 *//*********************************************************************/
fract16 OscDspl_cabs_fr16(complex_fract16 c);
#endif /* OSC_HOST */


#endif /*DSPL_PUB_H_*/

