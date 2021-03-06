// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uhttp.h - HTTP utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HTTP_H
#define ULIB_HTTP_H 1

#include <ulib/timeval.h>
#include <ulib/internal/chttp.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/utility/data_session.h>

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
#  include <ulib/pcre/pcre.h>
#else
#  include <ulib/container/vector.h>
#endif

#define U_HTTP_REALM "Protected Area" // HTTP Access Authentication

#define U_MAX_UPLOAD_PROGRESS   16
#define U_MIN_SIZE_FOR_DEFLATE 150 // NB: google advice...

#define U_HTTP_URI_EQUAL(str)               ((str).equal(U_HTTP_URI_TO_PARAM))
#define U_HTTP_URI_DOSMATCH(mask,len,flags) (UServices::dosMatchWithOR(U_HTTP_URI_TO_PARAM, mask, len, flags))

class UFile;
class ULock;
class UHTTP2;
class UEventFd;
class UCommand;
class UPageSpeed;
class USSIPlugIn;
class UHttpPlugIn;
class USSLSession;
class UMimeMultipart;
class UModProxyService;
class UClientImage_Base;

template <class T> class UClient;
template <class T> class URDBObjectHandler;

class U_EXPORT UHTTP {
public:

   static void init();
   static void dtor();

   // TYPE

   static bool isMobile() __pure;
   static bool isProxyRequest();

   static bool isTSARequest() __pure;
   static bool isSOAPRequest() __pure;

   static bool isGET()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGET()")

      if (U_http_method_type == HTTP_GET) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isHEAD()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isHEAD()")

      if (U_http_method_type == HTTP_HEAD) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPOST()")

      if (U_http_method_type == HTTP_POST) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPUT()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPUT()")

      if (U_http_method_type == HTTP_PUT) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPATCH()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPATCH()")

      if (U_http_method_type == HTTP_PATCH) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isDELETE()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isDELETE()")

      if (U_http_method_type == HTTP_DELETE) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isCOPY()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isCOPY()")

      if (U_http_method_type == HTTP_COPY) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorHEAD()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorHEAD()")

