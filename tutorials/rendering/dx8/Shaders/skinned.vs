vs.1.1

// def c0, 768, 1.0/4096.0, 0 1
// def c1, 0.5, 2.0, 1, 0

// dcl_position v0
// dcl_blendweight v1
// dcl_blendindices v2
// dcl_normal v3
// dcl_texcoord0 v4

// char WorldMatrixRegister[3][4];

// Scale the bone indices up to integers
mul r0.xyzw, c0.x, v2

mov a0.x, r0.x
mul r1.xyzw, v1.x, c[a0.x + 8]
mul r2.xyzw, v1.x, c[a0.x + 9]
mul r3.xyzw, v1.x, c[a0.x + 10]

mov a0.x, r0.y

mad r1.xyzw, v1.y, c[a0.x + 8] , r1
mad r2.xyzw, v1.y, c[a0.x + 9] , r2
mad r3.xyzw, v1.y, c[a0.x + 10], r3

mov a0.x, r0.z
mad r1.xyzw, v1.z, c[a0.x + 8] , r1
mad r2.xyzw, v1.z, c[a0.x + 9] , r2
mad r3.xyzw, v1.z, c[a0.x + 10], r3

mov a0.x, r0.w
mad r1.xyzw, v1.w, c[a0.x + 8] , r1
mad r2.xyzw, v1.w, c[a0.x + 9] , r2
mad r3.xyzw, v1.w, c[a0.x + 10], r3

// Transform the position by the blended matrix to get to world-space.
dp4 r4.x, v0, r1
dp4 r4.y, v0, r2
dp4 r4.z, v0, r3
mov r4.w, c0.w

// Then transform the world-space position into projection space.
dp4 oPos.x, r4, c4
dp4 oPos.y, r4, c5
dp4 oPos.z, r4, c6
dp4 oPos.w, r4, c7

// Transform the normal by the blended matrix
// (technically we should use the inverse transpose, so this is incorrect for shears or non-uniform scales).
dp3 r5.x, v3, r1
dp3 r5.y, v3, r2
dp3 r5.z, v3, r3

// Can't use nrm in VS1.1, so we do it manually.
dp3 r5.w, r5, r5
rsq r5.w, r5.w
mul r5.xyz, r5, r5.w

// Do some simple lighting.
// clamp(N.L)
dp3 r5.w, r5, c2
max r5.w, r5.w, c0.z

// Add in the light's ambient.
add r5.w, r5.w, c3.w

// And finally multiply by the actual colour of the light.
mul oD0.xyz, r5.w, c3
mov oD0.w, c2.w

// Copy over the texture coords
mov oT0.xy, v4.xy
