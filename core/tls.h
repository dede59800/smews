#ifndef __TLS_H__
#define __TLS_H__

#include "types.h"
#include "rand.h"


extern uint8_t tls_get_client_hello();
extern uint8_t tls_send_hello_cert_done();

/* partially precomputed server hello, certificate and done message together with TLS record header */
extern uint8_t s_hello_cert_done[];
extern CONST_VAR(uint8_t,ec_priv_key_256[]);

#define DEBUG  /* TODO this should be a build option */
#define DEBUG_DEEP
#define ECDH_ECDSA_RC4_128_SHA1 /* TODO this should be a build option */

#ifdef ECDH_ECDSA_RC4_128_SHA1

#include "rc4.h"
#include "sha1.h"
#include "md5.h"

#define CIPHER_KEYSIZE 	RC4_KEYSIZE 
#define MAC_KEYSIZE 	SHA1_KEYSIZE 

#endif

/* TLS version */
#define TLS_SUPPORTED_MAJOR 0x03
#define TLS_SUPPORTED_MINOR 0x01

/* supported by Firefox */
#define TLS1_ECDH_ECDSA_WITH_RC4_128_SHA 0xc007
#define TLS1_ECDH_ECDSA_WITH_RC4_128_SHA_KEXCH_LEN 70
#define TLS1_ECDHE_ECDSA_WITH_RC4_128_SHA 0xc007
#define TLS1_ECDH_ECDSA_WITH_AES_128_CBC_SHA 0xc004

/* supported by IE, Chrome, Firefox */
#define TLS1_ECDHE_ECDSA_WITH_AES_128_CBC_SHA 0xc009

/* general TLS return values */
#define HNDSK_ERR 0
#define HNDSK_OK 1

/* extension common values */
#define TLS_EXTENSION_SECP256R1 0x17
#define TLS_EXT_POINT_FORMATS 0x0b
#define TLS_EXT_EC 0x0a
#define TLS_EXT_LEN 0x06

/* Compression Method - no compression*/
#define TLS_COMPRESSION_NULL 0x00

/* lengths for Hello,Certificate and HelloDone handshake records
 * containing the size of handshake header (eg length(3) + handshake type(1)*/
#define TLS_HELLO_RECORD_LEN 50
#define TLS_CERT_RECORD_LEN 482
#define TLS_HDONE_RECORD_LEN 4

#define TLS_RECORD_HEADER_LEN 5
#define TLS_HELLO_CERT_DONE_LEN (TLS_RECORD_HEADER_LEN + TLS_HELLO_RECORD_LEN + TLS_CERT_RECORD_LEN + TLS_HDONE_RECORD_LEN)


#define PRF_LABEL_SIZE 13 /* label size for MS and Session Keys computation */
#define MS_LEN 48 	  	  /* master secret len */
#define PMS_LEN 32	  	  /* pre-master secret len */
#define KEY_MATERIAL_LEN 72		/* Material Len for Session Keys */


/* needed for sequence number manipulation */
union long_byte {
	uint8_t bytes[8];
	uint64_t long_int;
};



struct tls_connection {

	/* TLS connection */
	enum tls_state_e { 

			   client_hello, /* Waiting for Client Hello message */
			   server_hello, /* Server Hello (including Certificate Message and Hello Done) */

			   key_exchange, /* Waiting for Client Key Exchange */
			   ccs_recv,     /* Waiting for Change Cipher Spec */
			   fin_recv, 	 /* Waiting for Finished Message (encrypted) */
			   ccs_send,     /* Sending Change Cipher Spec */
			   fin_sent, 	 /* Sending Finished Message (encrypted) */
			   
			  } tls_state: 4;

	unsigned char tls_active: 1; /* flag which says that a TLS handshake should be expected on this connection */

	/* The sequence number ensures that attempts to delete or reorder messages will be detected.  
	   Since sequence numbers are 64 bits long, they should never overflow. */
	union long_byte encode_seq_no;
	union long_byte decode_seq_no;

	/* life of random variables is limited to a handshake (even if resumed) */
	uint8_t *client_random;
	union int_char server_random;

	uint8_t client_mac[MAC_KEYSIZE]; 		/* client write mac key */
	uint8_t server_mac[MAC_KEYSIZE]; 		/* server write mac key */
	uint8_t client_key[CIPHER_KEYSIZE]; 	/* client write key for symmetric alg */
	uint8_t server_key[CIPHER_KEYSIZE]; 	/* server write key for symmetric alg */

	/* this memory should be saved if session resumed will be implemented */
	uint8_t master_secret[MS_LEN];

	uint8_t ccs_sent:1; /* flag indicating if we have sent CCS message */
	uint8_t ccs_recv:1; /* flag indicating if we have recv CCS message */


	/* hash contexts for computation of handshake hashes required in finish message */
	struct md5_context *client_md5;
	struct sha1_context *client_sha1;
	      
}; /* ~ (min) 128 + 88 + 92 */




/* ContentType */
enum {
        TLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC = 20,
        TLS_CONTENT_TYPE_ALERT = 21,
        TLS_CONTENT_TYPE_HANDSHAKE = 22,
        TLS_CONTENT_TYPE_APPLICATION_DATA = 23
};

/* HandshakeType */
enum {
        TLS_HANDSHAKE_TYPE_HELLO_REQUEST = 0,
        TLS_HANDSHAKE_TYPE_CLIENT_HELLO = 1,
        TLS_HANDSHAKE_TYPE_SERVER_HELLO = 2,
        TLS_HANDSHAKE_TYPE_CERTIFICATE = 11,
        TLS_HANDSHAKE_TYPE_SERVER_KEY_EXCHANGE = 12,
        TLS_HANDSHAKE_TYPE_CERTIFICATE_REQUEST = 13,
        TLS_HANDSHAKE_TYPE_SERVER_HELLO_DONE = 14,
        TLS_HANDSHAKE_TYPE_CERTIFICATE_VERIFY = 15,
        TLS_HANDSHAKE_TYPE_CLIENT_KEY_EXCHANGE = 16,
        TLS_HANDSHAKE_TYPE_FINISHED = 20
};


/* Macros for printing DEBUG information */
#ifdef DEBUG

#define PRINT_ARRAY(x,len,msg) { \
		uint16_t i; \
		printf("%s",msg); \
		for(i = 0 ; i < len ; i++) printf("%02x",x[i]);\
		printf("\n");\
	}
#define DEBUG_MSG(x) printf("%s",x)
#define DEBUG_VAR(x,format) printf(format,x)

#endif

#endif