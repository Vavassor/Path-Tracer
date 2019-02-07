#include "assert.h"
#include "bmp.h"
#include "random.h"
#include "thread_pool.h"
#include "vector_math.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

typedef union Pack4x8
{
    struct
    {
        uint8_t b, g, r, a;
    };
    uint32_t packed;
} Pack4x8;

typedef union PixelU32
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t value;
} PixelU32;

typedef struct Image
{
    PixelU32* pixels;
    Int2 dimensions;
} Image;

typedef struct Rect
{
    Int2 bottom_left;
    Int2 dimensions;
} Rect;

typedef struct Camera
{
    Float3 position;
    Float3 target;
    float field_of_view;
} Camera;

typedef struct Material
{
    Float3 emissive_colour;
    Float3 reflective_colour;
    float glossiness;
} Material;

typedef struct Plane
{
    Float3 normal;
    float d;
    uint32_t material_index;
} Plane;

typedef struct Ray
{
    Float3 origin;
    Float3 direction;
} Ray;

typedef struct Sphere
{
    Float3 center;
    float radius;
    uint32_t material_index;
} Sphere;

typedef struct World
{
    Material materials[4];
    Plane planes[4];
    Sphere spheres[4];
    int materials_count;
    int planes_count;
    int spheres_count;
} World;

typedef struct Tile
{
    Rect image_region;
    Camera* camera;
    Image* image;
    World* world;
} Tile;

typedef struct MaybeFloat
{
    float value;
    bool valid;
} MaybeFloat;

void image_destroy(Image* image)
{
    deallocate(NULL, image->pixels, sizeof(PixelU32) * image->dimensions.x * image->dimensions.y);
}

static uint32_t pack_unorm3x8(Float3 v)
{
    Pack4x8 u;
    u.r = (uint8_t) (0xff * v.x);
    u.g = (uint8_t) (0xff * v.y);
    u.b = (uint8_t) (0xff * v.z);
    u.a = 0xff;
    return u.packed;
}

static bool is_unorm(float x)
{
    return x >= 0.0f && x <= 1.0f;
}

uint32_t rgb_to_uint32(Float3 c)
{
    ASSERT(is_unorm(c.x));
    ASSERT(is_unorm(c.y));
    ASSERT(is_unorm(c.z));
    return pack_unorm3x8(c);
}

static float linear_to_srgb_component(float x)
{
    if(x <= 0.0031308f)
    {
        return x * 12.92f;
    }
    else
    {
        return 1.055f * powf(x, 1.0f / 2.4f) - 0.055f;
    }
}

static Float3 linear_to_srgb(Float3 colour)
{
    Float3 result;
    result.x = linear_to_srgb_component(colour.x);
    result.y = linear_to_srgb_component(colour.y);
    result.z = linear_to_srgb_component(colour.z);
    return result;
}

MaybeFloat intersect_ray_plane(Ray ray, Plane plane)
{
    MaybeFloat result;
    result.valid = false;

    float d = float3_dot(plane.normal, ray.direction);

    if(fabsf(d) > 1e-6f)
    {
        float t = (-float3_dot(ray.origin, plane.normal) - plane.d) / d;
        result.valid = t >= 0.0f;
        result.value = t;
    }

    return result;
}

MaybeFloat intersect_ray_sphere(Ray ray, Sphere sphere)
{
    MaybeFloat result;
    result.valid = false;

    float radius2 = sphere.radius * sphere.radius;
    Float3 l = float3_subtract(sphere.center, ray.origin);
    float tca = float3_dot(l, ray.direction);

    if(tca < 0.0f)
    {
        return result;
    }

    float d2 = float3_squared_length(l) - (tca * tca);

    if(d2 > radius2)
    {
        return result;
    }

    float thc = sqrtf(radius2 - d2);

    float t[2];
    t[0] = tca - thc;
    t[1] = tca + thc;

    if(t[0] > t[1])
    {
        float temp = t[0];
        t[0] = t[1];
        t[1] = temp;
    }

    if(t[0] < 0.0f)
    {
        t[0] = t[1];

        if(t[0] < 0.0f)
        {
            return result;
        }
    }

    result.valid = true;
    result.value = t[0];

    return result;
}

