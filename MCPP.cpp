#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <new>
#include <utility>
#pragma comment(lib, "winhttp.lib")

inline constexpr size_t NPOS = SIZE_MAX;
struct Str {
    char*  p   = nullptr;
    size_t n   = 0;
    size_t cap = 0;

    Str() = default;
    Str(const Str& o) {
        if (o.n) { p = (char*)malloc(o.n + 1); memcpy(p, o.p, o.n + 1); n = cap = o.n; }
    }
    Str(Str&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = o.cap = 0; }
    ~Str() { free(p); }

    Str& operator=(const Str& o) {
        if (this != &o) {
            free(p); p = nullptr; n = cap = 0;
            if (o.n) { p = (char*)malloc(o.n + 1); memcpy(p, o.p, o.n + 1); n = cap = o.n; }
        }
        return *this;
    }
    Str& operator=(Str&& o) noexcept {
        if (this != &o) { free(p); p = o.p; n = o.n; cap = o.cap; o.p = nullptr; o.n = o.cap = 0; }
        return *this;
    }

    inline void grow(size_t need) {
    if (n + need <= cap) return;
    size_t nc = cap ? cap * 2 : 16;
    if (nc < n + need) nc = n + need;
    char* tmp = (char*)realloc(p, nc + 1);
    if (!tmp) return;
    p = tmp; cap = nc;
}
    inline void append(const char* s, size_t l) { if (!l) return; grow(l); if (n + l > cap) return; memcpy(p + n, s, l); n += l; p[n] = 0; }
    inline void append_c(char c)                 { grow(1); if (n >= cap) return; p[n++] = c; p[n] = 0; }
    inline void append_s(const char* s)          { if (s && *s) append(s, strlen(s)); }
    inline void assign(const char* s, size_t l)  { n = 0; append(s, l); }
    inline void assign_s(const char* s)          { n = 0; if (s) append_s(s); }
    inline void copy_from(const Str& o)          { n = 0; if (o.p && o.n) append(o.p, o.n); }

    inline const char* c_str() const { return p ? p : ""; }
    inline size_t      size()  const { return n; }
    inline bool        empty() const { return n == 0; }
    inline char        back()  const { return n ? p[n - 1] : '\0'; }
    inline void        pop_back()    { if (n) { --n; p[n] = 0; } }

    inline char  operator[](size_t i) const { return p[i]; }
    inline char& operator[](size_t i)       { return p[i]; }

    inline bool eq(const char* s) const {
        size_t l = strlen(s);
        return l == n && (!n || !memcmp(p, s, n));
    }
    inline bool eq_n(const char* s, size_t l) const {
        return l == n && (!n || !memcmp(p, s, n));
    }
    inline size_t find(char c, size_t from = 0) const {
        for (size_t i = from; i < n; ++i) if (p[i] == c) return i;
        return NPOS;
    }
    inline size_t find_s(const char* s) const {
        size_t sl = strlen(s);
        if (sl > n) return NPOS;
        for (size_t i = 0; i <= n - sl; ++i) if (!memcmp(p + i, s, sl)) return i;
        return NPOS;
    }
    inline Str substr(size_t from, size_t len = NPOS) const {
        Str r;
        if (from >= n) return r;
        if (len == NPOS || from + len > n) len = n - from;
        r.append(p + from, len);
        return r;
    }
    inline void replace_range(size_t pos, size_t rlen, const char* rep) {
        size_t rl = strlen(rep);
        Str t;
        if (pos) t.append(p, pos);
        t.append(rep, rl);
        if (pos + rlen <= n) t.append(p + pos + rlen, n - pos - rlen);
        *this = std::move(t);
    }
};

struct WStr {
    wchar_t* p   = nullptr;
    size_t   n   = 0;
    size_t   cap = 0;

    WStr() = default;
    WStr(const WStr& o) {
        if (o.n) { p = (wchar_t*)malloc((o.n + 1) * sizeof(wchar_t)); memcpy(p, o.p, (o.n + 1) * sizeof(wchar_t)); n = cap = o.n; }
    }
    WStr(WStr&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = o.cap = 0; }
    ~WStr() { free(p); }

    WStr& operator=(const WStr& o) {
        if (this != &o) {
            free(p); p = nullptr; n = cap = 0;
            if (o.n) { p = (wchar_t*)malloc((o.n + 1) * sizeof(wchar_t)); memcpy(p, o.p, (o.n + 1) * sizeof(wchar_t)); n = cap = o.n; }
        }
        return *this;
    }
    WStr& operator=(WStr&& o) noexcept {
        if (this != &o) { free(p); p = o.p; n = o.n; cap = o.cap; o.p = nullptr; o.n = o.cap = 0; }
        return *this;
    }

    inline void grow(size_t need) {
    if (n + need <= cap) return;
    size_t nc = cap ? cap * 2 : 16;
    if (nc < n + need) nc = n + need;
    wchar_t* tmp = (wchar_t*)realloc(p, (nc + 1) * sizeof(wchar_t));
    if (!tmp) return;
    p = tmp; cap = nc;
}
    inline void append(const wchar_t* s, size_t l) { if (!l) return; grow(l); if (n + l > cap) return; memcpy(p + n, s, l * sizeof(wchar_t)); n += l; p[n] = 0; }
    inline void append_c(wchar_t c)                { grow(1); if (n >= cap) return; p[n++] = c; p[n] = 0; }
    inline void append_w(const wchar_t* s)         { if (s && *s) append(s, wcslen(s)); }
    inline void assign_w(const wchar_t* s)         { n = 0; append_w(s); }
    inline void copy_from(const WStr& o)           { n = 0; if (o.p && o.n) append(o.p, o.n); }

    inline const wchar_t* c_str() const { return p ? p : L""; }
    inline size_t         size()  const { return n; }
    inline bool           empty() const { return n == 0; }
    inline wchar_t        back()  const { return n ? p[n - 1] : L'\0'; }
    inline void           pop_back()    { if (n) { --n; p[n] = 0; } }
};


template<typename T>
struct Vec {
    T*     p   = nullptr;
    size_t n   = 0;
    size_t cap = 0;

    Vec()           = default;
    Vec(const Vec&) = delete;
    Vec(Vec&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = o.cap = 0; }
    ~Vec() { for (size_t i = 0; i < n; ++i) p[i].~T(); free(p); }

    Vec& operator=(Vec&& o) noexcept {
        if (this != &o) {
            for (size_t i = 0; i < n; ++i) p[i].~T();
            free(p);
            p = o.p; n = o.n; cap = o.cap;
            o.p = nullptr; o.n = o.cap = 0;
        }
        return *this;
    }

    inline void reserve(size_t c) {
    if (c <= cap) return;
    size_t nc = cap ? cap * 2 : 4;
    if (nc < c) nc = c;
    T* tmp = (T*)realloc(p, nc * sizeof(T));
    if (!tmp) return;
    p = tmp; cap = nc;
}
    inline void push_back(const T& v) { reserve(n + 1); new (&p[n++]) T(v); }
    inline void push_back(T&& v)      { reserve(n + 1); new (&p[n++]) T(std::move(v)); }
    inline void pop_back()            { if (n) { --n; p[n].~T(); } }
    inline void clear()               { for (size_t i = 0; i < n; ++i) p[i].~T(); n = 0; }

    inline T&       operator[](size_t i)       { return p[i]; }
    inline const T& operator[](size_t i) const { return p[i]; }
    inline T&       back()                     { return p[n - 1]; }
    inline const T& back()              const  { return p[n - 1]; }
    inline size_t   size()              const  { return n; }
    inline bool     empty()             const  { return n == 0; }
};


struct JVal {
    enum Type : uint8_t { Null_, Bool_, Num_, Str_, Arr_, Obj_ } type = Null_;

    bool   bval  = false;
    double nval  = 0.0;
    Str    sval;

    JVal*  arr    = nullptr; size_t arr_n = 0, arr_cap = 0;
    Str*   obj_k  = nullptr;
    JVal*  obj_v  = nullptr; size_t obj_n = 0, obj_cap = 0;

    JVal() = default;
    JVal(JVal&& o) noexcept
        : type(o.type), bval(o.bval), nval(o.nval), sval(std::move(o.sval)),
          arr(o.arr),   arr_n(o.arr_n), arr_cap(o.arr_cap),
          obj_k(o.obj_k), obj_v(o.obj_v), obj_n(o.obj_n), obj_cap(o.obj_cap)
    {
        o.type  = Null_;
        o.arr   = nullptr; o.arr_n  = o.arr_cap  = 0;
        o.obj_k = nullptr; o.obj_v  = nullptr;
        o.obj_n = o.obj_cap = 0;
    }
    ~JVal() {
        if (type == Arr_) {
            for (size_t i = 0; i < arr_n; ++i) arr[i].~JVal();
            free(arr);
        } else if (type == Obj_) {
            for (size_t i = 0; i < obj_n; ++i) { obj_k[i].~Str(); obj_v[i].~JVal(); }
            free(obj_k);
            free(obj_v);
        }
    }
    JVal& operator=(JVal&& o) noexcept {
        if (this != &o) {
            this->~JVal();
            type  = o.type; bval = o.bval; nval = o.nval; sval = std::move(o.sval);
            arr   = o.arr;  arr_n = o.arr_n; arr_cap = o.arr_cap;
            obj_k = o.obj_k; obj_v = o.obj_v; obj_n = o.obj_n; obj_cap = o.obj_cap;
            o.type  = Null_;
            o.arr   = nullptr; o.arr_n  = o.arr_cap  = 0;
            o.obj_k = nullptr; o.obj_v  = nullptr;
            o.obj_n = o.obj_cap = 0;
        }
        return *this;
    }

    inline bool   is_null()   const { return type == Null_; }
    inline bool   is_string() const { return type == Str_;  }
    inline bool   is_array()  const { return type == Arr_;  }
    inline bool   is_object() const { return type == Obj_;  }
    inline size_t size()      const { return type == Arr_ ? arr_n : obj_n; }

