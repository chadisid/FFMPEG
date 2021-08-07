
#include <stdio.h>
#include <stdlib.h>

typedef int32_t sox_int32_t;
typedef sox_int32_t sox_sample_t;

#define SOX_SAMPLE_LOCALS sox_sample_t sox_macro_temp_sample LSX_UNUSED; \
  double sox_macro_temp_double LSX_UNUSED
#define LSX_USE_VAR(x)  ((void)(x=0)) /* During static analysis, initialize unused variables to 0. */
/**
Client API:
Max value for sox_sample_t = 0x7FFFFFFF.
*//**
Client API:
Returns the smallest (negative) value storable in a twos-complement signed
integer with the specified number of bits, cast to an unsigned integer;
for example, SOX_INT_MIN(8) = 0x80, SOX_INT_MIN(16) = 0x8000, etc.
@param bits Size of value for which to calculate minimum.
@returns the smallest (negative) value storable in a twos-complement signed
integer with the specified number of bits, cast to an unsigned integer.
*/
#define SOX_INT_MIN(bits) (1 <<((bits)-1))

/**
Client API:
Returns the largest (positive) value storable in a twos-complement signed
integer with the specified number of bits, cast to an unsigned integer;
for example, SOX_INT_MAX(8) = 0x7F, SOX_INT_MAX(16) = 0x7FFF, etc.
@param bits Size of value for which to calculate maximum.
@returns the largest (positive) value storable in a twos-complement signed
integer with the specified number of bits, cast to an unsigned integer.
*/
#define SOX_INT_MAX(bits) (((unsigned)-1)>>(33-(bits)))


#define SOX_SAMPLE_MAX (sox_sample_t)SOX_INT_MAX(32)

/**
Client API:
Min value for sox_sample_t = 0x80000000.
*/
#define SOX_SAMPLE_MIN (sox_sample_t)SOX_INT_MIN(32)

#define SOX_FLOAT_32BIT_TO_SAMPLE(d,clips) SOX_FLOAT_64BIT_TO_SAMPLE(d, clips)
#define SOX_FLOAT_64BIT_TO_SAMPLE(d, clips)                     \
  (sox_sample_t)(                                               \
    LSX_USE_VAR(sox_macro_temp_sample),                         \
    sox_macro_temp_double = (d) * (SOX_SAMPLE_MAX + 1.0),       \
    sox_macro_temp_double < 0 ?                                 \
      sox_macro_temp_double <= SOX_SAMPLE_MIN - 0.5 ?           \
        ++(clips), SOX_SAMPLE_MIN :                             \
        sox_macro_temp_double - 0.5 :                           \
      sox_macro_temp_double >= SOX_SAMPLE_MAX + 0.5 ?           \
        sox_macro_temp_double > SOX_SAMPLE_MAX + 1.0 ?          \
          ++(clips), SOX_SAMPLE_MAX :                           \
          SOX_SAMPLE_MAX :                                      \
        sox_macro_temp_double + 0.5                             \
  )
  
int main(int argc, char **argv)
{
    const char *outfilename, *filename;
    FILE *f, *outfile;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(1);
    }
    filename    = argv[1];
    outfilename = argv[2];
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    outfile = fopen(outfilename, "wb");
    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", outfilename);
        exit(1);
    }
    int64_t data_size, current_offset;
    fseek(f, 0, SEEK_END);
    data_size = ftell(f);
    printf("Size of input file %lld\n",data_size);
    fseek(f, 0, SEEK_SET);
    float inbuf,max_val,min_val;
    int bytes;
    while (current_offset <= data_size) {
        bytes = fread(&inbuf, sizeof(float), 1, f);
        if(bytes < 1)
            break;
        current_offset += sizeof(float);
        if(inbuf > max_val)  {
            max_val = inbuf;
        }    
        if(inbuf < min_val) {
            min_val = inbuf;
        }    
        printf("Sample values %.6f current offset %lld data_size %lld max_val %.6f min_val %.6f\n",inbuf,current_offset, data_size, max_val,min_val);    
        sox_macro_temp_double = (d) * (SOX_SAMPLE_MAX + 1.0);
        if(sox_macro_temp_double < 0) {                                 
          if(sox_macro_temp_double <= SOX_SAMPLE_MIN - 0.5) {           \
            ++(clips)
           } SOX_SAMPLE_MIN :                             \
        sox_macro_temp_double - 0.5 :                           \
      sox_macro_temp_double >= SOX_SAMPLE_MAX + 0.5 ?           \
        sox_macro_temp_double > SOX_SAMPLE_MAX + 1.0 ?          \
          ++(clips), SOX_SAMPLE_MAX :                           \
          SOX_SAMPLE_MAX :                                      \
        sox_macro_temp_double + 0.5      
            
LSX_USE_VAR(sox_macro_temp_sample), sox_macro_temp_double = (d) * (SOX_SAMPLE_MAX + 1.0), sox_macro_temp_double < 0 ? (sox_macro_temp_double <= SOX_SAMPLE_MIN - 0.5 ? (++(clips), SOX_SAMPLE_MIN) : sox_macro_temp_double - 0.5) : (sox_macro_temp_double >= SOX_SAMPLE_MAX + 0.5 ? (sox_macro_temp_double > SOX_SAMPLE_MAX + 1.0 ? (++(clips), SOX_SAMPLE_MAX) : SOX_SAMPLE_MAX) : sox_macro_temp_double + 0.5 )
    }
    return 0;
}