      if ((U_http_method_type & (HTTP_GET | HTTP_HEAD)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorPOST()")

      if ((U_http_method_type & (HTTP_GET | HTTP_POST)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorHEADorPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorHEADorPOST()")

      if ((U_http_method_type & (HTTP_GET | HTTP_HEAD | HTTP_POST)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPOSTorPUTorPATCH()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPOSTorPUTorPATCH()")

      if ((U_http_method_type & (HTTP_POST | HTTP_PUT | HTTP_PATCH)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   // SERVICES

   static UFile* file;
   static UString* ext;
   static UString* etag;
   static UString* request;
   static UString* qcontent;
   static UString* pathname;
   static UString* rpathname;
   static UString* upload_dir;
   static UString* string_HTTP_Variables;

   static URDB* db_not_found;
   static UModProxyService* service;
   static UVector<UString>* vmsg_error;
   static UHashMap<UString>* prequestHeader;
   static UVector<UModProxyService*>* vservice;

   static const char* uri_suffix;
   static const char* uri_basename;
   static char response_buffer[64];
   static int mime_index, cgi_timeout; // the time-out value in seconds for output cgi process
   static bool enable_caching_by_proxy_servers, skip_check_cookie_ip_address;
   static uint32_t limit_request_body, request_read_timeout, range_start, range_size;

   static bool readRequest();
   static bool handlerCache();
   static int  manageRequest();
   static void initDbNotFound();
   static void setStatusDescription();
   static void setEndRequestProcessing();
   static bool callService(const UString& path);
   static bool isUriRequestNeedCertificate() __pure;
   static bool isValidMethod(const char* ptr) __pure;
   static bool checkContentLength(const UString& response);
   static bool manageSendfile(const char* ptr, uint32_t len);
   static bool checkContentLength(uint32_t length, uint32_t pos);
   static bool scanfHeaderRequest(const char* ptr, uint32_t size);
   static bool scanfHeaderResponse(const char* ptr, uint32_t size);
   static bool readHeaderResponse(USocket* socket, UString& buffer);
   static bool readBodyResponse(USocket* socket, UString* buffer, UString& body);

   static bool isValidRequest(const char* ptr, uint32_t sz)
      {
      U_TRACE(0, "UHTTP::isValidRequest(%.*S,%u)", 30, ptr, sz)

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      U_INTERNAL_DUMP("sz = %u UClientImage_Base::size_request = %u", sz, UClientImage_Base::size_request)

      if (u_get_unalignedp32(ptr+sz-4) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isValidRequestExt(const char* ptr, uint32_t sz)
      {
      U_TRACE(0, "UHTTP::isValidRequestExt(%.*S,%u)", 30, ptr, sz)

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      if (sz >= U_CONSTANT_SIZE("GET / HTTP/1.0\r\n\r\n")        &&
          isValidMethod(ptr)                                     &&
          (isValidRequest(ptr, sz)                               ||
                              (UClientImage_Base::size_request   &&
           isValidRequest(ptr, UClientImage_Base::size_request)) ||
           memmem(ptr, sz, U_CONSTANT_TO_PARAM(U_CRLF2)) != U_NULLPTR))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

#ifndef U_HTTP2_DISABLE
   static bool    copyHeaders(UStringRep* key, void* elem);
   static bool upgradeHeaders(UStringRep* key, void* elem)
      {
      U_TRACE(0, "UHTTP::upgradeHeaders(%V,%p)", key, elem)

      if (key->equal(U_CONSTANT_TO_PARAM("Date"))   == false &&
          key->equal(U_CONSTANT_TO_PARAM("Server")) == false)
         {
         if (key->equal(U_CONSTANT_TO_PARAM("Set-Cookie"))) set_cookie->_assign(key);
         else                                               ext->snprintf_add(U_CONSTANT_TO_PARAM("%v: %v\r\n"), key, (const UStringRep*)elem);
         }

      U_RETURN(true);
      }

   static void upgradeResponse(UHashMap<UString>* ptable)
      {
      U_TRACE(0, "UHTTP::upgradeResponse(%p)", ptable)

      U_INTERNAL_DUMP("U_http_info.nResponseCode = %u U_http_info.clength = %u U_http_version = %C", U_http_info.nResponseCode, U_http_info.clength, U_http_version)

      ext->setBuffer(U_CAPACITY);

      ptable->callForAllEntry(upgradeHeaders);

      U_http_version = '2';

      handlerResponse();
      }
#endif

#ifdef USE_LOAD_BALANCE
   static UClient<USSLSocket>* client_http;

   static bool manageRequestOnRemoteServer();
#endif

#ifdef DEBUG
   static uint32_t parserExecute(const char* ptr, uint32_t len, bool response = false);
#endif

   static void setHostname(const char* ptr, uint32_t len);

   static void setHostname(const UString& name) { setHostname(U_STRING_TO_PARAM(name)); }

   static const char* getStatusDescription(uint32_t* plen = U_NULLPTR);

   static uint32_t getUserAgent()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getUserAgent()")

      uint32_t agent = (U_http_info.user_agent_len ? u_cdb_hash((unsigned char*)U_HTTP_USER_AGENT_TO_PARAM, -1) : 0);

      U_RETURN(agent);
      }

   static void setPathName()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setPathName()")

      U_INTERNAL_ASSERT(pathname->empty())
      U_ASSERT(pathname->capacity() >= u_cwd_len + U_http_info.uri_len)

      char* ptr = pathname->data();

      U_MEMCPY(ptr,                     u_cwd,           u_cwd_len);
      U_MEMCPY(ptr+u_cwd_len, U_http_info.uri, U_http_info.uri_len);

      pathname->size_adjust_force(u_cwd_len + U_http_info.uri_len); // NB: pathname can be referenced by file obj...
      }

   static void checkForPathName()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::checkForPathName()")

      if (pathname->empty())
         {
         setPathName();

         file->setPath(*pathname);

         U_INTERNAL_DUMP("file = %.*S", U_FILE_TO_TRACE(*file))
         }
      }

   static bool isSizeForSendfile(uint32_t sz)
      {
      U_TRACE(0, "UHTTP::isSizeForSendfile(%u)", sz)

      U_INTERNAL_DUMP("U_http_version = %C UServer_Base::min_size_for_sendfile = %u UServer_Base::bssl = %b", U_http_version, UServer_Base::min_size_for_sendfile, UServer_Base::bssl)

#  ifndef U_HTTP2_DISABLE
      if (U_http_version != '2') // NB: within HTTP/2 we can't use sendfile...
#  endif
      {
      if (sz >= UServer_Base::min_size_for_sendfile)
         {
         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false) // NB: we can't use sendfile with SSL...

         U_RETURN(true);
         }
      }

      U_RETURN(false);
      }

   static void addHTTPVariables(UString& buffer)
      {
      U_TRACE(0, "UHTTP::addHTTPVariables(%V)", buffer.rep)

      U_INTERNAL_ASSERT_POINTER(prequestHeader)
      U_INTERNAL_ASSERT_EQUALS(prequestHeader->empty(), false)

      prequestHeader->callForAllEntry(addHTTPVariables);

      (void) buffer.append(*string_HTTP_Variables);

      string_HTTP_Variables->clear();
      }

   static bool checkDirectoryForDocumentRoot(const char* ptr, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkDirectoryForDocumentRoot(%.*S,%u)", len, ptr, len)

      U_INTERNAL_DUMP("document_root(%u) = %V", UServer_Base::document_root_size, UServer_Base::document_root->rep)

      U_INTERNAL_ASSERT_POINTER(UServer_Base::document_root_ptr)

      if (len < UServer_Base::document_root_size         ||
            ptr[UServer_Base::document_root_size] != '/' ||
          memcmp(ptr, UServer_Base::document_root_ptr, UServer_Base::document_root_size) != 0)
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static void startRequest()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::startRequest()")

      UClientImage_Base::startRequest();

      // ------------------------------
      // U_http_info.uri
      // ....
      // U_http_info.nResponseCode
      // ....
      // ------------------------------
      U_HTTP_INFO_RESET(0);

      u_clientimage_info.flag.u = 0;
      }

   // UPLOAD

   static vPFi on_upload;

   static void setUploadDir(const UString& dir)
      {
      U_TRACE(0, "UHTTP::setUploadDir(%V)", dir.rep)

      U_INTERNAL_ASSERT(dir)
      U_INTERNAL_ASSERT_POINTER(upload_dir)

      UString result = checkDirectoryForUpload(dir);

      if (result) *upload_dir = result;
      }

   static void writeUploadData(const char* ptr, uint32_t len);

   static UString checkDirectoryForUpload(const char* ptr, uint32_t len);

   static UString checkDirectoryForUpload(const UString& dir) { return checkDirectoryForUpload(U_STRING_TO_PARAM(dir)); }

   static const char* getHeaderValuePtr(const UString& request, const char* name, uint32_t name_len, bool nocase)
      {
      U_TRACE(0, "UHTTP::getHeaderValuePtr(%V,%.*S,%u,%b)", request.rep, name_len, name, name_len, nocase)

      if (U_http_info.endHeader)
         {
         return UStringExt::getValueFromName(request, U_http_info.startHeader,
                                                      U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader, name, name_len, nocase);
         }

      U_RETURN((const char*)U_NULLPTR);
      }

#ifndef U_HTTP2_DISABLE
   static const char* getHeaderValuePtr(const char* name, uint32_t name_len, bool nocase);
#else
   static const char* getHeaderValuePtr(const char* name, uint32_t name_len, bool nocase) { return getHeaderValuePtr(*UClientImage_Base::request, name, name_len, nocase); }
#endif

   static const char* getHeaderValuePtr(                        const UString& name, bool nocase) { return getHeaderValuePtr(         U_STRING_TO_PARAM(name), nocase); }
   static const char* getHeaderValuePtr(const UString& request, const UString& name, bool nocase) { return getHeaderValuePtr(request, U_STRING_TO_PARAM(name), nocase); }

   static UString getHeaderMimeType(const char* content, uint32_t size, const char* content_type, time_t expire = 0L, bool content_length_changeable = false);

   // set HTTP response message

   enum RedirectResponseType {
      NO_BODY                         = 0x001,
      REFRESH                         = 0x002,
      PARAM_DEPENDENCY                = 0x004,
      NETWORK_AUTHENTICATION_REQUIRED = 0x008
   };

   static void setDynamicResponse();
   static void setResponse(bool btype, const UString& content_type, UString* pbody);
   static void setRedirectResponse(int mode, const char* ptr_location, uint32_t len_location);
   static void setErrorResponse(const UString& content_type, int code, const char* fmt, uint32_t fmt_size, bool bformat = true);

   static void setResponse()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setResponse()")

      U_ASSERT(ext->empty())

      UClientImage_Base::body->clear(); // clean body to avoid writev() in response...

      handlerResponse();
      }

   // set HTTP main error message

   static void setNotFound()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setNotFound()")

      setErrorResponse(*UString::str_ctype_html, HTTP_NOT_FOUND, U_CONSTANT_TO_PARAM("Your requested URL %.*S was not found on this server"));
      }

   static void setBadMethod()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setBadMethod()")

      U_INTERNAL_ASSERT_EQUALS(U_http_info.nResponseCode, HTTP_BAD_METHOD)

      setErrorResponse(*UString::str_ctype_html, HTTP_BAD_METHOD, U_CONSTANT_TO_PARAM("The requested method is not allowed for the URL %.*S"));
      }

   static void setBadRequest()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setBadRequest()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_BAD_REQUEST, U_CONSTANT_TO_PARAM("Your requested URL %.*S was a request that this server could not understand"));
      }

   static void setForbidden()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setForbidden()")

      UClientImage_Base::setRequestForbidden();

      setErrorResponse(*UString::str_ctype_html, HTTP_FORBIDDEN, U_CONSTANT_TO_PARAM("You don't have permission to access %.*S on this server"));
      }

   static void setMethodNotImplemented()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setMethodNotImplemented()")

      setErrorResponse(*UString::str_ctype_html, HTTP_NOT_IMPLEMENTED, U_CONSTANT_TO_PARAM("Sorry, the method you requested is not implemented"), false);
      }

