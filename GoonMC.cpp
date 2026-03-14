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

inline constexpr size_t NPOS = static_cast<size_t>(-1);

struct Str {
    char*  p;
    size_t n, cap;
    Str() : p(nullptr), n(0), cap(0) {}
    Str(const Str& o) : p(nullptr), n(0), cap(0) {
        if (o.n) { p = (char*)malloc(o.n + 1); memcpy(p, o.p, o.n + 1); n = o.n; cap = o.n; }
    }
    Str(Str&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = 0; o.cap = 0; }
    ~Str() { free(p); }
    Str& operator=(const Str& o) {
        if (this != &o) { free(p); p = nullptr; n = 0; cap = 0; if (o.n) { p = (char*)malloc(o.n + 1); memcpy(p, o.p, o.n + 1); n = o.n; cap = o.n; } }
        return *this;
    }
    Str& operator=(Str&& o) noexcept {
        if (this != &o) { free(p); p = o.p; n = o.n; cap = o.cap; o.p = nullptr; o.n = 0; o.cap = 0; }
        return *this;
    }
    void grow(size_t need) {
        if (n + need <= cap) return;
        size_t nc = cap ? cap * 2 : 16;
        if (nc < n + need) nc = n + need;
        p = (char*)realloc(p, nc + 1);
        cap = nc;
    }
    void append(const char* s, size_t l) {
        if (!l) return; grow(l);
        memcpy(p + n, s, l); n += l; p[n] = 0;
    }
    void append_c(char c)        { grow(1); p[n++] = c; p[n] = 0; }
    void append_s(const char* s) { if (s && *s) append(s, strlen(s)); }
    void assign(const char* s, size_t l) { n = 0; append(s, l); }
    void assign_s(const char* s)         { n = 0; append_s(s); }
    void copy_from(const Str& o)         { n = 0; if (o.p && o.n) append(o.p, o.n); }
    const char* c_str() const { return p ? p : ""; }
    size_t size()  const { return n; }
    bool   empty() const { return n == 0; }
    char   back()  const { return n ? p[n-1] : 0; }
    void   pop_back()    { if (n) { --n; p[n] = 0; } }
    char  operator[](size_t i) const { return p[i]; }
    char& operator[](size_t i)       { return p[i]; }
    bool eq(const char* s) const {
        size_t l = strlen(s);
        return l == n && (!n || !memcmp(p, s, n));
    }
    bool eq_n(const char* s, size_t l) const {
        return l == n && (!n || !memcmp(p, s, n));
    }
    size_t find(char c, size_t from = 0) const {
        for (size_t i = from; i < n; ++i) if (p[i] == c) return i;
        return NPOS;
    }
    size_t find_s(const char* s) const {
        size_t sl = strlen(s);
        if (sl > n) return NPOS;
        for (size_t i = 0; i <= n - sl; ++i) if (!memcmp(p + i, s, sl)) return i;
        return NPOS;
    }
    bool contains(const char* s) const { return find_s(s) != NPOS; }
    Str substr(size_t from, size_t len = NPOS) const {
        Str r{};
        if (from >= n) return r;
        if (len == NPOS || from + len > n) len = n - from;
        r.append(p + from, len);
        return r;
    }
    void replace_range(size_t pos, size_t rlen, const char* rep) {
        size_t rl = strlen(rep);
        Str t{};
        if (pos) t.append(p, pos);
        t.append(rep, rl);
        if (pos + rlen <= n) t.append(p + pos + rlen, n - pos - rlen);
        *this = std::move(t);
    }
    void to_lower() { for (size_t i = 0; i < n; ++i) p[i] = (char)tolower((uint8_t)p[i]); }
    bool ends_with(const char* s) const {
        size_t l = strlen(s);
        return n >= l && !memcmp(p + n - l, s, l);
    }
};

struct WStr {
    wchar_t* p;
    size_t   n, cap;
    WStr() : p(nullptr), n(0), cap(0) {}
    WStr(const WStr& o) : p(nullptr), n(0), cap(0) {
        if (o.n) { p = (wchar_t*)malloc((o.n + 1)*sizeof(wchar_t)); memcpy(p, o.p, (o.n + 1)*sizeof(wchar_t)); n = o.n; cap = o.n; }
    }
    WStr(WStr&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = 0; o.cap = 0; }
    ~WStr() { free(p); }
    WStr& operator=(const WStr& o) {
        if (this != &o) { free(p); p = nullptr; n = 0; cap = 0; if (o.n) { p = (wchar_t*)malloc((o.n + 1)*sizeof(wchar_t)); memcpy(p, o.p, (o.n + 1)*sizeof(wchar_t)); n = o.n; cap = o.n; } }
        return *this;
    }
    WStr& operator=(WStr&& o) noexcept {
        if (this != &o) { free(p); p = o.p; n = o.n; cap = o.cap; o.p = nullptr; o.n = 0; o.cap = 0; }
        return *this;
    }
    void grow(size_t need) {
        if (n + need <= cap) return;
        size_t nc = cap ? cap * 2 : 16;
        if (nc < n + need) nc = n + need;
        p = (wchar_t*)realloc(p, (nc + 1) * sizeof(wchar_t));
        cap = nc;
    }
    void append(const wchar_t* s, size_t l) {
        if (!l) return; grow(l);
        memcpy(p + n, s, l * sizeof(wchar_t)); n += l; p[n] = 0;
    }
    void append_c(wchar_t c)        { grow(1); p[n++] = c; p[n] = 0; }
    void append_w(const wchar_t* s) { if (s && *s) append(s, wcslen(s)); }
    void assign_w(const wchar_t* s) { n = 0; append_w(s); }
    void copy_from(const WStr& o)   { n = 0; if (o.p && o.n) append(o.p, o.n); }
    const wchar_t* c_str() const { return p ? p : L""; }
    size_t size()  const { return n; }
    bool   empty() const { return n == 0; }
    wchar_t back() const { return n ? p[n-1] : 0; }
    void pop_back()      { if (n) { --n; p[n] = 0; } }
    size_t find(wchar_t c, size_t from = 0) const {
        for (size_t i = from; i < n; ++i) if (p[i] == c) return i;
        return NPOS;
    }
};

template<typename T>
struct Vec {
    T*     p;
    size_t n, cap;
    Vec() : p(nullptr), n(0), cap(0) {}
    Vec(Vec&& o) noexcept : p(o.p), n(o.n), cap(o.cap) { o.p = nullptr; o.n = 0; o.cap = 0; }
    ~Vec() {
        for (size_t i = 0; i < n; ++i) p[i].~T();
        free(p);
    }
    Vec& operator=(Vec&& o) noexcept {
        if (this != &o) {
            for (size_t i = 0; i < n; ++i) p[i].~T();
            free(p);
            p = o.p; n = o.n; cap = o.cap;
            o.p = nullptr; o.n = 0; o.cap = 0;
        }
        return *this;
    }
    void reserve(size_t c) {
        if (c <= cap) return;
        size_t nc = cap ? cap * 2 : 4;
        if (nc < c) nc = c;
        p = (T*)realloc(p, nc * sizeof(T));
        cap = nc;
    }
    void push_back(const T& v) { reserve(n + 1); new (&p[n]) T(v);            ++n; }
    void push_back(T&& v)      { reserve(n + 1); new (&p[n]) T(std::move(v)); ++n; }
    T& back()                       { return p[n-1]; }
    const T& back() const           { return p[n-1]; }
    T& operator[](size_t i)         { return p[i]; }
    const T& operator[](size_t i) const { return p[i]; }
    size_t size()  const { return n; }
    bool   empty() const { return n == 0; }
    void   clear()       { for (size_t i = 0; i < n; ++i) p[i].~T(); n = 0; }
    void   pop_back()    { if (n) { --n; p[n].~T(); } }
};

struct JVal {
    enum Type : uint8_t { Null_, Bool_, Num_, Str_, Arr_, Obj_ } type;
    bool   bval;
    double nval;
    Str    sval;
    JVal*  arr;      size_t arr_n, arr_cap;
    Str*   obj_keys; JVal*  obj_vals; size_t obj_n, obj_cap;

    JVal() : type(Null_), bval(false), nval(0), arr(nullptr), arr_n(0), arr_cap(0), obj_keys(nullptr), obj_vals(nullptr), obj_n(0), obj_cap(0) {}
    JVal(JVal&& o) noexcept : type(o.type), bval(o.bval), nval(o.nval), sval(std::move(o.sval)),
                              arr(o.arr), arr_n(o.arr_n), arr_cap(o.arr_cap),
                              obj_keys(o.obj_keys), obj_vals(o.obj_vals), obj_n(o.obj_n), obj_cap(o.obj_cap) {
        o.type = Null_; o.arr = nullptr; o.obj_keys = nullptr; o.obj_vals = nullptr;
        o.arr_n = 0; o.arr_cap = 0; o.obj_n = 0; o.obj_cap = 0;
    }
    ~JVal() {
        if (type == Arr_) {
            for (size_t i = 0; i < arr_n; ++i) arr[i].~JVal();
            free(arr);
        } else if (type == Obj_) {
            for (size_t i = 0; i < obj_n; ++i) {
                obj_keys[i].~Str();
                obj_vals[i].~JVal();
            }
            free(obj_keys);
            free(obj_vals);
        }
    }
    JVal& operator=(JVal&& o) noexcept {
        if (this != &o) {
            this->~JVal();
            type = o.type; bval = o.bval; nval = o.nval; sval = std::move(o.sval);
            arr = o.arr; arr_n = o.arr_n; arr_cap = o.arr_cap;
            obj_keys = o.obj_keys; obj_vals = o.obj_vals; obj_n = o.obj_n; obj_cap = o.obj_cap;
            o.type = Null_; o.arr = nullptr; o.obj_keys = nullptr; o.obj_vals = nullptr;
            o.arr_n = 0; o.arr_cap = 0; o.obj_n = 0; o.obj_cap = 0;
        }
        return *this;
    }