    inline const char* str() const { return sval.c_str(); }
    inline double      num() const { return nval; }

    inline bool has(const char* k) const {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_k[i].eq_n(k, kl)) return true;
        return false;
    }

    // return a threadlical null sentinel for missing key lookups on the const path
    // the sentinel is never mutated through this path, the nonconst overload
    // insert a new key instead of returning this reference
    inline static const JVal& null_ref() {
        static JVal nv;
        return nv;
    }

    inline JVal& operator[](const char* k) {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_k[i].eq_n(k, kl)) return obj_v[i];
        if (obj_n >= obj_cap) {
            size_t nc = obj_cap ? obj_cap * 2 : 4;
            Str*  tk = (Str*)  realloc(obj_k, nc * sizeof(Str));
            JVal* tv = (JVal*) realloc(obj_v, nc * sizeof(JVal));
            if (!tk || !tv) {
                if (tk) obj_k = tk;
                if (tv) obj_v = tv;
                return obj_v[obj_n > 0 ? obj_n - 1 : 0];
            }
            obj_k = tk; obj_v = tv;
            for (size_t i = obj_cap; i < nc; ++i) { new (&obj_k[i]) Str(); new (&obj_v[i]) JVal(); }
            obj_cap = nc;
        }
        obj_k[obj_n].append(k, kl);
        return obj_v[obj_n++];
    }

    inline const JVal& operator[](const char* k) const {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_k[i].eq_n(k, kl)) return obj_v[i];
        return null_ref();
    }

    inline void push_obj_kv(Str k, JVal v) {
        if (obj_n >= obj_cap) {
            size_t nc = obj_cap ? obj_cap * 2 : 4;
            Str*  tk = (Str*)  realloc(obj_k, nc * sizeof(Str));
            JVal* tv = (JVal*) realloc(obj_v, nc * sizeof(JVal));
            if (!tk || !tv) { if (tk) obj_k = tk; if (tv) obj_v = tv; return; }
            obj_k = tk; obj_v = tv;
            for (size_t i = obj_cap; i < nc; ++i) { new (&obj_k[i]) Str(); new (&obj_v[i]) JVal(); }
            obj_cap = nc;
        }
        obj_k[obj_n] = std::move(k);
        obj_v[obj_n] = std::move(v);
        ++obj_n;
    }

    inline void push_arr_val(JVal v) {
        if (arr_n >= arr_cap) {
            size_t nc = arr_cap ? arr_cap * 2 : 4;
            JVal* ta = (JVal*) realloc(arr, nc * sizeof(JVal));
            if (!ta) return;
            arr = ta;
            for (size_t i = arr_cap; i < nc; ++i) new (&arr[i]) JVal();
            arr_cap = nc;
        }
        arr[arr_n++] = std::move(v);
    }
};

//json parser

inline void skip_ws(const char*& p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
}

inline Str parse_str_tok(const char*& p) {
    ++p; // skip opening '"'
    Str s;
    while (*p && *p != '"') {
        if (*p == '\\') {
            ++p;
            switch (*p) {
                case '"':  s.append_c('"');  break;
                case '\\': s.append_c('\\'); break;
                case '/':  s.append_c('/');  break;
                case 'n':  s.append_c('\n'); break;
                case 'r':  s.append_c('\r'); break;
                case 't':  s.append_c('\t'); break;
                default:   s.append_c('\\'); s.append_c(*p); break;
            }
        } else {
            s.append_c(*p);
        }
        ++p;
    }
    if (*p == '"') ++p;
    return s;
}

inline JVal parse_val(const char*& p);

inline JVal parse_obj(const char*& p) {
    JVal v; v.type = JVal::Obj_;
    ++p;
    for (;;) {
        skip_ws(p);
        if (!*p || *p == '}') break;
        if (*p != '"') break;
        Str key = parse_str_tok(p);
        skip_ws(p);
        if (*p == ':') ++p;
        JVal val = parse_val(p);
        v.push_obj_kv(std::move(key), std::move(val));
        skip_ws(p);
        if (*p == ',') ++p;
    }
    if (*p == '}') ++p;
    return v;
}

inline JVal parse_arr(const char*& p) {
    JVal v; v.type = JVal::Arr_;
    ++p;
    for (;;) {
        skip_ws(p);
        if (!*p || *p == ']') break;
        v.push_arr_val(parse_val(p));
        skip_ws(p);
        if (*p == ',') ++p;
    }
    if (*p == ']') ++p;
    return v;
}

inline JVal parse_val(const char*& p) {
    skip_ws(p);
    switch (*p) {
        case '{': return parse_obj(p);
        case '[': return parse_arr(p);
        case '"': { JVal v; v.type = JVal::Str_; v.sval = parse_str_tok(p); return v; }
        case 't': if (!strncmp(p, "true",  4)) { JVal v; v.type = JVal::Bool_; v.bval = true;  p += 4; return v; } break;
        case 'f': if (!strncmp(p, "false", 5)) { JVal v; v.type = JVal::Bool_; v.bval = false; p += 5; return v; } break;
        case 'n': if (!strncmp(p, "null",  4)) { p += 4; return JVal{}; } break;
    }
    JVal v; v.type = JVal::Num_;
    char* end; v.nval = strtod(p, &end); p = end;
    return v;
}

inline JVal parse_json(const Str& src) {
    const char* p = src.c_str();
    return parse_val(p);
}

//unicode and path utils

inline WStr to_wide(const char* s, int len = -1) {
    WStr r;
    if (!s || !*s) return r;
    int n = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
    if (n <= 0) return r;
    r.grow((size_t)n);
    MultiByteToWideChar(CP_UTF8, 0, s, len, r.p, n);
    r.n = (size_t)(len < 0 ? n - 1 : n);
    if (r.p) r.p[r.n] = 0;
    return r;
}

inline Str to_utf8(const wchar_t* w, int len = -1) {
    Str r;
    if (!w || !*w) return r;
    int n = WideCharToMultiByte(CP_UTF8, 0, w, len, nullptr, 0, nullptr, nullptr);
    if (n <= 0) return r;
    r.grow((size_t)n);
    WideCharToMultiByte(CP_UTF8, 0, w, len, r.p, n, nullptr, nullptr);
    r.n = (size_t)(len < 0 ? n - 1 : n);
    if (r.p) r.p[r.n] = 0;
    return r;
}

inline WStr path_join(const WStr& base, const char* comp) {
    WStr r; r.copy_from(base);
    if (r.n && r.back() != L'\\') r.append_c(L'\\');
    WStr w = to_wide(comp);
    if (w.p) r.append(w.p, w.n);
    return r;
}

inline bool path_exists(const WStr& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

inline LONGLONG path_size(const WStr& path) {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &d)) return -1;
    LARGE_INTEGER li; li.HighPart = (LONG)d.nFileSizeHigh; li.LowPart = d.nFileSizeLow;
    return li.QuadPart;
}

inline Str path_str(const WStr& path) { return to_utf8(path.c_str()); }

inline WStr path_parent(const WStr& path) {
    WStr r; r.copy_from(path);
    while (r.n && (r.back() == L'\\' || r.back() == L'/')) r.pop_back();
    while (r.n && r.back() != L'\\' && r.back() != L'/') r.pop_back();
    if (r.n) r.pop_back();
    return r;
}

inline void make_dirs(const WStr& path) {
    size_t len = path.n;
    wchar_t* tmp = (wchar_t*)malloc((len + 2) * sizeof(wchar_t));
    if (!tmp) return;
    memcpy(tmp, path.p, (len + 1) * sizeof(wchar_t));
    for (size_t i = 1; i <= len; ++i) {
        if (i == len || tmp[i] == L'\\' || tmp[i] == L'/') {
            wchar_t c = tmp[i]; tmp[i] = 0;
            CreateDirectoryW(tmp, nullptr);
            tmp[i] = c;
        }
    }
    free(tmp);
}

inline Str read_file(const WStr& path) {
    Str r;
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return r;
    DWORD sz = GetFileSize(h, nullptr);
    if (sz && sz != INVALID_FILE_SIZE) {
        r.grow((size_t)sz);
        DWORD rd = 0;
        ReadFile(h, r.p, sz, &rd, nullptr);
        r.n = (size_t)rd;
        if (r.p) r.p[r.n] = 0;
    }
    CloseHandle(h);
    return r;
}

inline void write_file(const WStr& path, const char* data, size_t len) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD wr = 0;
    WriteFile(h, data, (DWORD)len, &wr, nullptr);
    CloseHandle(h);
}

inline Str read_line() {
    Str r;
    int c;
    while ((c = getchar()) != EOF && c != '\n') r.append_c((char)c);
    if (!r.empty() && r.back() == '\r') r.pop_back();
    return r;
}

inline bool parse_int(const Str& s, int* out) {
    if (s.empty()) return false;
    size_t start = (s.p[0] == '-') ? 1 : 0;
    if (start >= s.n) return false;
    for (size_t i = start; i < s.n; ++i) if (!isdigit((uint8_t)s.p[i])) return false;
    *out = atoi(s.c_str());
    return true;
}

//http layer(winhttp)

// global session opened once at startup and closed at exit via destructor
// redirect policy is set to manual so we can track Location headers ourselve
struct WSession {
    HINTERNET h = nullptr;
    WSession() {
        h = WinHttpOpen(L"MCPP/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (h) {
            DWORD pol = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            WinHttpSetOption(h, WINHTTP_OPTION_REDIRECT_POLICY, &pol, sizeof(pol));
        }
    }
    ~WSession() { if (h) WinHttpCloseHandle(h); }
    WSession(const WSession&)            = delete;
    WSession& operator=(const WSession&) = delete;
} g_sess;

struct UrlParts { WStr host, path; INTERNET_PORT port; bool https; };

