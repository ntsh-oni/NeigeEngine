# Scripting Documentation - Vector
Types and functions associated with vectors.
Vectors can have 2, 3 or 4 elements.

## Vector of 2 elements
Prefix: ``vec2:``

### Functions
- **vec2 new(number x, number y)**: Constructor.
- **number length(vec2 v)**: Returns the length of the vector.
- **number dot(vec2 a, vec2 b)**: Returns the dot product between two vectors of 2 elements.
- **vec2 normalize(vec2 v)**: Returns a normalized vector of 2 elements.

## Vector of 3 elements
Prefix: ``vec3:``

### Functions
- **vec3 new(number x, number y, number z)**: Constructor.
- **number length(vec3 v)**: Returns the length of the vector.
- **number dot(vec3 a, vec3 b)**: Returns the dot product between two vectors of 3 elements.
- **vec3 normalize(vec3 v)**: Returns a normalized vector of 3 elements.
- **vec3 cross(vec3 a, vec3 b)** : Returns the cross product between two vectors of 3 elements.

## Vector of 4 elements
Prefix: ``vec4:``

### Functions
- **vec4 new(number x, number y, number z, number w)**: Constructor.
- **number length(vec4 v)**: Returns the length of the vector.
- **number dot(vec4 a, vec4 b)**: Returns the dot product between two vectors of 4 elements.
- **vec4 normalize(vec4 v)**: Returns a normalized vector of 4 elements.

[>> Scripting documentation index](../index.md)