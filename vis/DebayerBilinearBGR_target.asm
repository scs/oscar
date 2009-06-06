.text
        .align 4 

	.global _DebayerBilinearBGR

_preproc_row:	/* 	P0:	Temporary buffer
			R2: 	Width
			P2: 	Source one row up
			P3:	Source current row
			P4:	Source one row down
			Clobbers LC0, R0 - R4, P0 - P5, I0 - I3
	
	 For bilinear interpolation, there are 4 possible filters:
	 - Vertical average:     01 (02) 03  One row up
	                         11 !12! 13  Current row
	                         21 (22) 23  One row down
	 VertAvg[12] = (02 + 22)/2
	
	 - Horizontal average:   01  02  03  One row up
	                        (11)!12!(13) Current row
	                         21  22  23  One row down
	 HorizAvg[12] = (11 + 13)/2
	
	 - X average:           (01) 02 (03) One row up
	                         11 !12! 13  Current row
	                        (21) 22 (23) One row down
	 XAvg[12] = ((01 + 21)/2 + (03 + 23)/2)/2
	
	 - + average:            01 (02) 03  One row up
	                        (11)!12!(13) Current row
	                         21 (22) 23  One row down
	 PlusAvg[12] = ((11 + 13)/2 + (02 + 22)/2)/2
	 
	 The supplied temporary memory area is divided into 4 sections of size
	 <width> and the calculated filters are saved there per pixel in 
	 above error.
	*/

	I0 = P2		// Source Pointer one row up
	I1 = P3		// Current source row
	I2 = P4		// Source pointer one row down
	PREFETCH [ P4++]

	R0 = P0			// Vertical average temporary

	R0 = R0 + R2
	P1 = R0			// Horizontal average temporary

	R0 = R0 + R2
	P2 = R0			// X Horiz and vertical average temporary

	R0 = R0 + R2		// + Horiz and vertical average temporary
	P3 = R0

	R2 >>= 2
	R2 += -1
	LC0 = R2		// Save (width/4 - 1) to LC0
	
	I3 = P0		// Vertical average temporary

	/************** Prologue ******************/
	/* Vertical average */
	R0 = [I0++] || R2 = [I2++]
	R0 = BYTEOP1P (R1:0, R3:2)
	// R0.0 = ((0, -1) + (0, 1))/2
	// R0.1 = ((1, -1) + (1, 1))/2
	// R0.2 = ((2, -1) + (2, 1))/2
	// R0.3 = ((3, -1) + (3, 1))/2	
	[ P0++ ] = R0
	
	/************** Main Loop *****************/
	loop preproc_loop LC0
	loop_begin preproc_loop

	/* Vertical average */
	R0 = [I0++] || R2 = [I2++]
	PREFETCH [ P4]
	P4 += 4
	R4 = BYTEOP1P (R1:0, R3:2) || 		R0.l = W[I1++]
	// R0.0 = ((0, -1) + (0, 1))/2
	// R0.1 = ((1, -1) + (1, 1))/2
	// R0.2 = ((2, -1) + (2, 1))/2
	// R0.3 = ((3, -1) + (3, 1))/2

	
	/* Horizontal average */
	R0.h = W[I1] || R2.l = W[I1++]
	R2.h = W[I1] || 			[ P0++ ] = R4
	// R0.0 = (0, 0)
	// R0.1 = (1, 0)
	// R0.2 = (2, 0)
	// R0.3 = (3, 0)
	// R2.0 = (2, 0)
	// R2.1 = (3, 0)
	// R2.2 = (4, 0) 
	// R2.3 = (5, 0)
	R4 = BYTEOP1P (R1:0, R3:2) ||		R0.l = W[I3++]
	// R0.0 = ((0, 0) + (2, 0))/2
	// R0.1 = ((1, 0) + (3, 0))/2
	// R0.2 = ((2, 0) + (4, 0))/2
	// R0.3 = ((3, 0) + (5, 0))/2

	/* X Horizontal and vertical average. */
	R0.h = W[I3] || R2.l = W[I3++]
	R2.h = W[I3] || I3 -= 4
	// R0.0 = V(0, 0) = ((0, -1) + (0, 1))/2
	// R0.1 = V(1, 0) = ...
	// R0.2 = V(2, 0) = ...
	// R0.3 = V(3, 0) = ...
	// R2.0 = V(2, 0) = ...
	// R2.1 = V(3, 0) = ...
	// R2.2 = V(4, 0) = ...
	// R2.3 = V(5, 0) = ...
	R4 = BYTEOP1P (R1:0, R3:2) ||		[ P1 ] = R4
	// R0.0 = (V(0, 0) + V(2, 0))/2 = (((0, -1) + (0, 1))/2 + ((2, -1) + (2, 1))/2)/2
	// R0.1 = (V(1, 0) + V(3, 0))/2 = ...
	// R0.2 = (V(2, 0) + V(4, 0))/2 = ...
	// R0.3 = (V(3, 0) + V(5, 0))/2 = ...

	/* + Horizontal and vertical average. */
	R0 = [ I3++ ]
	R1 = [ I3 ]
	R0 = ALIGN8 (R1, R0) || R2 = [ P1++ ]
	// R0.0 = V(1, 0) = ((1, -1) + (1, 1))/2
	// R0.1 = V(2, 0) = ...
	// R0.2 = V(3, 0) = ...
	// R0.3 = V(4, 0) = ...
	// R2.0 = H(0, 0) = ((0, 0) + (2, 0))/2
	// R2.1 = H(1, 0) = ((1, 0) + (3, 0))/2
	// R2.2 = H(2, 0) = ((2, 0) + (4, 0))/2
	// R2.3 = H(3, 0) = ((3, 0) + (5, 0))/2	
	R0 = BYTEOP1P (R1:0, R3:2) || 		[ P2++ ] = R4
	// R0.0 = (V(1,0) + H(0, 0))/2 = (((1, -1) + (1, 1))/2 + ((0, 0) + (2, 0))/2)/2
	// R0.1 = (V(2,0) + H(1, 0))/2 = ...
	// R0.2 = (V(3,0) + H(2, 0))/2 = ...
	// R0.3 = (V(4,0) + H(3, 0))/2 = ...	
	[ P3++ ] = R0
	
	loop_end preproc_loop

	/*********** Epilogue **************/
	/* Horizontal average */
	R0.l = W[I1++] || R0.h = W[I1]
	R2.l = W[I1++] 
	// R0.0 = (0, 0)
	// R0.1 = (1, 0)
	// R0.2 = (2, 0)
	// R0.3 = (3, 0)
	// R2.0 = (2, 0)
	// R2.1 = (3, 0)
	// R2.2 = 0	Never used
	// R2.3 = 0	Never used
	R4 = BYTEOP1P (R1:0, R3:2) || 		R0.l = W[I3++]
	// R0.0 = ((0, 0) + (2, 0))/2
	// R0.1 = ((1, 0) + (3, 0))/2
	// R0.2 = ((2, 0))/2  Never used
	// R0.3 = ((3, 0))/2  Never used

	/* X Horizontal and vertical average. */
	R0.h = W[I3] || R2.l = W[I3--]
	// R0.0 = V(0, 0) = ((0, -1) + (0, 1))/2
	// R0.1 = V(1, 0) = ...
	// R0.2 = V(2, 0) = ...
	// R0.3 = V(3, 0) = ...
	// R2.0 = V(2, 0) = ...
	// R2.1 = V(3, 0) = ...
	// R2.2 = 0	Never used
	// R2.3 = 0	Never used
	R3 = BYTEOP1P (R1:0, R3:2) || 			[ P1 ] = R4
	// R0.0 = (V(0, 0) + V(2, 0))/2 = (((0, -1) + (0, 1))/2 + ((2, -1) + (2, 1))/2)/2
	// R0.1 = (V(1, 0) + V(3, 0))/2 = ...
	// R0.2 = (V(2, 0))/2  Never used
	// R0.3 = (V(3, 0))/2  Never used

	/* + Horizontal and vertical average. */
	R0 = [ I3++ ]
	R0 = ALIGN8 (R1, R0) || R2 = [ P1++ ]
	// R0.0 = V(1, 0) = ((1, -1) + (1, 1))/2
	// R0.1 = V(2, 0) = ...
	// R0.2 = V(3, 0) = ...
	// R0.3 = V(4, 0) = 0
	// R2.0 = H(0, 0) = ((0, 0) + (2, 0))/2
	// R2.1 = H(1, 0) = ((1, 0) + (3, 0))/2
	// R2.2 = 0
	// R2.3 = 0	
	R0 = BYTEOP1P (R1:0, R3:2) || 			[ P2++ ] = R3	
	// R0.0 = (V(1,0) + H(0, 0))/2 = (((1, -1) + (1, 1))/2 + ((0, 0) + (2, 0))/2)/2
	// R0.1 = (V(2,0) + H(1, 0))/2 = ...
	// R0.2 = 0	Never used
	// R0.3 = 0	Never used		
	[ P3++ ] = R0
	
	rts

