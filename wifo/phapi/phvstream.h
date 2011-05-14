#ifndef __PHVSTREAM_H__
#define __PHVSTREAM_H__ 1

#ifdef PHAPI_VIDEO_SUPPORT




/* Video frames fixed transmitting width */
#define PHMEDIA_VIDEO_FRAME_WIDTH 176
#define PHMEDIA_VIDEO_FRAME_HEIGHT 144

/**
 * structure that is used to hold an image buffer and its associated meta-datas.
 */
struct phm_videoframe
{
	piximage image;
#ifdef DEPRECATED	
	/** video frame buffer */
	uint8_t *buffer;
	/** video frame width */
	int width;
	/** video frame height */
	int height;
	/** video frame palette stated in the webcam osi palette (webcam library) */
	pixosi pix_osi;

	/** exact length of the video frame buffer */
	int len; 
	/**
	 * 0: buffer should not be freed upon freeing the struct,
	 * 1: buffer should be freed upon freeing the struct,
	 * 2: buffer should be av_freed upon freeing the struct
	 */
	int freetype;
#endif

};
typedef struct phm_videoframe phm_videoframe_t;

/**
 * structure that is used for a very simple ad-hoc 'frame server' architecture.
 *
 * at this stage it is used for creating a virtual webcam when no webcam can be opened
 */
struct phm_fserver
{
	/**
	 * FSM state for the frame server 'webcam'
	 * 0: n.a.
	 * 1: initialized
	 * 2: producing
	 */
	int state;
	/** internal buffer, specific to each fserver */
	uint8_t *buffer;
};
typedef struct phm_fserver phm_fserver_t;


/**
 * structure holding a rx rtp - packet with associated meta-data
 */
struct ph_video_stream_packet {
	mblk_t *mp;
	unsigned long timestamp;
	unsigned long seqnumber;
};


/**
 *	main structure for the video stream.
 *
 *	NB: it is CRUCIAL to memset this structure to 0 upon creation to initialize the sub-fields
 */
struct ph_video_stream
{
	struct phmstream ms; /** common stream sub-struct defined in phmstream.h */

	webcam *wt;
	phm_fserver_t phmfs_webcam;
	phm_fserver_t phmfs_onewaycam;

	struct osip_mutex *mtx; /** mutex used to protect inter-thread access to the osip_list frame queues. */
	struct osip_list rr_sent_q;
	struct osip_list webcam_frames_q; /** list of phm_videoframes that is the 'source' list for encoding */
	struct osip_list received_packets_q;

	/**
	 * some structures and buffers need to be pre-allocated to avoid
	 * keeping allocating/de-allocating memory for the images
	 * they are reffered to as the 'processing buffers' and
	 * usually have the 'proc' word in their name
	 */
	struct _piximage *local_frame_cache; /** this is a piximage for the most recent tx image */

    pixosi pix_proc_source; /** current pix of the source dedicated buffer */
	unsigned width_proc_source; /** current width of the source dedicated buffer */
	unsigned height_proc_source; /** current height of the source dedicated buffer */
    struct _piximage *image_ready_for_network; /** processing buffer */
    struct _piximage *image_wrong_pix; /** processing buffer used when webcam does not output yuv420P */
	
	
	void (*rtpCallback)(void *stream, void *data, int size,
			unsigned long ts, int eof);
	int mbCounter;
	
	/** number of encoded frames since the beginning of the stream. */
	int num_encoded_frames;
	/** how many decoded frames where un-sliced through markbit detection. */
	int stat_markbit_hit;
	/** how many decoded frames where un-sliced through timestamp detection. */
	int stat_timestamp_hit;
	int stat_num_decoded_total_over_5s;
	int stat_num_decoded_dropped_over_5s;
	struct timeval stat_ts_decoded_over_5s;

	/** number of encoding thread wake-ups since the beginning of the stream*/
	int threadcount_encoding;

	struct osip_thread *media_io_thread;
	struct osip_thread *media_bw_control_thread;

	void (*videoCallback)();
 
	int sent_cnt;
	int recv_cnt;
                     
	/**
	 * local timestamp evaluation for the tx path (transmission)  based on the local origin (0).
	 *
	 * the txtstamp is incremented for each frame
	 */
	unsigned long txtstamp;

	/** timestamp evaluation for the rx path (reception) based on the local origin (0) */
	int rx_trace_initialize;
	unsigned long rx_current_timestamp;
	unsigned long rx_current_seqnumber;

	unsigned long rxtstamp;
	unsigned long rxtstamp_start;

	unsigned long current_group_stamp;
	unsigned long rxts_inc;

	unsigned int fps; /* FPS for encoding & decoding */
	unsigned int fps_interleave_time; /* Maximum time between two frames, in milliseconds */
	struct timeval last_decode_time; /* timeval of the last decoding */
	struct timeval last_encode_time; /* timeval of the last encoding */

        void (*frameDisplayCallback)(void *ctx, void *event);

	phVideoFrameReceivedEvent_t frame_event;

};

typedef struct ph_video_stream phvstream_t;

void *ph_video_io_thread(void *_p);
void *ph_video_bwcontrol_thread(void *_p);

#if 0
void ph_media_video_stop(phcall_t *ca);
int ph_media_video_start(phcall_t *ca, int video_port, phFrameDisplayCbk frameDisplay);
#endif


void ph_video_bwcontrol_apply_user_params(phvstream_t *stream);
/*
void ph_video_rtcp_rr_sent(RtpSession *, rtcp_t *);
void ph_video_rtcp_rr_received(RtpSession *, rtcp_t *);
void ph_video_rtcp_sr_sent(RtpSession *, rtcp_t *);
void ph_video_rtcp_sr_received(RtpSession *, rtcp_t *);
*/
void ph_handle_video_network_data(phvstream_t *stream, unsigned long timestamp, unsigned long *ts_inc);
int ph_media_video_send_frame(phvstream_t *video_stream, phm_videoframe_t *phmvf, int cache);
int ph_media_video_flush_queue(phvstream_t *stream, unsigned long seqnumber_start, unsigned long seqnumber_end);
void ph_media_video_alloc_processing_buffers(phvstream_t *vstream, pixosi nego_pix, unsigned nego_width, unsigned nego_height);
void ph_media_video_check_processing_buffers(phvstream_t *vstream, pixosi pix_source, unsigned width_source, unsigned height_source);
void ph_media_video_free_processing_buffers(phvstream_t *vstream);

/** Create a new video frame from a piximage. */
phm_videoframe_t *ph_media_video_new_video_frame(piximage *image);

/** Free the video frame. */
void ph_media_free_video_frame(phm_videoframe_t *ptr);


#endif

#endif
