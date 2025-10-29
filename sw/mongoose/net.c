// Copyright (c) 2023 Cesanta Software Limited
// All rights reserved

#include "net.h"
#include "picowota/reboot.h"
#include "panel.h"

static void timer_sntp_fn(void *param) {  // SNTP timer function. Sync up time
  mg_sntp_connect(param, "udp://time.google.com:123", NULL, NULL);
}

extern bool should_do_ota;
char text_buffer[257];
char clear_param[16];

// HTTP request handler function
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_ACCEPT) {
    if (c->fn_data != NULL) {  // TLS listener!
      struct mg_tls_opts opts = {0};
      opts.cert = mg_unpacked("/certs/server_cert.pem");
      opts.key = mg_unpacked("/certs/server_key.pem");
      mg_tls_init(c, &opts);
    }
  } else if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_match(hm->uri, mg_str("/ota"), NULL)) {
      mg_http_reply(c, 200, "", "ok\n");
      should_do_ota = true;
    } else if (mg_match(hm->uri, mg_str("/message"), NULL)) {
      if(mg_http_get_var(&hm->query, "text", text_buffer, sizeof(text_buffer)) > 0) {
        current_message.type = MESSAGE_TEXT;
        current_message.text_message.message = text_buffer;
        current_message.text_message.clear = false;
        if(mg_http_get_var(&hm->query, "clear", clear_param, sizeof(clear_param)) > 0) {
          if((strcmp(clear_param, "true") == 0) || (strcmp(clear_param, "yes") == 0)) {
            current_message.text_message.clear = true;
          }
        }
        mg_http_reply(c, 200, "", "ok\n");
      } else {
        mg_http_reply(c, 200, "", "fail\n");
      }
    } else {
      struct mg_http_serve_opts opts;
      memset(&opts, 0, sizeof(opts));
#if MG_ARCH == MG_ARCH_UNIX || MG_ARCH == MG_ARCH_WIN32
      opts.root_dir = "web_root";  // On workstations, use filesystem
#else
      opts.root_dir = "/web_root";  // On embedded, use packed files
      opts.fs = &mg_fs_packed;
#endif
      mg_http_serve_dir(c, ev_data, &opts);
    }
    MG_DEBUG(("%lu %.*s %.*s -> %.*s", c->id, (int) hm->method.len,
              hm->method.buf, (int) hm->uri.len, hm->uri.buf, (int) 3,
              &c->send.buf[9]));
  }
}

void web_init(struct mg_mgr *mgr) {
  mg_http_listen(mgr, HTTP_URL, fn, NULL);
  mg_http_listen(mgr, HTTPS_URL, fn, (void *) 1);
  mg_timer_add(mgr, 3600 * 1000, MG_TIMER_RUN_NOW | MG_TIMER_REPEAT,
               timer_sntp_fn, mgr);
}
