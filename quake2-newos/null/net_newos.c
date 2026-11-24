/* net_newos.c - no-op network layer for single-player Quake 2 on NEWOS.
 * Provides the NET_* API the engine expects; the game runs locally. */
#include "../qcommon/qcommon.h"
#include <string.h>

unsigned short ntohs(unsigned short x);

/* netadr_t is declared in qcommon.h; provide a string form. */
char *NET_AdrToString(netadr_t a)
{
    static char s[64];
    snprintf(s, sizeof(s), "%u.%u.%u.%u:%u",
             a.ip[0], a.ip[1], a.ip[2], a.ip[3], (unsigned)ntohs(a.port));
    return s;
}

qboolean NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
    return memcmp(a.ip, b.ip, 4) == 0;
}

qboolean NET_CompareAdr(netadr_t a, netadr_t b)
{
    return NET_CompareBaseAdr(a, b) && a.port == b.port;
}

qboolean NET_IsLocalAddress(netadr_t a)
{
    (void)a;
    return true;
}

qboolean NET_GetPacket(netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message)
{
    (void)sock; (void)net_from; (void)net_message;
    return false;
}

void NET_SendPacket(netsrc_t sock, int length, void *data, netadr_t to)
{
    (void)sock; (void)length; (void)data; (void)to;
}

unsigned short ntohs(unsigned short x) { return ((x & 0xff) << 8) | ((x >> 8) & 0xff); }
void NET_Init(void) {}
void NET_OpenIP(void) {}
void NET_OpenIPX(void) {}
void NET_Sleep(int msec) { (void)msec; }
void NET_Shutdown(void) {}
void NET_Config(qboolean multiplayer) { (void)multiplayer; }

qboolean NET_IsValidAddress(netadr_t adr)
{
    (void)adr;
    return false;
}

qboolean NET_StringToAdr(char *s, netadr_t *a)
{
    (void)s;
    memset(a, 0, sizeof(*a));
    return false;
}