   static void setUnAuthorized();

   static void setInternalError()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setInternalError()")

      setErrorResponse(*UString::str_ctype_html, HTTP_INTERNAL_ERROR,
                       U_CONSTANT_TO_PARAM("The server encountered an internal error or misconfiguration "
                                           "and was unable to complete your request. Please contact the server "
                                           "administrator, and inform them of the time the error occurred, and "
                                           "anything you might have done that may have caused the error. More "
                                           "information about this error may be available in the server error log"), false);
      }

   static void setServiceUnavailable()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setServiceUnavailable()")

      setErrorResponse(*UString::str_ctype_html, HTTP_UNAVAILABLE,
                       U_CONSTANT_TO_PARAM("Sorry, the service you requested is not available at this moment. "
                                           "Please contact the server administrator and inform them about this"), false);
      }

#ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
   static UString* uri_strict_transport_security_mask;

   static bool isUriRequestStrictTransportSecurity() __pure;
#endif

   // URI PROTECTION (for example directory listing)

   static UString* htpasswd;
   static UString* htdigest;
   static bool digest_authentication; // authentication method (digest|basic)

#ifdef USE_LIBSSL
   static UString* uri_protected_mask;
   static UVector<UIPAllow*>* vallow_IP;
   static UString* uri_request_cert_mask;

   static bool checkUriProtected();
   static bool isUriRequestProtected() __pure;
