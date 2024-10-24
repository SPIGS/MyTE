#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vec2 vec2_clamp(vec2 vec, rect quad) {
    return (vec2) {
        .x = CLAMP(quad.x, vec.x, quad.x + quad.w),
        .y = CLAMP(quad.y, vec.y, quad.y + quad.h)
    };
}

vec2 vec2_lerp(vec2 start, vec2 end, f32 t) {
    vec2 result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    return result;
}

vec2 vec2_ease_out(vec2 start, vec2 end, f32 t) {
    // Cubic ease-out equation
    t = t - 1.0f;  // Adjust t for cubic easing
    f32 eased_t = t * t * t + 1.0f;  // Apply cubic ease-out

    vec2 result;
    result.x = start.x + eased_t * (end.x - start.x);
    result.y = start.y + eased_t * (end.y - start.y);
    return result;
}

vec3 vec3_mul(vec3 a, mat3 m) {
    return (vec3) {
        .x = (a.x * m.a[mat3_idx(0, 0)] + a.y * m.a[mat3_idx(1, 0)] + a.z * m.a[mat3_idx(2, 0)]),
        .y = (a.x * m.a[mat3_idx(0, 1)] + a.y * m.a[mat3_idx(1, 1)] + a.z * m.a[mat3_idx(2, 1)]),
        .z = (a.x * m.a[mat3_idx(0, 2)] + a.y * m.a[mat3_idx(1, 2)] + a.z * m.a[mat3_idx(2, 2)])
    };
}

vec3 vec3_lerp(vec3 start, vec3 end, f32 t) {
    vec3 result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    result.z = start.z + t * (end.z - start.z);
    return result;
}

vec4 vec4_mul(vec4 a, mat4 m) {
    return (vec4) {
        .x = (a.x * m.a[mat4_idx(0, 0)] + a.y * m.a[mat4_idx(1, 0)] + a.z * m.a[mat4_idx(2, 0)] + a.w * m.a[mat4_idx(3, 0)]),
        .y = (a.x * m.a[mat4_idx(0, 1)] + a.y * m.a[mat4_idx(1, 1)] + a.z * m.a[mat4_idx(2, 1)] + a.w * m.a[mat4_idx(3, 1)]),
        .z = (a.x * m.a[mat4_idx(0, 2)] + a.y * m.a[mat4_idx(1, 2)] + a.z * m.a[mat4_idx(2, 2)] + a.w * m.a[mat4_idx(3, 2)]),
        .w = (a.x * m.a[mat4_idx(0, 3)] + a.y * m.a[mat4_idx(1, 3)] + a.z * m.a[mat4_idx(2, 3)] + a.w * m.a[mat4_idx(3, 3)])
    };
}

mat3 mat3_identity() {
    return (mat3) {
        .a = {
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f,
        }
    };
}

mat4 mat4_identity() {
    return (mat4) {
        .a = {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        }
    };
}

mat3 mat3_mul(mat3 a, mat3 b) {
    mat3 result = mat3_identity();
    for (u16 j = 0; j < 3; j++) {
        for (u16 i = 0; i < 3; i++) {
            result.a[mat3_idx(i, j)] =
                a.a[mat3_idx(i, 0)] * b.a[mat3_idx(0, j)] +
                a.a[mat3_idx(i, 1)] * b.a[mat3_idx(1, j)] +
                a.a[mat3_idx(i, 2)] * b.a[mat3_idx(2, j)];
        }
    }
    return result;
}

void mat3_set(mat3* mat, mat3 o) {
    for (u16 j = 0; j < 3; j++) {
        for (u16 i = 0; i < 3; i++) {
            mat->a[mat3_idx(i, j)] = o.a[mat3_idx(i, j)];
        }
    }
}

mat3 mat3_translate(vec2 v) {
    return (mat3) {
        .a = {
            1.f, 0.f, v.x,
            0.f, 1.f, v.y,
            0.f, 0.f, 1.f,
        }
    };
}

mat3 mat3_rotate(f32 r) {
    return (mat3) {
        .a = {
            cos(radians(r)), -sin(radians(r)), 0.f,
            sin(radians(r)),  cos(radians(r)), 0.f,
            0.f,              0.f,             1.f,
        }
    };
}

mat3 mat3_scalev(vec2 s) {
    return (mat3) {
        .a = {
            s.x, 0.f, 0.f,
            0.f, s.y, 0.f,
            0.f, 0.f, 1.f,
        }
    };
}

mat3 mat3_scalef(f32 s) {
    return (mat3) {
        .a = {
            s,   0.f, 0.f,
            0.f, s,   0.f,
            0.f, 0.f, 1.f,
        }
    };
}

mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 result = mat4_identity();
    for (u16 j = 0; j < 4; j++) {
        for (u16 i = 0; i < 4; i++) {
            result.a[mat4_idx(i, j)] =
                a.a[mat4_idx(i, 0)] * b.a[mat4_idx(0, j)] +
                a.a[mat4_idx(i, 1)] * b.a[mat4_idx(1, j)] +
                a.a[mat4_idx(i, 2)] * b.a[mat4_idx(2, j)] +
                a.a[mat4_idx(i, 3)] * b.a[mat4_idx(3, j)];
        }
    }
    return result;
}

void mat4_set(mat4* mat, mat4 o) {
    for (u16 j = 0; j < 4; j++) {
        for (u16 i = 0; i < 4; i++) {
            mat->a[mat4_idx(i, j)] = o.a[mat4_idx(i, j)];
        }
    }
}

mat4 mat4_translate(vec3 v) {
    return (mat4) {
        .a = {
            1.f, 0.f, 0.f, v.x,
            0.f, 1.f, 0.f, v.y,
            0.f, 0.f, 1.f, v.z,
            0.f, 0.f, 0.f, 1.f,
        }
    };
}

mat4 mat4_scale(vec3 v) {
    return (mat4) {
        .a = {
            v.x, 0.f, 0.f, 0.f,
            0.f, v.y, 0.f, 0.f,
            0.f, 0.f, v.z, 0.f,
            0.f, 0.f, 0.f, 1.f,
        }
    };
}

mat4 mat4_rotX(f32 deg) {
    f64 rad = deg * DEG_TO_RAD;
    return (mat4) {
        .a = {
            1.f, 0.f,        0.f,       0.f,
            0.f, cosf(rad), -sinf(rad), 0.f,
            0.f, sinf(rad),  cosf(rad), 0.f,
            0.f, 0.f,        0.f,       1.f,
        }
    };
}

mat4 mat4_rotY(f32 deg) {
    f64 rad = deg * DEG_TO_RAD;
    return (mat4) {
        .a = {
            cosf(rad), 0.f, -sinf(rad), 0.f,
            0.f,       1.f,  0.f,       0.f,
            sinf(rad), 0.f,  cosf(rad), 0.f,
            0.f,       0.f,  0.f,       1.f,
        }
    };
}

mat4 mat4_rotZ(f32 deg) {
    f64 rad = deg * DEG_TO_RAD;
    return (mat4) {
        .a = {
            cosf(rad), -sinf(rad), 0.f, 0.f,
            sinf(rad),  cosf(rad), 0.f, 0.f,
            0.f,        0.f,       1.f, 0.f,
            0.f,        0.f,       0.f, 1.f,
        }
    };
}

mat4 mat4_transpose(mat4 a) {
    mat4 ret = mat4_identity();
    for (u16 j = 0; j < 4; j++) {
        for (u16 i = 0; i < 4; i++) {
            ret.a[mat4_idx(i, j)] = a.a[mat4_idx(j, i)];
        }
    }
    return ret;
}

mat4 mat4_ortho(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far) {
    f32 width = right - left;
    f32 height = top - bottom;
    f32 depth = far - near;
    return (mat4) {
        .a = {
            2.f / width,             0.f,                      0.f,                   0.f,
            0.f,                     2.f / height,             0.f,                   0.f,
            0.f,                     0.f,                      -2.f / depth,          0.f,
            -(right + left) / width, -(top + bottom) / height, -(far + near) / depth, 1.f,
        }
    };
}

mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far) {
    f32 top = tanf(fov * DEG_TO_RAD / 2) * near;
    f32 right = top * aspect_ratio;
    f32 depth = far - near;
    return (mat4) {
        .a = {
            1.f / right, 0.f,       0.f,                   0.f,
            0.f,         1.f / top, 0.f,                   0.f,
            0.f,         0.f,       -2.f / depth,          0.f,
            0.f,         0.f,       -(far + near) / depth, 1.f,
        }
    };
}

quat quat_identity() {
    return (quat) { 1, 0, 0, 0 };
}

quat quat_mul(quat a, quat b) {
    return (quat) {
        a.s * b.s - a.i * b.i - a.j * b.j - a.k * b.k,
        a.s * b.i + a.i * b.s + a.j * b.k + a.k * b.j,
        a.s * b.j + a.j * b.s + a.i * b.k + a.k * b.i,
        a.s * b.k + a.k * b.s + a.i * b.j + a.j * b.i
    };
}

