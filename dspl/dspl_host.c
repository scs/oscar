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

/*! @file
 * @brief Blackfin DSP runtime library implementation for host.
 * 
 * All functions of the DSP runtime library used on the target must
 * be implemented for the host. The implementation for the target on the
 * other hand must not be done in the framework, since it already exists
 * in optimized form in the library.
 */

#include <math.h>
#include "dspl.h"

/*! @brief The module singelton instance.  */
struct OSC_DSPL osc_dspl;

struct OscModule OscModule_dspl = {
	.create = OscDsplCreate,
	.destroy = OscDsplDestroy,
	.dependencies = {
		NULL // To end the flexible array.
	}
};

OSC_ERR OscDsplCreate(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;

	pFw = (struct OSC_FRAMEWORK *)hFw;
	if(pFw->dspl.useCnt != 0)
	{
		pFw->dspl.useCnt++;
		/* The module is already allocated */
		return SUCCESS;
	}
	
	osc_dspl = (struct OSC_DSPL) { 0 };
		
	/* Increment the use count */
	pFw->dspl.hHandle = (void*)&osc_dspl;
	pFw->dspl.useCnt++;

	return SUCCESS;
}

void OscDsplDestroy(void *hFw)
{
	struct OSC_FRAMEWORK *pFw;
		
	pFw = (struct OSC_FRAMEWORK *)hFw;
	/* Check if we really need to release or whether we still
	 * have users. */
	pFw->dspl.useCnt--;
	if(pFw->dspl.useCnt > 0)
	{
		return;
	}
	
	osc_dspl = (struct OSC_DSPL) { 0 };
}

inline float OscDsplFr16ToFloat(fract16 n)
{
	return (((float)n)/FR16_SCALE);
}

inline fract16 OscDsplFloatToFr16(float n)
{
	float ret = (n * FR16_SCALE);
	if(ret > FR16_MAX)
		ret = FR16_MAX;
	return (fract16)ret;
}

fract16 OscDsplHigh_of_fr2x16(fract2x16 x)
{
	return (fract16)(x >> 16);
}

fract16 OscDsplLow_of_fr2x16(fract2x16 x)
{
	return (fract16)( 0xffff & x);
}

fract16 OscDsplShl_fr1x16(fract16 x, int y)
{
	/* The C standard does not specify (to my knowledge) the result of a shift operation with a negative amount. */
	if (y > 0)
	{
	    x <<= y;
	} else if (y < 0) {
	    x >>= (-y);
	}
	
	return x;
}

fract2x16 OscDsplShl_fr2x16(fract2x16 x, int y)
{
	fract16 low = OscDsplShl_fr1x16(OscDsplLow_of_fr2x16(x), y);
	fract16 high  = OscDsplShl_fr1x16(OscDsplHigh_of_fr2x16(x), y);
	
	return OscDsplCompose_fr2x16(high, low);
}

fract2x16 OscDsplCompose_fr2x16(fract16 h, fract16 l)
{
	return (fract2x16)( (h << 16) | l );
}

/*********************************************************************//*!
 * @brief Transform a fract32 number being the result of the
 * multiplication of two fract16 numbers back to a fract16 with
 * rounding.
 * 
 * @see OscDsplTransRfr32fr16
 * 
 * @param multfr32 Input fract32.
 * @return Compacted result.
 *//*********************************************************************/
fract16 OscDsplTransRfr32fr16(fract32 multfr32)
{
	fract16 cutoffbits, resultfr16;
	
	resultfr16 = multfr32 >> 15;
	cutoffbits = multfr32 & 0x7fff;

	if(cutoffbits > 0x4000)
	{
		resultfr16++;
	} else if(cutoffbits == 0x4000) {
		/* see dsp manual about unbiased rounding */
		/* Round to the nearest even value. */
		if(resultfr16 & 0x1)
		{
			/* odd */
			resultfr16++;
		}
	}

	multfr32 = multfr32 >> 15;
	if(multfr32 >= FR16_MAX)
		resultfr16 = FR16_MAX;
	else if(multfr32 <= FR16_MIN)
		resultfr16 = FR16_MIN;
	
	return resultfr16;
}


fract16 OscDsplMultRFr16(fract16 a, fract16 b)
{
	fract32 multfr32;

	multfr32 = (fract32) a*b;
	return OscDsplTransRfr32fr16(multfr32);
}

/*********************************************************************//*!
 * @brief Transform a fract32 number being the result of the
 * multiplication of two fract16 numbers back to a fract16 without
 * rounding.
 * @see OscDsplTransRfr32fr16
 * 
 * @param multfr32 Input fract32.
 * @return Compacted result.
 *//*********************************************************************/
fract16 OscDsplTransfr32fr16(fract32 multfr32)
{
	fract16 resultfr16;
	
	resultfr16 = multfr32 >> 15;

	multfr32 = multfr32 >> 15;
	if(multfr32 >= FR16_MAX)
		resultfr16 = FR16_MAX;
	else if(multfr32 <= FR16_MIN)
		resultfr16 = FR16_MIN;
	
	return resultfr16;
}