Float3 get_random_direction(RandomGenerator* generator)
{
    Float3 result;
    result.x = random_float_range(generator, -1.0f, 1.0f);
    result.y = random_float_range(generator, -1.0f, 1.0f);
    result.z = random_float_range(generator, -1.0f, 1.0f);

    return float3_normalise(result);
}

Float3 cast_ray(Ray ray, World* world, RandomGenerator* generator)
{
    Float3 result = float3_zero;
    Float3 attenuation = float3_one;

    for(int ray_count = 0;
            ray_count < 4;
            ray_count += 1)
    {
        const float min_hit_distance = 0.0001f;
        float hit_distance = FLT_MAX;
        int hit_material_index = 0;
        Float3 hit_normal = float3_unit_z;

        for(int plane_index = 0;
                plane_index < world->planes_count;
                plane_index += 1)
        {
            Plane plane = world->planes[plane_index];

            MaybeFloat intersection = intersect_ray_plane(ray, plane);

            if(intersection.valid)
            {
                float distance = intersection.value;

                if(distance > min_hit_distance && distance < hit_distance)
                {
                    hit_material_index = plane.material_index;
                    hit_distance = distance;
                    hit_normal = plane.normal;
                }
            }
        }

        for(int sphere_index = 0;
                sphere_index < world->spheres_count;
                sphere_index += 1)
        {
            Sphere sphere = world->spheres[sphere_index];

            MaybeFloat intersection = intersect_ray_sphere(ray, sphere);

            if(intersection.valid)
            {
                float distance = intersection.value;

                if(distance > min_hit_distance && distance < hit_distance)
                {
                    hit_material_index = sphere.material_index;
                    hit_distance = distance;

                    Float3 hit_point = float3_add(float3_multiply(hit_distance, ray.direction), ray.origin);
                    hit_normal = float3_normalise(float3_subtract(hit_point, sphere.center));
                }
            }
        }

        if(hit_material_index)
        {
            Material material = world->materials[hit_material_index];

            result = float3_add(result, float3_pointwise_multiply(attenuation, material.emissive_colour));
            attenuation = float3_pointwise_multiply(attenuation, material.reflective_colour);

            Float3 pure_bounce = float3_normalise(float3_reflect(ray.direction, hit_normal));
            Float3 random_direction = get_random_direction(generator);
            Float3 scatter_bounce = float3_normalise(float3_add(hit_normal, random_direction));

            ray.origin = float3_add(float3_multiply(hit_distance, ray.direction), ray.origin);
            ray.direction = float3_normalise(float3_lerp(scatter_bounce, pure_bounce, material.glossiness));
        }
        else
        {
            Material material = world->materials[hit_material_index];

            result = float3_add(result, float3_pointwise_multiply(attenuation, material.emissive_colour));

            break;
        }
    }

    return result;
}

void render_tile(void* parameter)
{
    Tile* tile = parameter;
    Image* image = tile->image;

    RandomGenerator generator;
    random_seed_by_time(&generator);

    Rect region = tile->image_region;
    int left = region.bottom_left.x;
    int right = region.bottom_left.x + region.dimensions.x;
    int bottom = region.bottom_left.y;
    int top = region.bottom_left.y + region.dimensions.y;

    Camera* camera = tile->camera;
    Matrix4 view = matrix4_look_at(camera->position, camera->target, float3_unit_z);
    Matrix4 inverse_view = matrix4_inverse_view(view);

    float aspect_ratio = image->dimensions.x / (float) image->dimensions.y;
    float scale_y = tanf(0.5f * camera->field_of_view);
    float scale_x = aspect_ratio * scale_y;

    float half_pixel_width = scale_x * 0.5f / image->dimensions.x;
    float half_pixel_height = scale_y * 0.5f / image->dimensions.y;

    const int samples_per_pixel = 4;

    for(int y = bottom; y < top; y += 1)
    {
        float film_y = 2.0f * ((y + 0.5f) / image->dimensions.y) - 1.0f;
        film_y *= scale_y;

        for(int x = left; x < right; x += 1)
        {
            float film_x = 2.0f * ((x + 0.5f) / image->dimensions.x) - 1.0f;
            film_x *= scale_x;

            Float3 film_point = {film_x, film_y, -1.0f};

            Float3 colour = float3_zero;
            float contribution = 1.0f / samples_per_pixel;

            for(int sample_count = 0;
                    sample_count < samples_per_pixel;
                    sample_count += 1)
            {
                Float3 jitter;
                jitter.x = random_float_range(&generator, -half_pixel_width, half_pixel_width);
                jitter.y = random_float_range(&generator, -half_pixel_height, half_pixel_height);
                jitter.z = 0.0f;

                Float3 jittered_point = float3_add(film_point, jitter);
                Float3 ray_point = matrix4_transform_point(inverse_view, jittered_point);

                Ray ray;
                ray.origin = camera->position;
                ray.direction = float3_normalise(float3_subtract(ray_point, ray.origin));

                Float3 sample = cast_ray(ray, tile->world, &generator);
                colour = float3_add(colour, float3_multiply(contribution, sample));
            }

            Float3 srgb_colour = linear_to_srgb(colour);
            uint32_t pixel_value = rgb_to_uint32(srgb_colour);

            image->pixels[(image->dimensions.x * y) + x].value = pixel_value;
        }
    }
}

