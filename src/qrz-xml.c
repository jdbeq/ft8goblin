/*
 * Support for looking up callsigns via QRZ XML API.
 *
 * This is only useful for paid QRZ members.
 *
 * XXX: We should cache results in sqlite so lookups only happen once a day.
 */
#include "config.h"
#include "qrz-xml.h"
#include "sql.h"
#include "debuglog.h"
#include <curl/curl.h>

#define QRZ_API_URL "https://xmldata.qrz.com/xml/current/"

struct string {
  char *ptr;
  size_t len;
};

static void qrz_init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

static size_t qrz_http_post_cb(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }

  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}


char *http_post(const char *url, const char *postdata) {
   char *r = NULL;
   CURL *curl;
   CURLcode res;
   struct string s;

   curl_global_init(CURL_GLOBAL_ALL);
   if (!(curl = curl_easy_init())) {
      log_send(mainlog, LOG_WARNING, "qrz: http_post failed");
      return NULL;
   }

   qrz_init_string(&s);
   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, qrz_http_post_cb);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

   if (postdata != NULL) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
   }
   res = curl_easy_perform(curl);

   if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
   }

   curl_easy_cleanup(curl);
   curl_global_cleanup();

   return r;
}

int qrz_start_session(const char *user, const char *pass) {
   int rv = -1;
   char *post_reply = NULL;
   char buf[4096];
   memset(buf, 0, 4096);
   snprintf(buf, sizeof(buf), "%s?username=%s;password=%s", QRZ_API_URL, user, pass);

   post_reply = http_post(buf, NULL);
   if (post_reply == NULL) {
      // An error happened
   }

   return rv;
}