inline UrlParts crack_url(const WStr& url) {
    UrlParts r{};
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[512]{}, upath[4096]{};
    uc.lpszHostName = host;  uc.dwHostNameLength = 512;
    uc.lpszUrlPath  = upath; uc.dwUrlPathLength  = 4096;
    WinHttpCrackUrl(url.c_str(), 0, 0, &uc);
    r.host.assign_w(host);
    r.path.assign_w(upath);
    r.port  = uc.nPort;
    r.https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
    return r;
}

// opens a GET request, following up to max_redir http redirects manually.
// both out_conn and the returned request handle must be closed by the caller.
inline HINTERNET open_req(const Str& url_s, HINTERNET& out_conn, int max_redir = 10) {
    if (!g_sess.h) return nullptr;
    Str cur; cur.copy_from(url_s);

    for (int i = 0; i <= max_redir; ++i) {
        UrlParts pu = crack_url(to_wide(cur.c_str()));

        HINTERNET hConn = WinHttpConnect(g_sess.h, pu.host.c_str(), pu.port, 0);
        if (!hConn) return nullptr;

        DWORD     flags = pu.https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hReq  = WinHttpOpenRequest(hConn, L"GET", pu.path.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hReq) { WinHttpCloseHandle(hConn); return nullptr; }

        if (pu.https) {
            DWORD sec = SECURITY_FLAG_IGNORE_UNKNOWN_CA
                      | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
                      | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
            WinHttpSetOption(hReq, WINHTTP_OPTION_SECURITY_FLAGS, &sec, sizeof(sec));
        }

        if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
         || !WinHttpReceiveResponse(hReq, nullptr)) {
            WinHttpCloseHandle(hReq);
            WinHttpCloseHandle(hConn);
            return nullptr;
        }

        DWORD status = 0, sz = sizeof(status);
        WinHttpQueryHeaders(hReq,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);

        if (status == 301 || status == 302 || status == 303 ||
            status == 307 || status == 308) {
            // read the Location header before closing the request handle
            DWORD loc_sz = 0;
            WinHttpQueryHeaders(hReq, WINHTTP_QUERY_LOCATION,
                WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &loc_sz, WINHTTP_NO_HEADER_INDEX);
            if (loc_sz > 0) {
                size_t   wlen = loc_sz / sizeof(wchar_t) + 2;
                wchar_t* loc  = (wchar_t*)malloc(wlen * sizeof(wchar_t));
                if (loc) {
                    loc[0] = 0;
                    WinHttpQueryHeaders(hReq, WINHTTP_QUERY_LOCATION,
                        WINHTTP_HEADER_NAME_BY_INDEX, loc, &loc_sz, WINHTTP_NO_HEADER_INDEX);
                    size_t ln = wcslen(loc);
                    while (ln && loc[ln - 1] == 0) --ln;
                    loc[ln] = 0;
                    cur = to_utf8(loc);
                    free(loc);
                }
            }
            WinHttpCloseHandle(hReq);
            WinHttpCloseHandle(hConn);
            continue;
        }

        out_conn = hConn;
        return hReq;
    }
    return nullptr;
}

inline Str http_get(const Str& url) {
    HINTERNET hConn = nullptr, hReq = open_req(url, hConn);
    Str result;
    if (!hReq) return result;
    char buf[65536];
    DWORD rd = 0;
    while (WinHttpReadData(hReq, buf, sizeof(buf), &rd) && rd)
        result.append(buf, (size_t)rd);
    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConn);
    return result;
}

// guards directory creation across concurrent download threads
CRITICAL_SECTION g_mkdir_cs;

inline bool http_download(const Str& url, const WStr& dest) {
    HINTERNET hConn = nullptr, hReq = open_req(url, hConn);
    if (!hReq) return false;

    EnterCriticalSection(&g_mkdir_cs);
    WStr parent = path_parent(dest);
    if (!parent.empty()) make_dirs(parent);
    LeaveCriticalSection(&g_mkdir_cs);

    HANDLE hFile = CreateFileW(dest.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        WinHttpCloseHandle(hReq);
        WinHttpCloseHandle(hConn);
        return false;
    }

    bool ok = true;
    char buf[131072];
    DWORD rd = 0, wr = 0;
    while (WinHttpReadData(hReq, buf, sizeof(buf), &rd) && rd) {
        if (!WriteFile(hFile, buf, rd, &wr, nullptr) || wr != rd) { ok = false; break; }
    }

    CloseHandle(hFile);
    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConn);
    if (!ok) DeleteFileW(dest.c_str());
    return ok;
}

// skips the download if the file already exists and is non-empty
inline bool download_file(const Str& url, const WStr& dest) {
    if (path_exists(dest) && path_size(dest) > 0) return true;
    return http_download(url, dest);
}

//parallel downloader

struct DLTask { Str url; WStr dest; };

struct DLCtx {
    DLTask*        tasks;
    volatile LONG* cursor;
    volatile LONG* ndone;
    LONG           total;
};

inline DWORD WINAPI dl_worker(LPVOID arg) {
    DLCtx* ctx = (DLCtx*)arg;
    for (;;) {
        LONG i = InterlockedExchangeAdd(ctx->cursor, 1);
        if (i >= ctx->total) break;
        download_file(ctx->tasks[i].url, ctx->tasks[i].dest);
        InterlockedIncrement(ctx->ndone);
    }
    return 0;
}

inline void parallel_dl(Vec<DLTask>& tasks, int nthreads = 16) {
    if (tasks.empty()) return;

    volatile LONG cursor = 0, ndone = 0;
    LONG  total = (LONG)tasks.n;
    DLCtx ctx   = { tasks.p, &cursor, &ndone, total };

    int n = nthreads < (int)total ? nthreads : (int)total;
    Vec<HANDLE> pool;
    pool.reserve((size_t)n);
    for (int t = 0; t < n; ++t) {
        HANDLE h = CreateThread(nullptr, 0, dl_worker, &ctx, 0, nullptr);
        if (h) pool.push_back(h);
        else   InterlockedExchangeAdd(&ndone, 0);
    }

    while (ndone < (LONG)pool.n || cursor < total) {
        printf("  %ld/%ld\r", ndone, total);
        fflush(stdout);
        Sleep(100);
    }
    if (!pool.empty())
        WaitForMultipleObjects((DWORD)pool.n, pool.p, TRUE, INFINITE);
    for (size_t t = 0; t < pool.n; ++t) CloseHandle(pool.p[t]);
    printf("  %ld/%ld\n", total, total);
}

// config

struct Config {
    Str  username;
    Str  java_path;
    Str  java_args;
    int  ram_gb        = 4;
    int  theme_color   = 7;
    bool hide_launcher = true;
    bool show_console  = false;
};

inline Str json_esc(const Str& s) {
    Str r;
    for (size_t i = 0; i < s.n; ++i) {
        if      (s.p[i] == '"')  { r.append_c('\\'); r.append_c('"'); }
        else if (s.p[i] == '\\') { r.append_c('\\'); r.append_c('\\'); }
        else r.append_c(s.p[i]);
    }
    return r;
}

inline Config load_config(const WStr& path) {
    Config c;
    c.username.assign_s("Player");
    c.java_path.assign_s("javaw");
    if (!path_exists(path)) return c;
    Str src = read_file(path);
    if (src.empty()) return c;
    JVal j = parse_json(src);
    if (j.has("username"))      c.username.assign_s(j["username"].str());
    if (j.has("java_path"))     c.java_path.assign_s(j["java_path"].str());
    if (j.has("java_args"))     c.java_args.assign_s(j["java_args"].str());
    if (j.has("ram_gb"))        c.ram_gb        = (int)j["ram_gb"].num();
    if (j.has("theme_color"))   c.theme_color   = (int)j["theme_color"].num();
    if (j.has("hide_launcher")) c.hide_launcher = j["hide_launcher"].bval;
    if (j.has("show_console"))  c.show_console  = j["show_console"].bval;
    if (c.ram_gb < 1) c.ram_gb = 1;
    return c;
}

inline void save_config(const Config& c, const WStr& path) {
    Str eu = json_esc(c.username);
    Str ej = json_esc(c.java_path);
    Str ea = json_esc(c.java_args);
    char buf[1024];
    int n = snprintf(buf, sizeof(buf),
        "{\n"
        "  \"username\": \"%s\",\n"
        "  \"java_path\": \"%s\",\n"
        "  \"java_args\": \"%s\",\n"
        "  \"ram_gb\": %d,\n"
        "  \"theme_color\": %d,\n"
        "  \"hide_launcher\": %s,\n"
        "  \"show_console\": %s\n"
        "}\n",
        eu.c_str(), ej.c_str(), ea.c_str(),
        c.ram_gb, c.theme_color,
        c.hide_launcher ? "true" : "false",
        c.show_console  ? "true" : "false");
    if (n > 0) write_file(path, buf, (size_t)n);
}

//version logic

struct MCVer { int v[3]; };

inline MCVer parse_mc_ver(const char* s) {
    MCVer r{};
    int idx = 0;
    const char* p = s;
    while (*p && idx < 3) {
        if (!isdigit((uint8_t)*p)) { if (*p != '.') break; ++p; continue; }
        while (isdigit((uint8_t)*p)) r.v[idx] = r.v[idx] * 10 + (*p++ - '0');
        if (++idx < 3 && *p == '.') ++p;
    }
    return r;
}

inline int cmp_ver(const MCVer& a, const MCVer& b) {
    for (int i = 0; i < 3; ++i) {
        if (a.v[i] < b.v[i]) return -1;
        if (a.v[i] > b.v[i]) return  1;
    }
    return 0;
}

// version sentinels computed once at startup, never mutated
inline const MCVer VER_117  = parse_mc_ver("1.17");   // first release requiring jdk17
inline const MCVer VER_1205 = parse_mc_ver("1.20.5"); // first release requiring jdk21
inline const MCVer VER_260  = parse_mc_ver("26.0");   // first release requiring jdk25