_preproc_row_first_or_last:
		/* 	P0:	Temporary buffer
			R2: 	Width
			P2: 	Source one row up/down
			P3:	Source current row
			Clobbers LC0-1, R0 - R7, P0 - P5, I0 - I3 */

	I0 = P2		// Source Pointer one row up/down
	I1 = P3		// Current source row
	P4 = P2
	
	R7 = 3		// Divide by 3
	R6 = 15      /* Evaluate the quotient to 16 bits (Division). */
	
	PREFETCH [ P4++]

	R0 = P0			// Vertical average temporary

	R0 = R0 + R2
	P1 = R0			// Horizontal average temporary

	R0 = R0 + R2
	P2 = R0			// X Horiz and vertical average temporary

	R0 = R0 + R2		// + Horiz and vertical average temporary
	P3 = R0

	R2 >>= 2
	R2 += -1
	LC0 = R2		// Save (width/4 - 1) to LC0
	
	I3 = P0		// Vertical average temporary
	P5 = P0
	P5 += 1

	/************** Prologue ******************/
	/* Vertical average (just take point in row above/below) */
	R0 = [I0++]
	[ P0++ ] = R0
	
	/************** Main Loop *****************/
	loop preproc_first_or_last_loop LC0
	loop_begin preproc_first_or_last_loop

	/* Vertical average (just take point in row above/below) */
	R4 = [I0++]
	PREFETCH [ P4 ]
	P4 += 4
	R0.l = W[I1++]
	// R0.0 = (0, -1)
	// R0.1 = (1, -1)
	// R0.2 = (2, -1)
	// R0.3 = (3, -1)

	
	/* Horizontal average */
	R0.h = W[I1] || R2.l = W[I1++]
	R2.h = W[I1] || 			[ P0++ ] = R4
	// R0.0 = (0, 0)
	// R0.1 = (1, 0)
	// R0.2 = (2, 0)
	// R0.3 = (3, 0)
	// R2.0 = (2, 0)
	// R2.1 = (3, 0)
	// R2.2 = (4, 0) 
	// R2.3 = (5, 0)
	R4 = BYTEOP1P (R1:0, R3:2) ||		R0.l = W[I3++]
	// R0.0 = ((0, 0) + (2, 0))/2
	// R0.1 = ((1, 0) + (3, 0))/2
	// R0.2 = ((2, 0) + (4, 0))/2
	// R0.3 = ((3, 0) + (5, 0))/2

	/* X Horizontal and vertical average. */
	R0.h = W[I3] || R2.l = W[I3++]
	R2.h = W[I3]
	// R0.0 = V(0, 0) = ((0, -1) + (0, 1))/2
	// R0.1 = V(1, 0) = ...
	// R0.2 = V(2, 0) = ...
	// R0.3 = V(3, 0) = ...
	// R2.0 = V(2, 0) = ...
	// R2.1 = V(3, 0) = ...
	// R2.2 = V(4, 0) = ...
	// R2.3 = V(5, 0) = ...
	R4 = BYTEOP1P (R1:0, R3:2) ||		[ P1 ] = R4
	// R0.0 = (V(0, 0) + V(2, 0))/2 = ((0, -1) + (2, -1))/2
	// R0.1 = (V(1, 0) + V(3, 0))/2 = ...
	// R0.2 = (V(2, 0) + V(4, 0))/2 = ...
	// R0.3 = (V(3, 0) + V(5, 0))/2 = ...

	[ P2++ ] = R4
	
	/* + Horizontal and vertical average. */

	// Point1
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (r1, r7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_div_preproc_first_or_last1 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_div_preproc_first_or_last1
	divq (r1, r7)
	loop_end div_div_preproc_first_or_last1
	B[P3++] = R1

	// Point2
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (R1, R7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_div_preproc_first_or_last2 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_div_preproc_first_or_last2
	divq (R1, R7)
	loop_end div_div_preproc_first_or_last2
	B[P3++] = R1

	// Point3
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (R1, R7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_div_preproc_first_or_last3 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_div_preproc_first_or_last3
	divq (R1, R7)
	loop_end div_div_preproc_first_or_last3
	B[P3++] = R1

	// Point4
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (R1, R7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_div_preproc_first_or_last4 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_div_preproc_first_or_last4
	divq (R1, R7)
	loop_end div_div_preproc_first_or_last4
	
	B[P3++] = R1	

	
	loop_end preproc_first_or_last_loop

	/*********** Epilogue **************/
	/* Horizontal average */
	R0.l = W[I1++] || R0.h = W[I1]
	R2.l = W[I1++] 
	// R0.0 = (0, 0)
	// R0.1 = (1, 0)
	// R0.2 = (2, 0)
	// R0.3 = (3, 0)
	// R2.0 = (2, 0)
	// R2.1 = (3, 0)
	// R2.2 = 0	Never used
	// R2.3 = 0	Never used
	R4 = BYTEOP1P (R1:0, R3:2) || 		R0.l = W[I3++]
	// R0.0 = ((0, 0) + (2, 0))/2
	// R0.1 = ((1, 0) + (3, 0))/2
	// R0.2 = ((2, 0))/2  Never used
	// R0.3 = ((3, 0))/2  Never used

	/* X Horizontal and vertical average. */
	R0.h = W[I3] || R2.l = W[I3--]
	// R0.0 = V(0, 0) = ((0, -1) + (0, 1))/2
	// R0.1 = V(1, 0) = ...
	// R0.2 = V(2, 0) = ...
	// R0.3 = V(3, 0) = ...
	// R2.0 = V(2, 0) = ...
	// R2.1 = V(3, 0) = ...
	// R2.2 = 0	Never used
	// R2.3 = 0	Never used
	R3 = BYTEOP1P (R1:0, R3:2) || 			[ P1 ] = R4
	// R0.0 = (V(0, 0) + V(2, 0))/2 = (((0, -1) + (0, 1))/2 + ((2, -1) + (2, 1))/2)/2
	// R0.1 = (V(1, 0) + V(3, 0))/2 = ...
	// R0.2 = (V(2, 0))/2  Never used
	// R0.3 = (V(3, 0))/2  Never used

	[ P2++ ] = R3
	
	/* + Horizontal and vertical average. */

	// Point1
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (R1, R7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_preproc_first_or_last5 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_preproc_first_or_last5
	divq (R1, R7)
	loop_end div_preproc_first_or_last5
	B[P3++] = R1

	// Point2
	LC1 = R6
	R0 = B[P5++]
	R2 = B[P1++]
	R2 <<= 1
	R1 = R0 + R2
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (R1, R7)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_preproc_first_or_last6 lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_preproc_first_or_last6
	divq (R1, R7)
	loop_end div_preproc_first_or_last6
	B[P3++] = R1

	LC0 = R5
	rts
	
_bgbg_to_bgr:
	/*	Assemble the output (BGR format) for a BGBG Bayer row. 
		P0: 	Vertical average
		P1:	Horizontal average
		P2:	X Average
		P3:	+ Average
		P4:	Source row
		P5:	Destination row
		LC0:	width/4 - 1
		Clobbers LC0-1, P0-P5, R0-R6
		P4 and P5 contain the source resp. destination pointer
		for the next line at the end.

	Idea:	Read byte-by-byte from the preprocessed interpolation data and the
		source row. Abuse the accumulators and MAC instructions to deposit
		the resulting bytes 4 at a time into a 32-bit word. Then, write
		back the whole word.
	*/

	R4.l = 1
	R4.h = 256
	I0 = P5


	/******* First Blue pixel *******/
	// Blue color of first blue pixel	
	R0 = B[P4++]

	// Green color of first blue pixel
	R2 = B[P0++]
	R7 = B[P4]
	R2 <<= 1
	R1 = R2 + R7
	// Divide R1 by 3
	R6 = 15      /* Evaluate the quotient to 16 bits. */
	LC1 = R6
	R6 = 3
	R1 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (r1, r6)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_bgbg_to_bgr lC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_bgbg_to_bgr
	divq (r1, r6)
	loop_end div_bgbg_to_bgr

	// Red color of first blue pixel
	R2 = B[P0]
	
	/********** Green pixel ***********/
	// Blue color of green pixel
	R3 = B[P1++]

	// Green color of green pixel
	A0 = R4.l * R0.l (FU)   		|| R0 = B[P4++]

	// Red color of green pixel
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P0++]

	P3 += 1
	P2 += 1
	


	loop row_loop_bgbg_to_bgr LC0
	loop_begin row_loop_bgbg_to_bgr

	/********** Blue pixel ***********/
	// Blue color of blue pixel
	A1 = R4.l * R2.l (FU)   		|| R2 = B[P4++]

	// Green color of blue pixel
	R5.h=(A1 += R4.h * R3.l) (IU)        	|| R3 = B[P3++]

	// Red color of blue pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P2++]

	P0 += 1
	P1 += 1

	/********** Green pixel ***********/
	// Blue color of green pixel
	R5.l=(A0 += R4.h * R1.l) (IU)      	|| R1 = B[P1++] || [I0++] = R5

	// Green color of green pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P4++]
	
	// Red color of green pixel
	R5.h=(A1 += R4.h * R3.l) (IU)	  	|| R3 = B[P0++]

	P3 += 1
	P2 += 1

	/********** Blue pixel ***********/
	// Blue color of blue pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P4++]
	
	// Green color of blue pixel	
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P3++] || [I0++] = R5
	
	// Red color of blue pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P2++]

	P0 += 1
	P1 += 1

	/********** Green pixel ***********/
	// Blue color of green pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P1++]

	// Green color of green pixel
	A0 = R4.l * R0.l (FU)   		|| R0 = B[P4++]

	// Red color of green pixel
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P0++] || [I0++] = R5

	P3 += 1
	P2 += 1
		
	loop_end row_loop_bgbg_to_bgr



	
	/********** Blue pixel ***********/
	// Blue color of blue pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P4++]

	// Green color of blue pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P3++]

	// Red color of blue pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P2++]

	P0 += 1
	P1 += 1
	
	
	/******* Last green pixel *********/
	// Blue color of last green pixel
	R5.l=(A0 += R4.h * R1.l) (IU)				|| [I0++] = R5
	R1 = B[P4 - 1]

	// Green color of last green pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P4++]

	// Red color of last green pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P0++]


	
	A0 = R4.l * R0.l (FU)
	R5.l=(A0 += R4.h * R1.l) (IU)		|| [I0++] = R5
	A1 = R4.l * R2.l (FU)
	R5.h=(A1 += R4.h * R3.l) (IU)
	
	[I0++] = R5
	P5 = I0
	rts
	
	

_grgr_to_bgr:
	/*	Assemble the output (BGR format) for a GRGR Bayer row. 
		P0: 	Vertical average
		P1:	Horizontal average
		P2:	X Average
		P3:	+ Average
		P4:	Source row
		P5:	Destination row
		LC0:	width/4 - 1
		Clobbers LC0-1, P0-P5, R0-R6
		P4 and P5 contain the source resp. destination pointer
		for the next line at the end.

	Idea:	Read byte-by-byte from the preprocessed interpolation data and the
		source row. Abuse the accumulators and MAC instructions to deposit
		the resulting bytes 4 at a time into a 32-bit word. Then, write
		back the whole word.
	*/

	R4.l = 1
	R4.h = 256
	I0 = P5


	/******* First Green pixel *******/
	// Blue color of first green pixel	
	R0 = B[P0++]

	// Green color of first green pixel
	R1 = B[P4++]

	// Red color of first green pixel
	R2 = B[P4]
	
	/********** Red pixel ***********/
	// Blue color of red pixel
	R3 = B[P2++]

	// Green color of red pixel
	A0 = R4.l * R0.l (FU)   		|| R0 = B[P3++]

	// Red color of red pixel
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P4++]

	P0 += 1
	P1 += 1
	


	loop row_loop_grgr_to_bgr LC0
	loop_begin row_loop_grgr_to_bgr

	/********** Green pixel ***********/
	// Blue color of green pixel
	A1 = R4.l * R2.l (FU)   		|| R2 = B[P0++]

	// Green color of green pixel
	R5.h=(A1 += R4.h * R3.l) (IU)        	|| R3 = B[P4++]

	// Red color of green pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P1++]

	P2 += 1
	P3 += 1

	/********** Red pixel ***********/
	// Blue color of red pixel
	R5.l=(A0 += R4.h * R1.l) (IU)      	|| R1 = B[P2++] || [I0++] = R5

	// Green color of red pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P3++]
	
	// Red color of red pixel
	R5.h=(A1 += R4.h * R3.l) (IU)	  	|| R3 = B[P4++]

	P0 += 1
	P1 += 1

	/********** Green pixel ***********/
	// Blue color of green pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P0++]
	
	// Green color of green pixel	
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P4++] || [I0++] = R5
	
	// Red color of green pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P1++]

	P2 += 1
	P3 += 1

	/********** Red pixel ***********/
	// Blue color of red pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P2++]

	// Green color of red pixel
	A0 = R4.l * R0.l (FU)   		|| R0 = B[P3++]

	// Red color of red pixel
	R5.l=(A0 += R4.h * R1.l) (IU)	  	|| R1 = B[P4++] || [I0++] = R5

	P0 += 1
	P1 += 1
		
	loop_end row_loop_grgr_to_bgr



	
	/********** Green pixel ***********/
	// Blue color of green pixel
	A1 = R4.l * R2.l (FU)			|| R2 = B[P0++]

	// Green color of green pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P4++]

	// Red color of green pixel
	A0 = R4.l * R0.l (FU)			|| R0 = B[P1++]

	P2 += 1
	P3 += 1
	
	
	/******* Last red pixel *********/
	// Blue color of last red pixel
	R5.l=(A0 += R4.h * R1.l) (IU)				|| [I0++] = R5
	R1 = B[P0 - 1]

	// Green color of last red pixel
	A1 = R4.l * R2.l (FU)

	R2 = B[P0++]
	R7 = B[P4 - 1]
	R2 <<= 1
	R2 = R2 + R7
	// Divide R2 by 3
	R6 = 15      /* Evaluate the quotient to 16 bits. */
	LC1 = R6
	R6 = 3
	R2 <<= 1     /* Left shift dividend by 1 needed for integer divi-sion */
	divs (r2, r6)/* Evaluate quotient MSB. Initialize AQ status bit and dividend for the DIVQ loop. */
	loop div_grgr_to_bgr LC1      /* Evaluate DIVQ p0=15 times. */
	loop_begin div_grgr_to_bgr
	divq (r2, r6)
	loop_end div_grgr_to_bgr

	// Red color of last red pixel
	R5.h=(A1 += R4.h * R3.l) (IU)		|| R3 = B[P4++]


	
	A0 = R4.l * R0.l (FU)
	R5.l=(A0 += R4.h * R1.l) (IU)		|| [I0++] = R5
	A1 = R4.l * R2.l (FU)
	R5.h=(A1 += R4.h * R3.l) (IU)
	
	[I0++] = R5
	P5 = I0
	rts
	
	
	/* 	R0: DestPointer
		R1: Src Pointer
		R2: width
		Stack3: height
		Stack4: Temporary buffer
		Stack5:	Bayer order (0=BGBG, 1=RGRG, 2=GBGB, 3=GRGR) */