#endif

#if defined(U_HTTP_STRICT_TRANSPORT_SECURITY) || defined(USE_LIBSSL)
   static bool isValidation();
#endif

#ifdef U_ALIAS
   static UString* alias;
   static bool virtual_host;
   static UString* global_alias;
   static UVector<UString>* valias;
   static UString* maintenance_mode_page;

   static void setGlobalAlias(const UString& alias);
#endif

   // manage HTTP request service: the tables must be ordered alphabetically cause of binary search...

   typedef struct service_info {
      const char* name;
      uint32_t    len;
      vPF         function;
   } service_info;

#define  GET_ENTRY(name) {#name,U_CONSTANT_SIZE(#name), GET_##name}
#define POST_ENTRY(name) {#name,U_CONSTANT_SIZE(#name),POST_##name}

   static void manageRequest(service_info*  GET_table, uint32_t n1,
                             service_info* POST_table, uint32_t n2);

   // -----------------------------------------------------------------------
   // FORM
   // -----------------------------------------------------------------------
   // retrieve information on specific HTML form elements
   // (such as checkboxes, radio buttons, and text fields), or uploaded files
   // -----------------------------------------------------------------------

   static UString* tmpdir;
   static UMimeMultipart* formMulti;
   static UVector<UString>* form_name_value;

   static uint32_t processForm();

   static void getFormValue(UString& value, uint32_t pos)
      {
      U_TRACE(0, "UHTTP::getFormValue(%V,%u)", value.rep, pos)

      U_INTERNAL_ASSERT_POINTER(form_name_value)

      if (pos >= form_name_value->size()) value.clear();
      else                         (void) value.replace((*form_name_value)[pos]);
      }

   static int getFormFirstNumericValue(int _min, int _max) __pure;

   static void    getFormValue(UString& value, const char* name, uint32_t len);
   static UString getFormValue(                const char* name, uint32_t len, uint32_t start,               uint32_t end);
   static void    getFormValue(UString& value, const char* name, uint32_t len, uint32_t start, uint32_t pos, uint32_t end);

   // COOKIE

   static UString* set_cookie;
   static uint32_t sid_counter_gen;
   static UString* set_cookie_option;
   static UString* cgi_cookie_option;

   static bool    getCookie(      UString* cookie, UString* data);
   static void addSetCookie(const UString& cookie);

   // -----------------------------------------------------------------------------------------------------------------------------------
   // param: "[ data expire path domain secure HttpOnly ]"
   // -----------------------------------------------------------------------------------------------------------------------------------
   // string -- key_id or data to put in cookie    -- must
   // int    -- lifetime of the cookie in HOURS    -- must (0 -> valid until browser exit)
   // string -- path where the cookie can be used  --  opt
   // string -- domain which can read the cookie   --  opt
   // bool   -- secure mode                        --  opt
   // bool   -- only allow HTTP usage              --  opt
   // -----------------------------------------------------------------------------------------------------------------------------------
   // RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
   // -----------------------------------------------------------------------------------------------------------------------------------

   static void setCookie(const UString& param);

   // HTTP SESSION

   static uint32_t sid_counter_cur;
   static UDataSession* data_session;
   static UDataSession* data_storage;
   static URDBObjectHandler<UDataStorage*>* db_session;

   static void  initSession();
   static void clearSession();
   static void removeDataSession();

   static void removeCookieSession()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::removeCookieSession()")

      UString cookie(100U);

      cookie.snprintf(U_CONSTANT_TO_PARAM("ulib.s%u=; expires=%#8D"), sid_counter_cur, u_now->tv_sec - U_ONE_DAY_IN_SECOND);

      addSetCookie(cookie);
      }

   static void setSessionCookie(UString* param = U_NULLPTR);

   static bool getDataStorage();
   static bool getDataSession();
   static bool getDataStorage(uint32_t index, UString& value);
   static bool getDataSession(uint32_t index, UString& value);

   static void putDataStorage();
   static void putDataSession();
   static void putDataStorage(uint32_t index, const char* val, uint32_t sz);
   static void putDataSession(uint32_t index, const char* val, uint32_t sz);

   static bool    isNewSession()               { return data_session->isNewSession(); }
   static bool    isDataSession()              { return data_session->isDataSession(); }
   static UString getSessionCreationTime()     { return data_session->getSessionCreationTime(); }
   static UString getSessionLastAccessedTime() { return data_session->getSessionLastAccessedTime(); }

#ifdef USE_LIBSSL
   static USSLSession* data_session_ssl;
   static URDBObjectHandler<UDataStorage*>* db_session_ssl;

   static void  initSessionSSL();
   static void clearSessionSSL();