// returns the minimum jdk version the given minecraft version needs
inline int required_jdk(const char* mc) {
    MCVer v = parse_mc_ver(mc);
    if (cmp_ver(v, VER_117)  < 0) return 8;
    if (cmp_ver(v, VER_1205) < 0) return 17;
    if (cmp_ver(v, VER_260)  < 0) return 21;
    return 25; // 26.x ships microsoft build of openjdk 25
}

inline const char* runtime_component(const char* mc) {
    MCVer v = parse_mc_ver(mc);
    if (cmp_ver(v, VER_117)  < 0) return "jre-legacy";
    if (cmp_ver(v, VER_1205) < 0) return "java-runtime-gamma";
    if (cmp_ver(v, VER_260)  < 0) return "java-runtime-delta";
    return "java-runtime-epsilon";
}

// generate offline uuid from a player name

inline Str offline_uuid(const Str& name) {
    Str seed;
    seed.assign_s("OfflinePlayer:");
    seed.append(name.p, name.n);

    uint8_t h[16]{};
    for (size_t i = 0; i < seed.n; ++i) {
        h[i % 16]       ^= (uint8_t)((uint8_t)seed.p[i] * (uint8_t)(i + 1));
        h[(i + 3) % 16] += (uint8_t)seed.p[i];
    }
    h[6] = (h[6] & 0x0f) | 0x30; // version 3
    h[8] = (h[8] & 0x3f) | 0x80; // variant 2

    char buf[37];
    snprintf(buf, sizeof(buf),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        h[0],  h[1],  h[2],  h[3],  h[4],  h[5],  h[6],  h[7],
        h[8],  h[9],  h[10], h[11], h[12], h[13], h[14], h[15]);

    Str r; r.assign_s(buf);
    return r;
}


inline bool lib_applies(const JVal& lib) {
    if (!lib.has("rules")) return true;
    bool allowed = false;
    for (size_t i = 0; i < lib["rules"].arr_n; ++i) {
        const JVal& rule = lib["rules"].arr[i];
        bool match = !rule.has("os") || !strcmp(rule["os"]["name"].str(), "windows");
        if (match) allowed = !strcmp(rule["action"].str(), "allow");
    }
    return allowed;
}


inline bool lib_native_only(const JVal& lib) {
    if (lib.has("natives") && lib["natives"].has("windows")) return true;
    if (lib.has("downloads") && lib["downloads"].has("artifact")) {
        const char* p = lib["downloads"]["artifact"]["path"].str();
        return strstr(p, "natives-windows") != nullptr;
    }
    return false;
}


inline bool native_path_ok(const char* p) {
    if (!strstr(p, "natives-windows")) return false;
    if (strstr(p, "natives-windows-arm64"))  return false;
    bool is64 = sizeof(void*) == 8;
    if (strstr(p, "natives-windows-x86_64")) return is64;
    if (strstr(p, "natives-windows-x86"))    return !is64;
    return true;
}

//search a dir for the first javaw.exe it finds

inline void find_javaw(const wchar_t* dir, WStr* result) {
    if (!result->empty()) return;
    wchar_t pattern[4096];
    swprintf(pattern, 4096, L"%ls\\*", dir);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..")) continue;
        wchar_t full[4096];
        swprintf(full, 4096, L"%ls\\%ls", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            find_javaw(full, result);
        } else {
            wchar_t lower[MAX_PATH];
            size_t fl = wcslen(fd.cFileName);
            for (size_t i = 0; i <= fl; ++i) lower[i] = (wchar_t)tolower((uint8_t)fd.cFileName[i]);
            if (!wcscmp(lower, L"javaw.exe")) result->assign_w(full);
        }
    } while (FindNextFileW(h, &fd) && result->empty());
    FindClose(h);
}

inline WStr find_java_in(const WStr& dir) {
    WStr result;
    if (!dir.empty() && path_exists(dir)) find_javaw(dir.c_str(), &result);
    return result;
}

inline Str maven_path(const char* coords) {
    Str r;
    const char* c1 = strchr(coords, ':');  if (!c1) return r;
    const char* c2 = strchr(c1 + 1, ':'); if (!c2) return r;

    Str group;    group.append(coords, c1 - coords);
    Str artifact; artifact.append(c1 + 1, c2 - c1 - 1);
    Str ver;      ver.assign_s(c2 + 1);

    Str classifier;
    size_t cp = ver.find(':');
    if (cp != NPOS) { classifier = ver.substr(cp + 1); ver = ver.substr(0, cp); }

    for (size_t i = 0; i < group.n; ++i) if (group.p[i] == '.') group.p[i] = '/';

    Str fname;
    fname.append(artifact.p, artifact.n); fname.append_c('-');
    fname.append(ver.p, ver.n);
    if (!classifier.empty()) { fname.append_c('-'); fname.append(classifier.p, classifier.n); }
    fname.append_s(".jar");

    r.append(group.p, group.n);       r.append_c('/');
    r.append(artifact.p, artifact.n); r.append_c('/');
    r.append(ver.p, ver.n);           r.append_c('/');
    r.append(fname.p, fname.n);
    return r;
}

inline bool check_java(const Str& java) {
    Str cmd;
    cmd.append_c('"'); cmd.append(java.p, java.n); cmd.append_s("\" -version > NUL 2>&1");
    return system(cmd.c_str()) == 0;
}

// API endpoints

inline constexpr const char* URL_MANIFEST    = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
inline constexpr const char* URL_ASSETS      = "https://resources.download.minecraft.net/";
inline constexpr const char* URL_RUNTIME_ALL =
    "https://launchermeta.mojang.com/v1/products/java-runtime/"
    "2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";
inline constexpr const char* URL_FABRIC_META = "https://meta.fabricmc.net/v2/versions/";

//lib collection and native extraction

inline void collect_lib_tasks(const WStr& root, const JVal& vj, Vec<DLTask>& tasks) {
    if (!vj.has("libraries")) return;
    WStr lib_dir = path_join(root, "libraries");

    for (size_t i = 0; i < vj["libraries"].arr_n; ++i) {
        const JVal& lib = vj["libraries"].arr[i];
        if (!lib_applies(lib)) continue;

        if (lib.has("name") && !lib.has("downloads")) {
            Str rel = maven_path(lib["name"].str());
            if (rel.empty()) continue;
            Str url;
            url.assign_s(lib.has("url") ? lib["url"].str() : "https://libraries.minecraft.net/");
            if (!url.empty() && url.back() != '/') url.append_c('/');
            url.append(rel.p, rel.n);
            DLTask t; t.url = std::move(url); t.dest = path_join(lib_dir, rel.c_str());
            tasks.push_back(std::move(t));
            continue;
        }

        if (!lib.has("downloads")) continue;

        if (lib.has("natives") && lib["natives"].has("windows")) {
            Str cls; cls.assign_s(lib["natives"]["windows"].str());
            const char* arch = sizeof(void*) == 8 ? "64" : "32";
            size_t pos = cls.find_s("${arch}");
            if (pos != NPOS) cls.replace_range(pos, 7, arch);
            if (lib["downloads"].has("classifiers") &&
                lib["downloads"]["classifiers"].has(cls.c_str())) {
                const JVal& a = lib["downloads"]["classifiers"][cls.c_str()];
                const char* u = a["url"].str(), *p = a["path"].str();
                if (u && *u && p && *p) {
                    DLTask t; t.url.assign_s(u); t.dest = path_join(lib_dir, p);
                    tasks.push_back(std::move(t));
                }
            }
        }

        if (lib["downloads"].has("artifact")) {
            const JVal& a = lib["downloads"]["artifact"];
            const char* u = a["url"].str(), *p = a["path"].str();
            if (u && *u && p && *p) {
                DLTask t; t.url.assign_s(u); t.dest = path_join(lib_dir, p);
                tasks.push_back(std::move(t));
            }
        }
    }
}

inline void extract_natives(const WStr& root, const char* version, const JVal& vj) {
    if (!vj.has("libraries")) return;
    WStr lib_dir = path_join(root, "libraries");
    WStr nat_dir = path_join(path_join(path_join(root, "versions"), version), "natives");
    make_dirs(nat_dir);

    int extracted = 0;
    for (size_t i = 0; i < vj["libraries"].arr_n; ++i) {
        const JVal& lib = vj["libraries"].arr[i];
        if (!lib_applies(lib)) continue;

        WStr jar;
        if (lib.has("natives") && lib["natives"].has("windows")) {
            Str cls; cls.assign_s(lib["natives"]["windows"].str());
            const char* arch = sizeof(void*) == 8 ? "64" : "32";
            size_t pos = cls.find_s("${arch}");
            if (pos != NPOS) cls.replace_range(pos, 7, arch);
            if (!lib.has("downloads") || !lib["downloads"].has("classifiers") ||
                !lib["downloads"]["classifiers"].has(cls.c_str())) continue;
            const char* p = lib["downloads"]["classifiers"][cls.c_str()]["path"].str();
            if (!p || !*p) continue;
            jar = path_join(lib_dir, p);
        } else if (lib.has("downloads") && lib["downloads"].has("artifact")) {
            const char* p = lib["downloads"]["artifact"]["path"].str();
            if (!native_path_ok(p)) continue;
            jar = path_join(lib_dir, p);
        } else {
            continue;
        }

        if (!path_exists(jar)) {
            fprintf(stderr, "  [natives] jar missing: %s\n", path_str(jar).c_str());
            continue;
        }

        char cmd[8192];
        snprintf(cmd, sizeof(cmd),
            "tar -xf \"%s\" -C \"%s\" --exclude=META-INF 2>NUL",
            path_str(jar).c_str(), path_str(nat_dir).c_str());
        system(cmd);
        ++extracted;
    }
    printf("  Extracted %d native jar(s) -> %s\n", extracted, path_str(nat_dir).c_str());
}

