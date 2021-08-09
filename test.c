#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
typedef int32_t sox_int32_t;
typedef sox_int32_t sox_sample_t;
#define LOG2_10      3.32192809488736234787  /* log_2 10 */
#define PI	3.14159265359
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
#define SOX_SAMPLE_TO_FLOAT_32BIT(d,clips) ((d)*(1.0 / (SOX_SAMPLE_MAX + 1.0)))

/**
Client API:
Min value for sox_sample_t = 0x80000000.
*/
#define SOX_SAMPLE_MIN (sox_sample_t)SOX_INT_MIN(32)
#define SOX_SAMPLE_LOCALS sox_sample_t sox_macro_temp_sample; \
  double sox_macro_temp_double 
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
  size_t clips_t = 0;
size_t clips_two = 0;
/* Helper struct to generate and parse WAVE headers */
typedef struct wav_header {
	char riff[4];
	uint32_t len;
	char wave[4];
	char fmt[4];
	uint32_t formatsize;
	uint16_t format;
	uint16_t channels;
	uint32_t samplerate;
	uint32_t avgbyterate;
	uint16_t samplebytes;
	uint16_t channelbits;
	char data[4];
	uint32_t blocksize;
} wav_header;
typedef struct filter_type {
    double gain;
    double frequency;
    double mix;
    double i1, i2;
    double o1, o2;
    double a0, a1, a2;
    double b0, b1, b2;	
} filter_type;

static void filter (const void *input, void *output, int len,         
                          double *in1, double *in2,                         
                          double *out1, double *out2,                       
                          double b0, double b1, double b2,                  
                          double a1, double a2)                                     
{    
    //len is number of samples	
    printf("Inside biquad filter \n");                   
    float *ibuf = input;                                                 
    float *obuf = output;                                                      
    double i1 = *in1;                                                         
    double i2 = *in2;                                                         
    double o1 = *out1;                                                        
    double o2 = *out2;                                                        
    double wet = 1; //s->mix = 1;                                                      
    double dry = 1. - wet;                                                    
    double out;                                                               
    int i;                                                                    
    a1 = -a1;                                                                 
    a2 = -a2;                                                                 
                                                                              
    for (i = 0; i+1 < len; i++) {                                             
        o2 = i2 * b2 + i1 * b1 + ibuf[i] * b0 + o2 * a2 + o1 * a1;            
        i2 = ibuf[i];                                                         
        out = o2 * wet + i2 * dry;                                                                                                        
        obuf[i] = out;                                                                                                                       
        i++;                                                                  
        o1 = i1 * b2 + i2 * b1 + ibuf[i] * b0 + o1 * a2 + o2 * a1;            
        i1 = ibuf[i];                                                         
        out = o1 * wet + i1 * dry;                                                                                                        
        obuf[i] = out;                                                    
                                                                            
    }                                                                         
    if (i < len) {                                                            
        double o0 = ibuf[i] * b0 + i1 * b1 + i2 * b2 + o1 * a1 + o2 * a2;     
        i2 = i1;                                                              
        i1 = ibuf[i];                                                         
        o2 = o1;                                                              
        o1 = o0;                                                              
        out = o0 * wet + i1 * dry;                                                                                                        
        obuf[i] = out;                                                                                                                       
    }                                                                         
    *in1  = i1;                                                               
    *in2  = i2;                                                               
    *out1 = o1;                                                               
    *out2 = o2;                                                               
}



int main(int argc, char **argv)
{
    const char *outfilename, *filename;
    FILE *f, *outfile, *outraw;
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
    outraw = fopen("xx.raw", "wb");
    if (!outraw) {
        fprintf(stderr, "Could not open\n" );
        exit(1);
    }
    /* Write WAV header */
    wav_header header = {
	    {'R', 'I', 'F', 'F'},
	    0,
	    {'W', 'A', 'V', 'E'},
	    {'f', 'm', 't', ' '},
	    16,
	    0x0003,
	    1,
	    48000,
	    48000 * 2,
	    2,
	    16,
	    {'d', 'a', 't', 'a'},
	    0
	    };
    if(fwrite(&header, 1, sizeof(header), outfile) != sizeof(header)) {
	printf("Error writing WAV header...\n");
    }
    fflush(outfile);
    int64_t data_size, current_offset;
    fseek(f, 0, SEEK_END);
    data_size = ftell(f);
    printf("Size of input file %lld\n",data_size);
    fseek(f, 0, SEEK_SET);
    float inbuf[8192],max_val,min_val,max_val_two,min_val_two;
    int bytes; 
    double frequency_lowpass = 300;
    int sample_rate = 48000;
    double w0_lowpass = 2 * PI * frequency_lowpass / sample_rate;
    filter_type *filter_lowpass = (filter_type *)malloc(sizeof(filter_type));
    memset(filter_lowpass, 0, sizeof(filter_type));
    //Low pass start
    filter_lowpass->a0 = 1;
    filter_lowpass->a1 = -exp(-w0);
    filter_lowpass->a2 = 0;
    filter_lowpass->b0 = 1 + a1;
    filter_lowpass->b1 = 0;
    filter_lowpass->b2 = 0;
    // Low pass end	
    while (current_offset < data_size) {
        bytes = fread(&inbuf, sizeof(float), 8192, f);
        current_offset += sizeof(float)*bytes;
        int i = 0;
        for(i = 0; i<bytes; i++) {
		if(inbuf[i] > max_val)  {
		    max_val = inbuf[i];
		}    
		if(inbuf[i] < min_val) {
		    min_val = inbuf[i];
		}
		//inbuf[i] = 0.05*inbuf[i];
		SOX_SAMPLE_LOCALS;
		sox_sample_t ty = SOX_FLOAT_32BIT_TO_SAMPLE(inbuf[i],clips_t);
		float td = SOX_SAMPLE_TO_FLOAT_32BIT(ty,clips_two);
		inbuf[i] = td;
		if(td > max_val_two)  {
		    max_val_two = td;
		}
		if(td < min_val_two) {
		    min_val_two = td;
		}
        }
	filter (inbuf, output, bytes, cache_lowpass->i1, cache_lowpass->i2, cache_lowpass->o1, cache_lowpass->o2, b0, b1, b2, a1, a2)    
        fwrite(inbuf, sizeof(float), bytes, outfile);
        fwrite(inbuf, sizeof(float), bytes, outraw);
        printf("Sample values %f current offset %lld data_size %lld max_val %f min_val %f clips %i clips_two %i min_val_two %f max_val_two %f\n",inbuf[0],current_offset, data_size, max_val,min_val,clips_t,clips_two,min_val_two,max_val_two);    
     }
     fseek(outfile, 0, SEEK_END);
     long int size = ftell(outfile);
     if(size >= 8) {
	size -= 8;
	fseek(outfile, 4, SEEK_SET);
	fwrite(&size, sizeof(uint32_t), 1, outfile);
	size += 8;
	fseek(outfile, 40, SEEK_SET);
	fwrite(&size, sizeof(uint32_t), 1, outfile);
	fflush(outfile);
	fseek(outfile, 0, SEEK_END);
    }
    fclose(outfile);
    fflush(outraw);
    fclose(outraw);
    return 0;
}
