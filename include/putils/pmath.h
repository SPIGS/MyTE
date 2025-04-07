#pragma once
#include "defines.h"

#define PI 3.14159265389f

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(a,x,b) (((x)<(a))?(a):((b)<(x))?(b):(x))

f32 lerpF(f32 a, f32 b, f32 t);
f64 degToRad(f32 deg);
f32 radToDeg(f64 rad);

/* Vector 2 */
typedef struct { f32 x, y; } Vector2;
#define Vec2Fmt "Vec2(%f, %f)"
#define Vec2Arg(v) (float)(v).x, (float)(v).y
Vector2 vec2(f32 x, f32 y);
Vector2 vec2Add(Vector2 a, Vector2 b);
Vector2 vec2Sub(Vector2 a, Vector2 b);
Vector2 vec2Scale(Vector2 a, f32 s);
f32 vec2Mag(Vector2 v);
Vector2 vec2Norm(Vector2 v);
f32 vec2Dot(Vector2 a, Vector2 b);
Vector2 vec2Lerp(Vector2 a, Vector2 b, f32 t);

/* Vector 3 */
typedef struct { f32 x, y, z; } Vector3;
#define Vec3Fmt "Vec3(%f, %f, %f)"
#define Vec3Arg(v) (float)(v).x, (float)(v).y, (float)(v).z
Vector3 vec3(f32 x, f32 y, f32 z);
Vector3 vec3Add(Vector3 a, Vector3 b);
Vector3 vec3Sub(Vector3 a, Vector3 b);
Vector3 vec3Scale(Vector3 a, f32 s);
f32 vec3Mag(Vector3 v);
Vector3 vec3Norm (Vector3 v);
f32 vec3Dot (Vector3 a, Vector3 b);
Vector3 vec3Cross(Vector3 a, Vector3 b);
Vector3 vec3Lerp(Vector3 a, Vector3 b, f32 t);

/* Vector 4 */
typedef struct { f32 x, y, z, w; } Vector4;
#define Vec4Fmt "Vec4(%f, %f, %f, %f)"
#define Vec4Arg(v) (float)(v).x, (float)(v).y, (float)(v).z, (float)(v).w
Vector4 vec4(f32 x, f32 y, f32 z, f32 w);
Vector4 vec4Add(Vector4 a, Vector4 b);
Vector4 vec4Sub(Vector4 a, Vector4 b);
Vector4 vec4Scale(Vector4 a, f32 s);
f32 vec4Mag(Vector4 v);
Vector4 vec4Norm(Vector4 v);
f32 vec4Dot(Vector4 a, Vector4 b);
Vector4 vec4Lerp(Vector4 a, Vector4 b, f32 t);

/* 3x3 Matrix */
/*typedef struct { f32 a[3*3]; } Matrix3;*/
/*static inline Matrix3 mat3Identity();*/

/* Rect */
typedef struct { f32 x, y, w, h; } Rect;
#define RectFmt "Rect(%f, %f, %f, %f)"
#define RectArg(r) (float)(r).x, (float)(r).y, (float)(r).w, (float)(r).h
Rect rect(f32 x, f32 y, f32 w, f32 h);
bool rectContainsPoint(Rect a, Vector2 p);
bool rectOverlaps(Rect a, Rect b);
bool rectContainedByRect(Rect a, Rect b);



/* 4x4 Matrix */
typedef struct { f32 a[4*4]; } Matrix4;
Matrix4 mat4Identity(void);
Matrix4 mat4Mul(Matrix4 a, Matrix4 b);
Matrix4 mat4Transpose(Matrix4 m);
Matrix4 mat4Translate(Vector3 v);
Matrix4 mat4Scale(Vector3 v);
Matrix4 mat4RotX(f32 deg);
Matrix4 mat4RotY(f32 deg);
Matrix4 mat4RotZ(f32 deg);
Matrix4 orthoProj(f32 left, f32 right, f32 top,f32 bottom, f32 near, f32 far);
Matrix4 perspectProj(f32 fov, f32 aspect_ratio, f32 near, f32 far);