inline bool download_assets(const WStr& root, const JVal& vj) {
    const char* idx_url = vj["assetIndex"]["url"].str();
    const char* idx_id  = vj["assetIndex"]["id"].str();
    if (!idx_url || !*idx_url || !idx_id || !*idx_id) {
        fputs("  no asset index in version json.\n", stderr); return false;
    }

    WStr idx_dir = path_join(path_join(root, "assets"), "indexes");
    make_dirs(idx_dir);

    Str idx_fname; idx_fname.assign_s(idx_id); idx_fname.append_s(".json");
    WStr idx_file = path_join(idx_dir, idx_fname.c_str());

    Str idx_str;
    if (path_exists(idx_file)) {
        idx_str = read_file(idx_file);
    } else {
        Str u; u.assign_s(idx_url);
        idx_str = http_get(u);
        if (idx_str.empty()) { fputs("  failed to fetch asset index.\n", stderr); return false; }
        write_file(idx_file, idx_str.c_str(), idx_str.n);
    }

    JVal idx_json = parse_json(idx_str);
    const JVal& objs = idx_json["objects"];
    WStr obj_dir = path_join(path_join(root, "assets"), "objects");

    Vec<DLTask> tasks;
    size_t already = 0;
    for (size_t i = 0; i < objs.obj_n; ++i) {
        const char* hash = objs.obj_v[i]["hash"].str();
        if (!hash || strlen(hash) < 2) continue;
        char pfx[3] = { hash[0], hash[1], 0 };
        WStr dest = path_join(path_join(obj_dir, pfx), hash);
        if (path_exists(dest) && path_size(dest) > 0) { ++already; continue; }
        DLTask t;
        t.url.assign_s(URL_ASSETS);
        t.url.append_s(pfx); t.url.append_c('/'); t.url.append_s(hash);
        t.dest = std::move(dest);
        tasks.push_back(std::move(t));
    }

    printf("  fetching %zu assets (%zu already cached)...\n", objs.obj_n, already);
    parallel_dl(tasks, 24);
    return true;
}

// mc base installer

inline bool install_mc_base(const WStr& root, const char* version,
                              const JVal& manifest, bool steps = true) {
    const char* ver_url = nullptr;
    for (size_t i = 0; i < manifest["versions"].arr_n; ++i) {
        const JVal& v = manifest["versions"].arr[i];
        if (!strcmp(v["id"].str(), version)) { ver_url = v["url"].str(); break; }
    }
    if (!ver_url || !*ver_url) {
        fprintf(stderr, "version %s not found in manifest.\n", version); return false;
    }

    WStr ver_dir  = path_join(path_join(root, "versions"), version);
    Str  json_name; json_name.assign_s(version); json_name.append_s(".json");
    Str  jar_name;  jar_name.assign_s(version);  jar_name.append_s(".jar");
    WStr ver_json = path_join(ver_dir, json_name.c_str());
    WStr ver_jar  = path_join(ver_dir, jar_name.c_str());
    make_dirs(ver_dir);

    if (steps) printf("[1/5] fetching %s version json...\n", version);
    Str ver_str;
    if (path_exists(ver_json)) {
        ver_str = read_file(ver_json);
    } else {
        Str u; u.assign_s(ver_url);
        ver_str = http_get(u);
        if (ver_str.empty()) { fputs("failed to fetch version json.\n", stderr); return false; }
        write_file(ver_json, ver_str.c_str(), ver_str.n);
    }
    JVal vj = parse_json(ver_str);

    if (steps) fputs("[2/5] Downloading client jar...\n", stdout);
    Str jar_url; jar_url.assign_s(vj["downloads"]["client"]["url"].str());
    if (!download_file(jar_url, ver_jar)) { fputs("failed to download client jar.\n", stderr); return false; }

    if (steps) fputs("[3/5] Downloading libraries...\n", stdout);
    Vec<DLTask> lib_tasks;
    collect_lib_tasks(root, vj, lib_tasks);
    parallel_dl(lib_tasks, 16);

    if (steps) fputs("[4/5] Extracting natives...\n", stdout);
    extract_natives(root, version, vj);

    if (steps) fputs("[5/5] Downloading assets...\n", stdout);
    download_assets(root, vj);
    return true;
}

// fabric installer

inline bool install_fabric(const WStr& root, const char* mc_ver, const JVal& manifest) {
    printf("fetching fabric loaders for minecraft %s...\n", mc_ver);

    Str loaders_url; loaders_url.assign_s(URL_FABRIC_META);
    loaders_url.append_s("loader/"); loaders_url.append_s(mc_ver);
    Str loaders_str = http_get(loaders_url);
    if (loaders_str.empty()) { fputs("failed to fetch fabric loader list.\n", stderr); return false; }

    JVal loaders = parse_json(loaders_str);
    if (!loaders.is_array() || !loaders.arr_n) {
        fputs("no fabric loaders available for this version.\n", stderr); return false;
    }

    const char* loader_ver = loaders.arr[0]["loader"]["version"].str();
    if (!loader_ver || !*loader_ver) { fputs("could not determine fabric loader version.\n", stderr); return false; }
    printf("using fabric loader: %s\n", loader_ver);

    Str fabric_id;
    fabric_id.assign_s("fabric-loader-");
    fabric_id.append_s(loader_ver); fabric_id.append_c('-'); fabric_id.append_s(mc_ver);

    WStr ver_dir  = path_join(path_join(root, "versions"), fabric_id.c_str());
    Str  vjname;  vjname.copy_from(fabric_id); vjname.append_s(".json");
    WStr ver_json = path_join(ver_dir, vjname.c_str());
    make_dirs(ver_dir);

    fputs("[1/5] fetching fabric profile json...\n", stdout);
    Str profile_url; profile_url.assign_s(URL_FABRIC_META);
    profile_url.append_s("loader/"); profile_url.append_s(mc_ver);
    profile_url.append_c('/');       profile_url.append_s(loader_ver);
    profile_url.append_s("/profile/json");

    Str profile_str;
    if (path_exists(ver_json)) {
        profile_str = read_file(ver_json);
    } else {
        profile_str = http_get(profile_url);
        if (profile_str.empty()) { fputs("failed to fetch fabric profile json.\n", stderr); return false; }
        write_file(ver_json, profile_str.c_str(), profile_str.n);
    }
    JVal fabric_vj = parse_json(profile_str);

    printf("[2/5] Downloading base minecraft %s...\n", mc_ver);
    if (!install_mc_base(root, mc_ver, manifest, false)) {
        fputs("failed to download base minecraft for fabric.\n", stderr); return false;
    }

    fputs("[3/5] Downloading fabric libraries...\n", stdout);
    Vec<DLTask> fabric_tasks;
    collect_lib_tasks(root, fabric_vj, fabric_tasks);
    parallel_dl(fabric_tasks, 16);

    fputs("[4/5] Extracting fabric natives (if any)...\n", stdout);
    extract_natives(root, mc_ver, fabric_vj);

    fputs("[5/5] (assets already fetched with base mc)\n", stdout);
    printf("\nfabric install complete: %s\n", fabric_id.c_str());
    return true;
}

// bundled jre installer

inline bool install_bundled_jre(const WStr& root, Config& cfg, const WStr& cfg_path,
                                  const char* mc_ver = "") {
    const char* component = (!mc_ver || !*mc_ver) ? "jre-legacy" : runtime_component(mc_ver);
    WStr jre_dir = path_join(path_join(root, "runtime"), component);

    WStr found = find_java_in(jre_dir);
    if (!found.empty()) {
        Str fs = path_str(found);
        printf("  found mojang jre (%s): %s\n", component, fs.c_str());
        if (!check_java(cfg.java_path)) { cfg.java_path = std::move(fs); save_config(cfg, cfg_path); }
        return true;
    }

    printf("\nno bundled jre found for '%s'.\n", component);
    printf("download mojang jre (%s) automatically? (y/n): ", component);
    Str ans = read_line();
    if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return false;

    fputs("  Fetching mojang runtime index...\n", stdout);
    Str all_url; all_url.assign_s(URL_RUNTIME_ALL);
    Str all_str = http_get(all_url);
    if (all_str.empty()) { fputs("  failed to fetch runtime index.\n", stderr); return false; }
    JVal all_j = parse_json(all_str);

    const char* platform = "windows-x64";
    if (!all_j.has(platform) || !all_j[platform].has(component)) {
        fprintf(stderr, "  component '%s' not found for %s.\n", component, platform);
        return false;
    }
    const JVal& comp_arr = all_j[platform][component];
    if (!comp_arr.is_array() || !comp_arr.arr_n) { fputs("  empty component entry.\n", stderr); return false; }

    const char* mf_url = comp_arr.arr[0]["manifest"]["url"].str();
    if (!mf_url || !*mf_url) { fputs("  no manifest url.\n", stderr); return false; }

    printf("  Fetching file manifest for '%s'...\n", component);
    Str mu; mu.assign_s(mf_url);
    Str mf_str = http_get(mu);
    if (mf_str.empty()) { fputs("  failed to fetch file manifest.\n", stderr); return false; }
    JVal mf = parse_json(mf_str);

    make_dirs(jre_dir);
    const JVal& files = mf["files"];

    Vec<DLTask> tasks;
    tasks.reserve(files.obj_n);
    for (size_t i = 0; i < files.obj_n; ++i) {
        const JVal& entry = files.obj_v[i];
        WStr rel = path_join(jre_dir, files.obj_k[i].c_str());
        if (!strcmp(entry["type"].str(), "directory")) { make_dirs(rel); continue; }
        if (strcmp(entry["type"].str(), "file") != 0)   continue;
        if (!entry.has("downloads") || !entry["downloads"].has("raw")) continue;
        const char* dl_url = entry["downloads"]["raw"]["url"].str();
        if (!dl_url || !*dl_url) continue;
        DLTask t; t.url.assign_s(dl_url); t.dest = std::move(rel);
        tasks.push_back(std::move(t));
    }

    printf("  Downloading %zu jre files...\n", tasks.n);
    parallel_dl(tasks, 16);

    found = find_java_in(jre_dir);
    if (found.empty()) {
        fprintf(stderr, "  javaw.exe not found after install in: %s\n", path_str(jre_dir).c_str());
        return false;
    }

    Str found_s = path_str(found);
    printf("  Mojang jre (%s) installed: %s\n", component, found_s.c_str());
    cfg.java_path = std::move(found_s);
    save_config(cfg, cfg_path);
    return true;
}

