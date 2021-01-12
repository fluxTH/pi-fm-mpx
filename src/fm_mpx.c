/*
    PiFmRds - FM/RDS transmitter for the Raspberry Pi
    Copyright (C) 2014 Christophe Jacquet, F8FTK
    
    See https://github.com/ChristopheJacquet/PiFmRds
    
    rds_wav.c is a test program that writes a RDS baseband signal to a WAV
    file. It requires libsndfile.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    fm_mpx.c: generates an FM multiplex signal containing RDS plus possibly
    monaural or stereo audio.
*/

#include <sndfile.h>
#include <soxr.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>


size_t length;
int channels;
int audio_rate;
uint32_t mpx_rate;

float *audio_buf = NULL;
int audio_len = 0;

float *leftover_buf = NULL;
uint32_t leftover_len = 0;

float resample_ratio;

SNDFILE *inf;
soxr_t soxr_h = NULL;

float *alloc_empty_buffer(size_t length) {
    float *p = malloc(length * sizeof(float));
    if(p == NULL) return NULL;
    
    bzero(p, length * sizeof(float));
    
    return p;
}


int fm_mpx_open(char* filename, size_t len, uint32_t* mpx_rate_param) {
    length = len;

    if(filename != NULL) {
        // Open the input file
        SF_INFO sfinfo;
 
        // stdin or file on the filesystem?
        if(filename[0] == '-') {
            if(! (inf = sf_open_fd(fileno(stdin), SFM_READ, &sfinfo, 0))) {
                fprintf(stderr, "Error: could not open stdin for audio input.\n") ;
                return -1;
            } else {
                printf("Using stdin for audio input.\n");
            }
        } else {
            if(! (inf = sf_open(filename, SFM_READ, &sfinfo))) {
                fprintf(stderr, "Error: could not open input file %s.\n", filename) ;
                return -1;
            } else {
                printf("Using audio file: %s\n", filename);
            }
        }
        
        audio_rate = sfinfo.samplerate;

        if (*mpx_rate_param == 0) {
            if (audio_rate < 176400 || audio_rate > 228000) {
                // TODO: Add resampler
                fprintf(stderr, "Error: Input sample rate %d Hz not supported.\n", audio_rate);
                return -1;
            } else {
                // MPX Rate = Input Rate
                *mpx_rate_param = audio_rate;
            }
        }

        mpx_rate = *mpx_rate_param;
        channels = sfinfo.channels;

        if (channels > 1) {
            printf("%d channels detected, multichannel audio is not supported.\n", channels);
            return -1;
        }

        printf("Input sample rate: %d Hz, 1 channel MPX ", audio_rate);
        if (mpx_rate == audio_rate) {
            printf("passthrough.\n");
            resample_ratio = 1.f;
        } else {
            printf("resampled.\n");

            soxr_error_t soxr_e;
            soxr_h = soxr_create(
                audio_rate, /* Input sample-rate. */
                mpx_rate,   /* Output sample-rate. */
                1,          /* Number of channels to be used. */

                /* All following arguments are optional (may be set to NULL). */
                &soxr_e,    /* To report any error during creation. */
                NULL,       /* To specify non-default I/O formats. */
                NULL,       /* To specify non-default resampling quality.*/
                NULL
            );

            if (soxr_e != 0) {
                fprintf(stderr, "Error: Resampling engine failed to initialize.\n");
                return -1;
            }

            leftover_buf = alloc_empty_buffer(floor((float)length * 0.2f));
            resample_ratio = (float)audio_rate / (float)mpx_rate;
        }
        
        audio_buf = alloc_empty_buffer(floor((float)length / resample_ratio));
        if(audio_buf == NULL) return -1;

    } // end if(filename != NULL)
    else {
        inf = NULL;
        // inf == NULL indicates that there is no audio
    }
    
    return 0;
}

int fm_mpx_get_samples(float *mpx_buffer) {
    if(inf  == NULL) return 0; // if there is no audio, stop here

    audio_len = 0;

    int tries_left = 5;
    int last_read_len;

    int target_len = (float)length / resample_ratio;

    do {
        last_read_len = sf_read_float(inf, audio_buf + audio_len, target_len - audio_len);
        if (last_read_len < 0) {
            fprintf(stderr, "Error reading audio\n");
            return -1;
        }

        audio_len += last_read_len;
        tries_left--;
    } while (audio_len < target_len && tries_left >= 0);

    if (soxr_h != NULL && resample_ratio != 1.f) {
        // Resampling enabled
        soxr_error_t soxr_e = soxr_process(
            soxr_h,

            /* Input (to be resampled): */
            audio_buf,       /* Input buffer(s); may be NULL (see below). */
            audio_len,          /* Input buf. length (samples per channel). */
            NULL,               /* To return actual # samples used (<= ilen). */

            /* Output (resampled): */
            mpx_buffer,                         /* Output buffer(s).*/
            (float)audio_len * resample_ratio,         /* Output buf. length (samples per channel). */
            NULL
        );

        audio_len *= resample_ratio;
    } else {
        // Sample passthrough
        memcpy(mpx_buffer, audio_buf, audio_len * sizeof(float));
    }

    if (audio_len < length) {
        memset(mpx_buffer + audio_len, 0.f, length - audio_len);
    }
    
    return 0;
}


int fm_mpx_close() {
    if(sf_close(inf) ) {
        fprintf(stderr, "Error closing audio file");
    }
    
    if (audio_buf != NULL) free(audio_buf);
    if (leftover_buf != NULL) free(leftover_buf);
    if (soxr_h != NULL) soxr_delete(soxr_h);
    
    return 0;
}
