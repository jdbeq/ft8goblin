/*
 * Support for looking up callsigns via QRZ XML API.
 *
 * This is only useful for paid QRZ members.
 *
 * We cache results into etc/qrz-cache.db for cfg:callsign-lookup/cache-expiry (3 days by default)
 *
 * Reference: https://www.qrz.com/XML/current_spec.html
 * Current Version: 1.34
 */
#define	_XOPEN_SOURCE
#include "config.h"
#include "debuglog.h"
#include "ft8goblin_types.h"
#include "qrz-xml.h"
#include "sql.h"
#include <curl/curl.h>
#include <string.h>

extern char *progname;
time_t now;
static const char *qrz_user = NULL, *qrz_pass = NULL, *qrz_api_key = NULL, *qrz_api_url;
static qrz_session_t *qrz_session = NULL;

static void qrz_init_string(qrz_string_t *s) {
  s->len = 0;
  s->ptr = malloc(s->len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "qrz_init_string: out of memory!\n");
    exit(ENOMEM);
  }

  s->ptr[0] = '\0';
}

static size_t qrz_http_post_cb(void *ptr, size_t size, size_t nmemb, qrz_string_t *s) {
  size_t new_len = s->len + size * nmemb;
  if (qrz_session != NULL) {
     qrz_session->last_rx = time(NULL);
  }

  s->ptr = realloc(s->ptr, new_len+1);

  if (s->ptr == NULL) {
    fprintf(stderr, "qrz_http_post_cb: Out of memory!\n");
    exit(ENOMEM);
  }

  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  log_send(mainlog, LOG_DEBUG, "qrz_http_post_cb: read (len=%lu): |%s|", s->len, s->ptr);

  // should we parse it?
// if failed
//   free(q);
   char *key = NULL, *message = NULL, *error = NULL;
   uint64_t count = 0;
   time_t sub_exp_time = -1, qrz_gmtime = -1;
   char *newkey = NULL;
   char *new_calldata = NULL;

   qrz_session_t *q = qrz_session;

   if (q == NULL) {
      if ((q = malloc(sizeof(qrz_session_t))) == NULL) {
         fprintf(stderr, "qrz_start_session: out of memory!\n");
         exit(ENOMEM);
      }
      memset(q, 0, sizeof(qrz_session_t));
      qrz_session = q;
      q->count = -1;
      q->sub_expiration = 0;
   }

   // set last received message time to now
   q->last_rx = time(NULL);

   // this is ugly...
   key = strstr(s->ptr, "<Key>");
   if (key != NULL) {
      // skip opening tag...
      key += 5;
      char *key_end = strstr(key, "</Key>");
      size_t key_len = -1;

      if (key_end != NULL && key_end > key) {
         key_len = (key_end - key);

         char newkey[key_len + 1];
         memset(newkey, 0, key_len + 1);
         snprintf(newkey, key_len + 1, "%s", key);
            
         log_send(mainlog, LOG_INFO, "qrz_xml_api: Got session key: %s, key_len: %lu", newkey, key_len);

         memset(q->key, 0, 33);
         snprintf(q->key, 33, "%s", newkey);
      }
   }	// key != NULL

   char *sub_exp = strstr(s->ptr, "<SubExp>");
   char *new_sub_exp = NULL;

   if (sub_exp != NULL) {
      // skip opening tag...
      sub_exp += 8;
      char *sub_exp_end = strstr(sub_exp, "</SubExp>");
      size_t sub_exp_len = -1;

      if (sub_exp_end != NULL && sub_exp_end > sub_exp) {
         sub_exp_len = (sub_exp_end - sub_exp);
         char new_sub_exp[sub_exp_len + 1];
         memset(new_sub_exp, 0, sub_exp_len + 1);
         snprintf(new_sub_exp, sub_exp_len + 1, "%s", sub_exp);
            
         log_send(mainlog, LOG_INFO, "qrz_xml_api: Got SubExp: %s, len: %lu", new_sub_exp, sub_exp_len);

         struct tm tm;
         time_t myret = -1;
         memset(&tm, 0, sizeof(tm));
         strptime(new_sub_exp, "%a %b %d %H:%M:%S %Y", &tm);
         myret = mktime(&tm);

         qrz_session->sub_expiration = myret;
      }
   }	// sub_exp != NULL
   char *countp = strstr(s->ptr, "<Count>");
   uint64_t new_count = 0;

   if (countp != NULL) {
      // skip opening tag...
      countp += 7;
      char *count_end = strstr(countp, "</Count>");
      size_t count_len = -1;

      if (count_end != NULL && count_end > countp) {
         count_len = (count_end - countp);
         char buf[256];
         memset(buf, 0, 256);
         snprintf(buf, count_len + 1, "%s", countp);

//         log_send(mainlog, LOG_DEBUG, ">>>countp: %s (%p), count_end: %s (%p)<<<", countp, countp, count_end, count_end);
//         log_send(mainlog, LOG_DEBUG, ">>>buf: %s (%d)<<<", buf, count_len);
         int n = -1;
         n = atoi(buf);
         if (n == 0 && errno != 0) {
            // an error happened
            log_send(mainlog, LOG_CRIT, "qrz_xml_api: Got invalid response from atoi: %d: %s", errno, strerror(errno));
         } else {
            q->count = n;
            log_send(mainlog, LOG_INFO, "qrz_xml_api: Got Count: %d", q->count);
         }
      }
   }	// sub_exp != NULL

   if (q->sub_expiration > 0 && q->key[0] != '\0' && count >= -1) {
      char datebuf[128];
      struct tm *tm;
      memset(datebuf, 0, 128);
      tm = localtime(&q->sub_expiration);
      strptime(datebuf, "%a %b %d %H:%M:%S %Y", tm);
      log_send(mainlog, LOG_INFO, "Logged into QRZ. Your subscription expires %s. You've used %d queries today.", datebuf, q->count);
   }

   char *callsign = strstr(s->ptr, "<Callsign>");
   if (callsign != NULL) { 			// we got a valid callsign reply
      callsign += 10;
      char *callsign_end = strstr(callsign, "</Callsign>");
      size_t callsign_len = (callsign_end - callsign);

      if (callsign_len >= 10) {
         char new_calldata[callsign_len + 1];
         memset(new_calldata, 0, callsign_len + 1);
         snprintf(new_calldata, callsign_len, "%s", callsign);
         log_send(mainlog, LOG_INFO, "Got Callsign data <%lu bytes>: %s", callsign_len, new_calldata);
      }
      free(new_calldata);
   }
   return size * nmemb;
}

