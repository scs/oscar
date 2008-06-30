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
 * 			firstGreen		firstOther
 * red			11				01
 * blue			10				00
 * */
enum EnBayerOrder
{
	ROW_BGBG = 0, 
	ROW_RGRG = 1,
	ROW_GBGB = 2,
	ROW_GRGR = 3
};

#endif /*BAYER_PUB_H_*/