//launch command

struct KVPair { Str key; Str val; };

struct VarMap {
    Vec<KVPair> pairs;

    inline void set(const char* k, const char* v) {
        size_t kl = strlen(k);
        for (size_t i = 0; i < pairs.n; ++i)
            if (pairs.p[i].key.eq_n(k, kl)) { pairs.p[i].val.assign_s(v); return; }
        KVPair p; p.key.append(k, kl); p.val.assign_s(v);
        pairs.push_back(std::move(p));
    }
    inline const Str* get(const char* k, size_t kl) const {
        for (size_t i = 0; i < pairs.n; ++i)
            if (pairs.p[i].key.eq_n(k, kl)) return &pairs.p[i].val;
        return nullptr;
    }
};

inline Str tok_replace(const char* s, size_t slen, const VarMap& m) {
    Str r;
    for (size_t i = 0; i < slen; ) {
        if (s[i] == '$' && i + 1 < slen && s[i + 1] == '{') {
            size_t e = i + 2;
            while (e < slen && s[e] != '}') ++e;
            if (e < slen) {
                const Str* val = m.get(s + i + 2, e - i - 2);
                if (val) r.append(val->p, val->n);
                else     r.append(s + i, e - i + 1);
                i = e + 1;
                continue;
            }
        }
        r.append_c(s[i++]);
    }
    return r;
}

inline Str win_quote(const Str& s) {
    bool needs = s.empty();
    for (size_t i = 0; i < s.n && !needs; ++i)
        if (s.p[i] == ' ' || s.p[i] == '\t' || s.p[i] == '"') needs = true;
    if (!needs) { Str r; r.copy_from(s); return r; }
    Str r; r.append_c('"');
    int sl = 0;
    for (size_t i = 0; i < s.n; ++i) {
        char c = s.p[i];
        if (c == '\\')     { ++sl; }
        else if (c == '"') { for (int j = 0; j < sl * 2 + 1; ++j) r.append_c('\\'); r.append_c('"'); sl = 0; }
        else               { for (int j = 0; j < sl; ++j) r.append_c('\\'); sl = 0; r.append_c(c); }
    }
    for (int j = 0; j < sl * 2; ++j) r.append_c('\\');
    r.append_c('"');
    return r;
}

inline Str maven_ga(const char* path) {
    Str r;
    if (!path || !*path) return r;
    size_t len = strlen(path);
    int slashes = 0; size_t cut = len;
    for (size_t i = len; i > 0; --i)
        if (path[i - 1] == '/' || path[i - 1] == '\\')
            if (++slashes == 2) { cut = i - 1; break; }
    r.append(path, cut);
    return r;
}

struct CPEntry { Str ga; Str full; };

inline Str build_classpath(const WStr& root, const JVal& vj, const JVal& parent_vj,
                             const char* jar_ver) {
    WStr lib_dir = path_join(root, "libraries");
    Vec<CPEntry> entries;
    bool has_parent = !parent_vj.is_null();

    auto add_libs = [&](const JVal& j) {
        if (!j.has("libraries")) return;
        for (size_t i = 0; i < j["libraries"].arr_n; ++i) {
            const JVal& lib = j["libraries"].arr[i];
            if (!lib_applies(lib) || lib_native_only(lib)) continue;

            const char* path = nullptr;
            Str mp;
            if (lib.has("downloads") && lib["downloads"].has("artifact"))
                path = lib["downloads"]["artifact"]["path"].str();
            if (!path || !*path) {
                if (lib.has("name")) mp = maven_path(lib["name"].str());
                if (!mp.empty()) path = mp.c_str();
            }
            if (!path || !*path) continue;

            WStr jar = path_join(lib_dir, path);
            if (!path_exists(jar)) continue;

            Str ga = maven_ga(path), full = path_str(jar);
            bool found = false;
            for (size_t k = 0; k < entries.n; ++k) {
                if (entries.p[k].ga.eq(ga.c_str())) {
                    entries.p[k].full = std::move(full); found = true; break;
                }
            }
            if (!found) {
                CPEntry e; e.ga = std::move(ga); e.full = std::move(full);
                entries.push_back(std::move(e));
            }
        }
    };

    if (has_parent) { add_libs(parent_vj); add_libs(vj); }
    else              add_libs(vj);

    Str cp;
    for (size_t i = 0; i < entries.n; ++i) {
        cp.append(entries.p[i].full.p, entries.p[i].full.n);
        cp.append_c(';');
    }
    Str jf; jf.assign_s(jar_ver); jf.append_s(".jar");
    WStr main_jar = path_join(path_join(path_join(root, "versions"), jar_ver), jf.c_str());
    cp.append_s(path_str(main_jar).c_str());
    return cp;
}

//game launcherf

