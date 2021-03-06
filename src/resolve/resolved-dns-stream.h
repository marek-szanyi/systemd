/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/***
  This file is part of systemd.

  Copyright 2014 Lennart Poettering
***/

#include "socket-util.h"

typedef struct DnsStream DnsStream;

#include "resolved-dns-packet.h"
#include "resolved-dns-transaction.h"
#include "resolved-manager.h"

#if HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif

/* Streams are used by three subsystems:
 *
 *   1. The normal transaction logic when doing a DNS or LLMNR lookup via TCP
 *   2. The LLMNR logic when accepting a TCP-based lookup
 *   3. The DNS stub logic when accepting a TCP-based lookup
 */

struct DnsStream {
        Manager *manager;
        int n_ref;

        DnsProtocol protocol;

        int fd;
        union sockaddr_union peer;
        socklen_t peer_salen;
        union sockaddr_union local;
        socklen_t local_salen;
        int ifindex;
        uint32_t ttl;
        bool identified;

        /* only when using TCP fast open */
        union sockaddr_union tfo_address;
        socklen_t tfo_salen;

#if HAVE_GNUTLS
        gnutls_session_t tls_session;
        int tls_handshake;
        bool tls_bye;
#endif

        sd_event_source *io_event_source;
        sd_event_source *timeout_event_source;

        be16_t write_size, read_size;
        DnsPacket *write_packet, *read_packet;
        size_t n_written, n_read;
        OrderedSet *write_queue;

        int (*on_connection)(DnsStream *s);
        int (*on_packet)(DnsStream *s);
        int (*complete)(DnsStream *s, int error);

        LIST_HEAD(DnsTransaction, transactions); /* when used by the transaction logic */
        DnsServer *server;                       /* when used by the transaction logic */
        DnsQuery *query;             /* when used by the DNS stub logic */

        /* used when DNS-over-TLS is enabled */
        bool encrypted:1;

        LIST_FIELDS(DnsStream, streams);
};

int dns_stream_new(Manager *m, DnsStream **s, DnsProtocol protocol, int fd, const union sockaddr_union *tfo_address);
#if HAVE_GNUTLS
int dns_stream_connect_tls(DnsStream *s, gnutls_session_t tls_session);
#endif
DnsStream *dns_stream_unref(DnsStream *s);
DnsStream *dns_stream_ref(DnsStream *s);

DEFINE_TRIVIAL_CLEANUP_FUNC(DnsStream*, dns_stream_unref);

int dns_stream_write_packet(DnsStream *s, DnsPacket *p);

static inline bool DNS_STREAM_QUEUED(DnsStream *s) {
        assert(s);

        if (s->fd < 0) /* already stopped? */
                return false;

        return !!s->write_packet;
}
