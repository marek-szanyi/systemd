/* SPDX-License-Identifier: LGPL-2.1+ */
/***
  This file is part of systemd.

  Copyright 2017 Zbigniew Jędrzejewski-Szmek
***/

/* Some unit tests for the helper functions in timesyncd. */

#include "log.h"
#include "macro.h"
#include "timesyncd-conf.h"

static void test_manager_parse_string(void) {
        /* Make sure that NTP_SERVERS is configured to something
         * that we can actually parse successfully. */

        _cleanup_(manager_freep) Manager *m = NULL;

        assert_se(manager_new(&m) == 0);

        assert_se(!m->have_fallbacks);
        assert_se(manager_parse_server_string(m, SERVER_FALLBACK, NTP_SERVERS) == 0);
        assert_se(m->have_fallbacks);
        assert_se(manager_parse_fallback_string(m, NTP_SERVERS) == 0);

        assert_se(manager_parse_server_string(m, SERVER_SYSTEM, "time1.foobar.com time2.foobar.com axrfav.,avf..ra 12345..123") == 0);
        assert_se(manager_parse_server_string(m, SERVER_FALLBACK, "time1.foobar.com time2.foobar.com axrfav.,avf..ra 12345..123") == 0);
        assert_se(manager_parse_server_string(m, SERVER_LINK, "time1.foobar.com time2.foobar.com axrfav.,avf..ra 12345..123") == 0);
}

int main(int argc, char **argv) {
        log_set_max_level(LOG_DEBUG);
        log_parse_environment();

        test_manager_parse_string();

        return 0;
}
