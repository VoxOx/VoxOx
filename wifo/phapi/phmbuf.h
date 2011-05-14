#ifndef __PHMBUF_H__
#define __PHMBUF_H__ 1

/**
 * @brief abstraction for a media buffer
 *
 * a media buffer can be used to store samples and keep a context
 * of where we're at in the reading of this buffer
 */
struct ph_mediabuf
{
  short  *buf; /** a pointer to a buffer of samples (short) */
  int    next; /** current offset from the beginning of the buffer */
  int    size; /** total size of the buffer in samples */
};
typedef struct ph_mediabuf ph_mediabuf_t;

/**
 * @brief initialize a pre-existing mediabuffer with a buffer and its size in bytes
 */
void ph_mediabuf_init(ph_mediabuf_t *mb, void *buf, int size);
/**
 * @brief free the internal buffer of a mediabuffer
 */
void ph_mediabuf_cleanup(ph_mediabuf_t *mb);


ph_mediabuf_t *ph_mediabuf_new(int size);
void ph_mediabuf_free(ph_mediabuf_t *mb);

/**
 * @brief mixes a mediabuffer with a given buffer of samples
 *
 * mixing begins on the ->next sample, and the mediabuffer has a max size of ->size
 *
 * @param mb destination mediabuffer that the mix buffer will be mixed with
 * @param mix buffer of samples
 * @param samples size in samples ('short' not 'bytes') of the mix buffer
 * @return number of mixed samples
 */
int ph_mediabuf_mixaudio(ph_mediabuf_t *mb, short *mix, int samples);

/**
 * @brief mixes a source mediabuffer with a destination mediabuffer
 *
 * The 2 buffers are mixed up to the shortest of the 2 mediabuffers
 * mixing begins at the start of the mediabuffers
 * sizes are decided with the ->next data
 *
 * @param dmb destination media buffer
 * @param smb1 source media buffer (source 1)
 */
void ph_mediabuf_mixmedia(ph_mediabuf_t *dmb, ph_mediabuf_t *smb1);

/**
 * @brief mixes 2 source mediabuffers into an empty destination mediabuffer
 *
 * mixing begins at the start of the mediabuffers
 * sizes are defined by the ->next data
 *
 * @param dmb destination media buffer
 * @param smb1 source media buffer (source 1)
 * @param smb2 source media buffer (source 2)
 * @param framesize we want to mix up to framesize samples (in 'short' samples, not in bytes)
 */
void ph_mediabuf_mixmedia2(ph_mediabuf_t *dmb, ph_mediabuf_t *smb1, ph_mediabuf_t *smb2, int framesize);

ph_mediabuf_t *ph_mediabuf_load(const char *filename, int samplerate);


#ifndef PH_MEDIA_NATIVE_FILE_RATE
#define PH_MEDIA_NATIVE_FILE_RATE 16000
#endif

#endif