inline bool launch_version(const WStr& root, const Config& cfg, const char* version) {
    Str vjname; vjname.assign_s(version); vjname.append_s(".json");
    WStr vj_path = path_join(path_join(path_join(root, "versions"), version), vjname.c_str());
    if (!path_exists(vj_path)) { fprintf(stderr, "not installed: %s\n", version); return false; }

    JVal vj = parse_json(read_file(vj_path));
    JVal parent_vj;
    Str  base_ver; base_ver.assign_s(version);

    if (vj.has("inheritsFrom")) {
        base_ver.assign_s(vj["inheritsFrom"].str());
        Str pvjname; pvjname.copy_from(base_ver); pvjname.append_s(".json");
        WStr pj_path = path_join(path_join(path_join(root, "versions"), base_ver.c_str()), pvjname.c_str());
        if (!path_exists(pj_path)) {
            fprintf(stderr, "base version '%s' not installed.\n", base_ver.c_str()); return false;
        }
        parent_vj = parse_json(read_file(pj_path));
    }

    bool        has_parent = !parent_vj.is_null();
    const JVal& base_vj   = has_parent ? parent_vj : vj;

    Str cp       = build_classpath(root, vj, parent_vj, base_ver.c_str());
    Str uuid     = offline_uuid(cfg.username);
    Str nat_path = path_str(path_join(path_join(path_join(root, "versions"), base_ver.c_str()), "natives"));
    Str assets   = path_str(path_join(root, "assets"));
    Str game_dir = path_str(root);

    const char* asset_idx = base_vj["assetIndex"]["id"].str();
    const char* ver_type  = vj.has("type")      ? vj["type"].str()      :
                            base_vj.has("type") ? base_vj["type"].str() : "release";
    const char* main_cls  = vj["mainClass"].str();
    if (!main_cls || !*main_cls) main_cls = base_vj["mainClass"].str();
    if (!main_cls || !*main_cls) main_cls = "net.minecraft.client.main.Main";

    VarMap vars;
    vars.set("auth_player_name",  cfg.username.c_str());
    vars.set("auth_uuid",         uuid.c_str());
    vars.set("auth_access_token", "0");
    vars.set("user_type",         "legacy");
    vars.set("user_properties",   "{}");
    vars.set("version_name",      version);
    vars.set("version_type",      ver_type);
    vars.set("game_directory",    game_dir.c_str());
    vars.set("assets_root",       assets.c_str());
    vars.set("game_assets",       assets.c_str());
    vars.set("assets_index_name", asset_idx ? asset_idx : "");
    vars.set("natives_directory", nat_path.c_str());
    vars.set("classpath",         cp.c_str());
    vars.set("launcher_name",     "MCPP");
    vars.set("launcher_version",  "1.0");

    Vec<Str> args;
    args.reserve(48);

    char ram_buf[32];
    snprintf(ram_buf, sizeof(ram_buf), "-Xmx%dG", cfg.ram_gb);
    { Str a; a.assign_s(ram_buf);    args.push_back(std::move(a)); }
    { Str a; a.assign_s("-Xms512m"); args.push_back(std::move(a)); }

    // jvm args
    if (!cfg.java_args.empty()) {
        const char* p = cfg.java_args.c_str();
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p) break;
            const char* start = p;
            while (*p && *p != ' ' && *p != '\t') ++p;
            Str tok; tok.append(start, p - start);
            args.push_back(std::move(tok));
        }
    }

    int jdk = required_jdk(base_ver.c_str());
    if (jdk <= 8) {
        const char* cms[] = {
            "-XX:+UseConcMarkSweepGC",
            "-XX:+CMSIncrementalMode",
            "-XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe_minecraft.exe.heapdump"
        };
        for (auto& a : cms) { Str s; s.assign_s(a); args.push_back(std::move(s)); }
    } else if (jdk < 25) {
        const char* g1[] = {
            "-XX:+UseG1GC", "-XX:+UnlockExperimentalVMOptions",
            "-XX:G1NewSizePercent=20",  "-XX:G1ReservePercent=20",
            "-XX:MaxGCPauseMillis=50",  "-XX:G1HeapRegionSize=32M"
        };
        for (auto& a : g1) { Str s; s.assign_s(a); args.push_back(std::move(s)); }
    } else {
        const char* zgc[] = {
            "-XX:+UseZGC",
            "-XX:+ZGenerational",          
            "-XX:+UseCompactObjectHeaders", 
            "-XX:+AlwaysPreTouch",     
            "-XX:+UseStringDeduplication"
        };
        for (auto& a : zgc) { Str s; s.assign_s(a); args.push_back(std::move(s)); }
    }


    auto collect_args = [&](const JVal& src, const char* which) {
        if (!src.has("arguments") || !src["arguments"].has(which)) return;
        const JVal& arr = src["arguments"][which];
        for (size_t i = 0; i < arr.arr_n; ++i) {
            const JVal& e = arr.arr[i];
            if (e.is_string()) {
                args.push_back(tok_replace(e.str(), strlen(e.str()), vars)); continue;
            }
            if (!e.is_object()) continue;
            bool ok = true;
            if (e.has("rules")) {
                ok = false;
                for (size_t ri = 0; ri < e["rules"].arr_n; ++ri) {
                    const JVal& rule = e["rules"].arr[ri];
                    bool match = !rule.has("os") || !strcmp(rule["os"]["name"].str(), "windows");
                    if (rule.has("features")) match = false;
                    if (match) ok = !strcmp(rule["action"].str(), "allow");
                }
            }
            if (!ok) continue;
            const JVal& val = e["value"];
            if (val.is_string()) {
                args.push_back(tok_replace(val.str(), strlen(val.str()), vars));
            } else if (val.is_array()) {
                for (size_t vi = 0; vi < val.arr_n; ++vi)
                    args.push_back(tok_replace(val.arr[vi].str(), strlen(val.arr[vi].str()), vars));
            }
        }
    };

    Str main_cls_str; main_cls_str.assign_s(main_cls);

    if (base_vj.has("arguments")) {
        collect_args(base_vj, "jvm");
        if (has_parent) collect_args(vj, "jvm");
        args.push_back(std::move(main_cls_str));
        collect_args(base_vj, "game");
    } else {
        { Str a; a.assign_s("-Djava.library.path=");    a.append(nat_path.p, nat_path.n); args.push_back(std::move(a)); }
        { Str a; a.assign_s("-Dorg.lwjgl.librarypath="); a.append(nat_path.p, nat_path.n); args.push_back(std::move(a)); }
        { Str a; a.assign_s("-Dfile.encoding=UTF-8"); args.push_back(std::move(a)); }
        { Str a; a.assign_s("-cp"); args.push_back(std::move(a)); }
        args.push_back(std::move(cp));
        args.push_back(std::move(main_cls_str));
        const char* mc_args = base_vj["minecraftArguments"].str();
        const char* p = mc_args;
        while (p && *p) {
            while (*p == ' ') ++p;
            if (!*p) break;
            const char* start = p;
            while (*p && *p != ' ') ++p;
            Str tok; tok.append(start, p - start);
            args.push_back(tok_replace(tok.c_str(), tok.n, vars));
        }
    }

    Str java_exec; java_exec.copy_from(cfg.java_path);
    if (!cfg.show_console) {
        size_t pos = java_exec.find_s("java.exe");
        if (pos != NPOS) java_exec.replace_range(pos, 8, "javaw.exe");
        else {
            size_t len = java_exec.n;
            if (len >= 4 && !memcmp(java_exec.p + len - 4, "java", 4) &&
                (len == 4 || java_exec.p[len - 5] == '\\' || java_exec.p[len - 5] == '/'))
                java_exec.append_c('w');
        }
    } else {
        size_t pos = java_exec.find_s("javaw.exe");
        if (pos != NPOS) java_exec.replace_range(pos, 9, "java.exe");
    }

    Str qexe = win_quote(java_exec);
    Str cmd;  cmd.append(qexe.p, qexe.n);
    for (size_t i = 0; i < args.n; ++i) {
        cmd.append_c(' ');
        Str qa = win_quote(args.p[i]);
        cmd.append(qa.p, qa.n);
    }

    printf("\nlaunching minecraft %s as %s...\n[cmd] %s\n\n",
           version, cfg.username.c_str(), cmd.c_str());

    STARTUPINFOW       si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    WStr wcmd  = to_wide(cmd.c_str()); wcmd.append_c(L'\0');
    WStr wgame = to_wide(game_dir.c_str());

    if (!cfg.show_console) {
        si.dwFlags    |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    if (!CreateProcessW(nullptr, wcmd.p, nullptr, nullptr, FALSE,
                        CREATE_NEW_CONSOLE, nullptr, wgame.c_str(), &si, &pi)) {
        fprintf(stderr, "createprocess failed: %lu\n", GetLastError()); return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

//installed version scanner

inline Vec<Str> installed_versions(const WStr& root) {
    Vec<Str> v;
    WStr ver_dir = path_join(root, "versions");
    if (!path_exists(ver_dir)) return v;

    wchar_t pattern[4096];
    swprintf(pattern, 4096, L"%ls\\*", ver_dir.c_str());
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return v;

    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..")) continue;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

        Str name = to_utf8(fd.cFileName);
        WStr entry_dir = path_join(ver_dir, name.c_str());

        Str json_n; json_n.copy_from(name); json_n.append_s(".json");
        WStr json_p = path_join(entry_dir, json_n.c_str());
        if (!path_exists(json_p)) continue;

        Str jar_n; jar_n.copy_from(name); jar_n.append_s(".jar");
        WStr jar = path_join(entry_dir, jar_n.c_str());
        if (path_exists(jar) && path_size(jar) > 1024) { v.push_back(std::move(name)); continue; }

        Str js = read_file(json_p);
        if (js.empty()) continue;
        JVal jv = parse_json(js);
        if (jv.has("inheritsFrom")) {
            const char* base = jv["inheritsFrom"].str();
            Str bjar; bjar.assign_s(base); bjar.append_s(".jar");
            WStr base_jar = path_join(path_join(ver_dir, base), bjar.c_str());
            if (path_exists(base_jar) && path_size(base_jar) > 1024)
                v.push_back(std::move(name));
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);

    if (v.n > 1) {
        qsort(v.p, v.n, sizeof(Str), [](const void* a, const void* b) {
            return strcmp(((Str*)a)->c_str(), ((Str*)b)->c_str());
        });
    }
    return v;
}


inline int g_theme = 7;

inline void apply_theme() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)(g_theme & 0x0F));
}

inline void print_header(const char* title) {
    apply_theme();
    printf("\n------------------------------------------------\n  %s\n------------------------------------------------\n", title);
}

inline void init_console() {
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) return;

    // transparency
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, 229, LWA_ALPHA);

    // buffer and window size
    HANDLE     hOut  = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD      coord = { 100, 500 };
    SMALL_RECT rect  = { 0, 0, 99, 49 };
    SetConsoleScreenBufferSize(hOut, coord);
    SetConsoleWindowInfo(hOut, TRUE, &rect);
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_HSCROLL);

    // font
    CONSOLE_FONT_INFOEX cfi{};
    cfi.cbSize     = sizeof(cfi);
    cfi.dwFontSize = { 8, 16 };
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);

    ShowWindow(hwnd, SW_MAXIMIZE);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

//ui selections..

inline void section_download(const WStr& root, Config& cfg, const WStr& cfg_path) {
    print_header("DOWNLOAD");

    fputs("\nLOADER TYPE:\n  [1] Vanilla\n  [2] Fabric\nchoice: ", stdout);
    Str loader_choice = read_line();
    bool use_fabric = loader_choice.eq("2");

    struct VE { Str id, type; };
    Vec<VE> entries;
    JVal    manifest;

    if (use_fabric) {
        fputs("Fetching fabric supported versions...\n", stdout);
        Str fu; fu.assign_s(URL_FABRIC_META); fu.append_s("game");
        Str fv_str = http_get(fu);
        if (fv_str.empty()) {
            fputs("failed to fetch fabric game versions.\npress enter to continue...", stdout);
            getchar(); return;
        }
        JVal fv = parse_json(fv_str);
        if (!fv.is_array()) {
            fputs("unexpected fabric version response.\npress enter to continue...", stdout);
            getchar(); return;
        }
        entries.reserve(fv.arr_n);
        for (size_t i = 0; i < fv.arr_n; ++i) {
            VE e;
            e.id.assign_s(fv.arr[i]["version"].str());
            e.type.assign_s(fv.arr[i]["stable"].bval ? "release" : "snapshot");
            entries.push_back(std::move(e));
        }
        fputs("Fetching mojang manifest (needed for base download)...\n", stdout);
        Str mu; mu.assign_s(URL_MANIFEST);
        Str ms = http_get(mu);
        if (!ms.empty()) manifest = parse_json(ms);
    } else {
        fputs("Fetching version manifest...\n", stdout);
        Str mu; mu.assign_s(URL_MANIFEST);
        Str ms = http_get(mu);
        if (ms.empty()) {
            fputs("failed to fetch manifest.\npress enter to continue...", stdout);
            getchar(); return;
        }
        manifest = parse_json(ms);
        entries.reserve(manifest["versions"].arr_n);
        for (size_t i = 0; i < manifest["versions"].arr_n; ++i) {
            VE e;
            e.id.assign_s(manifest["versions"].arr[i]["id"].str());
            e.type.assign_s(manifest["versions"].arr[i]["type"].str());
            entries.push_back(std::move(e));
        }
    }

    fputs("\nfilter: (1) Releases only  (2) All Versions\nchoice: ", stdout);
    Str fin = read_line();
    bool releases_only = !fin.eq("2");

    Vec<VE> filtered;
    filtered.reserve(entries.n);
    for (size_t i = 0; i < entries.n; ++i) {
        if (!releases_only || entries.p[i].type.eq("release")) {
            VE e; e.id.copy_from(entries.p[i].id); e.type.copy_from(entries.p[i].type);
            filtered.push_back(std::move(e));
        }
    }

    int page = 0;
    const int PAGE = 20;
    for (;;) {
        int pages = ((int)filtered.n + PAGE - 1) / PAGE;
        int start = page * PAGE;
        int end   = (start + PAGE < (int)filtered.n) ? start + PAGE : (int)filtered.n;

        printf("\nversions (page %d/%d):\n", page + 1, pages);
        for (int i = start; i < end; ++i) {
            printf("  [%d] %s", i - start + 1, filtered.p[i].id.c_str());
            if (!filtered.p[i].type.eq("release")) printf(" (%s)", filtered.p[i].type.c_str());
            putchar('\n');
        }
        fputs("\nenter number, 'n' next, 'p' prev, 'q' cancel: ", stdout);
        Str input = read_line();

        if (input.eq("q") || input.eq("Q")) return;
        if (input.eq("n") || input.eq("N")) { if (page + 1 < pages) ++page; continue; }
        if (input.eq("p") || input.eq("P")) { if (page > 0) --page; continue; }

        int idx = -1;
        if (!parse_int(input, &idx)) { fputs("invalid input.\n", stdout); continue; }
        idx = idx - 1 + start;
        if (idx < 0 || idx >= (int)filtered.n) { fputs("invalid selection.\n", stdout); continue; }

        const char* chosen = filtered.p[idx].id.c_str();

        if (use_fabric) {
            printf("\ndownload fabric for minecraft %s? (y/n): ", chosen);
            Str ans = read_line();
            if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return;
            if (!install_bundled_jre(root, cfg, cfg_path, chosen))
                fputs("continuing without bundled jre.\n", stdout);
            if (manifest.is_null()) {
                fputs("mojang manifest unavailable; cannot download base mc.\n", stderr);
            } else if (!install_fabric(root, chosen, manifest)) {
                fputs("\nfabric download failed.\n", stderr);
            } else {
                printf("\nfabric for minecraft %s is ready.\n", chosen);
            }
        } else {
            Str jar_name; jar_name.assign_s(chosen); jar_name.append_s(".jar");
            WStr jar = path_join(path_join(path_join(root, "versions"), chosen), jar_name.c_str());
            if (path_exists(jar) && path_size(jar) > 1024) {
                printf("\nversion %s is already installed.\n", chosen);
            } else {
                printf("\ndownload minecraft %s? (y/n): ", chosen);
                Str ans = read_line();
                if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return;
                if (!install_bundled_jre(root, cfg, cfg_path, chosen))
                    fputs("continuing without bundled jre.\n", stdout);
                fputs("\n[1/5] manifest already fetched.\n", stdout);
                if (!install_mc_base(root, chosen, manifest))
                    fputs("\ndownload failed.\n", stderr);
                else
                    printf("\ndownload complete! %s is ready.\n", chosen);
            }
        }
        fputs("Press enter to continue...", stdout); getchar();
    }
}

