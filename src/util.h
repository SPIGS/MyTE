#pragma once
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <sys/stat.h>

// Defines
#define DEG_TO_RAD 0.0174532925f
#define RAD_TO_DEG 57.2957795131

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(a,x,b) (((x)<(a))?(a):((b)<(x))?(b):(x))

#define UNUSED(x) ((void)x)

// Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Regular int types
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

// Floating point types
typedef float f32;
typedef double f64;

static inline f64 radians(f32 deg) { return (f64) (deg * DEG_TO_RAD); }
static inline f32 degrees(f64 rad) { return (f32) (rad * RAD_TO_DEG); }

// Matrices
typedef struct { f32 a[3*3]; } mat3;
typedef struct { f32 a[4*4]; } mat4;

//Rect
typedef struct { f32 x; f32 y; f32 w; f32 h; } rect;

// Vectors
// Vec2
typedef struct { f32 x; f32 y; } vec2;
static inline vec2 vec2_init(f32 x, f32 y) { return (vec2) { x, y }; }
static inline vec2 vec2_add(vec2 a, vec2 b) { return (vec2) { .x = a.x + b.x, .y = a.y + b.y }; }
static inline vec2 vec2_sub(vec2 a, vec2 b) { return (vec2) { .x = a.x - b.x, .y = a.y - b.y }; }
static inline vec2 vec2_scale(vec2 a, f32 s) { return (vec2) { .x = a.x * s, .y = a.y * s }; }
static inline f32  vec2_mag(vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
static inline f32  vec2_magsq(vec2 v) { return v.x * v.x + v.y * v.y; }
static inline vec2 vec2_normalize(vec2 v) { return vec2_scale(v, 1.f / vec2_mag(v)); }
static inline vec2 vec2_neg(vec2 v) { return (vec2) { .x = -v.x, .y = -v.y }; }
static inline f32  vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
vec2 vec2_clamp(vec2 vec, rect quad);
vec2 vec2_lerp (vec2 start, vec2 end, f32 t);
vec2 vec2_ease_out(vec2 start, vec2 end, f32 t);

// Vec3
typedef struct { f32 x; f32 y; f32 z; } vec3;
static inline vec3 vec3_init(f32 x, f32 y, f32 z) { return (vec3) { x, y, z }; }
static inline vec3 vec3_add(vec3 a, vec3 b) { return (vec3) { .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z }; }
static inline vec3 vec3_sub(vec3 a, vec3 b) { return (vec3) { .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z }; }
static inline vec3 vec3_scale(vec3 a, f32 s) { return (vec3) { .x = a.x * s, .y = a.y * s, .z = a.z * s }; }
static inline vec3 vec3_cross(vec3 a, vec3 b) { return (vec3) { .x = a.y * b.z - b.y * a.z, .y = a.x * b.z - b.x * a.z, .z = a.x * b.y + b.x * a.y }; }
vec3 vec3_mul(vec3 a, mat3 m);
vec3 vec3_lerp(vec3 start, vec3 end, f32 t);

// Vec4
typedef struct { f32 x; f32 y; f32 z; f32 w; } vec4;
static inline vec4 vec4_init(f32 x, f32 y, f32 z, f32 w) { return (vec4) { x, y, z, w }; }
static inline vec4 vec4_add(vec4 a, vec4 b) { return (vec4) { .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w }; }
static inline vec4 vec4_sub(vec4 a, vec4 b) { return (vec4) { .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w }; }
static inline vec4 vec4_scale(vec4 a, f32 s) { return (vec4) { .x = a.x * s, .y = a.y * s, .z = a.z * s, .w = a.w * s }; }
vec4 vec4_mul(vec4 a, mat4 m);


// Matrices

static inline u16 mat3_idx(u16 x, u16 y) { return y * 3 + x; }
static inline u16 mat4_idx(u16 x, u16 y) { return y * 4 + x; }

mat3 mat3_identity();
mat4 mat4_identity();

mat3 mat3_mul(mat3 a, mat3 b);
void mat3_set(mat3* mat, mat3 o);
mat3 mat3_translate(vec2 v);
mat3 mat3_rotate(f32 r);
mat3 mat3_scalev(vec2 s);
mat3 mat3_scalef(f32 s);

mat4 mat4_mul(mat4 a, mat4 b);
void mat4_set(mat4* mat, mat4 o);
mat4 mat4_transpose(mat4 a);
mat4 mat4_translate(vec3 v);
mat4 mat4_scale(vec3 v);
mat4 mat4_rotX(f32 deg);
mat4 mat4_rotY(f32 deg);
mat4 mat4_rotZ(f32 deg);
mat4 mat4_ortho(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far);
mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far);


// Quaternions
typedef struct quat { f32 s; f32 i; f32 j; f32 k; } quat;
quat quat_identity();
quat quat_mul(quat a, quat b);
f32  quat_length(quat q);
quat quat_norm(quat q);
quat quat_rotate_axis(quat q, f32 x, f32 y, f32 z, f32 a);
quat quat_from_euler(f32 yaw, f32 pitch, f32 roll);
mat4 quat_to_rotation_mat(quat q);


// Rect
static inline rect rect_init(f32 x, f32 y, f32 w, f32 h) { return (rect) { x, y, w, h }; }
bool rect_contains_point(rect a, vec2 p);
bool rect_overlaps(rect a, rect b);
bool rect_contained_by_rect(rect a, rect b);
rect rect_get_overlap(rect a, rect b);
rect rect_uv_cull(rect quad, rect uv, rect cull_quad);


// Color 
typedef struct { f32 r; f32 g; f32 b; f32 a; } Color;
Color color_from_hex(u32 color_hex);

// Common Colors
#define COLOR_BLACK     color_from_hex(0x000000FF)
#define COLOR_WHITE     color_from_hex(0xFFFFFFFF)
#define COLOR_SILVER    color_from_hex(0x808080FF)
#define COLOR_GRAY      color_from_hex(0x232323FF)
#define COLOR_RED       color_from_hex(0xFF0000FF)
#define COLOR_ORANGE    color_from_hex(0xFFA500FF)
#define COLOR_GOLD      color_from_hex(0xFFD700FF)
#define COLOR_YELLOW    color_from_hex(0xFFFF00FF)
#define COLOR_GREEN     color_from_hex(0x00FF00FF)
#define COLOR_CYAN      color_from_hex(0x00FFFFFF)
#define COLOR_BLUE      color_from_hex(0x0000FFFF)
#define COLOR_NAVY      color_from_hex(0x000080FF)
#define COLOR_PURPLE    color_from_hex(0x800080FF)
#define COLOR_MAGENTA   color_from_hex(0xFF00FFFF)

// File handling
typedef enum {
    FILE_TYPE_C,
    FILE_TYPE_MAKEFILE,
    FILE_TYPE_TOML,
    FILE_TYPE_PYTHON,
    FILE_TYPE_UNKNOWN
} FileType;

// Returns 0 if the path points to a file, 1 if the path points to a directory and -1 if there is an error.
i32 checkPath (const char *path);
const char *getFileExtFromPath(const char *path);

/* Be sure to call free()! */
char *getFileNameFromPath(const char *path);
char* get_filename_from_path(const char* filepath);

FileType getFileType(const char *file_name, const char *file_ext);

char *readFile(const char *file_name);

// Some logging stuff

enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

#define LOG_ERROR(fmt, ...) log_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log_log(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_log(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_log(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void log_log(i32 level, const char *file, int line, const char *fmt, ...);

f32 lerp (f32 start, f32 end, f32 t);
f32 ease_out (f32 start, f32 end, f32 t);