    bool is_null()   const { return type == Null_; }
    bool is_string() const { return type == Str_; }
    bool is_array()  const { return type == Arr_; }
    bool is_object() const { return type == Obj_; }
    size_t size()    const { return type == Arr_ ? arr_n : obj_n; }
    const char* str() const { return sval.c_str(); }
    double      num() const { return nval; }

    bool has(const char* k) const {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_keys[i].eq_n(k, kl)) return true;
        return false;
    }

    static JVal& null_ref() { static JVal nv{}; return nv; }

    const JVal& operator[](const char* k) const {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_keys[i].eq_n(k, kl)) return obj_vals[i];
        return null_ref();
    }
    JVal& operator[](const char* k) {
        size_t kl = strlen(k);
        for (size_t i = 0; i < obj_n; ++i)
            if (obj_keys[i].eq_n(k, kl)) return obj_vals[i];
        if (obj_n >= obj_cap) {
            size_t nc = obj_cap ? obj_cap * 2 : 4;
            obj_keys = (Str*)  realloc(obj_keys, nc * sizeof(Str));
            obj_vals = (JVal*) realloc(obj_vals, nc * sizeof(JVal));
            for (size_t i = obj_cap; i < nc; ++i) {
                new (&obj_keys[i]) Str();
                new (&obj_vals[i]) JVal();
            }
            obj_cap = nc;
        }
        obj_keys[obj_n].append(k, kl);
        return obj_vals[obj_n++];
    }
    const JVal& operator[](size_t i) const { return arr[i]; }
    JVal&       operator[](size_t i)       { return arr[i]; }

    void push_arr_val(JVal v) {
        if (arr_n >= arr_cap) {
            size_t nc = arr_cap ? arr_cap * 2 : 4;
            arr = (JVal*)realloc(arr, nc * sizeof(JVal));
            for (size_t i = arr_cap; i < nc; ++i) new (&arr[i]) JVal();
            arr_cap = nc;
        }
        arr[arr_n++] = std::move(v);
    }
    void push_obj_kv(Str k, JVal v) {
        if (obj_n >= obj_cap) {
            size_t nc = obj_cap ? obj_cap * 2 : 4;
            obj_keys = (Str*)  realloc(obj_keys, nc * sizeof(Str));
            obj_vals = (JVal*) realloc(obj_vals, nc * sizeof(JVal));
            for (size_t i = obj_cap; i < nc; ++i) {
                new (&obj_keys[i]) Str();
                new (&obj_vals[i]) JVal();
            }
            obj_cap = nc;
        }
        obj_keys[obj_n] = std::move(k);
        obj_vals[obj_n] = std::move(v);
        ++obj_n;
    }
};

static inline void skip_ws(const char*& p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
}

static Str parse_str_tok(const char*& p) {
    ++p;
    Str s{};
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

static JVal parse_val(const char*& p);

static JVal parse_obj(const char*& p) {
    JVal v{}; v.type = JVal::Obj_;
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

static JVal parse_arr(const char*& p) {
    JVal v{}; v.type = JVal::Arr_;
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

static JVal parse_val(const char*& p) {
    skip_ws(p);
    switch (*p) {
        case '{': return parse_obj(p);
        case '[': return parse_arr(p);
        case '"': { JVal v{}; v.type = JVal::Str_; v.sval = parse_str_tok(p); return v; }
        case 't': if (!strncmp(p, "true",  4)) { JVal v{}; v.type = JVal::Bool_; v.bval = true;  p += 4; return v; } break;
        case 'f': if (!strncmp(p, "false", 5)) { JVal v{}; v.type = JVal::Bool_; v.bval = false; p += 5; return v; } break;
        case 'n': if (!strncmp(p, "null",  4)) { p += 4; return JVal{}; } break;
    }
    JVal v{}; v.type = JVal::Num_;
    char* end;
    v.nval = strtod(p, &end); p = end;
    return v;
}

static JVal parse_json(const Str& src) {
    const char* p = src.c_str();
    return parse_val(p);
}

static WStr to_wide_str(const char* s, int len = -1) {
    WStr r{};
    if (!s || !*s) return r;
    int n = MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
    if (n <= 0) return r;
    r.grow((size_t)n);
    MultiByteToWideChar(CP_UTF8, 0, s, len, r.p, n);
    r.n = (size_t)(len == -1 ? n - 1 : n);
    if (r.p) r.p[r.n] = 0;
    return r;
}

static Str to_utf8_str(const wchar_t* w, int len = -1) {
    Str r{};
    if (!w || !*w) return r;
    int n = WideCharToMultiByte(CP_UTF8, 0, w, len, nullptr, 0, nullptr, nullptr);
    if (n <= 0) return r;
    r.grow((size_t)n);
    WideCharToMultiByte(CP_UTF8, 0, w, len, r.p, n, nullptr, nullptr);
    r.n = (size_t)(len == -1 ? n - 1 : n);
    if (r.p) r.p[r.n] = 0;
    return r;
}

static WStr pjoin(const WStr& base, const char* comp) {
    WStr r{}; r.copy_from(base);
    if (r.n && r.back() != L'\\') r.append_c(L'\\');
    WStr w = to_wide_str(comp);
    if (w.p) r.append(w.p, w.n);
    return r;
}

static WStr pjoin_w(const WStr& base, const wchar_t* comp) {
    WStr r{}; r.copy_from(base);
    if (r.n && r.back() != L'\\') r.append_c(L'\\');
    r.append_w(comp);
    return r;
}

static bool path_exists(const WStr& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static bool path_exists_w(const wchar_t* path) {
    return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

static LONGLONG path_file_size(const WStr& path) {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &d)) return -1;
    LARGE_INTEGER li; li.HighPart = (LONG)d.nFileSizeHigh; li.LowPart = d.nFileSizeLow;
    return li.QuadPart;
}

static bool path_is_dir(const WStr& path) {
    DWORD a = GetFileAttributesW(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

static Str path_to_str(const WStr& path) {
    return to_utf8_str(path.c_str());
}

static WStr path_parent(const WStr& path) {
    WStr r{}; r.copy_from(path);
    while (r.n && (r.back() == L'\\' || r.back() == L'/')) r.pop_back();
    while (r.n && r.back() != L'\\' && r.back() != L'/') r.pop_back();
    if (r.n) r.pop_back();
    return r;
}

static void create_dirs(const WStr& path) {
    size_t len = path.n;
    wchar_t* tmp = (wchar_t*)malloc((len + 2) * sizeof(wchar_t));
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

static Str read_file(const WStr& path) {
    Str r{};
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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

static void write_file(const WStr& path, const char* data, size_t len) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD wr = 0;
    WriteFile(h, data, (DWORD)len, &wr, nullptr);
    CloseHandle(h);
}

static Str read_line() {
    Str r{};
    int c;
    while ((c = getchar()) != EOF && c != '\n')
        r.append_c((char)c);
    if (!r.empty() && r.back() == '\r') r.pop_back();
    return r;
}

static bool parse_int(const Str& s, int* out) {
    if (s.empty()) return false;
    size_t start = (s.p[0] == '-') ? 1 : 0;
    if (start >= s.n) return false;
    for (size_t i = start; i < s.n; ++i)
        if (!isdigit((uint8_t)s.p[i])) return false;
    *out = atoi(s.c_str());
    return true;
}

struct WSession {
    HINTERNET h = nullptr;
    WSession() {
        h = WinHttpOpen(L"GoonMC/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (h) {
            DWORD pol = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            WinHttpSetOption(h, WINHTTP_OPTION_REDIRECT_POLICY, &pol, sizeof(pol));
        }
    }
    ~WSession() { if (h) WinHttpCloseHandle(h); }
    WSession(const WSession&) = delete;
    WSession& operator=(const WSession&) = delete;
} g_sess;

struct CrackResult { WStr host, path; INTERNET_PORT port; bool https; };

static CrackResult crack_url(const WStr& url) {
    CrackResult r{};
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[512]{}, pth[4096]{};
    uc.lpszHostName = host; uc.dwHostNameLength = 512;
    uc.lpszUrlPath  = pth;  uc.dwUrlPathLength  = 4096;
    WinHttpCrackUrl(url.c_str(), 0, 0, &uc);
    r.host.assign_w(host);
    r.path.assign_w(pth);
    r.port  = uc.nPort;
    r.https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
    return r;
}

static HINTERNET open_req(const Str& url_s, HINTERNET& out_conn, int max_redir = 10) {
    if (!g_sess.h) return nullptr;
    Str cur{}; cur.copy_from(url_s);

    for (int i = 0; i <= max_redir; ++i) {
        WStr wurl = to_wide_str(cur.c_str());
        CrackResult pu = crack_url(wurl);

        HINTERNET hConn = WinHttpConnect(g_sess.h, pu.host.c_str(), pu.port, 0);
        if (!hConn) return nullptr;

        DWORD flags = pu.https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hReq = WinHttpOpenRequest(hConn, L"GET", pu.path.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hReq) { WinHttpCloseHandle(hConn); return nullptr; }

        if (pu.https) {
            DWORD sec = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                        SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                        SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
            WinHttpSetOption(hReq, WINHTTP_OPTION_SECURITY_FLAGS, &sec, sizeof(sec));
        }

        if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
            !WinHttpReceiveResponse(hReq, nullptr)) {
            WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn);
            return nullptr;
        }

        DWORD status = 0, sz = sizeof(status);
        WinHttpQueryHeaders(hReq,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);

        if (status == 301 || status == 302 || status == 303 ||
            status == 307 || status == 308) {
            DWORD loc_sz = 0;
            WinHttpQueryHeaders(hReq, WINHTTP_QUERY_LOCATION,
                WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &loc_sz, WINHTTP_NO_HEADER_INDEX);
            // Read Location header BEFORE closing the request handle.
            if (loc_sz > 0) {
                size_t wlen = loc_sz / sizeof(wchar_t) + 2;
                wchar_t* loc = (wchar_t*)malloc(wlen * sizeof(wchar_t));
                loc[0] = 0;
                WinHttpQueryHeaders(hReq, WINHTTP_QUERY_LOCATION,
                    WINHTTP_HEADER_NAME_BY_INDEX, loc, &loc_sz, WINHTTP_NO_HEADER_INDEX);
                size_t ln = wcslen(loc);
                while (ln && loc[ln-1] == 0) --ln;
                loc[ln] = 0;
                cur = to_utf8_str(loc);
                free(loc);
            }
            WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn);
            continue;
        }

        out_conn = hConn;
        return hReq;
    }
    return nullptr;
}

[[nodiscard]] static Str http_get_str(const Str& url) {
    HINTERNET hConn = nullptr;
    HINTERNET hReq  = open_req(url, hConn);
    Str result{};
    if (!hReq) return result;
    char buf[65536];
    DWORD rd = 0;
    while (WinHttpReadData(hReq, buf, sizeof(buf), &rd) && rd)
        result.append(buf, (size_t)rd);
    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn);
    return result;
}

inline CRITICAL_SECTION g_mkdir_cs;

static bool http_download(const Str& url, const WStr& dest) {
    HINTERNET hConn = nullptr;
    HINTERNET hReq  = open_req(url, hConn);
    if (!hReq) return false;

    {
        EnterCriticalSection(&g_mkdir_cs);
        WStr parent = path_parent(dest);
        if (!parent.empty()) create_dirs(parent);
        LeaveCriticalSection(&g_mkdir_cs);
    }

    HANDLE hFile = CreateFileW(dest.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn);
        return false;
    }

    bool ok = true;
    char buf[131072];
    DWORD rd = 0, wr = 0;
    while (WinHttpReadData(hReq, buf, sizeof(buf), &rd) && rd) {
        if (!WriteFile(hFile, buf, rd, &wr, nullptr) || wr != rd) { ok = false; break; }
    }

    CloseHandle(hFile);
    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn);
    if (!ok) DeleteFileW(dest.c_str());
    return ok;
}

