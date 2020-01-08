#include <stdio.h>
#include <curl/curl.h>

int main(void)
{
  curl_version_info_data *data;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  data = curl_version_info(CURLVERSION_NOW);
  printf("ssl version: %s\nlibcurl version: %s\n",
          data->ssl_version, data->version);
  curl_global_cleanup();

  return 0;
}