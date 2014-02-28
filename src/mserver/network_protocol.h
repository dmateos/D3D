#ifndef _NETWORK_PROTOCOL_H_
#define _NETWORK_PROTOCOL_H_

enum commands {
	HELLO = 0x01,
	GOODBYE,

	NEW_PPOS,
	UPD_PPOS,

	NEW_OPOS,
	UPD_OPOS,
};

struct packet {
	char cmd; /* Command */
	int length; /* Length, prob not needed. */
	int oid; /* The object id */
	int cid; /* Client ID */
	float x, y, z; /* New coordinates */
};

#endif
