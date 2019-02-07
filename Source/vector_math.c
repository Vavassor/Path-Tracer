#include "vector_math.h"

#include "assert.h"

#include <math.h>

static float lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b;
}


float float2_squared_length(Float2 a)
{
    return (a.x * a.x) + (a.y * a.y);
}

float float2_squared_distance(Float2 a, Float2 b)
{
    return float2_squared_length(float2_subtract(a, b));
}

Float2 float2_subtract(Float2 a, Float2 b)
{
    Float2 result = {a.x - b.x, a.y - b.y};
    return result;
}


Float3 float3_add(Float3 a, Float3 b)
{
    Float3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

Float3 float3_cross(Float3 a, Float3 b)
{
    Float3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}

Float3 float3_divide(Float3 v, float s)
{
    ASSERT(s != 0.0f && isfinite(s));
    Float3 result = {v.x / s, v.y / s, v.z / s};
    return result;
}

float float3_dot(Float3 a, Float3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float float3_length(Float3 v)
{
    return sqrtf(float3_squared_length(v));
}

Float3 float3_lerp(Float3 a, Float3 b, float t)
{
    Float3 result;
    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);
    result.z = lerp(a.z, b.z, t);
    return result;
}

Float3 float3_multiply(float s, Float3 v)
{
    Float3 result = {s * v.x, s * v.y, s * v.z};
    return result;
}

Float3 float3_negate(Float3 v)
{
    Float3 result = {-v.x, -v.y, -v.z};
    return result;
}

Float3 float3_normalise(Float3 v)
{
    float l = float3_length(v);
    return float3_divide(v, l);
}

Float3 float3_pointwise_multiply(Float3 a, Float3 b)
{
    Float3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

Float3 float3_reflect(Float3 incident, Float3 normal)
{
    float d = 2.0f * float3_dot(incident, normal);
    Float3 result = float3_subtract(incident, float3_multiply(d, normal));

    return result;
}

float float3_squared_length(Float3 v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

Float3 float3_subtract(Float3 a, Float3 b)
{
    Float3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}


Int2 int2_divide(Int2 a, int b)
{
    ASSERT(b != 0);
    Int2 result = {a.x / b, a.y / b};
    return result;
}

Int2 int2_pointwise_multiply(Int2 a, Int2 b)
{
    Int2 result = {a.x * b.x, a.y * b.y};
    return result;
}


Matrix4 matrix4_inverse_view(Matrix4 m)
{
    float a = -((m.e[0] * m.e[3]) + (m.e[4] * m.e[7]) + (m.e[8]  * m.e[11]));
    float b = -((m.e[1] * m.e[3]) + (m.e[5] * m.e[7]) + (m.e[9]  * m.e[11]));
    float c = -((m.e[2] * m.e[3]) + (m.e[6] * m.e[7]) + (m.e[10] * m.e[11]));

    Matrix4 result =
    {
        m.e[0], m.e[4], m.e[8],  a,
        m.e[1], m.e[5], m.e[9],  b,
        m.e[2], m.e[6], m.e[10], c,
        0.0f,   0.0f,   0.0f,    1.0f
    };

    return result;
}

// This function assumes a right-handed coordinate system.
Matrix4 matrix4_look_at(Float3 position, Float3 target, Float3 world_up)
{
    Float3 forward = float3_normalise(float3_subtract(position, target));
    Float3 right = float3_normalise(float3_cross(world_up, forward));
    Float3 up = float3_normalise(float3_cross(forward, right));
    return matrix4_view(right, up, forward, position);
}

Float3 matrix4_transform_point(Matrix4 m, Float3 v)
{
    float a = (m.e[12] * v.x) + (m.e[13] * v.y) + (m.e[14] * v.z) + m.e[15];

    Float3 result;
    result.x = ((m.e[0] * v.x) + (m.e[1] * v.y) + (m.e[2]  * v.z) + m.e[3])  / a;
    result.y = ((m.e[4] * v.x) + (m.e[5] * v.y) + (m.e[6]  * v.z) + m.e[7])  / a;
    result.z = ((m.e[8] * v.x) + (m.e[9] * v.y) + (m.e[10] * v.z) + m.e[11]) / a;
    return result;
}

Float3 matrix4_transform_vector(Matrix4 m, Float3 v)
{
    float a = (m.e[12] * v.x) + (m.e[13] * v.y) + (m.e[14] * v.z) + 1.0f;

    Float3 result;
    result.x = ((m.e[0] * v.x) + (m.e[1] * v.y) + (m.e[2]  * v.z)) / a;
    result.y = ((m.e[4] * v.x) + (m.e[5] * v.y) + (m.e[6]  * v.z)) / a;
    result.z = ((m.e[8] * v.x) + (m.e[9] * v.y) + (m.e[10] * v.z)) / a;
    return result;
}

Matrix4 matrix4_view(Float3 x_axis, Float3 y_axis, Float3 z_axis, Float3 position)
{
    Matrix4 result;

    result.e[0]  = x_axis.x;
    result.e[1]  = x_axis.y;
    result.e[2]  = x_axis.z;
    result.e[3]  = -float3_dot(x_axis, position);

    result.e[4]  = y_axis.x;
    result.e[5]  = y_axis.y;
    result.e[6]  = y_axis.z;
    result.e[7]  = -float3_dot(y_axis, position);

    result.e[8]  = z_axis.x;
    result.e[9]  = z_axis.y;
    result.e[10] = z_axis.z;
    result.e[11] = -float3_dot(z_axis, position);

    result.e[12] = 0.0f;
    result.e[13] = 0.0f;
    result.e[14] = 0.0f;
    result.e[15] = 1.0f;

    return result;
}


const Float3 float3_zero   = {0.0f, 0.0f, 0.0f};
const Float3 float3_one    = {1.0f, 1.0f, 1.0f};
const Float3 float3_unit_x = {1.0f, 0.0f, 0.0f};
const Float3 float3_unit_y = {0.0f, 1.0f, 0.0f};
const Float3 float3_unit_z = {0.0f, 0.0f, 1.0f};