int main(int argc, const char** argv)
{
    int cores = get_logical_core_count();

    ThreadPool* pool = thread_pool_create(NULL, cores - 1);

    if(!pool)
    {
        fprintf(stderr, "Pool not created!\n");
    }
    else
    {
        printf("Thread pool created with %i threads.\n", cores - 1);

        Camera camera =
        {
            .position = {0.0f, -5.0f, 1.0f},
            .target = float3_zero,
            .field_of_view = M_PI_4,
        };

        Material background =
        {
            .emissive_colour = {0.3f, 0.4f, 0.5f},
        };

        Material red =
        {
            .reflective_colour = {0.5f, 0.5f, 0.5f},
        };

        Material cyan =
        {
            .reflective_colour = {0.7f, 0.5f, 0.3f},
        };

        Material boyfriend_material =
        {
            .reflective_colour = {0.7f, 0.5f, 0.3f},
            .glossiness = 0.7f,
        };

        Plane plane =
        {
            .normal = float3_unit_z,
            .d = 0.0f,
            .material_index = 1,
        };

        Sphere sphere =
        {
            .center = {1.0f, 0.0f, 1.0f},
            .radius = 1.0f,
            .material_index = 2,
        };

        Sphere small_fella =
        {
            .center = {-1.0f, -2.0f, 0.0f},
            .radius = 0.5f,
            .material_index = 3,
        };

        Sphere yo =
        {
            .center = {-2.0f, 3.0f, 1.5f},
            .radius = 1.0f,
            .material_index = 3,
        };

        Sphere hi =
        {
            .center = {1.0f, -3.0f, 0.5f},
            .radius = 0.6f,
            .material_index = 3,
        };

        World world = {0};
        world.materials_count = 3;
        world.materials[0] = background;
        world.materials[1] = red;
        world.materials[2] = cyan;
        world.materials[3] = boyfriend_material;
        world.planes_count = 1;
        world.planes[0] = plane;
        world.spheres_count = 4;
        world.spheres[0] = sphere;
        world.spheres[1] = small_fella;
        world.spheres[2] = yo;
        world.spheres[3] = hi;

        Image image;
        image.dimensions.x = 1280;
        image.dimensions.y = 720;
        image.pixels = allocate(NULL, sizeof(PixelU32) * image.dimensions.x * image.dimensions.y);

        Tile tiles[16];
        for(int y = 0; y < 4; y += 1)
        {
            for(int x = 0; x < 4; x += 1)
            {
                Tile* tile = &tiles[(4 * y) + x];
                tile->camera = &camera;
                tile->image = &image;
                tile->world = &world;

                Int2 tile_dimensions = int2_divide(image.dimensions, 4);
                Int2 bottom_left = {x, y};
                tile->image_region.bottom_left = int2_pointwise_multiply(bottom_left, tile_dimensions);
                tile->image_region.dimensions = tile_dimensions;
            }
        }

        for(int work_index = 0;
                work_index < 15;
                work_index += 1)
        {
            Task task =
            {
                .call = render_tile,
                .parameter = &tiles[work_index],
            };
            thread_pool_add_task(pool, task);
        }

        render_tile(&tiles[15]);

        thread_pool_wait_all(pool);

        bmp_write_file("test.bmp", (uint8_t*) image.pixels, image.dimensions.x, image.dimensions.y, NULL);

        image_destroy(&image);
    }

    thread_pool_destroy(pool);

    return 0;
}