static bool download_file(const Str& url, const WStr& dest) {
    if (path_exists(dest) && path_file_size(dest) > 0) return true;
    return http_download(url, dest);
}

struct DLTask { Str url; WStr dest; };

struct DLThreadCtx {
    DLTask*        tasks;
    volatile LONG* cursor;
    volatile LONG* ndone;
    LONG           total;
};

static DWORD WINAPI dl_worker(LPVOID arg) {
    DLThreadCtx* ctx = (DLThreadCtx*)arg;
    for (;;) {
        LONG i = InterlockedExchangeAdd(ctx->cursor, 1);
        if (i >= ctx->total) break;
        download_file(ctx->tasks[i].url, ctx->tasks[i].dest);
        InterlockedIncrement(ctx->ndone);
    }
    return 0;
}

static void parallel_dl(Vec<DLTask>& tasks, int nthreads = 16) {
    if (tasks.empty()) return;
    volatile LONG cursor = 0, ndone = 0;
    LONG total = (LONG)tasks.n;
    DLThreadCtx ctx = { tasks.p, &cursor, &ndone, total };

    int n = nthreads < (int)total ? nthreads : (int)total;
    Vec<HANDLE> pool{};
    pool.reserve((size_t)n);
    for (int t = 0; t < n; ++t)
        pool.push_back(CreateThread(nullptr, 0, dl_worker, &ctx, 0, nullptr));

    while (ndone < total) {
        printf("  %ld/%ld\r", ndone, total);
        fflush(stdout);
        Sleep(100);
    }
    WaitForMultipleObjects((DWORD)pool.n, pool.p, TRUE, INFINITE);
    for (size_t t = 0; t < pool.n; ++t) CloseHandle(pool.p[t]);
    printf("  %ld/%ld\n", total, total);
}

struct Config {
    Str username;
    Str java_path;
    Str java_args;
    int ram_gb;
    int theme_color;
    bool hide_launcher;
    bool show_console;
};

static Config make_default_config() {
    Config c{};
    c.username.assign_s("Player");
    c.java_path.assign_s("javaw");
    c.ram_gb = 2;
    c.theme_color = 7;
    c.hide_launcher = true;
    c.show_console = false;
    return c;
}

static Str esc_json(const Str& s) {
    Str r{};
    for (size_t i = 0; i < s.n; ++i) {
        if      (s.p[i] == '"')  { r.append_c('\\'); r.append_c('"'); }
        else if (s.p[i] == '\\') { r.append_c('\\'); r.append_c('\\'); }
        else r.append_c(s.p[i]);
    }
    return r;
}

static Config load_config(const WStr& path) {
    Config c = make_default_config();
    if (!path_exists(path)) return c;
    Str s = read_file(path);
    if (s.empty()) return c;
    JVal j = parse_json(s);
    if (j.has("username"))    c.username.assign_s(j["username"].str());
    if (j.has("java_path"))   c.java_path.assign_s(j["java_path"].str());
    if (j.has("java_args"))   c.java_args.assign_s(j["java_args"].str());
    if (j.has("ram_gb"))      c.ram_gb      = (int)j["ram_gb"].num();
    if (j.has("theme_color")) c.theme_color = (int)j["theme_color"].num();
    if (j.has("hide_launcher")) c.hide_launcher = j["hide_launcher"].bval;
    if (j.has("show_console"))  c.show_console  = j["show_console"].bval;
    if (c.ram_gb < 1) c.ram_gb = 1;
    return c;
}

static void save_config(const Config& c, const WStr& path) {
    Str eu = esc_json(c.username);
    Str ej = esc_json(c.java_path);
    Str ea = esc_json(c.java_args);
    char buf[1024];
    int n = snprintf(buf, sizeof(buf),
        "{\n  \"username\": \"%s\",\n  \"java_path\": \"%s\","
        "\n  \"java_args\": \"%s\",\n  \"ram_gb\": %d,\n  \"theme_color\": %d,"
        "\n  \"hide_launcher\": %s,\n  \"show_console\": %s\n}\n",
        eu.c_str(), ej.c_str(), ea.c_str(), c.ram_gb, c.theme_color,
        c.hide_launcher ? "true" : "false", c.show_console ? "true" : "false");
    if (n > 0) write_file(path, buf, (size_t)n);
}

inline int g_theme_color = 7;

static void apply_theme() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
        (WORD)(g_theme_color & 0x0F));
}

static bool check_java(const Str& java) {
    Str cmd{};
    cmd.append_c('"');
    cmd.append(java.p, java.n);
    cmd.append_s("\" -version > NUL 2>&1");
    return system(cmd.c_str()) == 0;
}

struct MCVer { int v[3]; };

static MCVer parse_mc_ver(const char* s) {
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

static int cmp_ver(const MCVer& a, const MCVer& b) {
    for (int i = 0; i < 3; ++i) {
        if (a.v[i] < b.v[i]) return -1;
        if (a.v[i] > b.v[i]) return  1;
    }
    return 0;
}

// Pre-computed version sentinels — initialized once at startup, never mutated.
inline const MCVer MC_VER_117  = parse_mc_ver("1.17");
inline const MCVer MC_VER_121  = parse_mc_ver("1.21");
inline const MCVer MC_VER_1205 = parse_mc_ver("1.20.5");

static int required_jdk(const char* mc) {
    MCVer v = parse_mc_ver(mc);
    if (cmp_ver(v, MC_VER_117) < 0) return 8;
    if (cmp_ver(v, MC_VER_121) < 0) return 17;
    return 21;
}

static const char* get_runtime_component(const char* mc) {
    MCVer v = parse_mc_ver(mc);
    if (cmp_ver(v, MC_VER_117)  < 0) return "jre-legacy";
    if (cmp_ver(v, MC_VER_1205) < 0) return "java-runtime-gamma";
    return "java-runtime-delta";
}

static Str make_offline_uuid(const Str& name) {
    Str seed{};
    seed.assign_s("OfflinePlayer:");
    seed.append(name.p, name.n);
    uint8_t h[16]{};
    for (size_t i = 0; i < seed.n; ++i) {
        h[i % 16]     ^= (uint8_t)((uint8_t)seed.p[i] * (uint8_t)(i + 1));
        h[(i+3) % 16] += (uint8_t)seed.p[i];
    }
    h[6] = (h[6] & 0x0f) | 0x30;
    h[8] = (h[8] & 0x3f) | 0x80;
    char buf[37];
    snprintf(buf, sizeof(buf),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        h[0],h[1],h[2],h[3],h[4],h[5],h[6],h[7],
        h[8],h[9],h[10],h[11],h[12],h[13],h[14],h[15]);
    Str r{};
    r.assign_s(buf);
    return r;
}

static bool lib_applies(const JVal& lib) {
    if (!lib.has("rules")) return true;
    bool allowed = false;
    for (size_t i = 0; i < lib["rules"].size(); ++i) {
        const JVal& rule = lib["rules"].arr[i];
        bool match = !rule.has("os") || !strcmp(rule["os"]["name"].str(), "windows");
        if (match) allowed = !strcmp(rule["action"].str(), "allow");
    }
    return allowed;
}

static void find_javaw_recursive(const wchar_t* dir, WStr* result) {
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
            find_javaw_recursive(full, result);
        } else {
            wchar_t lower[MAX_PATH];
            size_t fl = wcslen(fd.cFileName);
            for (size_t i = 0; i <= fl; ++i)
                lower[i] = (wchar_t)tolower((uint8_t)fd.cFileName[i]);
            if (!wcscmp(lower, L"javaw.exe"))
                result->assign_w(full);
        }
    } while (FindNextFileW(h, &fd) && result->empty());
    FindClose(h);
}

