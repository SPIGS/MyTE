#include "pmath.h"
#include "math.h"


f32 lerpF(f32 a, f32 b, f32 t) { return a + (b - a) * t; }

f32 easeOutF (f32 start, f32 end, f32 t) {
    t = t - 1.0f;
    f32 eased_t = t * t * t + 1.0f;
    return start + eased_t * (end - start);
}

f64 degToRad(f32 deg) {return (f64) (deg * 0.0174532925f); }
f32 radToDeg(f64 rad) {return (f32) (rad * 57.2957795131); }

/* Vector2 */

Vector2 vec2(f32 x, f32 y) {
  return (Vector2) { x, y };
}

Vector2 vec2Add(Vector2 a, Vector2 b) {
  return (Vector2) { a.x + b.x, a.y + b.y };
}

Vector2 vec2Sub(Vector2 a, Vector2 b) {
  return (Vector2) { a.x - b.x, a.y - b.y };
}

Vector2 vec2Scale(Vector2 a, f32 s) {
  return (Vector2) {a.x * s, a.y * s};
}

f32 vec2Mag(Vector2 v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

Vector2 vec2Norm(Vector2 v) {
  float mag = vec2Mag(v);
  Vector2 result = (Vector2) {0, 0};

  if (mag > 0) {
    result = (Vector2) {v.x / mag, v.y / mag};
  }

  return result;
}

f32 vec2Dot(Vector2 a, Vector2 b) {
  return a.x * b.x + a.y * b.y;
}

Vector2 vec2Lerp(Vector2 a, Vector2 b, f32 t) {
  t = CLAMP(0,t,1);

  return (Vector2) {
    a.x + (b.x - a.x) * t,
    a.y + (b.y - a.y) * t
  };
}

Vector2 vec2EaseOut(Vector2 start, Vector2 end, f32 t) {
  t = t - 1.0f;
  f32 eased_t = t * t * t + 1.0f;

  return vec2(
    start.x + eased_t * (end.x - start.x), 
    start.y + eased_t * (end.y - start.y)
  );
}

/* Vector 3 */

Vector3 vec3(f32 x, f32 y, f32 z) {
  return (Vector3) { x, y, z };
}

Vector3 vec3Add(Vector3 a, Vector3 b) {
  return (Vector3) {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 vec3Sub(Vector3 a, Vector3 b) {
  return (Vector3) {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 vec3Scale(Vector3 a, f32 s) {
  return (Vector3) {a.x * s, a.y * s, a.z * s};
}

f32 vec3Mag(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 vec3Norm(Vector3 v) {
    float mag = vec3Mag(v);
    Vector3 result = (Vector3) {0, 0, 0};
    if (mag > 0) {
        result = (Vector3) {v.x / mag, v.y / mag, v.z / mag};
    }
  return result;
}

f32 vec3Dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 vec3Cross(Vector3 a, Vector3 b) {
   return (Vector3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vector3 vec3Lerp(Vector3 a, Vector3 b, f32 t) {
  t = CLAMP (0, t, 1);

  return (Vector3) {
      a.x + (b.x - a.x) * t,
      a.y + (b.y - a.y) * t,
      a.z + (b.z - a.z) * t
  };
}

/* Vector 4 */

Vector4 vec4(f32 x, f32 y, f32 z, f32 w) {
  return (Vector4) { x, y, z, w};
}

Vector4 vec4Add(Vector4 a, Vector4 b) {
  return (Vector4) {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

Vector4 vec4Sub(Vector4 a, Vector4 b) {
  return (Vector4) {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

Vector4 vec4Scale(Vector4 a, f32 s) {
  return (Vector4) {a.x * s, a.y * s, a.z * s, a.w * s};
}

f32 vec4Mag(Vector4 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

Vector4 vec4Norm(Vector4 v) {
    f32 mag = vec4Mag(v);
    Vector4 result = (Vector4) {0, 0, 0, 0};
    if (mag > 0) {
        result = (Vector4) {v.x / mag, v.y / mag, v.z / mag, v.w / mag};
    }
    return result;
}

f32 vec4Dot(Vector4 a, Vector4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vector4 vec4Lerp(Vector4 a, Vector4 b, f32 t) {
  t = CLAMP (0, t, 1);

  return (Vector4) {
    a.x + (b.x - a.x) * t,
    a.y + (b.y - a.y) * t,
    a.z + (b.z - a.z) * t,
    a.w + (b.w - a.w) * t
  };
}

/* Rect */
Rect rect(f32 x, f32 y, f32 w, f32 h) {
  return (Rect) { x, y, w, h };
}

bool rectContainsPoint(Rect a, Vector2 p) {
  return a.x <= p.x && a.y <= p.y && a.x + a.w >= p.x && a.y + a.h >= p.y;
}

bool rectOverlaps(Rect a, Rect b) {
  bool x = (a.x >= b.x && a.x <= b.x + b.w) || (a.x + a.w >= b.x && a.x + a.w <= b.x + b.w) || (a.x <= b.x && a.x + a.w >= b.x + b.w);
  bool y = (a.y >= b.y && a.y <= b.y + b.h) || (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h) || (a.y <= b.y && a.y + a.h >= b.y + b.h);
  return x && y;
}

bool rectContainedByRect(Rect a, Rect b) {
    bool x = (a.x >= b.x && a.x <= b.x + b.w) && (a.x + a.w >= b.x && a.x + a.w <= b.x + b.w);
    bool y = (a.y >= b.y && a.y <= b.y + b.h) && (a.y + a.h >= b.y && a.y + a.h <= b.y + b.h);
    return x && y;
}

/* Matrix4 */
Matrix4 mat4Identity(void) {
  Matrix4 result = {0};
  result.a[0] = 1.0f;
  result.a[5] = 1.0f;
  result.a[10] = 1.0f;
  result.a[15] = 1.0f;
  return result;
}

Matrix4 mat4Mul(Matrix4 a, Matrix4 b) {
  Matrix4 result = {0};
  for (i32 row = 0; row < 4; row++) {
      for (i32 col = 0; col < 4; col++) {
          float sum = 0.0f;
          for (i32 i = 0; i < 4; i++) {
              sum += a.a[row * 4 + i] * b.a[i * 4 + col];
          }
          result.a[row * 4 + col] = sum;
      }
  }
  return result;
}

Matrix4 mat4Transpose(Matrix4 m) {
    Matrix4 result = {0};

    for (i32 row = 0; row < 4; row++) {
        for (i32 col = 0; col < 4; col++) {
            result.a[col * 4 + row] = m.a[row * 4 + col];
        }
    }
    return result;
}

Matrix4 mat4Translate(Vector3 v) {
    Matrix4 result = mat4Identity();
    result.a[12] = v.x;
    result.a[13] = v.y;
    result.a[14] = v.z;
    return result;
}

Matrix4 mat4Scale(Vector3 v) {
    Matrix4 result = {0};
    result.a[0] = v.x;
    result.a[5] = v.y;
    result.a[10] = v.z;
    result.a[15] = 1.0f;
    return result;
}

Matrix4 mat4RotX(f32 deg) {
    Matrix4 result = mat4Identity();
    f32 rad = degToRad(deg);
    f32 c = cosf(rad);
    f32 s = sinf(rad);
    
    result.a[5] = c;
    result.a[6] = s;
    result.a[9] = -s;
    result.a[10] = c;
    
    return result;
}

Matrix4 mat4RotY(f32 deg) {
    Matrix4 result = mat4Identity();
    f32 rad = degToRad(deg);
    f32 c = cosf(rad);
    f32 s = sinf(rad);
    
    result.a[0] = c;
    result.a[2] = -s;
    result.a[8] = s;
    result.a[10] = c;
    
    return result;
}

Matrix4 mat4RotZ(f32 deg) {
    Matrix4 result = mat4Identity();
    f32 rad = degToRad(deg);
    f32 c = cosf(rad);
    f32 s = sinf(rad);
    
    result.a[0] = c;
    result.a[1] = s;
    result.a[4] = -s;
    result.a[5] = c;
    
    return result;
}

Matrix4 orthoProj(f32 left, f32 right, f32 top,f32 bottom, f32 near, f32 far) {
    Matrix4 result = {0};
    
    f32 rl = right - left;
    f32 tb = top - bottom;
    f32 fn = far - near;
    
    result.a[0] = 2.0f / rl;
    result.a[5] = 2.0f / tb;
    result.a[10] = -2.0f / fn;
    result.a[12] = -(right + left) / rl;
    result.a[13] = -(top + bottom) / tb;
    result.a[14] = -(far + near) / fn;
    result.a[15] = 1.0f;
    
    return result;
}

Matrix4 perspectProj(f32 fov, f32 aspect_ratio, f32 near, f32 far) {
    Matrix4 result = {0};
    
    f32 tanHalfFov = tanf((fov * 0.5f) * (PI / 180.0f));
    f32 fn = far - near;
    
    result.a[0] = 1.0f / (aspect_ratio * tanHalfFov);
    result.a[5] = 1.0f / tanHalfFov;
    result.a[10] = -(far + near) / fn;
    result.a[11] = -1.0f;
    result.a[14] = -(2.0f * far * near) / fn;
    
    return result;
}

