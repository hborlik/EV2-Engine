#include "tree.h"

#include <utility>

#include "game.h"
#include "skinning.hpp"
#include "glm/fwd.hpp"

/**
 * @brief Monopodial tree-like structures of Honda
 *  based on pg. 56 of Algorithmic Botany
 * 
 */
namespace monopodial {

using namespace ptree;

constexpr uint32_t S_A      = ptree::TurtleCommands::S_A;
constexpr uint32_t S_B      = S_A + 1;
constexpr uint32_t S_C      = S_B + 1;

const std::map<std::string, float> DefaultParamsA = {
    {"R_1", 0.9f},
    {"R_2", 0.6f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsB = {
    {"R_1", 0.9f},
    {"R_2", 0.9f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsC = {
    {"R_1", 0.9f},
    {"R_2", 0.8f},
    {"a_0", ptree::degToRad(45)},
    {"a_2", ptree::degToRad(45)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

const std::map<std::string, float> DefaultParamsD = {
    {"R_1", 0.9f},
    {"R_2", 0.7f},
    {"a_0", ptree::degToRad(30)},
    {"a_2", ptree::degToRad(-30)},
    {"d",   ptree::degToRad(137.5f)},
    {"w_r", 0.707f}
};

constexpr Symbol<TreeSymbol> Axiom = {S_A, {2, 0.5}};

struct MonopodialProduction : public Production<TreeSymbol> {

    const float R_1   = 0.9f;             /* contraction ratio for the trunk */
    const float R_2   = 0.7f;             /* contraction ratio for the branches */
    const float a_0   = ptree::degToRad(30);     /* branching angle from the trunk */
    const float a_2   = ptree::degToRad(-30);     /* branching angle for the lateral axes */
    const float d     = ptree::degToRad(137.5f); /* divergence angle */
    const float w_r   = 0.707f;           /* width decrease rate */

    MonopodialProduction() = default;

    MonopodialProduction(float p, uint32_t sym) : Production<TreeSymbol>{p, sym} {}

    MonopodialProduction(const std::map<std::string, float> &param, float p, uint32_t sym) : Production<TreeSymbol>{p, sym},
        R_1{param.at("R_1")},
        R_2{param.at("R_2")},
        a_0{param.at("a_0")}, 
        a_2{param.at("a_2")},
        d{param.at("d")},
        w_r{param.at("w_r")}
    {
    }

    bool matches(const SymbolN<TreeSymbol>& sym) const override {
        return sym.center()->RepSym == this->A;
    }
};


struct P_1 : public MonopodialProduction {

    P_1() : MonopodialProduction{1.0f, S_A} {}
    P_1(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_A} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(l) [ &(a0) B(l * R_2, w * w_r) ] /(d) A(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_pitch, {a_0}});
            ret.push_back({S_B, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({TurtleCommands::S_roll, {d}});
            ret.push_back({S_A, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

struct P_2 : public MonopodialProduction {
    P_2() : MonopodialProduction{1.0f, S_B} {}
    P_2(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_B} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(L) [ -(a_2) $ C(l * R_2, w * w_r) ] C(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_yaw, {-a_2}});
            ret.push_back({TurtleCommands::S_Dollar});
            ret.push_back({S_C, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({S_C, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

struct P_3 : public MonopodialProduction {
    P_3() : MonopodialProduction{1.0f, S_C} {}
    P_3(const std::map<std::string, float> &param) : MonopodialProduction{param, 1.0f, S_C} {}

    SymbolString<TreeSymbol> translate(const SymbolN<TreeSymbol>& sym) const override {
        const TreeSymbol& value = sym.center()->value;
        float L = value.l;
        float W = value.w;
        SymbolString<TreeSymbol> ret{};
        if (matches(sym)) {
            // !(w) F(L) [ +(a_2) $ B(l * R_2, w * w_r) ] B(l * R_1, w * w_r)
            ret.push_back({TurtleCommands::SetWidth, {W}});
            ret.push_back({TurtleCommands::S_forward, {L}});
            ret.push_back({TurtleCommands::S_push});
            ret.push_back({TurtleCommands::S_yaw, {a_2}});
            ret.push_back({TurtleCommands::S_Dollar});
            ret.push_back({S_B, {L * R_2, W * w_r}});
            ret.push_back({TurtleCommands::S_pop});
            ret.push_back({S_B, {L * R_1, W * w_r}});
        }
        return ret;
    }
};

}

TreeNode::TreeNode(GameState* game, const std::string& name, bool has_leafs, int u_id, std::shared_ptr<ev2::renderer::Material> leaf_material) : 
    ev2::VisualInstance{name}, 
    game{game}, 
    has_leafs{has_leafs}, 
    leaf_material{leaf_material} {

    buffer_layout.add_attribute(ev2::renderer::VertexAttributeLabel::Vertex)
        .add_attribute(ev2::renderer::VertexAttributeLabel::Normal)
        .add_attribute(ev2::renderer::VertexAttributeLabel::Color)
        .finalize();
    this->plantInfo.ID = u_id;

    params = monopodial::DefaultParamsA;
}

void TreeNode::on_init() {
    ev2::VisualInstance::on_init();

    if (has_leafs) {

        if (!leaf_material) {
            leaf_material = ev2::renderer::Renderer::get_singleton().create_material();
            leaf_material->diffuse = glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
            leaf_material->emissive = randomFloatRange(0.0001, 1.) * glm::vec3{randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.), randomFloatRange(0.0001, 1.)};
            leaf_material->metallic = randomFloatRange(0.0001, 0.3);
            leaf_material->subsurface = randomFloatRange(0.5, 1);
            leaf_material->specular = randomFloatRange(0.0001, 0.2);;
            leaf_material->roughness = randomFloatRange(0.0001, 1);;
            leaf_material->specularTint = 0.f;
            leaf_material->clearcoat = randomFloatRange(0.0001, 1.0);;
            leaf_material->clearcoatGloss = 0.63;
            leaf_material->sheen = randomFloatRange(0.0001, 0.8);;
            leaf_material->sheenTint = 0.5f;
            leaf_material->diffuse_tex = ResourceManager::get_singleton().get_image("coffee_leaf1.png")->texture;
        }

        leafs = create_child_node<ev2::InstancedGeometry>("leafs");
        leafs->set_material_override(leaf_material);
    }

    tree_geometry = std::make_shared<ev2::renderer::Drawable>(
        ev2::renderer::VertexBuffer::vbInitArrayVertexSpecIndexed({}, {}, buffer_layout),
        std::vector<ev2::renderer::Primitive>{},
        std::vector<std::shared_ptr<ev2::renderer::Material>>{},
        AABB{},
        Sphere{glm::vec3{0.f}, 1.f}, // TODO: make sphere correct
        renderer::FrustumCull::Sphere,
        ev2::gl::CullMode::BACK,
        ev2::gl::FrontFacing::CCW
    );
    tree_geometry->vertex_color_weight = 1.0f;
    set_model(tree_geometry);
    generate(5);
}

void TreeNode::on_destroy() {
    VisualInstance::on_destroy();
}

void TreeNode::setParams(const std::map<std::string, float>& paramsNew, int iterations, float growth) {
    growth_current = growth;
    auto temp_params = paramsNew;
    if (growth_current < growth_max)
    {
        temp_params["thickness"] *= growth_current;
        temp_params["w_r"] *= growth_current;
        temp_params["R_1"] *= growth_current;
        temp_params["R_2"] *= growth_current;
    }

    thickness = temp_params.at("thickness");
    params = temp_params;
    this->generate((iterations * growth_current) + 1);
    params = paramsNew; // save target parameters after generation of vertex buffers

    /*
    fruit_params.n1 = params.at("n1");
    fruit_params.n2 = params.at("n2");
    fruit_params.n3 = params.at("n3");
    fruit_params.m  = params.at("m");
    fruit_params.a  = params.at("a");
    fruit_params.b  = params.at("b");
    fruit_params.q1 = params.at("q1");
    fruit_params.q2 = params.at("q2");
    fruit_params.q3 = params.at("q3");
    fruit_params.k  = params.at("k");
    fruit_params.c  = params.at("c");
    fruit_params.d  = params.at("d_f");
    */
}

void TreeNode::generate(int iterations) {
    using namespace monopodial;
    LSystemTr<TreeSymbol> mlsys{};

    P_1 p1{params};
    P_2 p2{params};
    P_3 p3{params};

    mlsys.add_rule(&p1);
    mlsys.add_rule(&p2);
    mlsys.add_rule(&p3);

    SymbolString<TreeSymbol> str{Axiom};

    for (int i = 0; i < iterations; ++i) {
        str = mlsys.evaluate(str);
    }

    if (tree.from_symbol_string(str)) {
        // tree.simple_skeleton(5);
        tree_skeleton = tree.to_skeleton();

        if (leafs) {
            std::vector<glm::mat4> instance_transforms{};
            instance_transforms.reserve(tree_skeleton.endpoints.size());
            const glm::mat4 rot_leaf = glm::mat4(glm::rotate<float>(glm::identity<glm::quat>(), M_PI / 2.1f, glm::vec3{1, 0, 0}));
            for (const auto& ind : tree_skeleton.endpoints) {
                glm::mat4 tr = glm::translate(glm::identity<glm::mat4>(), tree_skeleton.joints[ind].position) 
                    * glm::mat4(glm::quatLookAt(tree_skeleton.joints[ind].tangent, glm::vec3{0, 1, 0}))
                    * rot_leaf
                    * glm::translate(glm::identity<glm::mat4>(), {0, 2 * leaf_scale * growth_current * -0.05, 0})
                    * glm::scale(glm::identity<glm::mat4>(), glm::vec3{leaf_scale * growth_current, 2 * leaf_scale * growth_current, 1.0f});
                instance_transforms.push_back(tr);
            }
            leafs->set_instance_transforms(std::move(instance_transforms));
        }

        ptree::DefaultColorizer dc{c0, c1, tree.max_joint_depth};

        std::vector<ptree::Vertex> vertices;
        std::vector<uint32_t> indices;
        ptree::Skin_GO(5, tree_skeleton, vertices, indices, false, thickness, &dc);

        std::vector<PNVertex> g_vertices(vertices.size());
        for (int i =0; i < vertices.size(); ++i) {
            auto& sv = vertices[i];
            g_vertices[i].position = sv.pos;
            g_vertices[i].normal = sv.normal;
            g_vertices[i].color = sv.color;
        }

        tree_geometry->vertex_buffer.get_buffer(0)->copy_data(g_vertices);
        tree_geometry->vertex_buffer.get_buffer(tree_geometry->vertex_buffer.get_indexed())->copy_data(indices);

        tree_geometry->primitives.clear();
        tree_geometry->primitives.push_back(ev2::renderer::Primitive{0, indices.size(), -1});
    }
}


FireFlies::FireFlies(GameState* game, const std::string& name, int32_t n_flies) : ev2::Node{name}, NFlies{n_flies} {

}

void FireFlies::on_init() {
    auto material = ResourceManager::get_singleton().get_material("flies_material");
    material->emissive = glm::vec3{10, 10, 5};
    flies = create_child_node<ev2::InstancedGeometry>("flies");
    flies->set_material_override(material);
    
    for (int i = 0; i < NFlies; i++) {
        Particle p{};
        p.m_fSize = 0.9f;
        p.m_fAge = randomFloatRange(0, 5);
        p.m_Position = glm::vec3{randomFloatRange(-50, 50), randomFloatRange(0, 15), randomFloatRange(-50, 50)};
        p.m_fLifeTime = 10.0f;
        particles.push_back(p);
    }
}

void FireFlies::on_process(float dt) {
    std::vector<glm::mat4> instance_transforms(NFlies);
    int i = 0;
    for (auto& p : particles) {
        p.m_fAge += dt;
        p.m_Velocity = glm::vec3(glm::sin(p.m_fAge) + glm::sin(2 * p.m_Position.y + randomFloatRange(-1, 1)), glm::cos(p.m_fAge) + glm::sin(randomFloatRange(-2, 2)), 0);
        p.m_Position += dt * p.m_Velocity;
        instance_transforms[i++] = p.particle_transform();
    }
    flies->set_instance_transforms(std::move(instance_transforms));
}