fract16 OscDsplMultFr16(fract16 a, fract16 b)
{
	fract32 multfr32;

	multfr32 = (fract32) a*b;
	return OscDsplTransfr32fr16(multfr32);
}


fract16 OscDspl_sin_fr16(fract16 x)
{
	fract32 accufr32 = 0;
	fract16 resultfr16,afr16,bfr16;
	fract16 coef[]={0x6480,0x0059,0xD54D,0x0252,0x0388};
	
	/* result = x*coef[0] + x^2*coef[1] + ... + x^5*coef[4] */
	
	afr16=x;
	if(x<0)
		afr16=-x;
	bfr16=afr16;
	int i=0;
	for(i=0; i<5;i++)
	{
		accufr32 += (fract32)afr16 * (fract32)coef[i] << 1;
		afr16= OscDsplMultRFr16(afr16,bfr16);
	}
	if((uint32_t) accufr32 > 0x3fffC000) /* saturating */
		resultfr16= FR16_MAX;
	else
		resultfr16 =  accufr32 >> 15 ;
	if(x < 0)
		resultfr16= -resultfr16;
	return resultfr16;
}

fract16 OscDspl_cos_fr16(fract16 x)
{
	fract32 accufr32 = 0;
	fract16 resultfr16,afr16,bfr16;
	fract16 coef[]={0x6480,0x0059,0xD54D,0x0252,0x0388};
	
	/* result = x*coef[0] + x^2*coef[1] + ... + x^5*coef[4] */
	
	afr16=x;
	if(x<0)
		afr16=-x;
	afr16 = FR16_MIN - afr16;
	bfr16=afr16;
	int i=0;
	for(i=0; i<5;i++)
	{
		accufr32 += (fract32)afr16 * (fract32)coef[i] << 1;
		afr16= OscDsplMultRFr16(afr16,bfr16);
	}
	if((uint32_t) accufr32 > 0x3fffC000) /* saturating */
		resultfr16= FR16_MAX;
	else
		resultfr16 =  accufr32 >> 15 ;
	return resultfr16;
}

fract16 OscDspl_mean_fr16(const fract16 x[], int length)
{
	int i=0;
	fract32 tmp=0;
	fract16 result;
	
	if(length <=0)
		return (fract16) 0;
	
	for(i=0; i<length; i++)
	{
		tmp += (fract32) x[i];
	}
	
	result = (fract16) (tmp/(fract32)length) ;
	return result;
}

// Copyright (C) 2000 Analog Devices, Inc.
// This file is subject to the terms and conditions of the GNU Lesser
// General Public License. See the file COPYING.LIB for more details.
//
// Non-LGPL License is also available as part of VisualDSP++
// from Analog Devices, Inc.

/***************************************************************
	Func Name:    cadd_fr16

	Description:  addition of two complex numbers

***************************************************************/
complex_fract16 OscDspl_cadd_fr16(complex_fract16 a, complex_fract16 b)
{
	complex_fract16 result;
	fract32 real, imag;
		
	real = a.re + b.re;
	imag = a.im + b.im;

	if(real >= FR16_MAX)
		result.re = FR16_MAX;
	else if(real <= FR16_MIN)
		result.re = FR16_MIN;
	else
		result.re = real;

	if(imag >= FR16_MAX)
		result.im = FR16_MAX;
	else if(imag <= FR16_MIN)
		result.im = FR16_MIN;
	else
		result.im = imag;

	return (result);
}

// Copyright (C) 2000 Analog Devices, Inc.
// This file is subject to the terms and conditions of the GNU Lesser
// General Public License. See the file COPYING.LIB for more details.
//
// Non-LGPL License is also available as part of VisualDSP++
// from Analog Devices, Inc.

/*******************************************************************
	Func Name:    csub_fr16

	Description:  subtraction of two complex numbers

*******************************************************************/
complex_fract16 OscDspl_csub_fr16( complex_fract16 a, complex_fract16 b)
{
	complex_fract16 result;
	fract32 real, imag;
		
	real = a.re - b.re;
	imag = a.im - b.im;

	if(real >= FR16_MAX)
		result.re = FR16_MAX;
	else if(real <= FR16_MIN)
		result.re = FR16_MIN;
	else
		result.re = real;

	if(imag >= FR16_MAX)
		result.im = FR16_MAX;
	else if(imag <= FR16_MIN)
		result.im = FR16_MIN;
	else
		result.im = imag;

	return (result);
}

fract16 OscDsplSatFr16(fract32 satfr32)
{
	fract16 resultfr16;
	if(satfr32 >= FR16_MAX)
		resultfr16 = FR16_MAX;
	else if(satfr32 <= FR16_MIN)
		resultfr16 = FR16_MIN;
	else
		resultfr16=satfr32;
	
	return resultfr16;
}