#endif

   static UString getKeyIdDataSession()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getKeyIdDataSession()")

      U_INTERNAL_ASSERT_POINTER(data_session)

      U_RETURN_STRING(data_session->keyid);
      }

   static UString getKeyIdDataSession(const UString& data)
      {
      U_TRACE(0, "UHTTP::getKeyIdDataSession(%V)", data.rep)

      U_INTERNAL_ASSERT_POINTER(data_session)

      UString keyid = data_session->setKeyIdDataSession(++sid_counter_gen, data);

      U_RETURN_STRING(keyid);
      }

   // HTML Pagination

   static uint32_t num_page_end,
                   num_page_cur,
                   num_page_start,
                   num_item_tot,
                   num_item_for_page;

   static UString getLinkPagination();

   static void addLinkPagination(UString& link, uint32_t num_page)
      {
      U_TRACE(0, "UHTTP::addLinkPagination(%V,%u)", link.rep, num_page)

#  ifdef U_HTML_PAGINATION_SUPPORT
      UString x(100U);

      U_INTERNAL_DUMP("num_page_cur = %u", num_page_cur)

      if (num_page == num_page_cur) x.snprintf(U_CONSTANT_TO_PARAM("<span class=\"pnow\">%u</span>"),             num_page);
      else                          x.snprintf(U_CONSTANT_TO_PARAM("<a href=\"?page=%u\" class=\"pnum\">%u</a>"), num_page, num_page);

      (void) link.append(x);
             link.push_back(' ');
#  endif
      }

   // CGI

   typedef struct ucgi {
      const char* interpreter;
      char        environment_type;
      char        dir[503];
   } ucgi;

   static bool bnph;
   static UCommand* pcmd;
   static UString* geoip;
   static UString* fcgi_uri_mask;
   static UString* scgi_uri_mask;

   static bool isFCGIRequest() __pure;
   static bool isSCGIRequest() __pure;

   static bool runCGI(bool set_environment);
   static bool getCGIEnvironment(UString& environment, int type);
   static bool processCGIOutput(bool cgi_sh_script, bool bheaders);
   static bool processCGIRequest(UCommand* cmd, UHTTP::ucgi* cgi = U_NULLPTR);
   static bool setEnvironmentForLanguageProcessing(int type, void* env, vPFpvpcpc func);

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
   class RewriteRule {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UPCRE key;
   UString replacement;

   RewriteRule(const UString& _key, const UString& _replacement) : key(_key, PCRE_FOR_REPLACE), replacement(_replacement)
      {
      U_TRACE_REGISTER_OBJECT(0, RewriteRule, "%V,%V", _key.rep, _replacement.rep)

      key.study();
      }

   ~RewriteRule()
      {
      U_TRACE_UNREGISTER_OBJECT(0, RewriteRule)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_ASSIGN(RewriteRule)
   };

   static UVector<RewriteRule*>* vRewriteRule;
#endif      

   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // COMMON LOG FORMAT (APACHE LIKE LOG)
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // The Common Log Format, also known as the NCSA Common log format, is a standardized text file format used by web servers
   // when generating server log files. Because the format is standardized, the files may be analyzed by a variety of web analysis programs.
   // Each line in a file stored in the Common Log Format has the following syntax: host ident authuser date request status bytes
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // 10.10.25.2 - - [21/May/2012:16:29:41 +0200] "GET / HTTP/1.1" 200 598 "-" "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/536.5 (KHTML, like Gecko)"
   // 10.10.25.2 - - [21/May/2012:16:29:41 +0200] "GET /unirel_logo.gif HTTP/1.1" 200 3414 "http://www.unirel.com/" "Mozilla/5.0 (X11; Linux x86_64)"
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 

#ifndef U_LOG_DISABLE
   static char iov_buffer[20];
   static struct iovec iov_vec[10];
# if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   static uint32_t request_offset, referer_offset, agent_offset;
# endif

   static void    initApacheLikeLog();
   static void prepareApacheLikeLog();
   static void   resetApacheLikeLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::resetApacheLikeLog()")

      iov_vec[6].iov_len  =
      iov_vec[8].iov_len  = 1;
      iov_vec[6].iov_base =
      iov_vec[8].iov_base = (caddr_t) "-";
      }