static WStr find_java_in_dir(const WStr& dir) {
    WStr result{};
    if (!dir.empty() && path_exists(dir))
        find_javaw_recursive(dir.c_str(), &result);
    return result;
}

static Str maven_path(const char* coords) {
    Str r{};
    const char* c1 = strchr(coords, ':');
    if (!c1) return r;
    const char* c2 = strchr(c1 + 1, ':');
    if (!c2) return r;

    Str group{};  group.append(coords, c1 - coords);
    Str artifact{}; artifact.append(c1 + 1, c2 - c1 - 1);
    Str ver{};    ver.assign_s(c2 + 1);

    Str classifier{};
    size_t cp = ver.find(':');
    if (cp != NPOS) {
        classifier = ver.substr(cp + 1);
        Str vt = ver.substr(0, cp);
        ver = vt;
    }

    for (size_t i = 0; i < group.n; ++i)
        if (group.p[i] == '.') group.p[i] = '/';

    Str fname{};
    fname.append(artifact.p, artifact.n);
    fname.append_c('-');
    fname.append(ver.p, ver.n);
    if (!classifier.empty()) { fname.append_c('-'); fname.append(classifier.p, classifier.n); }
    fname.append_s(".jar");

    r.append(group.p, group.n);
    r.append_c('/');
    r.append(artifact.p, artifact.n);
    r.append_c('/');
    r.append(ver.p, ver.n);
    r.append_c('/');
    r.append(fname.p, fname.n);
    return r;
}

static bool is_native_artifact_path(const char* p) {
    return strstr(p, "natives-windows") != nullptr;
}

static bool native_path_matches_arch(const char* p) {
    bool is64 = sizeof(void*) == 8;
    if (strstr(p, "natives-windows-arm64"))  return false;
    if (strstr(p, "natives-windows-x86_64")) return is64;
    if (strstr(p, "natives-windows-x86"))    return !is64;
    return true;
}

inline constexpr const char* MANIFEST_URL    = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
inline constexpr const char* RESOURCES_URL   = "https://resources.download.minecraft.net/";
inline constexpr const char* RUNTIME_ALL_URL =
    "https://launchermeta.mojang.com/v1/products/java-runtime/"
    "2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";
inline constexpr const char* FABRIC_META_BASE = "https://meta.fabricmc.net/v2/versions/";

static void download_libraries_to_tasks(const WStr& root, const JVal& vj,
                                         Vec<DLTask>& tasks) {
    if (!vj.has("libraries")) return;
    WStr lib_dir = pjoin(root, "libraries");

    for (size_t i = 0; i < vj["libraries"].arr_n; ++i) {
        const JVal& lib = vj["libraries"].arr[i];
        if (!lib_applies(lib)) continue;

        if (lib.has("name") && !lib.has("downloads")) {
            Str path = maven_path(lib["name"].str());
            if (path.empty()) continue;
            Str base_url{};
            if (lib.has("url")) base_url.assign_s(lib["url"].str());
            else base_url.assign_s("https://libraries.minecraft.net/");
            if (!base_url.empty() && base_url.back() != '/') base_url.append_c('/');
            base_url.append(path.p, path.n);
            DLTask t{};
            t.url = std::move(base_url);
            t.dest = pjoin(lib_dir, path.c_str());
            tasks.push_back(std::move(t));
            continue;
        }

        if (!lib.has("downloads")) continue;

        if (lib.has("natives") && lib["natives"].has("windows")) {
            Str nat_cls{};
            nat_cls.assign_s(lib["natives"]["windows"].str());
            const char* arch = sizeof(void*) == 8 ? "64" : "32";
            size_t pos = nat_cls.find_s("${arch}");
            if (pos != NPOS) nat_cls.replace_range(pos, 7, arch);

            if (lib["downloads"].has("classifiers") &&
                lib["downloads"]["classifiers"].has(nat_cls.c_str())) {
                const JVal& a = lib["downloads"]["classifiers"][nat_cls.c_str()];
                const char* u = a["url"].str();
                const char* p = a["path"].str();
                if (u && *u && p && *p) {
                    DLTask t{};
                    t.url.assign_s(u);
                    t.dest = pjoin(lib_dir, p);
                    tasks.push_back(std::move(t));
                }
            }
        }

        if (lib["downloads"].has("artifact")) {
            const JVal& a = lib["downloads"]["artifact"];
            const char* u = a["url"].str();
            const char* p = a["path"].str();
            if (u && *u && p && *p) {
                DLTask t{};
                t.url.assign_s(u);
                t.dest = pjoin(lib_dir, p);
                tasks.push_back(std::move(t));
            }
        }
    }
}

static void extract_natives(const WStr& root, const char* version, const JVal& vj) {
    if (!vj.has("libraries")) return;
    WStr lib_dir = pjoin(root, "libraries");
    WStr nat_dir = pjoin(pjoin(pjoin(root, "versions"), version), "natives");
    create_dirs(nat_dir);

    int extracted = 0;
    for (size_t i = 0; i < vj["libraries"].arr_n; ++i) {
        const JVal& lib = vj["libraries"].arr[i];
        if (!lib_applies(lib)) continue;

        WStr jar_path{};

        if (lib.has("natives") && lib["natives"].has("windows")) {
            Str nat_cls{};
            nat_cls.assign_s(lib["natives"]["windows"].str());
            const char* arch = sizeof(void*) == 8 ? "64" : "32";
            size_t pos = nat_cls.find_s("${arch}");
            if (pos != NPOS) nat_cls.replace_range(pos, 7, arch);

            if (!lib.has("downloads") ||
                !lib["downloads"].has("classifiers") ||
                !lib["downloads"]["classifiers"].has(nat_cls.c_str())) continue;

            const char* p = lib["downloads"]["classifiers"][nat_cls.c_str()]["path"].str();
            if (!p || !*p) continue;
            jar_path = pjoin(lib_dir, p);
        } else if (lib.has("downloads") && lib["downloads"].has("artifact")) {
            const char* p = lib["downloads"]["artifact"]["path"].str();
            if (!is_native_artifact_path(p)) continue;
            if (!native_path_matches_arch(p)) continue;
            jar_path = pjoin(lib_dir, p);
        } else {
            continue;
        }

        if (!path_exists(jar_path)) {
            Str js = path_to_str(jar_path);
            fprintf(stderr, "  [natives] JAR missing: %s\n", js.c_str());
            continue;
        }

        Str jar_s = path_to_str(jar_path);
        Str nat_s = path_to_str(nat_dir);
        char cmd[8192];
        snprintf(cmd, sizeof(cmd),
            "tar -xf \"%s\" -C \"%s\" --exclude=META-INF 2>NUL",
            jar_s.c_str(), nat_s.c_str());
        system(cmd);
        ++extracted;
    }

    Str nat_s = path_to_str(nat_dir);
    printf("  Extracted %d native JAR(s) -> %s\n", extracted, nat_s.c_str());
}

static bool lib_is_native_only(const JVal& lib) {
    if (lib.has("natives") && lib["natives"].has("windows")) return true;
    if (lib.has("downloads") && lib["downloads"].has("artifact")) {
        const char* p = lib["downloads"]["artifact"]["path"].str();
        if (is_native_artifact_path(p)) return true;
    }
    return false;
}