// cdiv
complex_fract16 OscDspl_cdiv_fr16(complex_fract16 a, complex_fract16 b)
{
	fract32 denum, numRe, numIm;
	complex_fract16 result;
	
	result.re=0;
	result.im=0;
	
	
	denum = (fract32) (b.re*b.re) + (fract32)(b.im * b.im);
//  printf("denum = %lx\n", denum);
	if(denum <= 0)
		return result;
	
	denum = denum >> 15;
//  printf("denum = %lx\n", denum);
	
	numRe = (fract32) (a.re*b.re) + (fract32)(a.im * b.im);
//  printf("numRe = %lx\n", numRe);
	numIm = (fract32) (a.im*b.re) - (fract32)(a.re * b.im);
//  printf("numIm = %lx\n", numIm);
	
	result.re = OscDsplSatFr16(numRe/denum);
	result.im = OscDsplSatFr16(numIm/denum);
		
	return result;

}


// Copyright (C) 2000-2005 Analog Devices, Inc.
// This file is subject to the terms and conditions of the GNU Lesser
// General Public License. See the file COPYING.LIB for more details.
//
// Non-LGPL License is also available as part of VisualDSP++
// from Analog Devices, Inc.

/******************************************************************
	Func Name:    cmlt_fr16

	Description:  multiplication of two complex numbers

******************************************************************/
complex_fract16 OscDspl_cmlt_fr16 ( complex_fract16 a, complex_fract16 b )
{
	complex_fract16 result;
	fract32 real, imag;

	real = (a.re * b.re - a.im * b.im)>> 15;
	imag = (a.re * b.im  + a.im * b.re)>> 15;
		
	if(real >= FR16_MAX)
		result.re = FR16_MAX;
	else if(real <= FR16_MIN)
		result.re = FR16_MIN;
	else
		result.re = real;

	if(imag >= FR16_MAX)
		result.im = FR16_MAX;
	else if(imag <= FR16_MIN)
		result.im = FR16_MIN;
	else
		result.im = imag;

	return (result);
}


complex_fract16 OscDspl_conj_fr16 ( complex_fract16 a )
{
	complex_fract16 c;
	
	c.re = a.re;
	c.im = -a.im;

	return (c);
}


// Copyright (C) 2002-2006 Analog Devices, Inc.
// This file is subject to the terms and conditions of the GNU Lesser
// General Public License. See the file COPYING.LIB for more details.
//
// Non-LGPL License is also available as part of VisualDSP++
// from Analog Devices, Inc.
//******************************************************************************
void OscDspl_twidfftrad2_fr16(complex_fract16 w[], int n )
{
	int      i, idx;
	int      nquart = n/4;
#ifdef __USE_FAST_LOOKUP__
	fract16  val[nquart+1]; //index starting at 1!
#else
	fract16  val;
#endif
	float    step;

	step = 1.0/(float)nquart;
	idx  = 0;

	// 1. Quadrant
	// Compute cosine and -sine values for the range [0..PI/2)
	w[idx].re = FR16_MAX;  //=cos(0)
	w[idx].im = 0x0;     //=sin(0)
	for(i = 1; i < nquart; i++)
	{
	idx++;
#ifdef __USE_FAST_LOOKUP__
	val[i] = (fract16) ((i*step) * 32767.0); //count up
	w[idx].re = OscDspl_cos_fr16(val[i]);
	w[idx].im = -OscDspl_sin_fr16(val[i]);
#else
	val = (fract16) ((i*step) * 32767.0); //count up
	w[idx].re = OscDspl_cos_fr16(val);
	w[idx].im = -OscDspl_sin_fr16(val);
#endif
	}

	// 2. Quadrant
	// Compute cosine values for the range [PI/2..PI)
	// Since sin( [PI/2..PI] ) a mirror image of sin( [0..PI/2] )
	// no need to compute sine again
	idx++;
	w[idx].re = 0x0;      //=cos(PI/2)
	w[idx].im = FR16_MIN;   //=-sin(PI/2);
	for(i = 1; i < nquart; i++)
	{
	idx++;
#ifdef __USE_FAST_LOOKUP__
	w[idx].re = -OscDspl_cos_fr16(val[nquart-i]);
	w[idx].im = w[nquart-i].im;
#else
	val = (fract16) (((nquart-i)*step) * 32767.0); //count down
	w[idx].re = -OscDspl_cos_fr16(val);
	w[idx].im = w[nquart-i].im;
#endif
	}
}

// Taken from http://www.codeproject.com/KB/recipes/howtofft.aspx
void OscDsplRBitrev(const fract16 in[], fract16 out[], int size)
{
	fract16 tmp;
	int i,j;
	for (i=0;i<size;i++)
		out[i]=in[i];
	
	j=1;
	for (i=1;i<size;i++)
	{
		if (j > i)
		{
			tmp=out[i-1];
			out[i-1]=out[j-1];
			out[j-1]=tmp;
		}
		int  m=size/2;
		while (m >= 2 && j > m)
		{
			j -= m;
			m = m/2;
		}
		j += m;
	}
}