#endif

   // USP (ULib Servlet Page)

   class UServletPage : public UDynamic {
   public:

   vPFi runDynamicPage;
   UString path, basename;

   UServletPage(const void* name, uint32_t nlen, const char* base = U_NULLPTR, uint32_t blen = 0, vPFi _runDynamicPage = U_NULLPTR) : runDynamicPage(_runDynamicPage), path(name, nlen)
      {
      U_TRACE_REGISTER_OBJECT(0, UServletPage, "%.*S,%u,%.*S,%u,%p", nlen, name, nlen, blen, base, blen, _runDynamicPage)

      if (blen) (void) basename.replace(base, blen);
      }

   ~UServletPage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UServletPage)
      }

   // SERVICE

   bool operator<(const UServletPage& other) const { return cmp_obj(&basename, &other.basename); }

   static int cmp_obj(const void* a, const void* b)
      {
      U_TRACE(0, "UServletPage::cmp_obj(%p,%p)", a, b)

#  ifdef U_STDCPP_ENABLE
      /**
       * The comparison function must follow a strict-weak-ordering
       *
       * 1) For all x, it is not the case that x < x (irreflexivity)
       * 2) For all x, y, if x < y then it is not the case that y < x (asymmetry)
       * 3) For all x, y, and z, if x < y and y < z then x < z (transitivity)
       * 4) For all x, y, and z, if x is incomparable with y, and y is incomparable with z, then x is incomparable with z (transitivity of incomparability)
       */

      return (((const UServletPage*)a)->basename.compare(((const UServletPage*)b)->basename) < 0);
#  else
      return (*(const UServletPage**)a)->basename.compare((*(const UServletPage**)b)->basename);
#  endif
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   bool load() U_NO_EXPORT;
   bool isPath(const char* pathname, uint32_t len)
      {
      U_TRACE(0, "UServletPage::isPath(%.*S,%u)", len, pathname, len)

      if (path.equal(pathname, len)) U_RETURN(true);

      U_RETURN(false);
      }

   U_DISALLOW_ASSIGN(UServletPage)

                      friend class UHTTP;
   template <class T> friend void u_construct(const T**,bool);
   };

   static bool bcallInitForAllUSP;
   static UVector<UServletPage*>* vusp;

   static void       callEndForAllUSP();
   static void      callInitForAllUSP();
   static void    callSigHUPForAllUSP();
   static void callAfterForkForAllUSP();

   static UServletPage* getUSP(const char* key, uint32_t key_len);

   // CSP (C Servlet Page)

   typedef int (*iPFipvc)(int,const char**);

   class UCServletPage {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int size;
   void* relocated;
   iPFipvc prog_main;

   UCServletPage()
      {
      U_TRACE_REGISTER_OBJECT(0, UCServletPage, "", 0)

      size      = 0;
      relocated = U_NULLPTR;
      prog_main = U_NULLPTR;
      }

   ~UCServletPage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UCServletPage)

      if (relocated) UMemoryPool::_free(relocated, size, 1);
      }

   bool compile(const UString& program);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UCServletPage)
   };

#ifdef USE_PHP // (wrapper to embed the PHP interpreter)
   class UPHP : public UDynamic {
   public:

   bPF initPHP;
   bPF  runPHP;
   vPF  endPHP;

   UPHP()
      {
      U_TRACE_REGISTER_OBJECT(0, UPHP, "", 0)

      initPHP =
       runPHP = U_NULLPTR;
       endPHP = U_NULLPTR;
      }

   ~UPHP()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPHP)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPHP)
   };

   static UPHP* php_embed;
#endif
   static uint32_t npathinfo;
   static UString* php_mount_point;

#ifdef USE_RUBY // (wrapper to embed the RUBY interpreter)
   class URUBY : public UDynamic {
   public:

   bPF initRUBY;
   bPF  runRUBY;
   vPF  endRUBY;

   URUBY()
      {
      U_TRACE_REGISTER_OBJECT(0, URUBY, "", 0)

      initRUBY =
       runRUBY = U_NULLPTR;
       endRUBY = U_NULLPTR;
      }

   ~URUBY()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URUBY)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(URUBY)
   };

   static URUBY* ruby_embed;
   static bool ruby_on_rails;
   static UString* ruby_libdir;
#endif

#ifdef USE_PYTHON // (wrapper to embed the PYTHON interpreter)
   class UPYTHON : public UDynamic {
   public:

   bPF initPYTHON;
   bPF  runPYTHON;
   vPF  endPYTHON;

   UPYTHON()
      {
      U_TRACE_REGISTER_OBJECT(0, UPYTHON, "", 0)

      initPYTHON =
       runPYTHON = U_NULLPTR;
       endPYTHON = U_NULLPTR;
      }

   ~UPYTHON()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPYTHON)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPYTHON)
   };

   static UPYTHON* python_embed;
   static UString* py_project_app;
   static UString* py_project_root;
   static UString* py_virtualenv_path;
#endif

#if defined(USE_PAGE_SPEED) || defined(USE_LIBV8)
   typedef void (*vPFstr)(UString&);
#endif

#ifdef USE_PAGE_SPEED // (Google Page Speed)
   typedef void (*vPFpcstr)(const char*, UString&);

   class UPageSpeed : public UDynamic {
   public:

   vPFpcstr minify_html;
   vPFstr optimize_gif, optimize_png, optimize_jpg;

   UPageSpeed()
      {
      U_TRACE_REGISTER_OBJECT(0, UPageSpeed, "", 0)

      minify_html  = 0;
      optimize_gif = optimize_png = optimize_jpg = 0;
      }

   ~UPageSpeed()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPageSpeed)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPageSpeed)
   };

   static UPageSpeed* page_speed;
#endif

#ifdef USE_LIBV8 // (Google V8 JavaScript Engine)
   class UV8JavaScript : public UDynamic {
   public:

   vPFstr runv8;

   UV8JavaScript()
      {
      U_TRACE_REGISTER_OBJECT(0, UV8JavaScript, "", 0)

      runv8 = U_NULLPTR;
      }

   ~UV8JavaScript()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UV8JavaScript)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UV8JavaScript)
   };

   static UV8JavaScript* v8_javascript;
#endif

   // DOCUMENT ROOT CACHE

   class UFileCacheData {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   void* ptr;               // data
   UVector<UString>* array; // content, header, gzip(content, header)
#ifndef U_HTTP2_DISABLE
   UVector<UString>* http2; //          header, gzip(header)
#endif
   time_t mtime;            // time of last modification
   time_t expire;           // expire time of the entry
   uint32_t size;           // size content
   int wd;                  // if directory it is a "watch list" associated with an inotify instance...
   mode_t mode;             // file type
   int mime_index;          // index file mime type
   int fd;                  // file descriptor
   bool link;               // true => ptr data point to another entry

    UFileCacheData();
    UFileCacheData(const UFileCacheData& elem);
   ~UFileCacheData();

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UFileCacheData& d);
   friend U_EXPORT ostream& operator<<(ostream& os, const UFileCacheData& d);

