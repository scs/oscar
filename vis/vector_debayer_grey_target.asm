.text
        .align 4 

	.global _debayer_grey
	
_debayer_grey:
	link 0
	[ --sp ] = ( P5:3 )
	[ --sp ] = R4
	

	P3 = R0 		// Destination pointer
	P4 = R1			// Source Line 1
	R0 = R1 + R2		// Source Line 2
	P5 = R0		

	I0 = 0			// Define byte alignment for vector operation later

	R4 = R2			// Load width
	R0 = [ FP + 20 ]	// Load height/2 to loop counter
	R0 = R0 >> 1
	LC0 = R0

	loop y_loop LC0		// Vertical loop
	loop_begin y_loop

	R0 = R4 >> 2
	LC1 = R0		// Load width/4 to loop counter
	
	loop x_loop LC1		// Row loop
	loop_begin x_loop
	
	R0 = [P4++]		// Load 4 Pixels of rows x and x+1
	R2 = [P5++]

	//; Average our 2 x (G + R + G + B) pixels together into 2 x 1 grey pixels
	R0 = BYTEOP2P ( R3:2, R1:0 ) (RNDL)

	//; Write result back to destination pointer
	R0 = R0 >> 16 || B[P3++] = R0 
	B[P3++] = R0
	
	loop_end x_loop

	R0 = P4
	R0 = R0 + R4
	P4 = R0		// Leave out a row (because we always merge 2 rows to one pixel)

	R0 = P5
	R0 = R0 + R4
	P5 = R0		
	
	loop_end y_loop
	
	R4 = [sp++]
	( P5:3 ) = [sp++]
	unlink
	rts
	