void OscDsplCBitrev(const complex_fract16 in[],complex_fract16 out[],int size)
{
	complex_fract16 tmp;
	int i,j;
	for (i=0;i<size;i++)
		out[i]=in[i];
	
	j=1;
	for (i=1;i<size;i++)
	{
		if (j > i)
		{
			tmp=out[i-1];
			out[i-1]=out[j-1];
			out[j-1]=tmp;
		}
		int  m=size/2;
		while (m >= 2 && j > m)
		{
			j -= m;
			m = m/2;
		}
		j += m;
	}
}


int OscDsplRDscale( fract16 butterflyfr16[], int fft_size)
{
	int i;
	for(i=0; i< fft_size; i++)
	{
		if(butterflyfr16[i] > 0x3FFF || butterflyfr16[i] < (fract16) 0xC000 )
			return 1;
	}
	return 0;
}


int OscDsplCDscale( complex_fract16 butterflycfr16[], int fft_size)
{
	int i;
	for(i=0; i< fft_size; i++)
	{
		if(butterflycfr16[i].re > 0x3FFF || butterflycfr16[i].im > 0x3FFF ||
				butterflycfr16[i].re < (fract16) 0xC000 || butterflycfr16[i].im < (fract16) 0xC000 )
			return 1;
	}
	return 0;
}


void  OscDspl_rfft_fr16( const fract16          in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling )
{
	fract16 bitrevfr16[fft_size];
	complex_fract16 butterflycfr16[fft_size];
	fract32 accu0fr32,accu1fr32;
	complex_fract16 R3,R2;
	int i,j, dscale;
	int group,bOffset;
	int twOffset, twAct;
	

	for(i=0;i<fft_size;i++)
	{
		butterflycfr16[i].re=0;
		butterflycfr16[i].im=0;
	}
	
	dscale=0;
	*block_exponent=0;
	
	
	/*  Bit reversing */
	OscDsplRBitrev(in,bitrevfr16,fft_size);
	
	/*stage 1 static scaling,no saturation
	 * (for more details about ASM: R6 = R2 +|+ R3, R7 = R2 -|- R3 (ASR)
	 * refer to manual page 708*/
	if(scaling == 2)
		dscale = OscDsplRDscale(bitrevfr16,fft_size);
	if(dscale || scaling == 1)
		*block_exponent = *block_exponent + 1;
	
	for(i=0;i<fft_size;i=i+2)
	{
		if(scaling == 3) /* no scaling */
		{
			butterflycfr16[i].re=OscDsplSatFr16((fract32)(bitrevfr16[i] + bitrevfr16[i+1]));
			butterflycfr16[i+1].re=OscDsplSatFr16((fract32)(bitrevfr16[i] - bitrevfr16[i+1]));
		}
		else if(scaling == 2)
		{
			if(dscale==1)
			{
				butterflycfr16[i].re=(bitrevfr16[i] + bitrevfr16[i+1]) >> 1;
				butterflycfr16[i+1].re=(bitrevfr16[i] - bitrevfr16[i+1]) >> 1;
			}
			else
			{
				butterflycfr16[i].re=(bitrevfr16[i] + bitrevfr16[i+1]);
				butterflycfr16[i+1].re=(bitrevfr16[i] - bitrevfr16[i+1]);
			}
		}
		else /* static scaling */
		{
			butterflycfr16[i].re=(bitrevfr16[i] + bitrevfr16[i+1]) >> 1;
			butterflycfr16[i+1].re=(bitrevfr16[i] - bitrevfr16[i+1]) >> 1;
		}
	}
	/* other stages */
	bOffset=2;
	group=fft_size/4;
	twOffset=stride * fft_size/4;
	twAct=0;
	dscale=0;
	while(group>0)
	{
		if(scaling == 2)
			dscale = OscDsplCDscale(butterflycfr16,fft_size);
		if(dscale || scaling == 1)
			*block_exponent = *block_exponent + 1;

		for(i=0; i< group; i++)
		{
			for(j=0; j<bOffset; j++)
			{
				R2=butterflycfr16[2*i*bOffset+j];
				R3=butterflycfr16[(2*i+1)*bOffset+j];

				if(j==0)
				{
					twAct = 0;
				}
				else
				{
					twAct = (twAct + twOffset) % (stride*fft_size/2);
					accu1fr32=(fract32) R3.re * twiddle[twAct].im;
					accu0fr32=(fract32) R3.re * twiddle[twAct].re;
					accu1fr32 += (fract32)R3.im * twiddle[twAct].re;
					accu0fr32 -= (fract32)R3.im * twiddle[twAct].im;
					R3.im = OscDsplTransRfr32fr16(accu1fr32);
					R3.re = OscDsplTransRfr32fr16(accu0fr32);
				}
				if(scaling == 3)    /* no scaling */
				{
					butterflycfr16[2*i*bOffset+j].re    =OscDsplSatFr16((fract32)(R2.re + R3.re));
					butterflycfr16[2*i*bOffset+j].im    =OscDsplSatFr16((fract32)(R2.im + R3.im));
					butterflycfr16[(2*i+1)*bOffset+j].re=OscDsplSatFr16((fract32)(R2.re - R3.re));
					butterflycfr16[(2*i+1)*bOffset+j].im=OscDsplSatFr16((fract32)(R2.im - R3.im));
				}
				else if(scaling == 2)
				{
					if(dscale == 1)
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
					}
					else
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re);
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im);
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re);
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im);
					}
				}
				else                /* static scaling */
				{
					butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
					butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
				}
			}
		}
		group /=2;
		bOffset *=2;
		twOffset /= 2;
		dscale=0;
	}
	
	for(i=0;i<fft_size;i++)
		out[i]=butterflycfr16[i];
}