// We should probably move to the curl_multi_* api to avoid blocking, or maybe spawn this whole mess into a thread
char *http_post(const char *url, const char *postdata) {
   char *r = NULL;
   CURL *curl;
   CURLcode res;
   qrz_string_t s;

   curl_global_init(CURL_GLOBAL_ALL);
   if (!(curl = curl_easy_init())) {
      log_send(mainlog, LOG_WARNING, "qrz: http_post failed on curl_easy_init()");
      return NULL;
   }

   qrz_init_string(&s);
   curl_easy_setopt(curl, CURLOPT_URL, url);
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, qrz_http_post_cb);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
   char buf[128];
   memset(buf, 0, 128);
   snprintf(buf, 128, "%s/%s", progname, VERSION);
  
   curl_easy_setopt(curl, CURLOPT_USERAGENT, buf);
   curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

   if (postdata != NULL) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
   }
//   log_send(mainlog, LOG_DEBUG, "qrz:http_post: Fetching %s", url);
   res = curl_easy_perform(curl);

   if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
   }

   // free the string
   free(s.ptr);
   s.len = -1;
   curl_easy_cleanup(curl);
   curl_global_cleanup();

   return r;
}

//int qrz_start_session(const char *user, const char *pass) {
bool qrz_start_session(void) {
   char buf[4096];
   memset(buf, 0, 4096);

   qrz_user = cfg_get_str(cfg, "callsign-lookup/qrz-username");
   qrz_pass = cfg_get_str(cfg, "callsign-lookup/qrz-password");
   qrz_api_key = cfg_get_str(cfg, "callsign-lookup/qrz-api-key");
   qrz_api_url = cfg_get_str(cfg, "callsign-lookup/qrz-api-url");

   // if any settings are missing cry and return error
   if (qrz_user == NULL || qrz_pass == NULL || qrz_api_key == NULL || qrz_api_url == NULL) {
      log_send(mainlog, LOG_CRIT, "please make sure callsign-lookup/qrz-username qrz-password and qrz-api-key are all set in config.json and try again!");
      return NULL;
   }

   snprintf(buf, sizeof(buf), "%s?username=%s;password=%s;agent=%s-%s", qrz_api_url, qrz_user, qrz_pass, progname, VERSION);

   log_send(mainlog, LOG_DEBUG, "Trying to log into QRZ XML API...");
   // send the request, we'll get the answer in the callback
   http_post(buf, NULL);
   return true;
}

bool qrz_lookup_callsign(const char *callsign) {
   char buf[4096];

   if (callsign == NULL || qrz_api_url == NULL || qrz_session == NULL) {
      log_send(mainlog, LOG_WARNING, "qrz_lookup_callsign failed, XML API session is not yet active!");
      return false;
   }

   memset(buf, 0, 4096);
   snprintf(buf, 4096, "%s?s=%s;callsign=%s", qrz_api_url, qrz_session->key, callsign);
   log_send(mainlog, LOG_DEBUG, "looking up callsign %s via QRZ XML API", callsign);
   http_post(buf, NULL);

   return true;
}