inline void section_settings(Config& cfg, const WStr& cfg_path) {
    for (;;) {
        print_header("SETTINGS");
        printf("  [1] Username      : %s\n"
               "  [2] RAM (GB)      : %d\n"
               "  [3] Java Path     : %s\n"
               "  [4] Java Args     : %s\n"
               "  [5] Hide Launcher : %s\n"
               "  [6] Show Console  : %s\n"
               "  [7] Back\n\nchoice: ",
               cfg.username.c_str(), cfg.ram_gb,
               cfg.java_path.c_str(),
               cfg.java_args.empty() ? "(none)" : cfg.java_args.c_str(),
               cfg.hide_launcher ? "ON" : "OFF",
               cfg.show_console  ? "ON" : "OFF");

        Str input = read_line();
        if (input.eq("1")) {
            printf("New Username [%s]: ", cfg.username.c_str());
            Str val = read_line();
            if (!val.empty()) cfg.username = std::move(val);
        } else if (input.eq("2")) {
            printf("RAM in GB [%d]: ", cfg.ram_gb);
            Str val = read_line();
            int gb = 0;
            if (parse_int(val, &gb) && gb >= 1 && gb <= 64) cfg.ram_gb = gb;
            else fputs("invalid. must be 1-64.\n", stdout);
        } else if (input.eq("3")) {
            printf("Java Path [%s]: ", cfg.java_path.c_str());
            Str val = read_line();
            if (!val.empty()) {
                if (!check_java(val)) fputs("warning: could not verify java at that path.\n", stdout);
                cfg.java_path = std::move(val);
            }
        } else if (input.eq("4")) {
            printf("Java Args [%s]: ", cfg.java_args.empty() ? "none" : cfg.java_args.c_str());
            Str val = read_line();
            cfg.java_args = std::move(val);
        } else if (input.eq("5")) {
            cfg.hide_launcher = !cfg.hide_launcher;
        } else if (input.eq("6")) {
            cfg.show_console = !cfg.show_console;
        } else if (input.eq("7") || input.eq("q") || input.eq("Q")) {
            break;
        }
        save_config(cfg, cfg_path);
        fputs("settings saved.\n", stdout);
    }
}

inline void section_themes(Config& cfg, const WStr& cfg_path) {
    struct ThemeEntry { const char* name; int color; };
    constexpr ThemeEntry themes[] = {
        { "White",           15 },
        { "Cyan",            11 },
        { "Green",           10 },
        { "Yellow",          14 },
        { "Red",             12 },
        { "Magenta",         13 },
        { "Blue",             9 },
        { "Gray",             8 },
    };
    constexpr int NUM_THEMES = (int)(sizeof(themes) / sizeof(themes[0]));

    for (;;) {
        print_header("THEMES");
        fputs("  select a text color:\n\n", stdout);
        for (int i = 0; i < NUM_THEMES; ++i) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)themes[i].color);
            printf("  [%d] %s%s\n", i + 1, themes[i].name, cfg.theme_color == themes[i].color ? " *" : "");
        }
        apply_theme();
        fputs("\nchoice (or 'q' to go back): ", stdout);

        Str input = read_line();
        if (input.eq("q") || input.eq("Q")) break;

        int idx = -1;
        if (!parse_int(input, &idx)) { fputs("invalid input.\n", stdout); continue; }
        idx -= 1;
        if (idx < 0 || idx >= NUM_THEMES) { fputs("invalid selection.\n", stdout); continue; }
        cfg.theme_color = themes[idx].color;
        g_theme         = cfg.theme_color;
        apply_theme();
        save_config(cfg, cfg_path);
        fputs("theme applied.\n", stdout);
    }
}

inline void section_launch(const WStr& root, Config& cfg, const WStr& cfg_path) {
    print_header("LAUNCH");
    Vec<Str> versions = installed_versions(root);
    if (versions.empty()) {
        fputs("\nno installed versions. go to download first.\npress enter to continue...", stdout);
        getchar(); return;
    }

    fputs("\nInstalled versions:\n", stdout);
    for (size_t i = 0; i < versions.n; ++i)
        printf("  [%zu] %s\n", i + 1, versions.p[i].c_str());
    fputs("\nselect version (or 'q' to cancel): ", stdout);

    Str input = read_line();
    if (input.eq("q") || input.eq("Q")) return;

    int idx = -1;
    if (!parse_int(input, &idx)) {
        fputs("invalid input.\npress enter to continue...", stdout);
        getchar(); return;
    }
    idx -= 1;
    if (idx < 0 || idx >= (int)versions.n) {
        fputs("invalid selection.\npress enter to continue...", stdout);
        getchar(); return;
    }

    const char* chosen = versions.p[idx].c_str();

    Str base_ver; base_ver.assign_s(chosen);
    {
        Str vjname; vjname.assign_s(chosen); vjname.append_s(".json");
        WStr vj_path = path_join(path_join(path_join(root, "versions"), chosen), vjname.c_str());
        if (path_exists(vj_path)) {
            JVal jv = parse_json(read_file(vj_path));
            if (jv.has("inheritsFrom")) base_ver.assign_s(jv["inheritsFrom"].str());
        }
    }

    if (!check_java(cfg.java_path)) {
        printf("\njava not found at: %s\nlocating bundled jre...\n", cfg.java_path.c_str());
    Str base_ver; base_ver.assign_s(chosen);
    {
        Str vjname; vjname.assign_s(chosen); vjname.append_s(".json");
        WStr vj_path = path_join(path_join(path_join(root, "versions"), chosen), vjname.c_str());
        if (path_exists(vj_path)) {
            JVal jv = parse_json(read_file(vj_path));
            if (jv.has("inheritsFrom")) base_ver.assign_s(jv["inheritsFrom"].str());
        }
    }
        if (!install_bundled_jre(root, cfg, cfg_path, base_ver.c_str())) {
            fputs("java unavailable. set java path in settings.\npress enter to continue...", stderr);
            getchar(); return;
        }
    }
    if (!launch_version(root, cfg, chosen)) {
        fputs("press enter to continue...", stdout); getchar();
    } else {
        fputs("game launched! exiting launcher...\n", stdout);
        Sleep(1500);
        if (cfg.hide_launcher) exit(0);
    }
}

//entry points

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleW(L"MC++ by TryFast");
    init_console();
    InitializeCriticalSection(&g_mkdir_cs);

    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    size_t elen = wcslen(exe);
    while (elen && exe[elen - 1] != L'\\') --elen;
    if (elen) exe[--elen] = 0;
    WStr root; root.assign_w(exe);

    WStr cfg_path = path_join(root, "config.json");
    Config cfg = load_config(cfg_path);
    g_theme = cfg.theme_color;
    apply_theme();

    if (cfg.username.empty() || cfg.username.eq("Player")) {
        fputs("---- MC++ by TryFast ----\n\nEnter your name: ", stdout);
        Str uname = read_line();
        if (!uname.empty()) cfg.username = std::move(uname);
        if (cfg.username.empty()) cfg.username.assign_s("Player");
        save_config(cfg, cfg_path);
    }

    for (;;) {
        print_header("MC++ by TryFast");
        fputs("  [1] Launch\n  [2] Download\n  [3] Settings\n  [4] Themes\n  [5] Exit\n\nchoice: ", stdout);
        Str input = read_line();
        if      (input.eq("1")) section_launch(root, cfg, cfg_path);
        else if (input.eq("2")) section_download(root, cfg, cfg_path);
        else if (input.eq("3")) section_settings(cfg, cfg_path);
        else if (input.eq("4")) section_themes(cfg, cfg_path);
        else if (input.eq("5") || input.eq("q") || input.eq("Q")) break;
    }

    DeleteCriticalSection(&g_mkdir_cs);
    return 0;
}