void  OscDspl_cfft_fr16( const complex_fract16   in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling )
{
	complex_fract16 bitrevfr16[fft_size];
	complex_fract16 butterflycfr16[fft_size];
	fract32 accu0fr32,accu1fr32;
	complex_fract16 R3,R2;
	int i,j,dscale;
	int group,bOffset;
	int twOffset, twAct;

	for(i=0;i<fft_size;i++)
	{
		butterflycfr16[i].re=0;
		butterflycfr16[i].im=0;
	}
	
	dscale = 0;
	*block_exponent = 0;
	
	/*  Bit reversing */
	OscDsplCBitrev(in,bitrevfr16,fft_size);
	
	/*stage 1 static scaling,no saturation
	 * (for more details about ASM: R6 = R2 +|+ R3, R7 = R2 -|- R3 (ASR)
	 * refer to manual page 708*/
	if(scaling == 2)
		dscale = OscDsplCDscale(bitrevfr16,fft_size);
	if( dscale == 1 || scaling == 1)
		*block_exponent = *block_exponent + 1;

	
	for(i=0;i<fft_size;i=i+2)
	{
		if(scaling == 3 )   /* no scaling */
		{
			butterflycfr16[i].re  =OscDsplSatFr16((fract32)(bitrevfr16[i].re + bitrevfr16[i+1].re));
			butterflycfr16[i].im  =OscDsplSatFr16((fract32)(bitrevfr16[i].im + bitrevfr16[i+1].im));
			butterflycfr16[i+1].re=OscDsplSatFr16((fract32)(bitrevfr16[i].re - bitrevfr16[i+1].re));
			butterflycfr16[i+1].im=OscDsplSatFr16((fract32)(bitrevfr16[i].im - bitrevfr16[i+1].im));
		}
		else if(scaling == 2)
		{
			if(dscale == 1)
			{
				butterflycfr16[i].re  =(bitrevfr16[i].re + bitrevfr16[i+1].re) >> 1;
				butterflycfr16[i].im  =(bitrevfr16[i].im + bitrevfr16[i+1].im) >> 1;
				butterflycfr16[i+1].re=(bitrevfr16[i].re - bitrevfr16[i+1].re) >> 1;
				butterflycfr16[i+1].im=(bitrevfr16[i].im - bitrevfr16[i+1].im) >> 1;
			}
			else
			{
				butterflycfr16[i].re  = bitrevfr16[i].re + bitrevfr16[i+1].re;
				butterflycfr16[i].im  = bitrevfr16[i].im + bitrevfr16[i+1].im;
				butterflycfr16[i+1].re= bitrevfr16[i].re - bitrevfr16[i+1].re;
				butterflycfr16[i+1].im= bitrevfr16[i].im - bitrevfr16[i+1].im;
			}
		}
		else                /* static scaling */
		{
			butterflycfr16[i].re  =(bitrevfr16[i].re + bitrevfr16[i+1].re) >> 1;
			butterflycfr16[i].im  =(bitrevfr16[i].im + bitrevfr16[i+1].im) >> 1;
			butterflycfr16[i+1].re=(bitrevfr16[i].re - bitrevfr16[i+1].re) >> 1;
			butterflycfr16[i+1].im=(bitrevfr16[i].im - bitrevfr16[i+1].im) >> 1;
		}
	}

	/* other stages */
	bOffset=2;
	group=fft_size/4;
	twOffset=stride * fft_size/4;
	twAct=0;
	dscale=0;
	
	while(group>0)
	{
		if(scaling == 2)
			dscale = OscDsplCDscale(butterflycfr16,fft_size);
		if(dscale == 1 || scaling == 1)
			*block_exponent = *block_exponent + 1;
		for(i=0; i< group; i++)
		{
		
			for(j=0; j<bOffset; j++)
			{
				R2=butterflycfr16[2*i*bOffset+j];
				R3=butterflycfr16[(2*i+1)*bOffset+j];

				if(j==0)
				{
					twAct = 0;
				}
				else
				{
					twAct = (twAct + twOffset) % (stride*fft_size/2);
					accu1fr32=(fract32) R3.re * twiddle[twAct].im;
					accu0fr32=(fract32) R3.re * twiddle[twAct].re;
					accu1fr32 += (fract32)R3.im * twiddle[twAct].re;
					accu0fr32 -= (fract32)R3.im * twiddle[twAct].im;
					R3.im = OscDsplTransRfr32fr16(accu1fr32);
					R3.re = OscDsplTransRfr32fr16(accu0fr32);
				}
				
				if(scaling == 3 )   /* no scaling */
				{
					butterflycfr16[2*i*bOffset+j].re    =OscDsplSatFr16((fract32)(R2.re + R3.re));
					butterflycfr16[2*i*bOffset+j].im    =OscDsplSatFr16((fract32)(R2.im + R3.im));
					butterflycfr16[(2*i+1)*bOffset+j].re=OscDsplSatFr16((fract32)(R2.re - R3.re));
					butterflycfr16[(2*i+1)*bOffset+j].im=OscDsplSatFr16((fract32)(R2.im - R3.im));
				}
				else if(scaling == 2)
				{
					if(dscale == 1)
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
					}
					else
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re);
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im);
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re);
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im);
					}
				}
				else
				{
					butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
					butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
				}
			}
		}
		group /=2;
		bOffset *=2;
		twOffset /= 2;
		dscale=0;
	}

	for(i=0;i<fft_size;i++)
		out[i]=butterflycfr16[i];
}

