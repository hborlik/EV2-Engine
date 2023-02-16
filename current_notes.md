## Thesis Notes

Modeling and rendering are two core research problems in computer graphics. Show how this tool can add to existing work. How it integrates with another project. Can find a paper to build on.

!!!WFC!!!

1. Break input image into tiles
2. Interpret tiles as values that can appear in a given neighborhood 
3. Build adjacency matrix (neighborhoods and values)
   1. associated weight for how often they appear in the input image
Jan 17 thoughts: Volume based WFC for objects and their neighboring objects (classes of object). For interior or exterior scenes. Eg garbage cans and nearby garbage on the ground. WFC allows for probabilistic/constrain driven modeling. Placement rules can drive orientation. Contributions would be WFC that can expand to accommodate objects with more positions that need to be evaluated.

Notes about good papers:
[example](https://ieeexplore-ieee-org.ezproxy.lib.calpoly.edu/stamp/stamp.jsp?tp=&arnumber=9709532)
2-3 pages on results, Introduction clearly states contributions and motivations. 

## Abstract
This thesis presents a tool that interactively generates procedural outdoor 3D scenes. This goal of this tool is to allow for iterative modifications to a partially complete scene. PCG is incorporated to the artistic process by allowing the existing structure of the scene to be extended. This thesis use a modified Wave Function Collapse (WFC), which allows it to a operate in an unstructured 3D environment. WFC solves problems known as Constraint Satisfaction Problems (CSP), that is, it finds solutions (valid placements of tiles) to problems where there is set of valid configurations for neighboring tiles. The main aim of this research is to take advantage of the properties of WFC in order to generate game content procedurally. In this case, the content is the environment of a 3D game.

## Introduction
Wave Function Collapse (WFC) is a probabilistic algorithm that takes patterns from a given input, and uses it to model outputs with similar local properties. It has been used in the creation of images, 2D textures, and more recently, 3D objects[cite]. In constraint satisfaction problems, local consistency is a property that describes the requirements of constraints and subsets of variables. WFC is a specific approach to solving the arc consistency problem. Two variables are arc consistent if its value is consistent with some allowed value of a second variable [cite]. 

CSPs are a type of problem that can be expressed in terms of variables, domains, and constraints. A CSP involves finding a solution to a set of constraints which must be satisfied in order to find a valid solution. The variables represent the objects that are being solved for, the domains represent the range of possible values for each variable, and the constraints are restrictions on the values the variables can take. A CSP is a mathematical problem and can be solved using various approaches, such as backtracking or branch and bound.

The goals of this project are to implement a flexible WFC graph algorithm in C++, and then use it to generate procedural placements for 3D objects. In addition, this project attempts to explore the possibilities of using WFC in a 3D environment by experimenting with different parameters and configurations. Experiments are performed in a custom rendering engine, created with the intent of supplying an environment for exploring procedural generation of 3D scenes.

The implementation is tested on different types of 3D objects, and the results are evaluated to determine the effectiveness of the algorithm in this context. 

The project includes the following components:

1. Designing and implementing a flexible WFC algorithm in C++.
2. Testing the algorithm on different types of 3D objects.
3. Evaluating the algorithm by comparing the results with other existing algorithms.
4. Exploring the possibilities of using WFC in a 3D environment by experimenting with different parameters and constraints.
5. Developing a custom rendering engine to visualize the generated placements.

## Design Goals

1. Flexible WFC for 3D irregular scenes
2. Easily reconfigurable and data defined behavior

## Research Questions

1. Is it possible to extend WFC to an unstructured 3d environment?
2. Does WFC generate plausible results when extended to this environment?
3. What are the implications for game development?
4. How does WFC compare to other methods of 3d procedural generation?
5. What other applications could WFC be used for in a 3d unstructured environment?

Jan 20 meeting notes: data structures for graph representation and understand WFC.
    Write an abstract! (three week deadline) Feb 10
    Graph queries needed for neighbors.
    OpenMesh??
    Asset Library for buildings

### Reading
2.  Jan 20 : 

    [Compositional procedural content generation](https://dl.acm.org/doi/pdf/10.1145/2538528.2538541)
        need to read.

    [Procedural content generation using neuroevolution and novelty search for diverse video game levels 2022](https://dl.acm.org/doi/pdf/10.1145/3512290.3528701)
        Genetic algorithm (NEAT) to create novel tile based video game levels. Authors focus on speed of the algorithm. Fitness functions introduced for novelty, and solvability.

    *[Automatic Generation of Game Content using a Graph-based Wave Function Collapse Algorithm 2019](https://dl.acm.org/doi/10.1109/CIG.2019.8848019)
        Extends WFC to Voronoi cells and volumes. WFC rules based on neighbors that "contact" cell. Graph based WFC

    *[Automatic Generation of Game Levels Based on Controllable Wave Function Collapse Algorithm 2019](https://dl.acm.org/doi/10.1109/CIG.2019.8848019)

    *[PCG Workshop Paper Database](https://pcgworkshop.com/database.php)
        Some cool stuff on WFC

3.  Jan 27 : 
    [A Procedural Model for Diverse Tree Species 2022](https://pcgworkshop.com/archive/hoetzlein2022aprocedural.pdf)

    *[Tessera: A Practical System for Extended WaveFunctionCollapse](https://pcgworkshop.com/archive/newgas2021tessera.pdf)

4.  Feb 03 : 
    *[WaveFunctionCollapse_Content_Generation_via_Constraint_Solving_and_Machine_Learning](file:///run/media/hborlik/T7/grad%20stuff/research/WaveFunctionCollapse_Content_Generation_via_Constraint_Solving_and_Machine_Learning.pdf)
        More detailed description of the WFC algorithm and some experiments that validate certain design decisions made by the original creator.
    
    *[Constraint Satisfaction Problems](file:///run/media/hborlik/T7/grad%20stuff/research/graph_theory/ConstraintSatisfactionProblems.pdf)

    
5.  Feb 10 : Abstract due!
6.  Feb 17 : 
7.  Feb 24 : 
8.  Mar 03 : 
9.  Mar 10 : 
10. Mar 17 : 

### Schedule

2.  Jan 20 : *Scene representation* Need to load from very simple format to enable procedural generation.
    1.  Think about the data structures that will be used to generate a scene graph.
   
3.  Jan 27 : 
4.  Feb 03 : *Graph progress* Data structures for WFC
5.  Feb 10 : 
6.  Feb 17 : 
7.  Feb 24 : 
8.  Mar 03 : 
9.  Mar 10 : 
10. Mar 17 : 

### current milestones
1. implement terrain rendering
   - geometry (done)
   - normals (done)
   - texture
2. Vegetation and terrain rendering (focus)
    - scene object placement.
    - what exactly is the problem
    - Example based?

A framework for both plant and building placement. Procedural open world scene generation.

1. by Dec 7
    - use a tree library
    - urban models library
    - define the constraints
        - forest can be close to the road but not on
    - Scene saving and loading? how to save scenes

### Other todos
1. BUGS:
   - The normals for terrain are wrong! this is why only part of it shades (fixed)
2. How are volumes going to be represented? sdf, voxels, AABB, something else?
3. Fix shutdown order! Terrain Renderer needs to be destroyed before Renderer and should be moved out of the class

### Terrain Generation
Example based terrain generation has been explored and GAN based models offer a smooth
authoring experience. Going from sketch to terrain.

### Road Generation
Roads serve the purpose of making the journey from point A to B easier. Road must follow grade limitations (imposed by the vehicles that travel on the road), and they are limited by cost of construction. 

## The framework that ties it together
From "A Proposal for a Procedural Terrain Modelling Framework"
1. It is not clear how to tune individual procedural algorithms
to work well together; there is no tool or integrating 
framework that combines these various algorithms in
a usable way.
2. The parameters of these algorithms and tools (e.g. noise
octaves, persistence) often require an *in-depth* knowledge
of the algorithm to predict the effect of a parameter on
the outcome. A user is virtually unable to declare their intentions: typically having little control over the generation
process, and is forced to use a trial and error approach.

Thoughts: these limitations are similar to the limitations of destructive modeling 
processes. Iterative editing relies on making small changes to the input and, in the case
of the above limitations, being unaware of how the change will affect the output. A user needs to know how their changes will affect the output and need to be in control of the scale of that change. Since changes are made to affect the output, the structure of the input is less meaningful. 
The user can pick which parts / aspects (?) of the output they want to keep.

### What is the contribution?

Things that have already been done:
    Rendering/shadows/scene graph.
    Shader preprocessing, Shader queries, Material management,