f32 quat_length(quat q) {
    return sqrtf(q.s * q.s + q.i * q.i + q.j * q.j + q.k * q.k);
}

quat quat_norm(quat q) {
    f32 len = quat_length(q);
    return (quat) { q.s / len, q.i / len, q.j / len, q.k / len };
}

quat quat_rotate_axis(quat q, f32 x, f32 y, f32 z, f32 a) {
    UNUSED(q);
    f32 factor = sinf(a / 2.0f);
    
    quat r = {0};
    r.i = x * factor;
    r.j = y * factor;
    r.k = z * factor;
    r.s = cosf(a / 2.0f);
    
    return quat_norm(r);
}

quat quat_from_euler(f32 yaw, f32 pitch, f32 roll) {
    // Abbreviations for the various angular functions
    f32 cy = cosf(yaw * 0.5);
    f32 sy = sinf(yaw * 0.5);
    f32 cp = cosf(pitch * 0.5);
    f32 sp = sinf(pitch * 0.5);
    f32 cr = cosf(roll * 0.5);
    f32 sr = sinf(roll * 0.5);
    
    quat q;
    q.s = cr * cp * cy + sr * sp * sy;
    q.i = sr * cp * cy - cr * sp * sy;
    q.j = cr * sp * cy + sr * cp * sy;
    q.k = cr * cp * sy - sr * sp * cy;
    return q;
}

mat4 quat_to_rotation_mat(quat q) {
    f32 isq = q.i * q.i;
    f32 jsq = q.j * q.j;
    f32 ksq = q.k * q.k;
    
    return (mat4) {
        .a = {
            1 - 2*jsq - 2*ksq, 2*q.i*q.j - 2*q.s*q.k, 2*q.i*q.k + 2*q.s*q.j, 0,
            2*q.i*q.j + 2*q.s*q.k, 1 - 2*isq - 2*ksq, 2*q.j*q.k - 2*q.s*q.i, 0,
            2*q.i*q.k - 2*q.s*q.j, 2*q.j*q.k + 2*q.s*q.i, 1 - 2*isq - 2*jsq, 0,
            0,                     0,                     0,                 1
        }
    };
}

bool rect_contains_point(rect a, vec2 p) {
    return a.x <= p.x && a.y <= p.y && a.x + a.w >= p.x && a.y + a.h >= p.y;
}

bool rect_overlaps(rect a, rect b) {
    bool x = (a.x >= b.x && a.x <= b.x + b.w) || (a.x + a.w >= b.x && a.x + a.w <= b.x + b.w) || (a.x <= b.x && a.x + a.w >= b.x + b.w);
    bool y = (a.y >= b.y && a.y <= b.y + b.h) || (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h) || (a.y <= b.y && a.y + a.h >= b.y + b.h);
    return x && y;
}

bool rect_contained_by_rect(rect a, rect b) {
    bool x = (a.x >= b.x && a.x <= b.x + b.w) && (a.x + a.w >= b.x && a.x + a.w <= b.x + b.w);
    bool y = (a.y >= b.y && a.y <= b.y + b.h) && (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h);
    return x && y;
}

rect rect_get_overlap(rect a, rect b) {
    vec2 min = (vec2) { MAX(a.x, b.x), MAX(a.y, b.y) };
    vec2 max = (vec2) { MIN(a.x + a.w, b.x + b.w), MIN(a.y + a.h, b.y + b.h) };
    return (rect) { min.x, min.y, max.x - min.x, max.y - min.y, };
}

rect rect_uv_cull(rect quad, rect uv, rect cull_quad) {
    if (!rect_overlaps(quad, cull_quad) || rect_contained_by_rect(quad, cull_quad)) {
        return uv;
    }
    
    bool x_shift_constant = !(quad.x >= cull_quad.x && quad.x <= cull_quad.x + cull_quad.w);
    bool y_shift_constant = !(quad.y >= cull_quad.y && quad.y <= cull_quad.y + cull_quad.h);
    
    f32 uv_xratio = uv.w / quad.w;
    f32 uv_yratio = uv.h / quad.h;
    rect overlap = rect_get_overlap(quad, cull_quad);
    f32 culled_x = uv.x + (quad.w - overlap.w) * uv_xratio * x_shift_constant;
    f32 culled_y = uv.y + (quad.h - overlap.h) * uv_yratio * y_shift_constant;
    f32 culled_w = overlap.w * uv_xratio;
    f32 culled_h = overlap.h * uv_yratio;
    return (rect) { culled_x, culled_y, culled_w, culled_h };
}