void  OscDspl_ifft_fr16( const complex_fract16   in[],
											complex_fract16  out[],
									const complex_fract16  twiddle[],
									int  stride,
									int  fft_size,
									int  *block_exponent,
									int  scaling )
{
	complex_fract16 bitrevfr16[fft_size];
	complex_fract16 butterflycfr16[fft_size];
	fract32 accu0fr32,accu1fr32;
	complex_fract16 R3,R2;
	int i,j,dscale;
	int group,bOffset;
	int twOffset, twAct=0;

	for(i=0;i<fft_size;i++)
	{
		butterflycfr16[i].re=0;
		butterflycfr16[i].im=0;
	}
	
	dscale = 0;
	*block_exponent = 0;
	/*  Bit reversing */
	OscDsplCBitrev(in,bitrevfr16,fft_size);
	
	/*stage 1 static scaling,no saturation
	 * (for more details about ASM: R6 = R2 +|+ R3, R7 = R2 -|- R3 (ASR)
	 * refer to manual page 708*/
	if(scaling == 2)
		dscale = OscDsplCDscale(bitrevfr16,fft_size);
	if(dscale == 1 || scaling == 1)
		*block_exponent = *block_exponent + 1;

	for(i=0;i<fft_size;i=i+2)
	{
		if(scaling == 3)
		{
			butterflycfr16[i].re  =OscDsplSatFr16((fract32)(bitrevfr16[i].re + bitrevfr16[i+1].re));
			butterflycfr16[i].im  =OscDsplSatFr16((fract32)(bitrevfr16[i].im + bitrevfr16[i+1].im));
			butterflycfr16[i+1].re=OscDsplSatFr16((fract32)(bitrevfr16[i].re - bitrevfr16[i+1].re));
			butterflycfr16[i+1].im=OscDsplSatFr16((fract32)(bitrevfr16[i].im - bitrevfr16[i+1].im));
		}
		else if(scaling == 2)
		{
			if(dscale == 1)
			{
				butterflycfr16[i].re  =(bitrevfr16[i].re + bitrevfr16[i+1].re) >> 1;
				butterflycfr16[i].im  =(bitrevfr16[i].im + bitrevfr16[i+1].im) >> 1;
				butterflycfr16[i+1].re=(bitrevfr16[i].re - bitrevfr16[i+1].re) >> 1;
				butterflycfr16[i+1].im=(bitrevfr16[i].im - bitrevfr16[i+1].im) >> 1;
			}
			else
			{
				butterflycfr16[i].re  =(bitrevfr16[i].re + bitrevfr16[i+1].re);
				butterflycfr16[i].im  =(bitrevfr16[i].im + bitrevfr16[i+1].im);
				butterflycfr16[i+1].re=(bitrevfr16[i].re - bitrevfr16[i+1].re);
				butterflycfr16[i+1].im=(bitrevfr16[i].im - bitrevfr16[i+1].im);
			}
		}
		else
		{
			butterflycfr16[i].re  =(bitrevfr16[i].re + bitrevfr16[i+1].re) >> 1;
			butterflycfr16[i].im  =(bitrevfr16[i].im + bitrevfr16[i+1].im) >> 1;
			butterflycfr16[i+1].re=(bitrevfr16[i].re - bitrevfr16[i+1].re) >> 1;
			butterflycfr16[i+1].im=(bitrevfr16[i].im - bitrevfr16[i+1].im) >> 1;
		}
	}

	/* other stages */
	bOffset=2;
	group=fft_size/4;
	twOffset=stride * fft_size/4;
	dscale=0;
	
	while(group>0)
	{
		if(scaling == 2)
			dscale = OscDsplCDscale(butterflycfr16,fft_size);
		if(dscale == 1 || scaling == 1)
			*block_exponent = *block_exponent + 1;

		for(i=0; i< group; i++)
		{
			for(j=0; j<bOffset; j++)
			{
				R2=butterflycfr16[2*i*bOffset+j];
				R3=butterflycfr16[(2*i+1)*bOffset+j];
				
				if(j==0)
				{
					twAct = 0;
				}
				else
				{
					twAct = (twAct + twOffset) % (stride*fft_size/2);
					accu1fr32=(fract32) R3.im * twiddle[twAct].re;
					accu0fr32=(fract32) R3.im * twiddle[twAct].im;
					accu1fr32 -= (fract32)R3.re * twiddle[twAct].im;
					accu0fr32 += (fract32)R3.re * twiddle[twAct].re;
					R3.im = OscDsplTransRfr32fr16(accu1fr32);
					R3.re = OscDsplTransRfr32fr16(accu0fr32);
				}
				if(scaling == 3)
				{
					butterflycfr16[2*i*bOffset+j].re    =OscDsplSatFr16((fract32)(R2.re + R3.re));
					butterflycfr16[2*i*bOffset+j].im    =OscDsplSatFr16((fract32)(R2.im + R3.im));
					butterflycfr16[(2*i+1)*bOffset+j].re=OscDsplSatFr16((fract32)(R2.re - R3.re));
					butterflycfr16[(2*i+1)*bOffset+j].im=OscDsplSatFr16((fract32)(R2.im - R3.im));
				}
				else if(scaling == 2)
				{
					if(dscale == 1)
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
					}
					else
					{
						butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re);
						butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im);
						butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re);
						butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im);
					}
				}
				else
				{
					butterflycfr16[2*i*bOffset+j].re    =(R2.re + R3.re) >> 1;
					butterflycfr16[2*i*bOffset+j].im    =(R2.im + R3.im) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].re=(R2.re - R3.re) >> 1;
					butterflycfr16[(2*i+1)*bOffset+j].im=(R2.im - R3.im) >> 1;
				}
			}
		}
		group /=2;
		bOffset *=2;
		twOffset /= 2;
		dscale = 0;
	}

	for(i=0;i<fft_size;i++)
		out[i]=butterflycfr16[i];
}

