#include "server.h"

const char* TAG = "server";
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define BUFFER_SIZE (8 * 1024)
#define MAX_SIZE (200 * 1024)
#define MAX_SIZE_STR "200KB"
#define MIN(a,b) ((a)<(b)? (a):(b))

struct file_server_data {
  const char base_path[ESP_VFS_PATH_MAX + 1];  // extra byte for '\0'
  char buffer[BUFFER_SIZE];                    // tmp buffer for copying the data
};

static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

static esp_err_t upload_post_handler(httpd_req_t* req) {
  char fullpath[PATH_MAX];
  const char* filename =
      get_path_from_uri(fullpath, ((struct file_server_data*)req->user_ctx)->base_path, req->uri + sizeof("/upload") - 1, sizeof(fullpath));
  //check wheter the file name lenght is ok
  if (filename == NULL) {
    ESP_LOGE(TAG, "The file name \"%s\" is too big", filename);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "The file name is to big");
    return ESP_FAIL;
  }
  //check wheter the file name itself is correct
  if (filename[sizeof(filename) - 1] == '/') {
    ESP_LOGE(TAG, "Wrong name %s", filename);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "The file name is to big");
    return ESP_FAIL;
  }
  //check wheter the file is not already present
  struct stat st;
  if (stat(fullpath, &st) == 0) {
    ESP_LOGE(TAG, "The file %s is already present on the server", filename);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "The file is already exist");
    return ESP_FAIL;
  }
  //check the size
  if (MAX_SIZE < req->content_len) {
    ESP_LOGE(TAG, "The file is too large (maximu is %d)", MAX_SIZE);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "The file too large maximum is " MAX_SIZE_STR);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Start to copy new file: %s", filename);
  FILE* file = fopen(fullpath, "w");
  char* buf = ((struct file_server_data*)req->user_ctx)->buffer;
  int remaining = req->content_len;
  while (remaining > 0) {
    int received = httpd_req_recv(req, buf, MIN(remaining, BUFFER_SIZE));
    if (received < 0) {
      if (HTTPD_SOCK_ERR_TIMEOUT == received) {
        continue;
      }
      ESP_LOGE(TAG, "An error occured");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "An eeror occrued during reading");
      fclose(file);
      unlink(fullpath);
      return ESP_FAIL;
    }

    size_t written = fwrite(buf, 1, received, file);
    if (written != received) {
      ESP_LOGE(TAG, "An error occured during writing the data");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "An eeror occrued during reading");
      fclose(file);
      unlink(fullpath);
      return ESP_FAIL;
    }
    remaining -= received;
  }

  fclose(file);
  ESP_LOGI(TAG, "The file is transfered");
  httpd_resp_set_status(req, "303 See Other");
  httpd_resp_set_hdr(req, "Location", "/");
  httpd_resp_sendstr(req, "The file is transfered");
  return ESP_OK;
}

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};

esp_err_t start_server(const char* base_path) {
  static struct file_server_data* server_data = NULL; //due to static only one instance is going to be even after few calls
  if (server_data != NULL) {
    ESP_LOGE(TAG, "The server is already running...");
    return ESP_FAIL;
  }

  server_data = calloc(1, sizeof(struct file_server_data));
  if (server_data == NULL) {
    ESP_LOGE(TAG, "Failed to start server");
    return ESP_FAIL;
  }

  strlcpy(server_data->base_path, base_path, sizeof(server_data->base_path));  // copy the base path

  httpd_handle_t server = NULL;
  httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
  server_config.uri_match_fn = httpd_uri_match_wildcard;
  if (httpd_start(&server, &server_config) == ESP_OK) {
    ESP_LOGI(TAG, "Registering URI...");
    httpd_register_uri_handler(server, &hello);
    httpd_uri_t upload = {
      .uri = "/upload/*",
      .method = HTTP_POST,
      .handler = upload_post_handler,
      .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &upload);
    ESP_LOGI(TAG, "URI registering complete!");
  }
  return ESP_OK;
}
