#include "sampling_detail.h"

#include "lighting.h"

using namespace glm;
using namespace std;

float Superposition::magic_compute_max_pdf(const DistributionImpl* source, const DistributionImpl* dest){
    assert(source->pdf_integrates_to_one);

    const Transform* transform_dist = dynamic_cast<const Transform*>(dest);
    mat3 transform = transform_dist->transformation;
    vec3 center_dir = transform * vec3(0.0f, 0.0f, 1.0f);

    const LightToDistribution* l2d = dynamic_cast<const LightToDistribution*>(source);
    const Light* light = dynamic_cast<const Light*>(l2d->light.get());
    vec3 origin = l2d->origin;

    if(dynamic_cast<const PointLight*>(light)){
        vec3 dir = normalize(light->origin - origin);
        return dest->pdf(dir);
    }// PointLight
    else if(dynamic_cast<const SphereLight*>(light)){
        const SphereLight* s_light = dynamic_cast<const SphereLight*>(light);
        optional<light_intersection> center_intersection = light->traceRay(origin, center_dir);
        if(center_intersection.has_value())
            return source->pdf(center_dir) * dest->pdf(center_dir);
        // else find closest to the center
        vec3 dir = normalize(light->origin - origin);
        vec3 side = normalize(cross(dir, center_dir));
        vec3 vec_to_center = cross(dir, side);
        vec3 dir_to_max = normalize(light->origin + vec_to_center * s_light->radius - origin);
        return source->pdf(dir_to_max) * dest->pdf(dir_to_max);
    }// SphereLight
    else if(dynamic_cast<const AreaLight*>(light)){
        const AreaLight* a_light = dynamic_cast<const AreaLight*>(light);
        optional<light_intersection> center_intersection = light->traceRay(origin, center_dir);
        if(center_intersection.has_value())
            return source->pdf(center_dir) * dest->pdf(center_dir);
        // else find closest to the center
        vec3 v1 = a_light->origin;
        vec3 v2 = a_light->origin + a_light->x_axis;
        vec3 v3 = a_light->origin + a_light->y_axis;
        vec3 v4 = a_light->type==AreaLight::TYPE_TRIANLE ? vec3() : a_light->origin + a_light->x_axis + a_light->y_axis;

        float pdf, max_pdf = 0.0f;

        pdf = source->pdf(normalize(v1-origin)) * dest->pdf(normalize(v1-origin));
        if(pdf > max_pdf)
            max_pdf = pdf;
        pdf = source->pdf(normalize(v2-origin)) * dest->pdf(normalize(v2-origin));
        if(pdf > max_pdf)
            max_pdf = pdf;
        pdf = source->pdf(normalize(v3-origin)) * dest->pdf(normalize(v3-origin));
        if(pdf > max_pdf)
            max_pdf = pdf;
        pdf = source->pdf(normalize(v4-origin)) * dest->pdf(normalize(v4-origin));
        if(v4 != vec3() && pdf > max_pdf)
            max_pdf = pdf;

        // TODO Compute here also mid-side values: see ray2_closest_point()

        return max_pdf;
    }// AreaLight
}