fract16 OscDspl_vecmax_fr16(const fract16 vec[], int length)
{
	fract16 max = FR16_MIN;
	int i;
	
	if(length <= 0)
		return 0;
	
	for(i=0; i<length;i++)
	{
		if(vec[i] > max)
			max = vec[i];
	}
	return max;
}

int OscDspl_vecmaxloc_fr16(const fract16 vec[], int length)
{
	fract16 max = FR16_MIN;
	int i;
	int maxLoc = 0;
	
	if(length <= 0)
		return maxLoc;
	
	for(i=0; i<length;i++)
	{
		if(vec[i] > max)
		{
			max=vec[i];
			maxLoc = i;
		}
	}
	return maxLoc;
}

fract16 OscDspl_vecmin_fr16(const fract16 vec[], int length)
{
	fract16 result = FR16_MAX;
	int i;
	
	if(length <= 0)
		return 0;
	
	for(i=0; i<length;i++)
	{
		if(vec[i] < result)
			result = vec[i];
	}
	return result;
}

fract16 OscDspl_vecminloc_fr16(const fract16 vec[], int length)
{
	fract16 result;
	int i,j;
	
	result = FR16_MAX;
	j=0;
	
	if(length <= 0)
		return j;
	
	for(i=0; i<length;i++)
	{
		if(vec[i] < result)
		{
			result=vec[i];
			j = i;
		}
	}
	return j;
}

long long int OscDsplSatFr64(long long int in)
{
	long long int result;
	if(in > FR32_MAX)
		result = FR32_MAX;
	else if(in < FR32_MIN)
		result = FR32_MIN;
	else
		result = in;
	
	return result;
}

fract16 OscDspl_var_fr16(const fract16 sample[], int length)
{
	int i;
	long long int sum64=0, sumsq64=0,sumn64=0;
	fract16 sumn16, result, mone;
	mone = FR16_MIN;
	
	if(length < 2)
		return 0;
	
	if(length < 256)
	{
		for(i=0; i< length; i++)
		{
			sum64 += sample[i];
			sumsq64 +=  sample[i]*sample[i];
		}
		sumsq64 = sumsq64 >> 15;
	}
	else
	{
		for(i=0; i< length; i++)
		{
			sum64 = OscDsplSatFr64(sum64 + sample[i]);
			sumsq64 = OscDsplSatFr64(sumsq64 + ((sample[i]*sample[i])>> 15));
		}
	}
	
	sumn16 = OscDsplSatFr16((fract32)sum64/length);
	sumn64 = (fract32) (sumn16);
	sumn64 = (sumn64*sum64) >> 15;
	
	result = OscDsplSatFr16(((fract32)sumsq64 - (fract32)sumn64)/(length-1));

	return result;
}

