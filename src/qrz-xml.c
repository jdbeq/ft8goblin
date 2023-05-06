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
#include "config.h"
#include "debuglog.h"
#include "ft8goblin_types.h"
#include "qrz-xml.h"
#include "sql.h"
#include <curl/curl.h>

extern char *progname;
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
  size_t new_len = s->len + size*nmemb;
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
   /* Server responds with a Session key
     <?xml version="1.0" ?> 
     <QRZDatabase version="1.34">
       <Session>
         <Key>2331uf894c4bd29f3923f3bacf02c532d7bd9</Key> 
         <Count>123</Count> 
         <SubExp>Wed Jan 1 12:34:03 2013</SubExp> 
         <GMTime>Sun Aug 16 03:51:47 2012</GMTime> 
       </Session>
     </QRZDatabase>
   */
   char *key = NULL, *message = NULL, *error = NULL;
   uint64_t count = 0;
   time_t sub_exp = -1, qrz_gmtime = -1;
   mxml_node_t *xml = NULL, *top = NULL;

   qrz_session_t *q = qrz_session;

   if (q == NULL) {
      if ((q = malloc(sizeof(qrz_session_t))) == NULL) {
         fprintf(stderr, "qrz_start_session: out of memory!\n");
         exit(ENOMEM);
      }
      memset(q, 0, sizeof(qrz_session_t));
      qrz_session = q;
   }

   // XXX: Parse the message here

   key = strstr(s->ptr, "<Key>");
   if (key == NULL) {
      // deal with error
      if (qrz_session == q) {
         qrz_session = NULL;
      }

      free(q);
      return -1;
   }
   key += 5;
   char *key_end = strchr(key, '<');
   size_t key_len = -1;

   if (key_end != NULL && key_end > key) {
      key_len = (key_end - key);
   }
   char *newkey = malloc(key_len + 1);
   memset(newkey, 0, key_len);
   snprintf(newkey, key_len + 1, "%s", key);
      
   log_send(mainlog, LOG_INFO, "got key at %p: %s, key_len: %lu", newkey, newkey, key_len);

   if (key) {
      q->key = strdup(key);
   } else {
      free(q);
      return -1;
   }

   q->count = count;
   q->sub_expiration = sub_exp;
   q->last_rx = qrz_gmtime;

   if (message) {
      q->last_msg = message;
   }

   if (error) {
      q->last_error = error;
   }

   log_send(mainlog, LOG_INFO, "Logged into QRZ. Your subscription expires %lu. You've used %lu queries today.", q->sub_expiration, q->count);
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

   log_send(mainlog, LOG_CRIT, "s: %s", s);

   if (postdata != NULL) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
   }
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

static mxml_type_t *qrz_start_session_cb(mxml_node_t *tree) {
   return NULL;
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

   // send the request, we'll get the answer in the callback
   http_post(buf, NULL);
   return true;
}
