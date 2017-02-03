#ifndef LTYPES_H_
#define LTYPES_H_

struct mutex {
	pthread_mutex_t lock;
};

static inline void mutex_init(struct mutex* mutex) {
	pthread_mutex_init(&mutex->lock, NULL);
}

static inline void mutex_lock(struct mutex* mutex) {
	pthread_mutex_lock(&mutex->lock);
}

static inline void mutex_unlock(struct mutex* mutex) {
	pthread_mutex_unlock(&mutex->lock);
}

static inline int mutex_trylock(struct mutex* mutex) {
	return pthread_mutex_trylock(&mutex->lock);
}

typedef union {
	struct {
		// Root Layer
		unsigned short preamble_size;
		unsigned short postamble_size;
		unsigned char  acn_id[12];
		unsigned short root_flength;
		unsigned int   root_vector;
		unsigned char  cid[16];

		// Frame Layer
		unsigned short frame_flength;
		unsigned int   frame_vector;
		unsigned char  source_name[64];
		unsigned char  priority;
		unsigned short reserved;
		unsigned char  sequence_number;
		unsigned char  options;
		unsigned short universe;

		// DMP Layer
		unsigned short dmp_flength;
		unsigned char  dmp_vector;
		unsigned char  type;
		unsigned short first_address;
		unsigned short address_increment;
		unsigned short property_value_count;
		unsigned char  property_values[513];
	} __attribute__((packed));
	unsigned char raw[638];
} e131_packet_t;


typedef struct _GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;

#define WM_APP 0x8000

//#define INVALID_SOCKET  (unsigned int)(~0)
//#define SOL_SOCKET      0xffff          /* options for socket level */
//#define SO_REUSEADDR    0x0004          /* allow local address reuse */
//#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define SOCKET int
#define closesocket close
#define SOCKET_ERROR -1
#define SOCKADDR sockaddr

inline int WSAGetLastError() {
	return 0;
}

#define RGB(r,g,b)          ((unsigned long)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))

static inline int sprintf_s(char* buf, size_t buf_count, const char* fmt, ...) {
	va_list args;
	int i;
	va_start(args, fmt);
	i = sprintf(buf, fmt, args);
	va_end(args);
	return i;
}

static inline void msleep(unsigned long msec) {
	usleep(1000 * msec);
}

#endif /* LTYPES_H_ */