# ifdef DEBUG
   const char* dump(bool reset) const U_EXPORT;
# endif
#endif

   private:
   U_DISALLOW_ASSIGN(UFileCacheData)

   template <class T> friend void u_construct(const T**,bool);
   };

   static UString* cache_file_mask;
   static UString* cache_avoid_mask;
   static UString* cache_file_store;
   static UString* nocache_file_mask;
   static UFileCacheData* file_data;
   static UFileCacheData* file_gzip_bomb;
   static UHashMap<UFileCacheData*>* cache_file;
   static UFileCacheData* file_not_in_cache_data;

   static bool isDataFromCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isDataFromCache()")

      U_INTERNAL_ASSERT_POINTER(file_data)

      U_INTERNAL_DUMP("file_data->array = %p", file_data->array)

      if (file_data->array != U_NULLPTR) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isDataCompressFromCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isDataCompressFromCache()")

      U_INTERNAL_ASSERT_POINTER(file_data)
      U_INTERNAL_ASSERT_POINTER(file_data->array)

      if (file_data->array->size() > 2) U_RETURN(true);

      U_RETURN(false);
      }

   static void checkFileForCache();
   static void renewFileDataInCache();

   static bool   getFileInCache(const char* path, uint32_t len);
   static bool checkFileInCache(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkFileInCache(%.*S,%u)", len, path, len)

      file_data = cache_file->at(path, len);

      if (file_data)
         {
         file->st_size  = file_data->size;
         file->st_mode  = file_data->mode;
         file->st_mtime = file_data->mtime;

         U_INTERNAL_DUMP("file_data->fd = %d st_size = %I st_mtime = %ld dir() = %b", file_data->fd, file->st_size, file->st_mtime, file->dir())

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool isFileInCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isFileInCache()")

      return checkFileInCache(U_FILE_TO_PARAM(*file));
      }

   static uint32_t old_path_len;

   static bool checkFileInCacheOld(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkFileInCacheOld(%.*S,%u)", len, path, len)

      U_INTERNAL_DUMP("old_path_len = %u", old_path_len)

      if (old_path_len != len) return checkFileInCache(path, (old_path_len = len));

      if (file_data) U_RETURN(true);

      U_RETURN(false);
      }

   static UString getDataFromCache(int idx);

   static UString getBodyFromCache()         { return getDataFromCache(0); }
   static UString getBodyCompressFromCache() { return getDataFromCache(2); }

#ifdef U_HTTP2_DISABLE
   static UString getHeaderFromCache()         { return getDataFromCache(1); };
   static UString getHeaderCompressFromCache() { return getDataFromCache(3); };
#else
   static UString getHeaderFromCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getHeaderFromCache()")

      UString result;

      U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

           if (U_http_version != '2') result = getDataFromCache(1);
      else if (file_data->http2)      result = file_data->http2->operator[](0);

      U_RETURN_STRING(result);
      }

   static UString getHeaderCompressFromCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getHeaderCompressFromCache()")

      UString result;

      U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

           if (U_http_version != '2') result = getDataFromCache(3);
      else if (file_data->http2)      result = file_data->http2->operator[](1);

      U_RETURN_STRING(result);
      }
#endif

   static UString contentOfFromCache(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::contentOfFromCache(%.*S,%u)", len, path, len)

      UString result;

      file_data = cache_file->at(path, len);

      if (file_data) result = getBodyFromCache();

      U_RETURN_STRING(result);
      }

   static UString contentOfFromCache(const UString& path) { return contentOfFromCache(U_STRING_TO_PARAM(path)); }

private:
   static uint32_t old_response_code;

   static void setMimeIndex()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setMimeIndex()")

      U_INTERNAL_ASSERT_POINTER(file)
      U_ASSERT_EQUALS(UClientImage_Base::isRequestNotFound(), false)

      uri_suffix = u_getsuffix(U_FILE_TO_PARAM(*file));

      U_INTERNAL_DUMP("uri_suffix = %p", uri_suffix)

      if (uri_suffix == U_NULLPTR) mime_index = U_unknow;
      else                         (void) u_get_mimetype(uri_suffix+1, &mime_index);
      }

   static const char* setMimeIndex(const char* suffix)
      {
      U_TRACE(0, "UHTTP::setMimeIndex(%S)", suffix)

      U_INTERNAL_ASSERT_POINTER(file)

      mime_index = U_unknow;

      const char* ctype = file->getMimeType(suffix, &mime_index);

      file_data->mime_index = mime_index;

      return ctype;
      }

   static int handlerREAD();
   static int processRequest();
   static void handlerResponse();

