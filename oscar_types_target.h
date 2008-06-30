/*! @file oscar_types_target.h
 * @brief Type definitions for the target.
 * 
 */
#ifndef TYPES_H_
#define TYPES_H_

#define int8 	char
#define uint8 	unsigned char
#define int16 	short
#define uint16 	unsigned short
#define int32 	long
#define uint32 	unsigned long
#define int64 	long long
#define uint64 	unsigned long long

#define OSC_ERR int

#define bool    short
#define TRUE    (1==1)
#define FALSE   (!TRUE)

/*! @brief Endianness of this machine. Blackfin uses LITTLE_ENDIAN */
#define CPU_LITTLE_ENDIAN

/*! @brief Macro to swap the endianness of a 16 bit number */
#define ENDIAN_SWAP_16(x)       \
    (                           \
            (x>>8) | (x<<8)     \
    )
/*! @brief Macro to swap the endianness of a 32 bit number */
#define ENDIAN_SWAP_32(x)               \
    (                                   \
            (x>>24) |                   \
            ((x<<8) & 0x00FF0000) |     \
            ((x>>8) & 0x0000FF00) |     \
            (x<<24)                     \
    )
/*! @brief Macro to swap the endianness of a 64 bit number */
#define ENDIAN_SWAP_64(x)                           \
    (                                               \
            (x>>56) |                               \
            ((x<<40) & 0x00FF000000000000) |        \
            ((x<<24) & 0x0000FF0000000000) |        \
            ((x<<8)  & 0x000000FF00000000) |        \
            ((x>>8)  & 0x00000000FF000000) |        \
            ((x>>24) & 0x0000000000FF0000) |        \
            ((x>>40) & 0x000000000000FF00) |        \
            (x<<56)                                 \
    )

/*! @brief Load a 32 bit value from a non-aligned address. */
#define LD_INT32(pData) \
    ( \
            ((int32)((pData)[0])) | (((int32)((pData)[1])) << 8) | \
            (((int32)((pData)[2])) << 16) | (((int32)((pData)[3])) << 24)\
    )

/*! @brief Write a 32 bit value to a non-aligned address. */
#define ST_INT32(pData, val) \
{ \
    ((pData)[0] = (uint8)(val & 0x000000FF));   \
    ((pData)[1] = (uint8)((val & 0x0000FF00) >> 8));   \
    ((pData)[2] = (uint8)((val & 0x00FF0000) >> 16));   \
    ((pData)[3] = (uint8)((val & 0xFF000000) >> 24));     \
}

/*! @brief Load a 16 bit value from a non-aligned address. */
#define LD_INT16(pData) \
    ( \
            ((int16)((pData)[0])) | (((int16)((pData)[1])) << 8) \
    )

/*! @brief Write a 16 bit value to a non-aligned address. */
#define ST_INT16(pData, val) \
{ \
    ((pData)[0] = (uint8)(val & 0x00FF));      \
    ((pData)[1] = (uint8)((val & 0xFF00) >> 8));      \
}

#endif /*TYPES_H_*/
