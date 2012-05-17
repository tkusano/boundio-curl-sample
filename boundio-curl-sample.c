#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <curl/curl.h>
#include <json/json.h>

/* REWRITE HERE!!!! */
#define BOUNDIO_REQUEST_URL "https://boundio.jp/api/vd1/XXXXXXXXXXXXX/call"
/* REWRITE HERE!!!! */
#define BOUNDIO_APIKEY "SecRetApiKey"

#define POST_FMT "key=%s&tel_to=%s&cast=%s"

#define BOUNDIO_ERROR_AUTHFAIL		1
#define BOUNDIO_ERROR_INVALID_TELNO	2
#define BOUNDIO_ERROR_AUDIO_NOT_FOUND	3
#define BOUNDIO_ERROR_SHORT_POINT	4
#define BOUNDIO_ERROR_NO_TELNO		5

#define BOUNDIO_ERROR__MAX		5

#define BOUNDIO_ERROR_UNKNOWN		99

const char *boundio_errstr[BOUNDIO_ERROR__MAX] = {
    "auth fail",
    "invalid telno",
    "audio not found",
    "short point",
    "no telno"
};

typedef struct _my_json_parser {
    json_tokener *tok;
    json_object *obj;
} my_json_parser;

void my_json_parser_init(my_json_parser * p)
{
    p->tok = json_tokener_new();
    p->obj = NULL;
}

void my_json_parser_free(my_json_parser * p)
{
    json_tokener_free(p->tok);
}

char *construct_parameter(CURL * c, char *telno, char *cast)
{
    char *post_str;
    size_t post_size, wrote;
    char *telno_encoded, *cast_encoded;;

    telno_encoded = curl_easy_escape(c, telno, 0);
    cast_encoded = curl_easy_escape(c, cast, 0);

    post_size =
	strlen(POST_FMT) - 2 * 3 + strlen(telno_encoded) +
	strlen(cast_encoded) + strlen(BOUNDIO_APIKEY) + 1;
    post_str = malloc(post_size);
    if (post_str == NULL) {
	perror("Failed to allocate buffer for POST parameter");
	exit(1);
    }
    wrote =
	snprintf(post_str, post_size, POST_FMT, BOUNDIO_APIKEY, telno,
		 cast);
    curl_free(telno_encoded);
    curl_free(cast_encoded);
    if (wrote < 0) {
	fprintf(stderr, "Error occured.\n");
	exit(1);
    }
    if (wrote >= post_size) {
	fprintf(stderr, "Too long.\n");
	exit(1);
    }

    return post_str;
}

size_t receive_json(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    my_json_parser *p = (my_json_parser *) userdata;
    size_t len = size * nmemb;

    p->obj = json_tokener_parse_ex(p->tok, ptr, len);

    return len;
}

void display_result(json_object * obj)
{
    const char *str;
    json_object *member;

    member = json_object_object_get(obj, "success");
    if (member == NULL) {
	printf("Cannot get 'success' member.\n");
	return;
    }

    str = json_object_get_string(member);
    if (str == NULL) {
	printf("error, failed to get 'success' string value\n");
	return;
    }

    if (strcmp(str, "true") == 0) {
	member = json_object_object_get(obj, "_id");
	if (member == NULL) {
	    printf("success, but cannot get '_id' member\n");
	    return;
	}
	str = json_object_get_string(member);
	if (str == NULL) {
	    printf("success, but cannot get '_id' string value\n");
	    return;
	}
	printf("success. _id=%s\n", str);
	return;
    }

    if (strcmp(str, "false") == 0) {
	int err;

	member = json_object_object_get(obj, "error");
	if (member == NULL) {
	    printf("error, but cannot get 'error' member.\n");
	    return;
	}

	str = json_object_get_string(member);
	if (str == NULL) {
	    printf("error, but cannot get 'error' string value.\n");
	    return;
	}

	err = atoi(str);
	if (err >= 1 && err <= BOUNDIO_ERROR__MAX) {
	    printf("error. err=%s\n", boundio_errstr[err - 1]);
	} else if (err == BOUNDIO_ERROR_UNKNOWN) {
	    printf("error. err=unknown\n");
	} else {
	    printf("error. error='%s'\n", str);
	}
	return;
    }

    printf("error, invalid 'success' value (%s)\n", str);
}

int main(int argc, char *argv[])
{
    CURL *c;
    char *post_str;
    char *telno, *cast;
    my_json_parser p;
    json_object *obj;

    if (argc != 3) {
	fprintf(stderr, "usage: %s tel-no cast\n", argv[0]);
	exit(EX_USAGE);
    }
    telno = argv[1];
    cast = argv[2];

    curl_global_init(CURL_GLOBAL_ALL);
    c = curl_easy_init();

    post_str = construct_parameter(c, telno, cast);

    /* Do not use progress bar */
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 1);
    /* URL to access */
    curl_easy_setopt(c, CURLOPT_URL, BOUNDIO_REQUEST_URL);
    /* Use POST method */
    curl_easy_setopt(c, CURLOPT_POST, 1);
    /* Set receiver */
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &p);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, receive_json);

    /* Set parameter */
    curl_easy_setopt(c, CURLOPT_POSTFIELDS, post_str);

    /* Initialize JSON parser */
    my_json_parser_init(&p);

    /* Do HTTP request */
    curl_easy_perform(c);

    /* Get JSON object */
    obj = p.obj;

    /* Destroy JSON Parser */
    my_json_parser_free(&p);

    /* Display result */
    if (obj == NULL) {
	fprintf(stderr, "Response is not JSON string.\n");
	exit(1);
    }

    display_result(obj);

    json_object_put(obj);

    exit(EX_OK);
}

/* end of boundio-curl-sample.c */
