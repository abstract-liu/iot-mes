#include <iostream>
#include <curl/curl.h>
#include "cJSON.h"
#include "faceSearch.h"

// libcurl库下载链接：https://curl.haxx.se/download.html
// jsoncpp库下载链接：https://github.com/open-source-parsers/jsoncpp/
//const static std::string request_url = "https://aip.baidubce.com/rest/2.0/face/v3/detect";
const static std::string request_url = "https://aip.baidubce.com/rest/2.0/face/v3/search";
static std::string faceDetect_result;
static std::string faceSearch_result;
/**
 * curl发送http请求调用的回调函数，回调函数中对返回的json格式的body进行了解析，解析结果储存在全局的静态变量当中
 * @param 参数定义见libcurl文档
 * @return 返回值定义见libcurl文档
 */
static size_t callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    // 获取到的body存放在ptr中，先将其转换为string格式
    std::string s((char *) ptr, size * nmemb);
    // 开始获取json中的access token项目
    //Json::Reader reader;
    //Json::Value root;
    // 使用boost库解析json
    //reader.parse(s,root);
    //std::string* access_token_result = static_cast<std::string*>(stream);
    //*access_token_result = root["access_token"].asString();
	std::cout<<"Callback Json:"<<s<<std::endl;

	faceSearch_result = s;
    return size * nmemb;
}
/**
 * 人脸搜索
 * @return 调用成功返回0，发生错误返回其他错误码
 */
int faceSearch(std::string & json_result, const std::string &access_token, std::string input) {
    std::string url = request_url + "?access_token=" + access_token;
    CURL *curl = NULL;
    CURLcode result_code;
    int is_success;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"image\":\"027d8308a2ec665acb1bdf63e513bcb9\",\"image_type\":\"FACE_TOKEN\",\"group_id_list\":\"group_repeat,group_233\",\"quality_control\":\"LOW\",\"liveness_control\":\"NORMAL\"}");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input.data() );

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		std::string access_token_result;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &access_token_result);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            is_success = 1;
            return is_success;
        }
		//std::cout<<"result_code:"<< result_code <<std::endl;
        json_result = faceSearch_result;
        curl_easy_cleanup(curl);
        is_success = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        is_success = 1;
    }
    return is_success;
}

std::string faceAuth(cJSON *root)
{
    const std::string access_token ="24.5429c0d4dcd29434f9870e8da829ec40.2592000.1608987872.282335-23043576";
    std::string result_json;

	std::string input = cJSON_Print(root);

	//std::cout << input << std::endl;

    //faceDetect(result_json,access_token);
	//faceMatch(result_json,access_token,input);
	printf("\nFace Searching begin****************\n");
	faceSearch(result_json,access_token,input);
	printf("\nFace Searching end****************\n");

	return result_json;
}


/**
 * 人脸检测与属性分析
 * @return 调用成功返回0，发生错误返回其他错误码
int faceDetect(std::string &json_result, const std::string &access_token) {
    std::string url = request_url + "?access_token=" + access_token;
    CURL *curl = NULL;
    CURLcode result_code;
    int is_success;
    curl = curl_easy_init();
    if (curl) 
	{
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "./cacert.pem");
		//std::cout << "setopt post" << std::endl;
        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"image\":\"027d8308a2ec665acb1bdf63e513bcb9\",\"image_type\":\"FACE_TOKEN\",\"face_field\":\"faceshape,facetype\"}");

		std::cout << "before perform curl:" << curl << std::endl;
	
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		//curl_easy_setopt(curl, CURLOPT_CAPATH, "/etc/ssl/certs/");	
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "./cacert.pem");
		//curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "pem");
		//curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);
		//curl_easy_setopt(curl, CURLOPT_SSLVERSION, 1);
		//curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 50L);
		//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		//curl_easy_setopt(curl, CURLOPT_FDEBUG, 1L);
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/cacert.pem");	


		result_code = curl_easy_perform(curl);
		std::cout << "curl easy perform" << std::endl;
		
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            is_success = 1;
            return is_success;
        }
        json_result = faceDetect_result;
        curl_easy_cleanup(curl);
		std::cout << "curl clean" << std::endl;
		
        is_success = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        is_success = 1;
    }
    return is_success;
}
 */

/**
 * 人脸对比
 * @return 调用成功返回0，发生错误返回其他错误码
int faceMatch(std::string &json_result, const std::string &access_token,std::string input) {
    std::string url = request_url + "?access_token=" + access_token;
    CURL *curl = NULL;
    CURLcode result_code;
    int is_success;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl,CURLOPT_POSTFIELDS,input.c_str());
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            is_success = 1;
            return is_success;
        }
        json_result = faceMatch_result;
        curl_easy_cleanup(curl);
        is_success = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        is_success = 1;
    }
    return is_success;
}
 */


