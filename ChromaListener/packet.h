#ifndef PACKET_H_
#define PACKET_H_

#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )

PACK(
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
	};
	unsigned char raw[638];
} e131_packet_t);

#endif /* PACKET_H_ */