#ifndef U_LOG_DISABLE
   static int handlerREADWithLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::handlerREADWithLog()")

      U_ASSERT(UServer_Base::isLog())

      (void) strcpy(UServer_Base::mod_name[0], "[http] ");

      U_ClientImage_state = handlerREAD();

      UServer_Base::mod_name[0][0] = '\0';

      U_RETURN(U_ClientImage_state);
      }

   static int processRequestWithLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::processRequestWithLog()")

      U_ASSERT(UServer_Base::isLog())

      (void) strcpy(UServer_Base::mod_name[0], "[http] ");

      U_ClientImage_state = processRequest();

      UServer_Base::mod_name[0][0] = '\0';

      U_RETURN(U_ClientImage_state);
      }
#endif

   static UString getHTMLDirectoryList() U_NO_EXPORT;

#ifdef DEBUG
   static bool cache_file_check_memory();
   static bool check_memory(UStringRep* key, void* value) U_NO_EXPORT;
#endif

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
   static void processRewriteRule() U_NO_EXPORT;
#endif

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT)
   static int             inotify_wd;
   static char*           inotify_name;
   static uint32_t        inotify_len;
   static UString*        inotify_pathname;
   static UStringRep*     inotify_dir;
   static UFileCacheData* inotify_file_data;

   static void in_READ();
   static void initInotify();
   static void setInotifyPathname() U_NO_EXPORT;
   static bool getInotifyPathDirectory(UStringRep* key, void* value) U_NO_EXPORT;
   static bool checkForInotifyDirectory(UStringRep* key, void* value) U_NO_EXPORT;
#endif

#ifdef U_STATIC_ONLY
   static void loadStaticLinkedServlet(const char* name, uint32_t len, vPFi runDynamicPage) U_NO_EXPORT;
#endif      

   static bool callService() U_NO_EXPORT;
   static void checkPathName() U_NO_EXPORT;
   static void checkIPClient() U_NO_EXPORT;
   static bool runDynamicPage() U_NO_EXPORT;
   static bool readBodyRequest() U_NO_EXPORT;
   static bool processFileCache() U_NO_EXPORT;
   static bool readHeaderRequest() U_NO_EXPORT;
   static bool processGetRequest() U_NO_EXPORT;
   static bool processAuthorization() U_NO_EXPORT;
   static bool checkRequestForHeader() U_NO_EXPORT;
   static bool checkGetRequestIfRange() U_NO_EXPORT;
   static bool checkGetRequestIfModified() U_NO_EXPORT;
   static void setCGIShellScript(UString& command) U_NO_EXPORT;
   static bool checkIfSourceHasChangedAndCompileUSP() U_NO_EXPORT;
   static bool compileUSP(const char* path, uint32_t len) U_NO_EXPORT;
   static void manageDataForCache(const UString& file_name) U_NO_EXPORT;
   static int  checkGetRequestForRange(const UString& data) U_NO_EXPORT;
   static int  sortRange(const void* a, const void* b) __pure U_NO_EXPORT;
   static bool addHTTPVariables(UStringRep* key, void* value) U_NO_EXPORT;
   static bool splitCGIOutput(const char*& ptr1, const char* ptr2) U_NO_EXPORT;
   static void putDataInCache(const UString& fmt, UString& content) U_NO_EXPORT;
   static bool readDataChunked(USocket* sk, UString* pbuffer, UString& body) U_NO_EXPORT;
   static void setResponseForRange(uint32_t start, uint32_t end, uint32_t header) U_NO_EXPORT;
   static bool checkDataSession(const UString& token, time_t expire, UString* data) U_NO_EXPORT;

   static inline void resetFileCache() U_NO_EXPORT;
   static inline void setUpgrade(const char* ptr) U_NO_EXPORT;
   static inline bool checkPathName(uint32_t len) U_NO_EXPORT;
   static inline void setIfModSince(const char* ptr) U_NO_EXPORT;
   static inline void setConnection(const char* ptr) U_NO_EXPORT;
   static        void setAcceptEncoding(const char* ptr) U_NO_EXPORT;
   static inline void setContentLength(const char* ptr1, const char* ptr2) U_NO_EXPORT;

   static inline bool checkDataChunked(UString* pbuffer) U_NO_EXPORT;
   static inline void setRange(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setCookie(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setUserAgent(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setAccept(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setReferer(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXRealIP(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setContentType(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setAcceptLanguage(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXForwardedFor(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXHttpForwardedFor(const char* ptr, uint32_t len) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UHTTP)

   friend class UHTTP2;
   friend class USSIPlugIn;
   friend class UHttpPlugIn;
   friend class UClientImage_Base;
};

template <> inline void u_destroy(const UHTTP::UFileCacheData* elem)
{
   U_TRACE(0, "u_destroy<UFileCacheData>(%p)", elem)

   if (elem <= (const void*)0x0000ffff) U_ERROR("u_destroy<UFileCacheData>(%p)", elem);

   // NB: we need this check if we are at the end of UHashMap<UHTTP::UFileCacheData*>::operator>>()...

   if (UHashMap<void*>::istream_loading)
      {
      ((UHTTP::UFileCacheData*)elem)->ptr   =
      ((UHTTP::UFileCacheData*)elem)->array = U_NULLPTR;
#  ifndef U_HTTP2_DISABLE
      ((UHTTP::UFileCacheData*)elem)->http2 = U_NULLPTR;
#  endif
      }

   delete elem;
}
#endif
