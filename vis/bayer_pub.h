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

/*! @file bayer_pub.h
 * @brief Header file shared by cam and vis modules (soft-linked).
 * Contains information about bayer patterns needed by both modules.
 */
#ifndef BAYER_PUB_H_
#define BAYER_PUB_H_

/*! @brief The order in which the colored pixels of a bayer pattern
 * appear in a row.
 * 
 * The colors are abbreviated as follows:
 * - G: Green
 * - R: Red
 * - B: Blue
 * 
 * The enum is constructed from two bools; one saying whether the first
 * pixel in the row is green and the other whether it is a red or blue
 * row.
 *          firstGreen      firstOther
 * red          11              01
 * blue         10              00
 * */
enum EnBayerOrder
{
	ROW_BGBG = 0,
	ROW_RGRG = 1,
	ROW_GBGB = 2,
	ROW_GRGR = 3
};

#endif /*BAYER_PUB_H_*/