void OscDspl_histogram_fr16(    const fract16 samples[],
							int histogram[],
							fract16 max_sample,
							fract16 min_sample,
							int sample_length,
							int bin_count)
{
	int bin_size,i,j;
	fract16 bin_max;
	
	if(sample_length <1 || bin_count < 1 || max_sample <= min_sample)
		return;
	
	bin_size = (max_sample - min_sample)/bin_count;
//  printf("bin_size = %d\n", bin_size);
	if(bin_size == 0)
		return;
	
	for(j=0; j < bin_count; j++)
		histogram[j]=0;
	
	for(i=0; i < sample_length; i++)
	{
		bin_max = min_sample;
		if( samples[i] < bin_max)
			continue;
		for(j=0;j < bin_count;j++)
		{
			bin_max +=bin_size;
			if(samples[i]< bin_max)
			{
				histogram[j]++;
				break;
			}
		}
	}
}

fract16 OscDspl_sqrt_fr16(fract16 x)
{
	fract16 sqrtcoeff[]={0x2d41,0xd2ce,0xe7e8,0xf848,0xac7c,0x2d42,0x2d31,0xea5d,0x1021,0xf89e};
	fract32 add32;
	fract16 xcp, xorig,result;
	int i,icp,m,signbits;
	
	xorig = x;
	result = 0x0;

	if(xorig <=  0x0)
	{
		return result;
	}
	signbits = 0;
	if(xorig < 0x2)
		signbits = 7;
	else if(xorig < 0x8)
		signbits = 6;
	else if(xorig < 0x20)
		signbits = 5;
	else if(xorig < 0x80)
		signbits = 4;
	else if(xorig < 0x200)
		signbits = 3;
	else if(xorig < 0x800)
		signbits = 2;
	else if(xorig < 0x2000)
		signbits = 1;
	
	x = x << (2*signbits);
	
	if(x >= 0x4000){
		i=5;
		x = x - 0x4000;
	}
	else
	{
		i = 0;
		x= -(x - 0x4000);
	}

	xcp = x;
	m = i+5;
	icp=i+1;
	add32 = sqrtcoeff[i] << 16;
	for(i=icp;i<m; i++)
	{
		add32 += (fract32)xcp* (fract32)sqrtcoeff[i] << 1;
		xcp = OscDsplMultRFr16(x,xcp);
	}
	
	result = (fract16) (add32>>15) ;
	result = result >> signbits;
	
	return result;
}

fract16 OscDspl_cabs_fr16(complex_fract16 c)
{
	fract16 result=0,div16;
	fract32 mult32;
	complex_fract16 ccp;
	
	if(c.re < 0)
	{
		if(c.re == FR16_MIN)
			ccp.re = FR16_MAX;
		else
		ccp.re = -c.re;
	}
	else
		ccp.re = c.re;
	
	if(c.im < 0)
	{
		if(c.im == FR16_MIN)
			ccp.im = FR16_MAX;
		else
			ccp.im = -c.im;
	}
	else
		ccp.im = c.im;
		
	if(ccp.re == 0 && ccp.im == 0)
		return result;
	else if(ccp.re == 0)
		return ccp.im;
	else if(ccp.im == 0)
		return ccp.re;
	
	if(ccp.re == ccp.im)
	{
		if(ccp.re >= 0x5a82)    /* overflow check if real >= 0.707 */
			return FR16_MAX;
		else
		{
			result = OscDsplMultRFr16(0x5a82,ccp.re) << 1; /* Result = (1.414 * real) * 2 */
			return result;
		}
	}
	
	if(ccp.re >= ccp.im)
	{
		div16 = OscDsplSatFr16(((((fract32)ccp.im) << 16)/(fract32)ccp.re)>>1);
		mult32 = ((fract32) div16 * div16) << 1;
		div16 = (fract16)(mult32 >> 0x12);
		div16 += 0x2000;
		div16 = OscDspl_sqrt_fr16(div16);
		mult32 = ((fract32) ccp.re*div16) >> 14;
		result = OscDsplSatFr16(mult32);
	}
	else
	{
		div16 = OscDsplSatFr16(((((fract32)ccp.re) << 16)/(fract32)ccp.im)>>1);
		mult32 = ((fract32) div16 * div16) << 1;
		div16 = (fract16)(mult32 >> 0x12);
		div16 += 0x2000;
		div16 = OscDspl_sqrt_fr16(div16);
		mult32 = ((fract32) ccp.im*div16) >> 14;
		result = OscDsplSatFr16(mult32);
	}
	return result;
}
