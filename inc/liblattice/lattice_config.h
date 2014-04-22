#ifndef LATTICE_CONFIG_H
#define LATTICE_CONFIG_H

#define LATTICE_XMIN 0
#define LATTICE_XMAX 0x000FFFFF
#define LATTICE_YMIN 0
#define LATTICE_YMAX 0x000FFFFF
#define LATTICE_ZMIN 0
#define LATTICE_ZMAX 0x000FFFFF


#define WRITE_LENGTH 65536
#define READ_LENGTH 65536

#define MTU 512 // message size max (including (\r)\n\0)
#define MAX_ARGS MTU

#define SENDQ_BLOCK 262144

#define VISUAL_UNIT  32  // sectors
#define BURST_UNIT_MAX VISUAL_UNIT

#define SERVER_RETRY 120

#define SERVER_HANDSHAKE_TIMEOUT 30

#define SERVER_SEND_PING 120
#define SERVER_PING_TIMEOUT 60

#endif
