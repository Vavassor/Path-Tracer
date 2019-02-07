#ifndef VECTOR_MATH_H_
#define VECTOR_MATH_H_

typedef union Float2
{
    struct
    {
        float x;
        float y;
    };

    float e[2];
} Float2;

typedef union Float3
{
    struct
    {
        float x;
        float y;
        float z;
    };

    float e[3];
} Float3;

typedef struct Int2
{
    int x;
    int y;
} Int2;

typedef struct Matrix4
{
    float e[16]; // elements in row-major order
} Matrix4;


float float2_squared_length(Float2 a);
float float2_squared_distance(Float2 a, Float2 b);
Float2 float2_subtract(Float2 a, Float2 b);


Float3 float3_add(Float3 a, Float3 b);
Float3 float3_cross(Float3 a, Float3 b);
Float3 float3_divide(Float3 v, float s);
float float3_dot(Float3 a, Float3 b);
float float3_length(Float3 v);
Float3 float3_lerp(Float3 a, Float3 b, float t);
Float3 float3_multiply(float s, Float3 v);
Float3 float3_negate(Float3 v);
Float3 float3_normalise(Float3 v);
Float3 float3_pointwise_multiply(Float3 a, Float3 b);
Float3 float3_reflect(Float3 incident, Float3 normal);
float float3_squared_length(Float3 v);
Float3 float3_subtract(Float3 a, Float3 b);


Int2 int2_divide(Int2 a, int b);
Int2 int2_pointwise_multiply(Int2 a, Int2 b);

Matrix4 matrix4_inverse_view(Matrix4 m);
Matrix4 matrix4_look_at(Float3 position, Float3 target, Float3 world_up);
Float3 matrix4_transform_point(Matrix4 m, Float3 v);
Float3 matrix4_transform_vector(Matrix4 m, Float3 v);
Matrix4 matrix4_view(Float3 x_axis, Float3 y_axis, Float3 z_axis, Float3 position);


extern const Float3 float3_zero;
extern const Float3 float3_one;
extern const Float3 float3_unit_x;
extern const Float3 float3_unit_y;
extern const Float3 float3_unit_z;

#endif // VECTOR_MATH_H_
