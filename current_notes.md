## Thesis Notes

Modeling and rendering are two core research problems in computer graphics.

### current milestones
1. implement terrain rendering
2. Vegetation and terrain rendering (focus)
    - what exactly is the problem?
    - procedural geometry or scene placement?
    - Or trees in Urban environments?
    - Data based?
    

### Tree generation methods
1. L-systems for branching skeletons

2. See *Modeling* *the* *Mighty* *Maple* for Ramiform geometry that is
used to model intersecting branches

### Terrain Generation
Example based terrain generation has been explored and GAN based models offer a smooth
authoring experience. Going from sketch to terrain.

### Road Generation
Roads serve the purpose of making the journey from point A to B easier. Road must follow grade limitations (imposed by the vehicles that travel on the road), and they are limited 
by cost of construction. 

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
of the above limitations, being unaware of how the change will affect the output. A user needs to know how their changes will affect the output and need to be in control of the scale of that change. 