_DebayerBilinearBGR:
	link 16

	[ --sp ] = ( P5:3 )
	[ --sp ] = ( R7:4 )
	[ --sp ] = ASTAT

	[ FP - 4 ] = R2		// Save width to stack
	[ FP - 8 ] = R0  	// Save destination pointer to stack
	[ FP - 12 ] = R1 	// Save source pointer to stack
	
	R0 = R1 - R2
	P2 = R0			// Source Pointer one row up
	
	P3 = R1			// Source pointer current row
	
	R0 = R1 + R2
	P4 = R0			// Source pointer one row down

	R4 = [ FP + 20 ]	// height
	[ FP - 16 ] = R4	// Save current row counter (= height)

	R0 = ASTAT		// Disable unbiased rounding
	BITCLR ( R0, 8 )
	ASTAT = R0
	
img_loop_begin:
	R2 = [ FP - 4 ]		// Width

	P3 = [ FP - 12 ]	// Source pointer
	R0 = P3
	R1 = R0 - R2
	P2 = R1			// Calculate pointer one row up
	R1 = R0 + R2
	P4 = R1			// Calculate pointer one row down
	
	R0 = [ FP + 24]
	P0 = R0			// Temporary pointer

	/****** Do all the calculations (filters) for the current row *****/
	CC = R4 == 1
	if CC jump preproc_last_land
	R0 = [ FP + 20 ]		// height
	CC = R4 == R0
	if CC jump preproc_first_land
