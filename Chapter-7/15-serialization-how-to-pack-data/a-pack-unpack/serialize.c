#include <stdio.h>          // printf()
#include <stdint.h>         // uint32_t : fixed - size integers
#include <string.h>         // memcpy(), strlen()

// pack 32 - bit integer into buffer ( big - endian )
void packi32( unsigned char *buf, uint32_t val ) {
    /*
        takes a buffer pointer : *buf
        takes a 32 - bit integer : val
        
        stores the integer in big-endian byte order
        
        example : 12345 = 0x00003039
                                    Byte0 = 0x00
                                    Byte1 = 0x00
                                    Byte2 = 0x30
                                    Byte3 = 0x39
        
        this is network - safe format
    */

    buf[0] = val >> 24;
    // get highest byte

    buf[1] = val >> 16;
    // extracts the next 8 bits (1 byte)

    buf[2] = val >> 8;
    buf[3] = val;
}


// unpack 32 - bit integer from buffer
uint32_t unpacki32( unsigned char *buf ) {
    
    /* 
        reads 4 bytes and reconstructs integer
        
        rebuilds number by :
                            shifting bytes back
                            combining them using bitwise OR
        example : 
                0x00 << 24
                0x00 << 16
                0x30 << 8
                0x39
                = 12345
    */
    return ( 
        ( buf[0] << 24 ) | ( buf[1] << 16 ) | ( buf[2] << 8 )  | buf[3]
    );
}


// pack float using memcpy ( just a demo, not fully IEEE easliblished packer )
void packf( unsigned char *buf, float f ) {

    /*
        copies raw float bytes into buffer

        this does not convert endianness
        just copies IEEE float representation
    */
    memcpy( buf, &f, sizeof( float ) );
}


// unpack float - reads raw bytes back into float
float unpackf( unsigned char *buf ) {
    
    float f;

    memcpy( &f, buf, sizeof( float ) );

    return f;
}


int main() {

    unsigned char buffer[ 100 ];
    // byte buffer where all data is packed

    // below are the values to serialize :
    int num = 12345;
    float pi = 3.14159f;
    char msg[] = "Hello";

    printf( " Original values : \n" );
    printf( " int = %d \n", num );
    printf( " float = %f \n", pi );
    printf( " string = %s \n \n ", msg );

    // keeps track of where to write next in buffer
    int offset = 0;

    // pack int : writes 4 bytes - advances offset by 4
    packi32( buffer + offset, num );
    offset += 4;

    // pack float : writes float bytes - moves offset forward
    packf( buffer + offset, pi );
    offset += sizeof( float );

    // pack string length : stores string length first ( to know how many bytes to be read later on )
    packi32( buffer + offset, strlen( msg ) );
    offset += 4;

    // pack string : copies characters - "Hello"
    memcpy( buffer + offset, msg, strlen( msg ) );
    offset += strlen( msg );

    // shows total bytes stored
    printf( " Packed %d bytes\n\n ", offset );



    // UNPACK

    // cursor for reading back
    int read_offset = 0;

    // to read int
    int num2 = unpacki32( buffer + read_offset );
    read_offset += 4;

    // to reads float
    float pi2 = unpackf( buffer + read_offset );
    read_offset += sizeof( float );

    // to read string length
    int len = unpacki32( buffer + read_offset );
    read_offset += 4;

    // copies string bytes - and adds null terminator
    char msg2[ 50 ];
    memcpy( msg2, buffer + read_offset, len );
    msg2[ len ] = '\0';

    // printing packed - unpacked values
    printf( " Unpacked values :\n " );
    printf( " int = %d\n ", num2 );
    printf( " float = %f\n ", pi2 );
    printf( " string = %s\n ", msg2 );

    /*
        buffer looks like this : [ int(4 bytes) ][ float(4 bytes) ][ strlen(4 bytes) ][ string bytes ]
    */
    return 0;
}
