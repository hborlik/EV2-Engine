#include <physics.h>

namespace ev2 {

CID Physics::create_sphere_collider(Ref<Object> owner) {
    int32_t id = next_sphere_id++;
    SphereInternal si{};
    si.owner = owner;
    auto ins = sphere_colldiers.emplace(id, si);
    if (ins.second)
        return {id};
    return {};
}

void Physics::set_sphere_collider(CID cid, Sphere s) {
    if (!cid.is_valid())
        return;
    
    auto mi = sphere_colldiers.find(cid.v);
    if (mi != sphere_colldiers.end()) {
        mi->second.sphere = s;
    }
}

Sphere& Physics::get_sphere_collider(CID cid) {
    auto mi = sphere_colldiers.find(cid.v);
    if (mi != sphere_colldiers.end()) {
        return mi->second.sphere;
    }
}

void Physics::destroy_sphere_collider(CID cid) {
    sphere_colldiers.erase(cid.v);
}

void Physics::pre_render() {

}

std::optional<SurfaceInteraction> Physics::raycast_scene(const Ray& ray) {
    float nearest = INFINITY;
    SurfaceInteraction closest_hit{};
    for (auto& si : sphere_colldiers) {
        SurfaceInteraction sfi{};
        if (intersect(ray, si.second.sphere, sfi)) {
            if (sfi.t < nearest) {
                nearest = sfi.t;
                closest_hit = sfi;
                closest_hit.hit = si.second.owner;
            }
        }
    }
    if (closest_hit.hit.get()) {
        return {closest_hit};
    }
    return {};
}

// Collider

void Collider::on_init() {
    cid = Physics::get_singleton().create_sphere_collider(get_ref());
}

void Collider::on_ready() {

}

void Collider::on_process(float delta) {
    get_shape().center = glm::vec3(get_transform()[3]);
}

void Collider::on_destroy() {

}

Sphere& Collider::get_shape() {
    return Physics::get_singleton().get_sphere_collider(cid);
}

}