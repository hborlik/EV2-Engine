## Thesis Notes

Modeling and rendering are two core research problems in computer graphics.

Jan 17 thoughts: Volume based WFC for objects and their neighboring objects (classes of object). For interior or exterior scenes. Eg garbage cans and nearby garbage on the ground. WFC allows for probabilistic modeling. Placement rules can drive orientation. Contributions would be WFC that can expand to accommodate objects with more positions that need to be evaluated.

### Reading
2.  Jan 20 : 

    [Compositional procedural content generation](https://dl.acm.org/doi/pdf/10.1145/2538528.2538541)
        need to read.

    [Procedural content generation using neuroevolution and novelty search for diverse video game levels 2022](https://dl.acm.org/doi/pdf/10.1145/3512290.3528701)
        Genetic algorithm (NEAT) to create novel tile based video game levels. Authors focus on speed of the algorithm. Fitness functions introduced for novelty, and solvability.

    [Automatic Generation of Game Content using a Graph-based Wave Function Collapse Algorithm 2019](https://dl.acm.org/doi/10.1109/CIG.2019.8848019)
        Extends WFC to Voroni cells and volumes. WFC rules based on neighbors that "contact" cell. 

    [Automatic Generation of Game Levels Based on Controllable Wave Function Collapse Algorithm 2019](https://dl.acm.org/doi/10.1109/CIG.2019.8848019)

    [PCG Workshop Paper Database](https://pcgworkshop.com/database.php)
        Some cool stuff on WFC
3.  Jan 27 : 
4.  Feb 03 : 
5.  Feb 10 : 
6.  Feb 17 : 
7.  Feb 24 : 
8.  Mar 03 : 
9.  Mar 10 : 
10. Mar 17 : 

### Schedule

2.  Jan 20 : *Scene representation* Need to load from very simple format to enable procedural generation.
    1.  Think about the data structures that will be used to generate a scene graph.
    2.  Zones are fundamental constructs of a city. They define the size and placement of buildings. Computational geometry, polygon operations. https://en.wikipedia.org/wiki/Greiner%E2%80%93Hormann_clipping_algorithm. Nodes of this data should be editable. Drag, delete, and create polygon vertices.
3.  Jan 27 : 
4.  Feb 03 : 
5.  Feb 10 : 
6.  Feb 17 : 
7.  Feb 24 : 
8.  Mar 03 : 
9.  Mar 10 : 
10. Mar 17 : 

### current milestones
1. implement terrain rendering
   - geometry (done)
   - normals
   - texture
2. Vegetation and terrain rendering (focus)
    - what exactly is the problem?
    - procedural geometry or scene placement?
    - Or trees in Urban environments?
    - Data based?

A framework for both plant and building placement. Procedural open world scene generation.

3. by Dec 7
    - use a tree library
    - urban models library
    - define the constraints
        - forest can be close to the road but not on
    - Scene saving and loading? how to save scenes

### Other todos
1. Convert gbuffers to view space for position and world for normal
2. BUGS:
   - The normals for terrain are wrong! this is why only part of it shades (fixed)
3. How are volumes going to be represented? sdf, voxels, something else?
4. Fix shutdown order! Terrain Renderer needs to be destroyed before Renderer and should be moved out of the class

### Tree generation methods
1. L-systems for branching skeletons

2. See *Modeling* *the* *Mighty* *Maple* for Ramiform geometry that is
used to model intersecting branches

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