preproc_land:	
	call _preproc_row
preproc_done:	
	//jump exit
	P5 = [ FP - 4 ]		// width
	
	P0 = [ FP + 24]		// Vertical average
	P1 = P0 + P5		// Horizontal average
	P2 = P1 + P5		// X Average
	P3 = P2 + P5		// + Average

	R0 = P5
	R0 >>= 2		// Set loop counter to width/4 - 1
	R0 += -1
	LC0 = R0
	
	P4 = [ FP - 12 ]	// Source pointer
	P5 = [ FP - 8 ]		// Destination pointer

	R0 = [ FP + 28 ]	// Bayer order
	CC = R0 == 0
	if !CC jump grgr_to_bgr_land
bgbg_to_bgr_land:
	call _bgbg_to_bgr
	
	R0 = 3
	[ FP + 28 ] = R0
	jump to_bgr_row_done
grgr_to_bgr_land:
	call _grgr_to_bgr
	R0 = 0
	[ FP + 28 ] = R0	
to_bgr_row_done:	
	
	[FP - 8] = P5		// Store destination pointer
	[FP - 12] = P4		// Store source pointer

	R4 = [ FP - 16 ]
	R4 += -1
	CC = R4 == 0
	[FP - 16] = R4
	if !CC jump img_loop_begin (BP)
img_loop_end:	

exit:
	ASTAT = [sp++]
	( R7:4 ) = [sp++]
	( P5:3 ) = [sp++]
	unlink
	rts

preproc_first_land:
	P2 = P4
preproc_last_land:	
	call _preproc_row_first_or_last
	jump preproc_done