Color color_from_hex(u32 color_hex) {
    Color result;
    u32 r = (color_hex >> (3 * 8)) & 0xFF;
    u32 g = (color_hex >> (2 * 8)) & 0xFF;
    u32 b = (color_hex >> (1 * 8)) & 0xFF;
    u32 a = (color_hex >> (0 * 8)) & 0xFF;
    result.r = r / 255.0f;
    result.g = g / 255.0f;
    result.b = b / 255.0f;
    result.a = a / 255.0f;
    return result;
}

// Returns 0 if the path points to a file, 1 if the path points to a directory and -1 if there is an error.
int checkPath(const char *path) {
    struct stat s;
    if (stat(path, &s) == 0) {
        if (s.st_mode & __S_IFREG) {
            return 0;  // It's a file
        } else if (s.st_mode & __S_IFDIR) {
            return 1;  // It's a directory
        } else {
            return -1; // Not a regular file or directory
        }
    } else {
        return -1;  // Error (e.g., file not found)
    }
}

const char *getFileExtFromPath (const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) {
        return "";
    }
    return dot + 1;
}

/* Be sure to call free()! */
char *getFileNameFromPath(const char *path) {
    const char *last_slash = strrchr(path, '/');

    // Need to check for WIN32 here for // slashes

    if (last_slash) {
        path = last_slash + 1;
    }

    const char *last_dot = strrchr(path, '.');
    size_t name_length = (last_dot) ? (size_t)(last_dot - path) : strlen(path);

    char *file_name = malloc(name_length + 1);
    if (!file_name) {
        return NULL;
    }
    strncpy(file_name, path, name_length);
    file_name[name_length] = '\0';

    return file_name;
}

char* get_filename_from_path(const char* filepath) {
    if (filepath == NULL || *filepath == '\0') {
        return NULL;
    }

    // Find the last occurrence of '/' or '\' to get the file name
    const char* last_slash = strrchr(filepath, '/');
    const char* last_backslash = strrchr(filepath, '\\');
    const char* filename = NULL;
    if (last_slash && last_backslash) {
        filename = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
    }   
     
    // If no slashes are found, the whole path is the file name
    if (filename == NULL || *filename == '\0') {
        filename = filepath;
    }

    // Allocate memory for the result and copy the filename
    size_t len = strlen(filename);
    char* result = (char*)malloc(len + 1);  // +1 for the null terminator
    if (result) {
        memcpy(result, filename, len);
        result[len] = '\0';  // Null-terminate the string
    }

    return result;
}

FileType getFileType(const char *file_name, const char *file_ext) {
    if (!strcmp(file_name, "Makefile") || !strcmp(file_name, "makefile")) {
        return FILE_TYPE_MAKEFILE;
    } else if (!strcmp(file_ext, "c") || !strcmp(file_ext, "C") || !strcmp(file_ext, "h") || !strcmp(file_ext, "H")) {
        return FILE_TYPE_C;
    } else if (!strcmp(file_ext, "toml") || !strcmp(file_ext, "TOML")) {
        return FILE_TYPE_TOML;
    } else if (!strcmp(file_ext, "py")) {
        return FILE_TYPE_PYTHON;
    } else {
        return FILE_TYPE_UNKNOWN;
    }
}

// Read into dynamic buffer - no chunking
char *readFile(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        LOG_ERROR("Could not open file \'%s\'", file_path);
        exit(1);
    }
    size_t capacity = 1024;
    char *read_buf = malloc(capacity);
    if (read_buf == NULL) {
        LOG_ERROR("Could allocate space for buffer to read file.", "");
        fclose(f);
        exit(1);
    }
    
    size_t nread;
    size_t size = 0;
    char *temp = NULL;
    while ((nread = fread(read_buf + size, 1, 1024, f)) > 0) {
        size += nread;
        if (size > capacity) {
            capacity *= 2;
            temp = realloc(read_buf, capacity);
            read_buf = temp;
        }
    }
    read_buf[size] = '\0';
    fclose(f);
    return read_buf;
}

static const char *level_strings[] = {
  "DEBUG", "INFO", "WARN", "ERROR",
};

void log_log(i32 level, const char *file, int line, const char *fmt, ...) {
    va_list args;

    fprintf(stderr, "%s %s:%d: ", level_strings[level], file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

f32 lerp (f32 start, f32 end, f32 t) {
    f32 result = start + t * (end - start);
    return result;
}

f32 ease_out (f32 start, f32 end, f32 t) {
    // Cubic ease-out equation
    t = t - 1.0f;  // Adjust t for cubic easing
    f32 eased_t = t * t * t + 1.0f;  // Apply cubic ease-out
    return start + eased_t * (end - start);;
}