static bool install_bundled_jre(const WStr& root, Config& cfg, const WStr& cfg_path,
                                 const char* mc_ver = "") {
    const char* component = (!mc_ver || !*mc_ver) ? "jre-legacy"
                                                   : get_runtime_component(mc_ver);
    WStr jre_dir = pjoin(pjoin(root, "runtime"), component);

    WStr existing = find_java_in_dir(jre_dir);
    if (!existing.empty()) {
        Str es = to_utf8_str(existing.c_str());
        printf("  Found Mojang JRE (%s): %s\n", component, es.c_str());
        if (!check_java(cfg.java_path)) {
            cfg.java_path = std::move(es);
            save_config(cfg, cfg_path);
        }
        return true;
    }

    printf("\nNo bundled JRE found for '%s'.\n", component);
    printf("Download Mojang JRE (%s) automatically? (y/n): ", component);
    Str ans = read_line();
    if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return false;

    fputs("  Fetching Mojang runtime index...\n", stdout);
    Str url{}; url.assign_s(RUNTIME_ALL_URL);
    Str all_str = http_get_str(url);
    if (all_str.empty()) { fputs("  Failed to fetch runtime index.\n", stderr); return false; }
    JVal all_j = parse_json(all_str);

    const char* platform = "windows-x64";
    if (!all_j.has(platform) || !all_j[platform].has(component)) {
        fprintf(stderr, "  Component '%s' not found for %s.\n", component, platform);
        return false;
    }
    const JVal& comp_arr = all_j[platform][component];
    if (!comp_arr.is_array() || !comp_arr.arr_n) {
        fputs("  Empty component entry.\n", stderr); return false;
    }
    const char* manifest_url = comp_arr.arr[0]["manifest"]["url"].str();
    if (!manifest_url || !*manifest_url) { fputs("  No manifest URL.\n", stderr); return false; }

    printf("  Fetching file manifest for '%s'...\n", component);
    Str mu{}; mu.assign_s(manifest_url);
    Str mf_str = http_get_str(mu);
    if (mf_str.empty()) { fputs("  Failed to fetch file manifest.\n", stderr); return false; }
    JVal mf = parse_json(mf_str);

    create_dirs(jre_dir);
    const JVal& files = mf["files"];

    Vec<DLTask> tasks{};
    tasks.reserve(files.obj_n);
    for (size_t i = 0; i < files.obj_n; ++i) {
        const JVal& entry = files.obj_vals[i];
        WStr rel = pjoin(jre_dir, files.obj_keys[i].c_str());

        if (!strcmp(entry["type"].str(), "directory")) {
            create_dirs(rel);
            continue;
        }
        if (strcmp(entry["type"].str(), "file") != 0) continue;
        if (!entry.has("downloads") || !entry["downloads"].has("raw")) continue;
        const char* dl_url = entry["downloads"]["raw"]["url"].str();
        if (!dl_url || !*dl_url) continue;

        DLTask t{};
        t.url.assign_s(dl_url);
        t.dest = std::move(rel);
        tasks.push_back(std::move(t));
    }

    printf("  Downloading %zu JRE files...\n", tasks.n);
    parallel_dl(tasks, 16);

    WStr found = find_java_in_dir(jre_dir);
    if (found.empty()) {
        Str jd = path_to_str(jre_dir);
        fprintf(stderr, "  javaw.exe not found after install in: %s\n", jd.c_str());
        return false;
    }

    Str found_s = to_utf8_str(found.c_str());
    printf("  Mojang JRE (%s) installed: %s\n", component, found_s.c_str());
    cfg.java_path = std::move(found_s);
    save_config(cfg, cfg_path);
    return true;
}

static bool download_assets(const WStr& root, const JVal& vj) {
    const char* idx_url = vj["assetIndex"]["url"].str();
    const char* idx_id  = vj["assetIndex"]["id"].str();
    if (!idx_url || !*idx_url || !idx_id || !*idx_id) {
        fputs("  No asset index in version JSON.\n", stderr);
        return false;
    }

    WStr idx_dir = pjoin(pjoin(root, "assets"), "indexes");
    create_dirs(idx_dir);
    Str idx_id_str{}; idx_id_str.assign_s(idx_id);
    idx_id_str.append_s(".json");
    WStr idx_file = pjoin(idx_dir, idx_id_str.c_str());

    Str idx_str{};
    if (path_exists(idx_file)) {
        idx_str = read_file(idx_file);
    } else {
        Str u{}; u.assign_s(idx_url);
        idx_str = http_get_str(u);
        if (idx_str.empty()) { fputs("  Failed to fetch asset index.\n", stderr); return false; }
        write_file(idx_file, idx_str.c_str(), idx_str.n);
    }

    JVal idx_json = parse_json(idx_str);
    const JVal& objs = idx_json["objects"];

    WStr obj_dir = pjoin(pjoin(root, "assets"), "objects");

    Vec<DLTask> tasks{};
    size_t already = 0;
    for (size_t i = 0; i < objs.obj_n; ++i) {
        const char* hash = objs.obj_vals[i]["hash"].str();
        if (!hash || strlen(hash) < 2) continue;
        char pfx[3] = { hash[0], hash[1], 0 };
        WStr dest = pjoin(pjoin(obj_dir, pfx), hash);
        if (path_exists(dest) && path_file_size(dest) > 0) { ++already; continue; }
        DLTask t{};
        t.url.assign_s(RESOURCES_URL);
        t.url.append_s(pfx);
        t.url.append_c('/');
        t.url.append_s(hash);
        t.dest = std::move(dest);
        tasks.push_back(std::move(t));
    }

    printf("  Fetching %zu assets (%zu already cached)...\n",
           objs.obj_n, already);
    parallel_dl(tasks, 24);
    return true;
}

static bool download_minecraft_base(const WStr& root, const char* version,
                                     const JVal& manifest, bool print_steps = true) {
    const char* ver_url = nullptr;
    for (size_t i = 0; i < manifest["versions"].arr_n; ++i) {
        const JVal& v = manifest["versions"].arr[i];
        if (!strcmp(v["id"].str(), version)) { ver_url = v["url"].str(); break; }
    }
    if (!ver_url || !*ver_url) {
        fprintf(stderr, "Version %s not found in manifest.\n", version);
        return false;
    }

    WStr ver_dir  = pjoin(pjoin(root, "versions"), version);
    Str  verjson_name{}; verjson_name.assign_s(version); verjson_name.append_s(".json");
    WStr ver_json = pjoin(ver_dir, verjson_name.c_str());
    Str  verjar_name{};  verjar_name.assign_s(version);  verjar_name.append_s(".jar");
    WStr ver_jar  = pjoin(ver_dir, verjar_name.c_str());
    create_dirs(ver_dir);

    if (print_steps) printf("[2/5] Fetching %s version JSON...\n", version);
    Str ver_str{};
    if (path_exists(ver_json)) {
        ver_str = read_file(ver_json);
    } else {
        Str u{}; u.assign_s(ver_url);
        ver_str = http_get_str(u);
        if (ver_str.empty()) { fputs("Failed to fetch version JSON.\n", stderr); return false; }
        write_file(ver_json, ver_str.c_str(), ver_str.n);
    }
    JVal vj = parse_json(ver_str);

    if (print_steps) fputs("[3/5] Downloading client JAR...\n", stdout);
    Str jar_url{}; jar_url.assign_s(vj["downloads"]["client"]["url"].str());
    if (!download_file(jar_url, ver_jar)) {
        fputs("Failed to download client JAR.\n", stderr); return false;
    }

    if (print_steps) fputs("[4/5] Downloading libraries...\n", stdout);
    Vec<DLTask> lib_tasks{};
    download_libraries_to_tasks(root, vj, lib_tasks);
    parallel_dl(lib_tasks, 16);

    if (print_steps) fputs("[4/5] Extracting natives...\n", stdout);
    extract_natives(root, version, vj);

    if (print_steps) fputs("[5/5] Downloading assets...\n", stdout);
    download_assets(root, vj);

    return true;
}

static bool download_fabric(const WStr& root, const char* mc_version, const JVal& manifest) {
    printf("Fetching Fabric loaders for Minecraft %s...\n", mc_version);
    Str loaders_url{};
    loaders_url.assign_s(FABRIC_META_BASE);
    loaders_url.append_s("loader/");
    loaders_url.append_s(mc_version);
    Str loaders_str = http_get_str(loaders_url);
    if (loaders_str.empty()) {
        fputs("Failed to fetch Fabric loader list.\n", stderr);
        return false;
    }
    JVal loaders_j = parse_json(loaders_str);
    if (!loaders_j.is_array() || !loaders_j.arr_n) {
        fputs("No Fabric loaders available for this Minecraft version.\n", stderr);
        return false;
    }

    const char* loader_ver = loaders_j.arr[0]["loader"]["version"].str();
    if (!loader_ver || !*loader_ver) { fputs("Could not determine Fabric loader version.\n", stderr); return false; }
    printf("Using Fabric Loader: %s\n", loader_ver);

    Str fabric_id{};
    fabric_id.assign_s("fabric-loader-");
    fabric_id.append_s(loader_ver);
    fabric_id.append_c('-');
    fabric_id.append_s(mc_version);

    WStr ver_dir  = pjoin(pjoin(root, "versions"), fabric_id.c_str());
    Str  vjname{}; vjname.copy_from(fabric_id); vjname.append_s(".json");
    WStr ver_json = pjoin(ver_dir, vjname.c_str());
    create_dirs(ver_dir);

    fputs("[1/5] Fetching Fabric profile JSON...\n", stdout);
    Str profile_url{};
    profile_url.assign_s(FABRIC_META_BASE);
    profile_url.append_s("loader/");
    profile_url.append_s(mc_version);
    profile_url.append_c('/');
    profile_url.append_s(loader_ver);
    profile_url.append_s("/profile/json");

    Str profile_str{};
    if (path_exists(ver_json)) {
        profile_str = read_file(ver_json);
    } else {
        profile_str = http_get_str(profile_url);
        if (profile_str.empty()) { fputs("Failed to fetch Fabric profile JSON.\n", stderr); return false; }
        write_file(ver_json, profile_str.c_str(), profile_str.n);
    }
    JVal fabric_vj = parse_json(profile_str);

    printf("[2/5] Downloading base Minecraft %s...\n", mc_version);
    if (!download_minecraft_base(root, mc_version, manifest, false)) {
        fputs("Failed to download base Minecraft for Fabric.\n", stderr);
        return false;
    }

    fputs("[3/5] Downloading Fabric libraries...\n", stdout);
    Vec<DLTask> fabric_lib_tasks{};
    download_libraries_to_tasks(root, fabric_vj, fabric_lib_tasks);
    parallel_dl(fabric_lib_tasks, 16);

    fputs("[3/5] Extracting Fabric natives (if any)...\n", stdout);
    extract_natives(root, mc_version, fabric_vj);

    fputs("[4/5] (assets already fetched with base MC)\n", stdout);

    printf("\nFabric install complete: %s\n", fabric_id.c_str());
    return true;
}

