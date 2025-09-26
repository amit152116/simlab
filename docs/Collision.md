# Physics of Circle-Circle Collisions

## 1. Collision Detection

### Distance-Based Detection

Two circles collide when they overlap, which happens when the distance between their centers is less than the sum of their radii.

**Mathematical Condition:**

```
distance(center1, center2) < radius1 + radius2
```

**Distance Formula:**

```
distance = √[(x₂-x₁)² + (y₂-y₁)²]
```

### Penetration Depth

When circles overlap, the penetration depth tells us how much they're intersecting:

```
penetration = (radius1 + radius2) - distance
```

This is crucial for separating the circles after collision.

## 2. Collision Normal Vector

The **collision normal** is the unit vector pointing from the first circle's center to the second circle's center along the line of impact.

**Formula:**

```
normal = (pos2 - pos1) / distance
```

This vector is essential because:

- It defines the **direction of force transfer**
- Only velocity components **along this direction** are affected by collision
- Velocity components **perpendicular** to this direction remain unchanged

## 3. Position Correction (Separation)

When circles overlap, we need to separate them to prevent "sinking" into each other.

### Mass-Weighted Separation

Objects with different masses should move different amounts:

```cpp
separation1 = penetration × (mass2 / totalMass)
separation2 = penetration × (mass1 / totalMass)
```

**Physics Principle:** Heavier objects resist movement more, so lighter objects move more during separation.

### Equal Mass Separation

For equal masses, each object moves half the penetration distance:

```cpp
separation = penetration × 0.5
```

## 4. Conservation Laws in Elastic Collisions

Elastic collisions conserve both **momentum** and **kinetic energy**.

### Conservation of Momentum

```
m₁v₁ᵢ + m₂v₂ᵢ = m₁v₁f + m₂v₂f
```

Where:

- `mᵢ` = mass of object i
- `vᵢᵢ` = initial velocity of object i
- `vᵢf` = final velocity of object i

### Conservation of Kinetic Energy

```
½m₁v₁ᵢ² + ½m₂v₂ᵢ² = ½m₁v₁f² + ½m₂v₂f²
```

## 5. Relative Velocity Analysis

### Why Use Relative Velocity?

We only care about how fast objects are approaching each other along the collision normal.

**Relative Velocity:**

```
vᵣₑₗ = v₂ - v₁
```

**Velocity Along Normal:**

```
vₙ = vᵣₑₗ · normal = (vᵣₑₗ.x × normal.x) + (vᵣₑₗ.y × normal.y)
```

### Separating vs. Approaching

- If `vₙ > 0`: Objects are **separating** → no collision response needed
- If `vₙ < 0`: Objects are **approaching** → apply collision response

## 6. Impulse-Momentum Theorem

Instead of calculating final velocities directly, we use **impulse** (change in momentum).

### Impulse Formula

```
J = -(1 + e) × vₙ / (1/m₁ + 1/m₂)
```

Where:

- `J` = impulse magnitude
- `e` = coefficient of restitution (0 = perfectly inelastic, 1 = perfectly elastic)
- `vₙ` = velocity along normal

### Physics Behind This Formula

The impulse formula comes from solving the conservation equations simultaneously:

1. **Momentum Conservation:** `J/m₁ + J/m₂ = change in relative velocity`
2. **Energy Conservation:** Determines the coefficient of restitution relationship
3. **Newton's Law:** `Impulse = change in momentum`

## 7. Applying Impulse to Velocities

The impulse vector is:

```
impulse = J × normal
```

**Velocity Changes:**

```cpp
v₁_new = v₁_old - impulse/m₁  // Object 1 moves opposite to normal
v₂_new = v₂_old + impulse/m₂  // Object 2 moves along normal
```

**Why opposite directions?** Newton's Third Law - forces are equal and opposite.

## 8. Coefficient of Restitution (e)

This determines how "bouncy" the collision is:

- **e = 0:** Perfectly inelastic (objects stick together)
- **e = 1:** Perfectly elastic (maximum bounce)
- **0 < e < 1:** Partially elastic (some energy lost)

**Physical meaning:** The ratio of separation speed to approach speed after collision.

## 9. Special Case: Equal Mass Collisions

When masses are equal (`m₁ = m₂`), the physics simplifies dramatically:

**The objects essentially "exchange" their velocity components along the collision normal.**

```cpp
// Only the normal components change
v₁_new = v₁_old + vₙ × normal
v₂_new = v₂_old - vₙ × normal
```

**Intuition:** Think of two billiard balls - a head-on collision causes them to exchange velocities.

## 10. Why Tangential Components Don't Change

During collision, forces act only along the collision normal (assuming no friction). This means:

- **Normal components:** Change due to collision forces
- **Tangential components:** Remain unchanged (no force in that direction)

This is why we only modify velocity components along the normal vector.

## 11. Real-World Considerations

### Numerical Stability

- **Minimum distance check:** Prevents division by zero when circles have identical centers
- **Separating velocity check:** Prevents "fighting" between overlapping objects

### Energy Loss in Real Systems

- Real collisions lose energy due to deformation, sound, heat
- Perfect elastic collisions are idealized - real `e < 1`

### Multiple Collisions

- When many objects collide simultaneously, the order of resolution matters
- Advanced physics engines use iterative solving or constraint-based approaches

## Summary

Circle collision physics combines:

1. **Geometry:** Distance and penetration calculations
2. **Vector Math:** Normal vectors and dot products
3. **Classical Mechanics:** Conservation of momentum and energy
4. **Numerical Methods:** Impulse-based velocity updates

The beauty is that complex 2D collision behavior emerges from these fundamental principles!