struct KVPair { Str key; Str val; };
struct VarMap {
    Vec<KVPair> pairs;
    void set(const char* k, const char* v) {
        size_t kl = strlen(k);
        for (size_t i = 0; i < pairs.n; ++i)
            if (pairs.p[i].key.eq_n(k, kl)) { pairs.p[i].val.assign_s(v); return; }
        KVPair p{};
        p.key.append(k, kl);
        p.val.assign_s(v);
        pairs.push_back(std::move(p));
    }
    const Str* get(const char* k, size_t kl) const {
        for (size_t i = 0; i < pairs.n; ++i)
            if (pairs.p[i].key.eq_n(k, kl)) return &pairs.p[i].val;
        return nullptr;
    }
};

static Str tok_replace(const char* s, size_t slen, const VarMap& m) {
    Str r{};
    for (size_t i = 0; i < slen; ) {
        if (s[i] == '$' && i + 1 < slen && s[i+1] == '{') {
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

static Str tok_replace_str(const Str& s, const VarMap& m) {
    return tok_replace(s.c_str(), s.n, m);
}

static Str win_quote(const Str& s) {
    bool needs = false;
    for (size_t i = 0; i < s.n && !needs; ++i)
        if (s.p[i] == ' ' || s.p[i] == '\t' || s.p[i] == '"') needs = true;
    if (!needs && !s.empty()) { Str r{}; r.copy_from(s); return r; }
    Str r{};
    r.append_c('"');
    int sl = 0;
    for (size_t i = 0; i < s.n; ++i) {
        char c = s.p[i];
        if (c == '\\') {
            ++sl;
        } else if (c == '"') {
            for (int j = 0; j < sl * 2 + 1; ++j) r.append_c('\\');
            r.append_c('"'); sl = 0;
        } else {
            for (int j = 0; j < sl; ++j) r.append_c('\\');
            sl = 0; r.append_c(c);
        }
    }
    for (int j = 0; j < sl * 2; ++j) r.append_c('\\');
    r.append_c('"');
    return r;
}

static Str maven_ga_key(const char* path) {
    Str r{};
    if (!path || !*path) return r;
    size_t len = strlen(path);
    int slashes = 0;
    size_t cut = len;
    for (size_t i = len; i > 0; --i) {
        if (path[i-1] == '/' || path[i-1] == '\\') {
            if (++slashes == 2) { cut = i - 1; break; }
        }
    }
    r.append(path, cut);
    return r;
}

struct CPEntry { Str ga_key; Str full_path; };

static Str build_classpath(const WStr& root, const JVal& vj, const JVal& parent_vj,
                            const char* jar_ver) {
    WStr lib_dir = pjoin(root, "libraries");
    Vec<CPEntry> entries{};

    auto add_libs = [&](const JVal& j) {
        if (!j.has("libraries")) return;
        for (size_t i = 0; i < j["libraries"].arr_n; ++i) {
            const JVal& lib = j["libraries"].arr[i];
            if (!lib_applies(lib)) continue;
            if (lib_is_native_only(lib)) continue;
            const char* path = nullptr;
            if (lib.has("downloads") && lib["downloads"].has("artifact"))
                path = lib["downloads"]["artifact"]["path"].str();
            Str mpath{};
            if (!path || !*path) {
                if (lib.has("name")) mpath = maven_path(lib["name"].str());
                if (!mpath.empty()) path = mpath.c_str();
            }
            if (!path || !*path) continue;
            WStr jar = pjoin(lib_dir, path);
            if (!path_exists(jar)) continue;

            Str ga   = maven_ga_key(path);
            Str full = path_to_str(jar);

            bool found = false;
            for (size_t k = 0; k < entries.n; ++k) {
                if (entries.p[k].ga_key.eq(ga.c_str())) {
                    entries.p[k].full_path = std::move(full);
                    found = true; break;
                }
            }
            if (!found) {
                CPEntry e{};
                e.ga_key = std::move(ga);
                e.full_path = std::move(full);
                entries.push_back(std::move(e));
            }
        }
    };

    if (!parent_vj.is_null()) { add_libs(parent_vj); add_libs(vj); }
    else                        add_libs(vj);

    Str cp{};
    for (size_t i = 0; i < entries.n; ++i) {
        cp.append(entries.p[i].full_path.p, entries.p[i].full_path.n);
        cp.append_c(';');
    }
    Str jar_name{}; jar_name.assign_s(jar_ver); jar_name.append_s(".jar");
    WStr main_jar = pjoin(pjoin(pjoin(root, "versions"), jar_ver), jar_name.c_str());
    { Str main_jar_s = path_to_str(main_jar); cp.append(main_jar_s.p, main_jar_s.n); }
    return cp;
}

static bool launch_version(const WStr& root, const Config& cfg, const char* version) {
    Str vname{}; vname.assign_s(version);
    Str vjname{}; vjname.copy_from(vname); vjname.append_s(".json");
    WStr vj_path = pjoin(pjoin(pjoin(root, "versions"), version), vjname.c_str());

    if (!path_exists(vj_path)) {
        fprintf(stderr, "Not installed: %s\n", version);
        return false;
    }

    Str ver_str = read_file(vj_path);
    JVal vj = parse_json(ver_str);

    JVal parent_vj{};
    Str base_ver{}; base_ver.assign_s(version);

    if (vj.has("inheritsFrom")) {
        base_ver.assign_s(vj["inheritsFrom"].str());
        Str pvjname{}; pvjname.copy_from(base_ver); pvjname.append_s(".json");
        WStr pj_path = pjoin(pjoin(pjoin(root, "versions"), base_ver.c_str()), pvjname.c_str());
        if (!path_exists(pj_path)) {
            fprintf(stderr, "Base version '%s' not installed.\n", base_ver.c_str());
            return false;
        }
        Str ps = read_file(pj_path);
        parent_vj = parse_json(ps);
    }

    bool has_parent    = !parent_vj.is_null();
    const JVal& base_vj = has_parent ? parent_vj : vj;

    Str cp       = build_classpath(root, vj, parent_vj, base_ver.c_str());
    Str uuid     = make_offline_uuid(cfg.username);
    Str nat_path = path_to_str(pjoin(pjoin(pjoin(root, "versions"), base_ver.c_str()), "natives"));
    Str assets   = path_to_str(pjoin(root, "assets"));
    const char* asset_idx = base_vj["assetIndex"]["id"].str();
    Str game_dir = path_to_str(root);
    const char* ver_type_raw = vj.has("type") ? vj["type"].str() :
                               (base_vj.has("type") ? base_vj["type"].str() : "release");
    const char* main_cls_raw = vj["mainClass"].str();
    if (!main_cls_raw || !*main_cls_raw) main_cls_raw = base_vj["mainClass"].str();
    if (!main_cls_raw || !*main_cls_raw) main_cls_raw = "net.minecraft.client.main.Main";

    VarMap vars{};
    vars.set("auth_player_name",  cfg.username.c_str());
    vars.set("auth_uuid",         uuid.c_str());
    vars.set("auth_access_token", "0");
    vars.set("user_type",         "legacy");
    vars.set("user_properties",   "{}");
    vars.set("version_name",      version);
    vars.set("version_type",      ver_type_raw);
    vars.set("game_directory",    game_dir.c_str());
    vars.set("assets_root",       assets.c_str());
    vars.set("game_assets",       assets.c_str());
    vars.set("assets_index_name", asset_idx ? asset_idx : "");
    vars.set("natives_directory", nat_path.c_str());
    vars.set("classpath",         cp.c_str());
    vars.set("launcher_name",     "GoonMC");
    vars.set("launcher_version",  "1.0");

    Vec<Str> args{};
    args.reserve(48);

    char ram_buf[32];
    snprintf(ram_buf, sizeof(ram_buf), "-Xmx%dG", cfg.ram_gb);
    Str ram_arg{}; ram_arg.assign_s(ram_buf); args.push_back(std::move(ram_arg));
    Str ms_arg{}; ms_arg.assign_s("-Xms512m"); args.push_back(std::move(ms_arg));

    if (!cfg.java_args.empty()) {
        const char* p = cfg.java_args.c_str();
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p) break;
            const char* start = p;
            while (*p && *p != ' ' && *p != '\t') ++p;
            Str tok{}; tok.append(start, p - start);
            args.push_back(std::move(tok));
        }
    }

    if (required_jdk(base_ver.c_str()) <= 8) {
        Str a{}; a.assign_s("-XX:+UseConcMarkSweepGC"); args.push_back(std::move(a));
        a.assign_s("-XX:+CMSIncrementalMode"); args.push_back(std::move(a));
        a.assign_s("-XX:HeapDumpPath=MojangTricksIntelDriversForPerformance_javaw.exe_minecraft.exe.heapdump"); args.push_back(std::move(a));
    } else {
        const char* gc_args[] = {
            "-XX:+UseG1GC", "-XX:+UnlockExperimentalVMOptions",
            "-XX:G1NewSizePercent=20", "-XX:G1ReservePercent=20",
            "-XX:MaxGCPauseMillis=50", "-XX:G1HeapRegionSize=32M"
        };
        for (int i = 0; i < 6; ++i) {
            Str a{}; a.assign_s(gc_args[i]); args.push_back(std::move(a));
        }
    }

    auto collect_args = [&](const JVal& src, const char* which) {
        if (!src.has("arguments") || !src["arguments"].has(which)) return;
        const JVal& arr = src["arguments"][which];
        for (size_t i = 0; i < arr.arr_n; ++i) {
            const JVal& e = arr.arr[i];
            if (e.is_string()) {
                Str a = tok_replace(e.str(), strlen(e.str()), vars);
                args.push_back(std::move(a));
                continue;
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
                Str a = tok_replace(val.str(), strlen(val.str()), vars);
                args.push_back(std::move(a));
            } else if (val.is_array()) {
                for (size_t vi = 0; vi < val.arr_n; ++vi) {
                    Str a = tok_replace(val.arr[vi].str(), strlen(val.arr[vi].str()), vars);
                    args.push_back(std::move(a));
                }
            }
        }
    };

    Str main_cls_s{}; main_cls_s.assign_s(main_cls_raw);

    if (base_vj.has("arguments")) {
        size_t orig = args.n;
        collect_args(base_vj, "jvm");
        if (has_parent) collect_args(vj, "jvm");
        args.push_back(std::move(main_cls_s));
        collect_args(base_vj, "game");
    } else {
        Str a{};
        a.assign_s("-Djava.library.path="); a.append(nat_path.p, nat_path.n); args.push_back(std::move(a));
        a.assign_s("-Dorg.lwjgl.librarypath="); a.append(nat_path.p, nat_path.n); args.push_back(std::move(a));
        a.assign_s("-Dfile.encoding=UTF-8"); args.push_back(std::move(a));
        a.assign_s("-cp"); args.push_back(std::move(a));
        args.push_back(std::move(cp));
        args.push_back(std::move(main_cls_s));
        const char* mc_args = base_vj["minecraftArguments"].str();
        const char* p = mc_args;
        while (p && *p) {
            while (*p == ' ') ++p;
            if (!*p) break;
            const char* start = p;
            while (*p && *p != ' ') ++p;
            Str tok{}; tok.append(start, p - start);
            Str replaced = tok_replace_str(tok, vars);
            args.push_back(std::move(replaced));
        }
    }

    Str java_exec{}; java_exec.copy_from(cfg.java_path);
    if (!cfg.show_console) {
        size_t pos = java_exec.find_s("java.exe");
        if (pos != NPOS) {
            java_exec.replace_range(pos, 8, "javaw.exe");
        } else if (java_exec.n >= 4 &&
                   !memcmp(java_exec.p + java_exec.n - 4, "java", 4) &&
                   (java_exec.n == 4 ||
                    java_exec.p[java_exec.n-5] == '\\' ||
                    java_exec.p[java_exec.n-5] == '/')) {
            java_exec.append_c('w');
        }
    } else {
        size_t pos = java_exec.find_s("javaw.exe");
        if (pos != NPOS) {
            java_exec.replace_range(pos, 9, "java.exe");
        }
    }

    Str cmd{};
    Str qexe = win_quote(java_exec);
    cmd.append(qexe.p, qexe.n);
    for (size_t i = 0; i < args.n; ++i) {
        cmd.append_c(' ');
        Str qa = win_quote(args.p[i]);
        cmd.append(qa.p, qa.n);
    }

    printf("\nLaunching Minecraft %s as %s...\n[CMD] %s\n\n",
           version, cfg.username.c_str(), cmd.c_str());

    STARTUPINFOW si{}; si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    WStr wcmd = to_wide_str(cmd.c_str());
    wcmd.append_c(L'\0');

    Str game_dir_w = path_to_str(root);
    WStr wgame_dir = to_wide_str(game_dir_w.c_str());

    DWORD flags = CREATE_NEW_CONSOLE;
    if (!cfg.show_console) {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    if (!CreateProcessW(nullptr, wcmd.p, nullptr, nullptr, FALSE,
                        flags, nullptr, wgame_dir.c_str(), &si, &pi)) {
        fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

static Vec<Str> get_installed_versions(const WStr& root) {
    Vec<Str> v{};
    WStr ver_dir = pjoin(root, "versions");
    if (!path_exists(ver_dir)) return v;

    wchar_t pattern[4096];
    swprintf(pattern, 4096, L"%ls\\*", ver_dir.c_str());
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return v;

    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..")) continue;
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

        Str name = to_utf8_str(fd.cFileName);
        WStr entry_dir = pjoin(ver_dir, name.c_str());

        Str json_name{}; json_name.copy_from(name); json_name.append_s(".json");
        WStr json_p = pjoin(entry_dir, json_name.c_str());
        if (!path_exists(json_p)) continue;

        Str jar_name{}; jar_name.copy_from(name); jar_name.append_s(".jar");
        WStr jar = pjoin(entry_dir, jar_name.c_str());
        if (path_exists(jar) && path_file_size(jar) > 1024) { v.push_back(std::move(name)); continue; }

        Str js = read_file(json_p);
        if (js.empty()) continue;
        JVal jv = parse_json(js);
        if (jv.has("inheritsFrom")) {
            const char* base = jv["inheritsFrom"].str();
            Str bjar_name{}; bjar_name.assign_s(base); bjar_name.append_s(".jar");
            WStr base_jar = pjoin(pjoin(ver_dir, base), bjar_name.c_str());
            if (path_exists(base_jar) && path_file_size(base_jar) > 1024)
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

static void print_header(const char* title) {
    apply_theme();
    printf("\n================================================\n  %s\n================================================\n", title);
}

static void section_download(const WStr& root, Config& cfg, const WStr& cfg_path) {
    print_header("DOWNLOAD");

    fputs("\nLoader type:\n  [1] Vanilla\n  [2] Fabric\nChoice: ", stdout);
    Str loader_choice = read_line();
    bool use_fabric = loader_choice.eq("2");

    struct VE { Str id, type; };
    Vec<VE> entries{};
    JVal manifest{};

    if (use_fabric) {
        fputs("Fetching Fabric supported versions...\n", stdout);
        Str fu{}; fu.assign_s(FABRIC_META_BASE); fu.append_s("game");
        Str fv_str = http_get_str(fu);
        if (fv_str.empty()) {
            fputs("Failed to fetch Fabric game versions.\nPress Enter to continue...", stdout);
            getchar(); return;
        }
        JVal fv = parse_json(fv_str);
        if (!fv.is_array()) {
            fputs("Unexpected Fabric version response.\nPress Enter to continue...", stdout);
            getchar(); return;
        }
        entries.reserve(fv.arr_n);
        for (size_t i = 0; i < fv.arr_n; ++i) {
            VE e{};
            e.id.assign_s(fv.arr[i]["version"].str());
            e.type.assign_s(fv.arr[i]["stable"].bval ? "release" : "snapshot");
            entries.push_back(std::move(e));
        }
        fputs("Fetching Mojang manifest (needed for base download)...\n", stdout);
        Str mu{}; mu.assign_s(MANIFEST_URL);
        Str ms = http_get_str(mu);
        if (!ms.empty()) manifest = parse_json(ms);
    } else {
        fputs("Fetching version manifest...\n", stdout);
        Str mu{}; mu.assign_s(MANIFEST_URL);
        Str ms = http_get_str(mu);
        if (ms.empty()) {
            fputs("Failed to fetch manifest.\nPress Enter to continue...", stdout);
            getchar(); return;
        }
        manifest = parse_json(ms);
        entries.reserve(manifest["versions"].arr_n);
        for (size_t i = 0; i < manifest["versions"].arr_n; ++i) {
            VE e{};
            e.id.assign_s(manifest["versions"].arr[i]["id"].str());
            e.type.assign_s(manifest["versions"].arr[i]["type"].str());
            entries.push_back(std::move(e));
        }
    }

    fputs("\nFilter: (1) Releases only  (2) All versions\nChoice: ", stdout);
    Str fin = read_line();
    bool releases_only = !fin.eq("2");

    Vec<VE> filtered{};
    filtered.reserve(entries.n);
    for (size_t i = 0; i < entries.n; ++i)
        if (!releases_only || entries.p[i].type.eq("release")) {
            VE e{}; e.id.copy_from(entries.p[i].id); e.type.copy_from(entries.p[i].type);
            filtered.push_back(std::move(e));
        }

    int page = 0;
    const int PAGE = 20;
    for (;;) {
        int pages = ((int)filtered.n + PAGE - 1) / PAGE;
        int start = page * PAGE;
        int end   = (start + PAGE < (int)filtered.n) ? start + PAGE : (int)filtered.n;

        printf("\nVersions (page %d/%d):\n", page + 1, pages);
        for (int i = start; i < end; ++i) {
            printf("  [%d] %s", i - start + 1, filtered.p[i].id.c_str());
            if (!filtered.p[i].type.eq("release"))
                printf(" (%s)", filtered.p[i].type.c_str());
            putchar('\n');
        }
        fputs("\nEnter number, 'n' next, 'p' prev, 'q' cancel: ", stdout);
        Str input = read_line();

        if (input.eq("q") || input.eq("Q")) return;
        if (input.eq("n") || input.eq("N")) { if (page + 1 < pages) ++page; continue; }
        if (input.eq("p") || input.eq("P")) { if (page > 0) --page; continue; }

        int idx = -1;
        if (!parse_int(input, &idx)) { fputs("Invalid input.\n", stdout); continue; }
        idx = idx - 1 + start;
        if (idx < 0 || idx >= (int)filtered.n) { fputs("Invalid selection.\n", stdout); continue; }

        const char* chosen = filtered.p[idx].id.c_str();

        if (use_fabric) {
            printf("\nDownload Fabric for Minecraft %s? (y/n): ", chosen);
            Str ans = read_line();
            if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return;
            if (!install_bundled_jre(root, cfg, cfg_path, chosen))
                fputs("Continuing without bundled JRE.\n", stdout);
            if (manifest.is_null()) {
                fputs("Mojang manifest unavailable; cannot download base MC.\n", stderr);
            } else if (!download_fabric(root, chosen, manifest)) {
                fputs("\nFabric download failed.\n", stderr);
            } else {
                printf("\nFabric for Minecraft %s is ready.\n", chosen);
            }
        } else {
            Str jar_name{}; jar_name.assign_s(chosen); jar_name.append_s(".jar");
            WStr jar = pjoin(pjoin(pjoin(root, "versions"), chosen), jar_name.c_str());
            if (path_exists(jar) && path_file_size(jar) > 1024) {
                printf("\nVersion %s is already installed.\n", chosen);
            } else {
                printf("\nDownload Minecraft %s? (y/n): ", chosen);
                Str ans = read_line();
                if (ans.empty() || (ans.p[0] != 'y' && ans.p[0] != 'Y')) return;
                if (!install_bundled_jre(root, cfg, cfg_path, chosen))
                    fputs("Continuing without bundled JRE.\n", stdout);
                fputs("\n[1/5] Manifest already fetched.\n", stdout);
                if (!download_minecraft_base(root, chosen, manifest))
                    fputs("\nDownload failed.\n", stderr);
                else
                    printf("\nDownload complete! %s is ready.\n", chosen);
            }
        }
        fputs("Press Enter to continue...", stdout); getchar();
        return;
    }
}

static void section_settings(Config& cfg, const WStr& cfg_path) {
    for (;;) {
        print_header("SETTINGS");
        printf("  [1] Username      : %s\n"
               "  [2] RAM (GB)      : %dGB\n"
               "  [3] Java Path     : %s\n"
               "  [4] Java Args     : %s\n"
               "  [5] Hide Launcher : %s\n"
               "  [6] Show Console  : %s\n"
               "  [7] Back\n\nChoice: ",
               cfg.username.c_str(), cfg.ram_gb,
               cfg.java_path.c_str(),
               cfg.java_args.empty() ? "(none)" : cfg.java_args.c_str(),
               cfg.hide_launcher ? "ON" : "OFF",
               cfg.show_console  ? "ON" : "OFF");

        Str input = read_line();

        if (input.eq("1")) {
            printf("New username [%s]: ", cfg.username.c_str());
            Str val = read_line();
            if (!val.empty()) cfg.username = std::move(val);
        } else if (input.eq("2")) {
            printf("RAM in GB [%d]: ", cfg.ram_gb);
            Str val = read_line();
            int gb = 0;
            if (parse_int(val, &gb)) {
                if (gb >= 1 && gb <= 64) cfg.ram_gb = gb;
                else fputs("Invalid. Must be 1-64.\n", stdout);
            }
        } else if (input.eq("3")) {
            printf("Java path [%s]: ", cfg.java_path.c_str());
            Str val = read_line();
            if (!val.empty()) {
                if (!check_java(val)) fputs("Warning: could not verify java at that path.\n", stdout);
                cfg.java_path = std::move(val);
            }
        } else if (input.eq("4")) {
            printf("Extra Java args (space-separated) [%s]: ",
                   cfg.java_args.empty() ? "none" : cfg.java_args.c_str());
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
        fputs("Settings saved.\n", stdout);
    }
}

static void section_themes(Config& cfg, const WStr& cfg_path) {
    struct ThemeEntry { const char* name; int color; };
    static constexpr ThemeEntry themes[] = {
        { "White (Default)", 15 },
        { "Cyan",            11 },
        { "Green",           10 },
        { "Yellow",          14 },
        { "Red",             12 },
        { "Magenta",         13 },
        { "Light Blue",       9 },
        { "Gray",             8 },
    };
    constexpr int NUM_THEMES = static_cast<int>(sizeof(themes) / sizeof(themes[0]));

    for (;;) {
        print_header("THEMES");
        fputs("  Select a text color:\n\n", stdout);
        for (int i = 0; i < NUM_THEMES; ++i) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)themes[i].color);
            bool active = (cfg.theme_color == themes[i].color);
            printf("  [%d] %s%s\n", i + 1, themes[i].name, active ? " *" : "");
        }
        apply_theme();
        fputs("\nChoice (or 'q' to go back): ", stdout);

        Str input = read_line();
        if (input.eq("q") || input.eq("Q")) break;

        int idx = -1;
        if (!parse_int(input, &idx)) { fputs("Invalid input.\n", stdout); continue; }
        idx -= 1;
        if (idx < 0 || idx >= NUM_THEMES) { fputs("Invalid selection.\n", stdout); continue; }
        cfg.theme_color = themes[idx].color;
        g_theme_color   = cfg.theme_color;
        apply_theme();
        save_config(cfg, cfg_path);
        fputs("Theme applied.\n", stdout);
    }
}

static void section_launch(const WStr& root, Config& cfg, const WStr& cfg_path) {
    print_header("LAUNCH");
    Vec<Str> versions = get_installed_versions(root);
    if (versions.empty()) {
        fputs("\nNo installed versions. Go to Download first.\nPress Enter to continue...", stdout);
        getchar();
        return;
    }

    fputs("\nInstalled versions:\n", stdout);
    for (size_t i = 0; i < versions.n; ++i)
        printf("  [%zu] %s\n", i + 1, versions.p[i].c_str());
    fputs("\nSelect version (or 'q' to cancel): ", stdout);

    Str input = read_line();
    if (input.eq("q") || input.eq("Q")) return;

    int idx = -1;
    if (!parse_int(input, &idx)) {
        fputs("Invalid input.\nPress Enter to continue...", stdout);
        getchar(); return;
    }
    idx -= 1;
    if (idx < 0 || idx >= (int)versions.n) {
        fputs("Invalid selection.\nPress Enter to continue...", stdout);
        getchar(); return;
    }

    const char* chosen = versions.p[idx].c_str();
    Str base_ver{}; base_ver.assign_s(chosen);
    {
        Str vjname{}; vjname.assign_s(chosen); vjname.append_s(".json");
        WStr vj_path = pjoin(pjoin(pjoin(root, "versions"), chosen), vjname.c_str());
        if (path_exists(vj_path)) {
            Str vs = read_file(vj_path);
            JVal jv = parse_json(vs);
            if (jv.has("inheritsFrom")) base_ver.assign_s(jv["inheritsFrom"].str());
        }
    }
    if (!check_java(cfg.java_path)) {
        printf("\nJava not found at: %s\nLocating bundled JRE...\n", cfg.java_path.c_str());
        if (!install_bundled_jre(root, cfg, cfg_path, base_ver.c_str())) {
            fputs("Java unavailable. Set Java Path in Settings.\nPress Enter to continue...", stderr);
            getchar(); return;
        }
    }
    if (!launch_version(root, cfg, chosen)) {
        fputs("Press Enter to continue...", stdout);
        getchar();
    } else {
        fputs("Game launched! Exiting launcher...\n", stdout);
        Sleep(1500);
        if (cfg.hide_launcher) exit(0);
    }
}

static void init_console() {
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, 229, LWA_ALPHA);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coord = { 50, 15 };
        SMALL_RECT rect = { 0, 0, 49, 14 };

        SMALL_RECT tmp = { 0, 0, 0, 0 };
        SetConsoleWindowInfo(hOut, TRUE, &tmp);
        SetConsoleScreenBufferSize(hOut, coord);
        SetConsoleWindowInfo(hOut, TRUE, &rect);

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~WS_VSCROLL;
        style &= ~WS_HSCROLL;
        SetWindowLong(hwnd, GWL_STYLE, style);
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleW(L"GoonMC by TryFast");
    init_console();
    InitializeCriticalSection(&g_mkdir_cs);

    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    size_t elen = wcslen(exe);
    while (elen && exe[elen-1] != L'\\') --elen;
    if (elen) exe[--elen] = 0;
    WStr root{}; root.assign_w(exe);

    WStr cfg_path = pjoin(root, "config.json");
    Config cfg = load_config(cfg_path);

    g_theme_color = cfg.theme_color;
    apply_theme();

    if (cfg.username.empty() || cfg.username.eq("Player")) {
        fputs("=== GoonMC by TryFast ===\n\nEnter your username: ", stdout);
        Str uname = read_line();
        if (!uname.empty()) cfg.username = std::move(uname);
        if (cfg.username.empty()) cfg.username.assign_s("Player");
        save_config(cfg, cfg_path);
    }

    for (;;) {
        print_header("GoonMC by TryFast");
        fputs("  [1] Launch\n  [2] Download\n  [3] Settings\n  [4] Themes\n  [5] Exit\n\nChoice: ", stdout);
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